/* Renders every phone app to build/phone_<app>.ppm via SDL2+GL (run under xvfb-run). Includes the client so the REAL draw code is exercised. */
#define main client_main
#include "../apps/client/src/main.c"
#undef main
int main(void) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *w = SDL_CreateWindow("p", 0, 0, 900, 560, SDL_WINDOW_OPENGL);
    SDL_GLContext c = SDL_GL_CreateContext(w); (void)c;
    BigoPhone p; bigo_phone_init(&p);
    PcPlayerState own; memset(&own, 0, sizeof(own)); own.level = 3; own.xp = 40; own.xp_to_next = 120; own.unspent_points = 2; own.ability[1] = 1;
    PcInventorySlot inv[PC_INVENTORY_SLOTS]; memset(inv, 0, sizeof(inv)); inv[0].item_id = PC_ITEM_SCRAP; inv[0].count = 7;
    bigo_world_init();
    for (unsigned int t = 1000; t <= 620u * 1000u; t += 1000) bigo_world_step(&p, t);   /* ~17:20 on day 1, real world sim */
    bigo_phone_notify(&p, BP_MSG_THORNE_BRIEF, 620000);
    p.lab_sample_count = 2; p.lab_purity[0] = 40.0f; p.lab_contamination[0] = 15.0f; p.lab_integrity[0] = 90.0f; p.lab_generation[0] = 0;
    p.lab_purity[1] = 62.0f; p.lab_contamination[1] = 8.0f; p.lab_integrity[1] = 85.0f; p.lab_generation[1] = 1;
    p.trust[0] = 2; p.contacts_met = 3; p.zone_alert = 4; p.zone_current = 1;
    p.weapons_owned = 3; p.current_weapon = 1; p.photos = 4;
    for (int a = -1; a < BP_APP_COUNT; a++) {
        p.open = 1; p.app = a; p.cursor = 0; p.home_cursor = 4; p.detail = (a == BP_APP_MESSAGES);
        glViewport(0, 0, 900, 560); glClearColor(0.25f, 0.35f, 0.3f, 1); glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        draw_bigo_phone(900, 560, &p, &own, inv, 1000);
        static unsigned char px[900*560*3]; glReadPixels(0, 0, 900, 560, GL_RGB, GL_UNSIGNED_BYTE, px);
        char fn[80]; snprintf(fn, sizeof(fn), "build/phone_%02d.ppm", a + 1);
        FILE *f = fopen(fn, "wb"); fprintf(f, "P6\n900 560\n255\n");
        for (int y = 559; y >= 0; y--) fwrite(px + y*900*3, 1, 900*3, f);
        fclose(f);
    }
    return 0;
}
