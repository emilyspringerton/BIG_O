#include <stdio.h>
#include "bigo_skycfg.h"
static int fails = 0;
#define CHECK(c) do { if (!(c)) { printf("FAIL line %d: %s\n", __LINE__, #c); fails++; } } while (0)
int main(void) {
    BigoSkyConfig c; char err[128] = "";
    bigo_skycfg_defaults(&c);
    CHECK(c.weather[3].storm == 1.0f && c.weather[0].rain == 0.0f && c.sun_size == 3.8f);
    /* parse: comments, case-insensitive keys, vectors, weather profiles */
    int n = bigo_skycfg_parse(&c, "# a toxic sky\nDay.Zenith 0.2 0.5 0.1\nweather.storm.darkness 0.9   # very dark\nsun.size 5\n\nweather.rain.cloud_tint 0.5 0.9 0.4\n", err, sizeof err);
    CHECK(n == 4); CHECK(c.day.zenith[1] == 0.5f && c.weather[3].darkness == 0.9f && c.sun_size == 5.0f && c.weather[2].cloud_tint[1] == 0.9f);
    /* errors carry the line number and never crash */
    CHECK(bigo_skycfg_parse(&c, "sun.size 2\nbogus.key 1\n", err, sizeof err) == -1); CHECK(strstr(err, "line 2") && strstr(err, "unknown key"));
    CHECK(bigo_skycfg_parse(&c, "day.zenith 0.1 0.2\n", err, sizeof err) == -1); CHECK(strstr(err, "needs 3"));
    CHECK(bigo_skycfg_parse(&c, "sun.size abc\n", err, sizeof err) == -1); CHECK(strstr(err, "bad number"));
    CHECK(bigo_skycfg_parse(&c, "sun.size 1 2\n", err, sizeof err) == -1); CHECK(strstr(err, "extra"));
    /* sanitize clamps hostile values */
    bigo_skycfg_defaults(&c);
    bigo_skycfg_parse(&c, "day.zenith 5 -3 0.5\nstars.count 99999\nclouds.count -4\nweather.clear.cover 7\nfog.base 9\nsun.size 0\nweather.rain.fog -1\n", err, sizeof err);
    bigo_skycfg_sanitize(&c, 320, 44);
    CHECK(c.day.zenith[0] == 1 && c.day.zenith[1] == 0 && c.star_count == 320 && c.cloud_count == 0 && c.weather[0].cover == 1);
    CHECK(c.fog_base <= 0.05f && c.sun_size == 0.5f && c.weather[2].fog == 0);
    /* load(): invalid file leaves the config untouched; valid file replaces it; missing file reports */
    bigo_skycfg_defaults(&c);
    FILE *f = fopen("/tmp/skycfg_bad.cfg", "w"); fputs("sun.size 9\nnope 1\n", f); fclose(f);
    CHECK(bigo_skycfg_load(&c, "/tmp/skycfg_bad.cfg", 320, 44, err, sizeof err) == -1); CHECK(c.sun_size == 3.8f);
    f = fopen("/tmp/skycfg_ok.cfg", "w"); fputs("sun.size 9\n", f); fclose(f);
    CHECK(bigo_skycfg_load(&c, "/tmp/skycfg_ok.cfg", 320, 44, err, sizeof err) == 1); CHECK(c.sun_size == 9.0f);
    CHECK(bigo_skycfg_load(&c, "/tmp/definitely_missing.cfg", 320, 44, err, sizeof err) == -1);
    printf(fails ? "SKYCFG TEST FAILED\n" : "SKYCFG TEST OK\n");
    return fails != 0;
}
