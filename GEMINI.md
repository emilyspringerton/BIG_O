# GEMINI.md — Guidance for Gemini / Antigravity in BIG_O

## What This Is

`BIG_O` ("A SHANKPIT Story") is a hard sci-fi social-stealth management sandbox built on the SHANKPIT engine.
- **Day**: Harvest genetic data from feral vectors in a wasteland (SHANKPIT FPS).
- **Night**: Blend into a paranoid corporate night-society (Attention/Heat system: "if they don't see it, it isn't real").
- **Basement**: Off-books cloning lab + pheromone-commanded vector army in an async deterministic shadow war.

See `NORTHSTAR.md` for full design, capability audit, and roadmap.

## Build and Test (Bazel)

BIG_O is built and tested with Bazel (pinned to Bazel 9.2.0 in `.bazelversion`, managed via `bazelisk`):

```bash
# Build all targets
bazel build //...

# Run all unit and integration tests
bazel test //...

# Build specific simulation app
bazel build //:bigo_sim
```

## Related Repos

- `SHANKPIT`: Engine, level chain, doors, REFLUX, NOCK characters.
- `PARENA`: Rules modules and mod idiom.
- `DEADWEIGHT`: Server/bot/league/CI/IDUNA-guest pattern.
- `IDUNA`: Platform IAM, accounts, NOCK registry.
- `EMILY`: Meta-orchestration, backlog, and RSI task loop.
- `emily.cli`: Operator CLI tool (`emily`).

## Operating Protocols (The Emily Way)

1. **Backlog & Observations**: Any founder real-time direction routes through `emily observe -s info "Founder real-time: <summary>"` first, then logged into `EMILY/BACKLOG.md`.
2. **Apples**: File a completion Apple after meaningful milestones:
   ```bash
   emily apples post -t completion -repo BIG_O "<title>"
   ```
3. **CHANGELOG**: Update `CHANGELOG.md` with a dated entry for any meaningful change.
4. **Commits**:
   - Atomic commits following conventional commit format (`feat:`, `fix:`, `docs:`, etc.).
   - Every commit message must end with a blank line and the active session tag:
     ```
     session: <tag>
     ```
     (obtain tag with `emily session current`).
   - Push immediately to upstream after committing (`git push origin main`).
