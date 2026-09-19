#!/usr/bin/env python3
"""Derive zombie .gband clips for the UAL1 mannequin from its own idle/walk mocap clips.

Usage: gen_zombie_clips.py <goldenband_asset_dir> <out_dir>
Emits zombie_{idle,walk,attack,death}.gband + .gband.json (skeleton hash matches mannequin_npc.gskel).
Method: per-tick forward kinematics on the source clip, then re-aim / re-pose selected bones in world space
(arms forward, forward lean, head tilt, slowed shamble). Deterministic; no randomness.
"""
import hashlib, json, math, os, struct, sys

def qmul(a, b):
    ax, ay, az, aw = a; bx, by, bz, bw = b
    return (aw*bx+ax*bw+ay*bz-az*by, aw*by-ax*bz+ay*bw+az*bx,
            aw*bz+ax*by-ay*bx+az*bw, aw*bw-ax*bx-ay*by-az*bz)
def qinv(q): return (-q[0], -q[1], -q[2], q[3])
def qnorm(q):
    n = math.sqrt(sum(x*x for x in q)) or 1.0
    return tuple(x/n for x in q)
def qrot(q, v):
    p = qmul(qmul(q, (v[0], v[1], v[2], 0.0)), qinv(q))
    return (p[0], p[1], p[2])
def axang(axis, ang):
    n = math.sqrt(sum(x*x for x in axis)); s = math.sin(ang/2)/n
    return (axis[0]*s, axis[1]*s, axis[2]*s, math.cos(ang/2))
def arc(a, b):
    na = math.sqrt(sum(x*x for x in a)); nb = math.sqrt(sum(x*x for x in b))
    a = tuple(x/na for x in a); b = tuple(x/nb for x in b)
    d = sum(x*y for x, y in zip(a, b))
    if d < -0.99999:
        ax = (1, 0, 0) if abs(a[0]) < 0.9 else (0, 1, 0)
        c = (a[1]*ax[2]-a[2]*ax[1], a[2]*ax[0]-a[0]*ax[2], a[0]*ax[1]-a[1]*ax[0])
        return axang(c, math.pi)
    c = (a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0])
    return qnorm((c[0], c[1], c[2], 1.0+d))
def sub(a, b): return tuple(x-y for x, y in zip(a, b))
def add(a, b): return tuple(x+y for x, y in zip(a, b))
def norm(v):
    n = math.sqrt(sum(x*x for x in v)) or 1.0
    return tuple(x/n for x in v)

class Skel:
    def __init__(self, path):
        d = open(path, 'rb').read()
        self.hash = hashlib.sha256(d).digest()
        n = struct.unpack('<I', d[8:12])[0]
        self.names, self.parent, self.rt, self.rq = [], [], [], []
        for i in range(n):
            r = d[12+i*128:12+(i+1)*128]
            self.names.append(r[:32].split(b'\0')[0].decode())
            self.parent.append(struct.unpack('<i', r[32:36])[0])
            self.rt.append(struct.unpack('<3f', r[36:48]))
            self.rq.append(struct.unpack('<4f', r[48:64]))
        self.idx = {n: i for i, n in enumerate(self.names)}

class Clip:
    def __init__(self, base):
        d = open(base + '.gband', 'rb').read()
        self.man = json.load(open(base + '.gband.json'))
        _, _, self.rate, self.ticks, self.nch = struct.unpack('<4sIIII', d[:20])
        self.ch = self.man['channels']
        self.data = [struct.unpack('<%df' % self.nch, d[84+t*self.nch*4:84+(t+1)*self.nch*4]) for t in range(self.ticks)]
        self.col = {c: i for i, c in enumerate(self.ch)}

def pose_at(sk, clip, t):
    """Local (translation, quaternion) per joint at fractional tick t (nlerp between ticks, loop wrap)."""
    t0 = int(math.floor(t)) % clip.ticks; t1 = (t0+1) % clip.ticks; f = t - math.floor(t)
    def g(name):
        i = clip.col.get(name)
        if i is None: return None
        return clip.data[t0][i]*(1-f)+clip.data[t1][i]*f
    T, Q = [], []
    for j, n in enumerate(sk.names):
        tr = list(sk.rt[j])
        for k, ax in enumerate('xyz'):
            v = g(n+'.t'+ax)
            if v is not None: tr[k] = v
        q = list(sk.rq[j])
        if (n+'.qx') in clip.col:
            q = [g(n+'.q'+c) for c in 'xyzw']
        T.append(tuple(tr)); Q.append(qnorm(tuple(q)))
    return T, Q

