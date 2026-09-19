/* Renders the sky at a matrix of times/weathers to build/sky_*.ppm (run under xvfb-run). Usage: sky_preview [config.cfg] [prefix] */
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/glu.h>
#include "../packages/common/bigo_sky.h"

static void ground(const BigoSky *s) {
    /* a flat grey-green plane with a few pillars, so fog and the horizon join are visible */
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS); glColor3f(0.30f, 0.34f, 0.26f);
    glVertex3f(-400, 0, -400); glVertex3f(400, 0, -400); glVertex3f(400, 0, 400); glVertex3f(-400, 0, 400); glEnd();
    for (int i = 0; i < 40; i++) {
        float a = i * 2.399f, r = 30 + i * 7, x = cosf(a) * r, z = sinf(a) * r, h = 6 + (i % 5) * 5;
        glColor3f(0.42f, 0.42f, 0.45f);
        glBegin(GL_QUADS);
        glVertex3f(x - 2, 0, z - 2); glVertex3f(x + 2, 0, z - 2); glVertex3f(x + 2, h, z - 2); glVertex3f(x - 2, h, z - 2);
        glVertex3f(x - 2, 0, z + 2); glVertex3f(x + 2, 0, z + 2); glVertex3f(x + 2, h, z + 2); glVertex3f(x - 2, h, z + 2);
        glVertex3f(x - 2, 0, z - 2); glVertex3f(x - 2, 0, z + 2); glVertex3f(x - 2, h, z + 2); glVertex3f(x - 2, h, z - 2);
        glVertex3f(x + 2, 0, z - 2); glVertex3f(x + 2, 0, z + 2); glVertex3f(x + 2, h, z + 2); glVertex3f(x + 2, h, z - 2);
        glEnd();
    }
    (void)s;
}

int main(int argc, char **argv) {
    const char *cfg = argc > 1 ? argv[1] : NULL, *prefix = argc > 2 ? argv[2] : "sky";
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *w = SDL_CreateWindow("s", 0, 0, 640, 400, SDL_WINDOW_OPENGL);
    SDL_GLContext c = SDL_GL_CreateContext(w); (void)c;
    BigoSky sky; bigo_sky_init(&sky);
    if (cfg && strcmp(cfg, "-")) { char err[160]; int r = bigo_sky_load_config(&sky, cfg, err, sizeof err); if (r < 0) { fprintf(stderr, "cfg: %s\n", err); return 2; } fprintf(stderr, "loaded %d keys\n", r); }
    struct { const char *name; float minute; int wx; float yaw; } shots[] = {
        { "dawn",      340, 0, 0 },  { "morning",  450, 0, 25 },  { "noon",     720, 0, 60 },
        { "golden",   1050, 0, 165 }, { "sunset",  1110, 0, 178 }, { "dusk",    1160, 0, 190 },
        { "night",    1380, 0, 40 }, { "overcast", 720, 1, 60 },  { "rain",     720, 2, 60 },
        { "storm",     720, 3, 60 }, { "lightning", 1300, 3, 60 },  { "stormnight", 1380, 3, 60 }, { "cloudynight", 1380, 1, 40 },
    };
    for (unsigned i = 0; i < sizeof(shots) / sizeof(shots[0]); i++) {
        unsigned int t = 1000;
        for (int k = 0; k < 400; k++) { t += 100; bigo_sky_update(&sky, shots[i].minute, shots[i].wx, t); }   /* let the weather ease in */
        if (!strcmp(shots[i].name, "lightning")) { sky.flash = 0.35f; sky.bolt_age = 0.02f; sky.flash_dx = cosf(shots[i].yaw * BSKY_PI / 180); sky.flash_dz = sinf(shots[i].yaw * BSKY_PI / 180); sky.next_bolt_ms = 0xFFFFFFF; }
        glViewport(0, 0, 640, 400);
        glClearColor(sky.clear[0], sky.clear[1], sky.clear[2], 1); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluPerspective(65, 640.0 / 400.0, 0.1, 500);
        glMatrixMode(GL_MODELVIEW); glLoadIdentity();
        float yaw = shots[i].yaw * BSKY_PI / 180, pitch = (shots[i].minute > 1300 || shots[i].minute < 300) ? 0.75f : 0.30f;
        gluLookAt(0, 2, 0, cosf(yaw) * cosf(pitch), 2 + sinf(pitch), sinf(yaw) * cosf(pitch), 0, 1, 0);
        bigo_sky_draw(&sky);
        glEnable(GL_DEPTH_TEST); bigo_sky_fog_on(&sky); ground(&sky); bigo_sky_fog_off();
        bigo_sky_draw_precip(&sky, 0, 2, 0, t);
        bigo_sky_draw_grade(&sky, 640, 400);
        static unsigned char px[640 * 400 * 3]; glReadPixels(0, 0, 640, 400, GL_RGB, GL_UNSIGNED_BYTE, px);
        char fn[96]; snprintf(fn, sizeof fn, "build/%s_%02u_%s.ppm", prefix, i, shots[i].name);
        FILE *f = fopen(fn, "wb"); fprintf(f, "P6\n640 400\n255\n");
        for (int y = 399; y >= 0; y--) fwrite(px + y * 640 * 3, 1, 640 * 3, f);
        fclose(f);
    }
    return 0;
}