def fk(sk, T, Q):
    WP, WQ = [], []
    for j in range(len(sk.names)):
        p = sk.parent[j]
        if p < 0: WP.append(T[j]); WQ.append(Q[j])
        else:
            WP.append(add(WP[p], qrot(WQ[p], T[j]))); WQ.append(qmul(WQ[p], Q[j]))
    return WP, WQ

def frame(sk, T, Q):
    """World up/forward/left from a pose (Y-up mannequin after root -90deg X)."""
    WP, _ = fk(sk, T, Q)
    i = sk.idx
    up = norm(sub(WP[i['Head']], WP[i['pelvis']]))
    left = norm(sub(WP[i['clavicle_l']], WP[i['clavicle_r']]))
    fwd = norm((left[1]*up[2]-left[2]*up[1], left[2]*up[0]-left[0]*up[2], left[0]*up[1]-left[1]*up[0]))
    # sign: toes (ball) lie in front of the ankle
    toe = sub(WP[i['ball_l']], WP[i['foot_l']])
    if sum(a*b for a, b in zip(fwd, toe)) < 0: fwd = tuple(-x for x in fwd)
    return up, fwd, left

def apply_world_delta(sk, T, Q, j, dq_world):
    """Pre-multiply joint j's world rotation by dq_world, expressed as a local-rotation change."""
    _, WQ = fk(sk, T, Q)
    p = sk.parent[j]
    wp = WQ[p] if p >= 0 else (0, 0, 0, 1)
    Q[j] = qnorm(qmul(qmul(qinv(wp), qmul(dq_world, wp)), Q[j]))

def aim(sk, T, Q, j, child, direction):
    WP, WQ = fk(sk, T, Q)
    cur = sub(WP[sk.idx[child]], WP[j])
    apply_world_delta(sk, T, Q, j, arc(cur, direction))

def combo(up, fwd, left, u, f, l): return norm(tuple(up[k]*u+fwd[k]*f+left[k]*l for k in range(3)))

def zombify(sk, T, Q, phase, level=1.0, arms_raise=0.0):
    """Zombie pose overlay. arms_raise 0=hang forward-low (idle) .. 1=fully raised forward (walk/attack)."""
    up, fwd, left = frame(sk, T, Q)
    i = sk.idx
    lean = math.radians(14*level)
    for b in ('spine_01', 'spine_02', 'spine_03'):
        apply_world_delta(sk, T, Q, i[b], axang(left, lean/3))
    apply_world_delta(sk, T, Q, i['neck_01'], axang(fwd, math.radians(16)*math.sin(phase*0.5)*0+math.radians(14)*level))
    apply_world_delta(sk, T, Q, i['Head'], axang(left, math.radians(-8*level)))
    for s, sgn in (('l', 1), ('r', -1)):
        a = -0.35 + 0.35*arms_raise   # vertical component of the arm aim
        wob = 0.05*math.sin(phase + (0 if s == 'l' else math.pi))
        d = combo(up, fwd, left, a+wob, 1.0, sgn*0.18)
        aim(sk, T, Q, i['upperarm_'+s], 'lowerarm_'+s, d)
        d2 = combo(up, fwd, left, a+0.12, 1.0, sgn*0.12)
        aim(sk, T, Q, i['lowerarm_'+s], 'hand_'+s, d2)
    return T, Q

def encode(sk, name, rows, ch, rate, src_man, tags, outdir):
    n = len(rows)
    blob = b''.join(struct.pack('<%df' % len(ch), *r) for r in rows)
    ch_hash = hashlib.sha256(blob).digest()
    head = struct.pack('<4sIIII', b'GBND', 1, rate, n, len(ch)) + sk.hash + ch_hash
    open(os.path.join(outdir, name+'.gband'), 'wb').write(head+blob)
    man = {"gband_version": 1, "skeleton_hash": sk.hash.hex(), "content_hash": ch_hash.hex(), "tick_rate": rate,
           "duration_ticks": n, "channels": ch,
           "authorship": {"kind": "generative", "who": "BIG_O tools/gen_zombie_clips.py, derived from UAL1 mocap"},
           "intent_tags": tags, "loop_points": {"start_tick": 0, "end_tick": n}, "safety": src_man["safety"]}
    json.dump(man, open(os.path.join(outdir, name+'.gband.json'), 'w'), indent=2)

def channels_for(sk):
    ch = []
    for n in sk.names:
        ch += [n+'.tx', n+'.ty', n+'.tz', n+'.qx', n+'.qy', n+'.qz', n+'.qw']
    return ch

def row(T, Q):
    r = []
    for t, q in zip(T, Q): r += list(t)+list(q)
    return r

def main():
    src, out = sys.argv[1], sys.argv[2]
    os.makedirs(out, exist_ok=True)
    sk = Skel(os.path.join(src, 'mannequin_npc.gskel'))
    walk = Clip(os.path.join(src, 'UAL1_Standard_Walk_Loop'))
    idle = Clip(os.path.join(src, 'UAL1_Standard_Idle_Loop'))
    ch = channels_for(sk); rate = 30

    # walk: slow shamble (1.6x slower), arms out front
    n = int(walk.ticks*1.6); rows = []
    for k in range(n):
        T, Q = pose_at(sk, walk, k/1.6)
        T, Q = zombify(sk, list(T), list(Q), 2*math.pi*k/n, 1.0, 0.9); rows.append(row(T, Q))
    encode(sk, 'zombie_walk', rows, ch, rate, walk.man, ['gait', 'zombie'], out)

    # idle: arms hanging forward-low, slow sway
    n = idle.ticks; rows = []
    for k in range(n):
        T, Q = pose_at(sk, idle, k)
        T, Q = zombify(sk, list(T), list(Q), 2*math.pi*k/n, 0.8, 0.15); rows.append(row(T, Q))
    encode(sk, 'zombie_idle', rows, ch, rate, idle.man, ['idle', 'zombie'], out)

    # attack: 36 ticks, arms rise then slam down (lunge from lean), one-shot
    n = 36; rows = []
    for k in range(n):
        T, Q = pose_at(sk, idle, 0)
        u = k/(n-1)
        raise_amt = math.sin(min(u/0.45, 1.0)*math.pi/2) if u < 0.45 else max(0.0, 1.0-(u-0.45)/0.25)*1.0-0.8*min(1.0, (u-0.45)/0.25)
        T, Q = zombify(sk, list(T), list(Q), 0.0, 1.0+1.2*math.sin(u*math.pi), raise_amt); rows.append(row(T, Q))
    encode(sk, 'zombie_attack', rows, ch, rate, idle.man, ['attack', 'zombie'], out)

    # death: 45 ticks, topple backward to lying, hips dropping; holds final pose
    n = 45; rows = []
    up, fwd, left = frame(sk, *pose_at(sk, idle, 0))
    for k in range(n):
        T, Q = pose_at(sk, idle, 0); T = list(T); Q = list(Q)
        u = k/(n-1); e = u*u*(3-2*u)
        T, Q = zombify(sk, T, Q, 0.0, 0.5, 0.5*(1-e))
        pel = sk.idx['pelvis']
        apply_world_delta(sk, T, Q, pel, axang(left, -math.radians(90)*e))
        # lower the pelvis toward the ground (pelvis-local Y is hip height's parent-space axis; drop along world up via root-frame)
        wp, wq = fk(sk, T, Q)
        drop = 0.75*e
        rp = sk.parent[pel]
        loc = qrot(qinv(wq[rp]), tuple(-x*drop for x in up))
        T[pel] = add(T[pel], loc)
        rows.append(row(T, Q))
    encode(sk, 'zombie_death', rows, ch, rate, idle.man, ['death', 'zombie'], out)
    print('wrote zombie_{walk,idle,attack,death} to', out)

if __name__ == '__main__':
    main()
