/* level_loader.h — unified level loading for SHANKPIT/PAPERCRAFT/BIG_O
 * Abstracts JSON (NOCK registry) + PCM (procedural) level formats
 * Unblocks NOCK editor integration for BIG_O day/ client
 */
#ifndef LEVEL_LOADER_H
#define LEVEL_LOADER_H

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#ifndef typeof
#define typeof __typeof__
#endif

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Unified level representation (superset of SHANKPIT JSON + PAPERCRAFT PCM) */
typedef struct {
	uint32_t id;              /* IDUNA level_id or procedural hash */
	char name[128];           /* "nextown", "sector-2", etc. */

	/* Geometry */
	uint32_t wall_count;
	struct {
		float x, y, z;        /* Center position */
		float sx, sy, sz;     /* Extents (half-size) */
		float r, g, b;        /* Color */
		float friction;
		char material[32];    /* "concrete", "metal", "brick", "wood", "dirt" */
	} *walls;

	/* Spawning */
	uint32_t spawner_count;
	struct {
		float x, y, z;
		float yaw;
		int16_t team;         /* -1 = neutral, 0+ = team */
	} *spawners;

	/* Characters (NPCs, zombies, etc.) */
	uint32_t character_count;
	struct {
		uint16_t role;        /* AI_ROLE_* from SHANKPIT (7 = RELENTLESS_PURSUER) */
		float x, y, z;
		float yaw;
	} *characters;

	/* Level exit chain */
	uint32_t exit_count;
	struct {
		float x, y, z;
		float radius;
		uint32_t next_level_id;  /* Next level on exit */
	} *exits;

	/* Materials (textures, shaders) */
	uint32_t material_count;
	struct {
		char name[32];
		char shader[32];       /* "standard", "hps_light", "ips_light", etc. */
		float specular, shininess;
		char texture_url[256]; /* IDUNA texture fetch URL */
	} *materials;
} Level;

/* Load level from IDUNA HTTP API (NOCK registry)
 * Returns NULL on failure; check *error_out for details.
 * On success, caller owns the Level* (must free with level_free()) */
Level* level_load_from_iduna(const char *iduna_host, int iduna_port,
                             uint32_t level_id,
                             char *error_out, size_t error_len);

/* Load level from local JSON file (fallback for testing)
 * Same format as IDUNA shankpit_levels API export */
Level* level_load_from_file(const char *path,
                            char *error_out, size_t error_len);

/* Free level and all allocations */
void level_free(Level *lvl);

/* Convert PAPERCRAFT PCM chunks to unified Level format (future)
 * Currently a stub; deferred to Phase 2 */
Level* level_load_from_pcm(const char *pcm_data, size_t pcm_len,
                           char *error_out, size_t error_len);

#endif /* LEVEL_LOADER_H */

/* ────────────────────────────────────────────────────────────────────────────
 * Implementation
 * ──────────────────────────────────────────────────────────────────────────── */

#ifdef LEVEL_LOADER_IMPL

#include "http_client.h"
#include <ctype.h>

/* Minimal JSON parser for shankpit_levels API response */
typedef struct {
	const char *text;
	size_t pos;
	size_t len;
} JSONParser;

static void json_skip_whitespace(JSONParser *p) {
	while (p->pos < p->len && isspace((unsigned char)p->text[p->pos])) p->pos++;
}

static int json_match(JSONParser *p, const char *literal) {
	json_skip_whitespace(p);
	size_t literal_len = strlen(literal);
	if (p->pos + literal_len > p->len) return 0;
	if (strncmp(p->text + p->pos, literal, literal_len) != 0) return 0;
	p->pos += literal_len;
	return 1;
}

static int json_parse_string(JSONParser *p, char *out, size_t out_len) {
	json_skip_whitespace(p);
	if (p->pos >= p->len || p->text[p->pos] != '"') return 0;
	p->pos++;
	size_t out_i = 0;
	while (p->pos < p->len && p->text[p->pos] != '"' && out_i + 1 < out_len) {
		if (p->text[p->pos] == '\\' && p->pos + 1 < p->len) {
			p->pos++;
			char c = p->text[p->pos];
			if (c == 'n') out[out_i++] = '\n';
			else if (c == 't') out[out_i++] = '\t';
			else out[out_i++] = c;
		} else {
			out[out_i++] = p->text[p->pos];
		}
		p->pos++;
	}
	out[out_i] = '\0';
	if (p->pos >= p->len || p->text[p->pos] != '"') return 0;
	p->pos++;
	return 1;
}

static int json_parse_number(JSONParser *p, double *out) {
	json_skip_whitespace(p);
	char *endp = NULL;
	*out = strtod(p->text + p->pos, &endp);
	if (endp == (char*)(p->text + p->pos)) return 0;
	p->pos = endp - p->text;
	return 1;
}

static int json_parse_array_start(JSONParser *p) {
	json_skip_whitespace(p);
	if (p->pos >= p->len || p->text[p->pos] != '[') return 0;
	p->pos++;
	return 1;
}

static int json_parse_array_end(JSONParser *p) {
	json_skip_whitespace(p);
	if (p->pos >= p->len || p->text[p->pos] != ']') return 0;
	p->pos++;
	return 1;
}

static int json_parse_object_start(JSONParser *p) {
	json_skip_whitespace(p);
	if (p->pos >= p->len || p->text[p->pos] != '{') return 0;
	p->pos++;
	return 1;
}

static int json_parse_object_end(JSONParser *p) {
	json_skip_whitespace(p);
	if (p->pos >= p->len || p->text[p->pos] != '}') return 0;
	p->pos++;
	return 1;
}

static inline int json_parse_key(JSONParser *p, const char *key) {
	if (!json_match(p, "\"")) return 0;
	if (!json_match(p, key)) return 0;
	if (!json_match(p, "\":")) return 0;
	return 1;
}

/* json_skip_value: advances p past one JSON value (string/array/object/number/bool/null) without
 * storing it anywhere. Every per-object field loop below (walls/spawners/exits) needs this for
 * any real key it doesn't itself care about (e.g. a wall's "id" or "friction") -- without it,
 * position never moves past that field's value, the loop misreads the next field's key as a
 * literal, and parsing derails into a spurious "expected object end" error. Same logic the
 * top-level object skip already used inline; factored out so every nested loop can share it. */
static void json_skip_value(JSONParser *p) {
	json_skip_whitespace(p);
	if (p->pos < p->len && p->text[p->pos] == '"') {
		p->pos++;
		while (p->pos < p->len && p->text[p->pos] != '"') {
			if (p->text[p->pos] == '\\' && p->pos + 1 < p->len) p->pos++;
			p->pos++;
		}
		p->pos++;
	} else if (p->pos < p->len && p->text[p->pos] == '[') {
		int depth = 1;
		p->pos++;
		while (p->pos < p->len && depth > 0) {
			if (p->text[p->pos] == '[') depth++;
			else if (p->text[p->pos] == ']') depth--;
			p->pos++;
		}
	} else if (p->pos < p->len && p->text[p->pos] == '{') {
		int depth = 1;
		p->pos++;
		while (p->pos < p->len && depth > 0) {
			if (p->text[p->pos] == '{') depth++;
			else if (p->text[p->pos] == '}') depth--;
			p->pos++;
		}
	} else {
		while (p->pos < p->len && p->text[p->pos] != ',' && p->text[p->pos] != '}' && p->text[p->pos] != ']') p->pos++;
	}
}

/* Parse IDUNA shankpit_levels JSON response into Level struct */
static Level* parse_level_json(const char *json_text, size_t json_len, char *err, size_t err_len) {
	if (!json_text || json_len == 0) {
		snprintf(err, err_len, "Empty JSON response");
		return NULL;
	}

	Level *lvl = (Level*)malloc(sizeof(*lvl));
	if (!lvl) {
		snprintf(err, err_len, "Memory allocation failed");
		return NULL;
	}
	memset(lvl, 0, sizeof(*lvl));

	JSONParser p = { .text = json_text, .pos = 0, .len = json_len };

	/* Parse { "id": ..., "name": ..., "walls": [...], ... } */
	if (!json_parse_object_start(&p)) {
		snprintf(err, err_len, "Expected object start");
		goto error;
	}

	/* The real IDUNA /api/v1/shankpit-levels/:id/export payload carries no top-level "id" field
	 * at all (confirmed live against nextown, level 12) -- the id is implicit in the URL that
	 * was fetched. "id" stays an optional field here (set from JSON when present, otherwise left
	 * as whatever the caller already stamped on lvl->id) rather than a required one, so real
	 * exports don't fail to parse. */
	int found_name = 0, found_walls = 0;
	double next_level_id_val = 0;
	int found_next_level_id = 0;
	while (p.pos < p.len) {
		json_skip_whitespace(&p);
		if (p.pos < p.len && p.text[p.pos] == '}') break;

		/* Parse key */
		json_skip_whitespace(&p);
		if (p.text[p.pos] != '"') {
			snprintf(err, err_len, "Expected string key");
			goto error;
		}
		p.pos++;
		char key[64] = {0};
		size_t key_len = 0;
		while (p.pos < p.len && p.text[p.pos] != '"' && key_len < sizeof(key)-1) {
			key[key_len++] = p.text[p.pos++];
		}
		if (p.pos >= p.len) {
			snprintf(err, err_len, "Unterminated string key");
			goto error;
		}
		p.pos++; /* closing " */

		json_skip_whitespace(&p);
		if (p.pos >= p.len || p.text[p.pos] != ':') {
			snprintf(err, err_len, "Expected :");
			goto error;
		}
		p.pos++;

		/* Parse value */
		if (strcmp(key, "id") == 0) {
			double id_val;
			if (!json_parse_number(&p, &id_val)) {
				snprintf(err, err_len, "Invalid id value");
				goto error;
			}
			lvl->id = (uint32_t)id_val;
		} else if (strcmp(key, "next_level_id") == 0) {
			if (!json_parse_number(&p, &next_level_id_val)) {
				snprintf(err, err_len, "Invalid next_level_id value");
				goto error;
			}
			found_next_level_id = 1;
		} else if (strcmp(key, "name") == 0) {
			if (!json_parse_string(&p, lvl->name, sizeof(lvl->name))) {
				snprintf(err, err_len, "Invalid name value");
				goto error;
			}
			found_name = 1;
		} else if (strcmp(key, "walls") == 0) {
			/* Parse walls array */
			if (!json_parse_array_start(&p)) {
				snprintf(err, err_len, "Invalid walls array");
				goto error;
			}

			uint32_t wall_cap = 128;
			lvl->walls = (typeof(lvl->walls))malloc(wall_cap * sizeof(lvl->walls[0]));
			if (!lvl->walls) goto error;
			lvl->wall_count = 0;

			while (p.pos < p.len && p.text[p.pos] != ']') {
				if (lvl->wall_count >= wall_cap) {
					wall_cap *= 2;
					lvl->walls = (typeof(lvl->walls))realloc(lvl->walls, wall_cap * sizeof(lvl->walls[0]));
					if (!lvl->walls) goto error;
				}

				if (!json_parse_object_start(&p)) break;
				memset(&lvl->walls[lvl->wall_count], 0, sizeof(lvl->walls[0]));

				/* Parse wall fields */
				while (p.pos < p.len && p.text[p.pos] != '}') {
					json_skip_whitespace(&p);
					if (p.text[p.pos] == '"') p.pos++;
					else break;

					char wkey[32] = {0};
					size_t wkey_len = 0;
					while (p.pos < p.len && p.text[p.pos] != '"' && wkey_len < sizeof(wkey)-1) {
						wkey[wkey_len++] = p.text[p.pos++];
					}
					p.pos++; /* closing " */

					json_skip_whitespace(&p);
					p.pos++; /* : */

					double val = 0;
					if (strcmp(wkey, "x") == 0) {
						json_parse_number(&p, &val);
						lvl->walls[lvl->wall_count].x = (float)val;
					} else if (strcmp(wkey, "y") == 0) {
						json_parse_number(&p, &val);
						lvl->walls[lvl->wall_count].y = (float)val;
					} else if (strcmp(wkey, "z") == 0) {
						json_parse_number(&p, &val);
						lvl->walls[lvl->wall_count].z = (float)val;
					} else if (strcmp(wkey, "sx") == 0) {
						json_parse_number(&p, &val);
						lvl->walls[lvl->wall_count].sx = (float)val;
					} else if (strcmp(wkey, "sy") == 0) {
						json_parse_number(&p, &val);
						lvl->walls[lvl->wall_count].sy = (float)val;
					} else if (strcmp(wkey, "sz") == 0) {
						json_parse_number(&p, &val);
						lvl->walls[lvl->wall_count].sz = (float)val;
					} else if (strcmp(wkey, "r") == 0) {
						json_parse_number(&p, &val);
						lvl->walls[lvl->wall_count].r = (float)val;
					} else if (strcmp(wkey, "g") == 0) {
						json_parse_number(&p, &val);
						lvl->walls[lvl->wall_count].g = (float)val;
					} else if (strcmp(wkey, "b") == 0) {
						json_parse_number(&p, &val);
						lvl->walls[lvl->wall_count].b = (float)val;
					} else if (strcmp(wkey, "friction") == 0) {
						json_parse_number(&p, &val);
						lvl->walls[lvl->wall_count].friction = (float)val;
					} else if (strcmp(wkey, "material") == 0) {
						json_parse_string(&p, lvl->walls[lvl->wall_count].material, sizeof(lvl->walls[lvl->wall_count].material));
					} else {
						/* Real, unhandled fields (e.g. the export's per-wall "id") -- must still be
						 * consumed or the next iteration misreads this value as a key. */
						json_skip_value(&p);
					}

					json_skip_whitespace(&p);
					if (p.pos < p.len && p.text[p.pos] == ',') p.pos++;
				}

				if (!json_parse_object_end(&p)) break;
				lvl->wall_count++;

				json_skip_whitespace(&p);
				if (p.pos < p.len && p.text[p.pos] == ',') p.pos++;
			}

			if (!json_parse_array_end(&p)) {
				snprintf(err, err_len, "Unterminated walls array");
				goto error;
			}
			found_walls = 1;
		} else if (strcmp(key, "spawners") == 0) {
			if (!json_parse_array_start(&p)) {
				snprintf(err, err_len, "Invalid spawners array");
				goto error;
			}

			uint32_t spawner_cap = 8;
			lvl->spawners = (typeof(lvl->spawners))malloc(spawner_cap * sizeof(lvl->spawners[0]));
			if (!lvl->spawners) goto error;
			lvl->spawner_count = 0;

			while (p.pos < p.len && p.text[p.pos] != ']') {
				if (lvl->spawner_count >= spawner_cap) {
					spawner_cap *= 2;
					lvl->spawners = (typeof(lvl->spawners))realloc(lvl->spawners, spawner_cap * sizeof(lvl->spawners[0]));
					if (!lvl->spawners) goto error;
				}

				if (!json_parse_object_start(&p)) break;
				memset(&lvl->spawners[lvl->spawner_count], 0, sizeof(lvl->spawners[0]));

				while (p.pos < p.len && p.text[p.pos] != '}') {
					json_skip_whitespace(&p);
					if (p.text[p.pos] == '"') p.pos++;
					else break;

					char skey[32] = {0};
					size_t skey_len = 0;
					while (p.pos < p.len && p.text[p.pos] != '"' && skey_len < sizeof(skey)-1) {
						skey[skey_len++] = p.text[p.pos++];
					}
					p.pos++; /* closing " */

					json_skip_whitespace(&p);
					p.pos++; /* : */

					double val = 0;
					if (strcmp(skey, "x") == 0) {
						json_parse_number(&p, &val);
						lvl->spawners[lvl->spawner_count].x = (float)val;
					} else if (strcmp(skey, "y") == 0) {
						json_parse_number(&p, &val);
						lvl->spawners[lvl->spawner_count].y = (float)val;
					} else if (strcmp(skey, "z") == 0) {
						json_parse_number(&p, &val);
						lvl->spawners[lvl->spawner_count].z = (float)val;
					} else if (strcmp(skey, "yaw") == 0) {
						json_parse_number(&p, &val);
						lvl->spawners[lvl->spawner_count].yaw = (float)val;
					} else if (strcmp(skey, "team") == 0) {
						json_parse_number(&p, &val);
						lvl->spawners[lvl->spawner_count].team = (int16_t)val;
					} else {
						json_skip_value(&p);
					}

					json_skip_whitespace(&p);
					if (p.pos < p.len && p.text[p.pos] == ',') p.pos++;
				}

				if (!json_parse_object_end(&p)) break;
				lvl->spawner_count++;

				json_skip_whitespace(&p);
				if (p.pos < p.len && p.text[p.pos] == ',') p.pos++;
			}

			if (!json_parse_array_end(&p)) {
				snprintf(err, err_len, "Unterminated spawners array");
				goto error;
			}
		} else if (strcmp(key, "level_exits") == 0 || strcmp(key, "exits") == 0) {
			/* IDUNA's real export field is "level_exits"; "exits" (the Level struct's own field
			 * name) is accepted too in case a future export/local test file uses it directly. */
			if (!json_parse_array_start(&p)) {
				snprintf(err, err_len, "Invalid exits array");
				goto error;
			}

			uint32_t exit_cap = 4;
			lvl->exits = (typeof(lvl->exits))malloc(exit_cap * sizeof(lvl->exits[0]));
			if (!lvl->exits) goto error;
			lvl->exit_count = 0;

			while (p.pos < p.len && p.text[p.pos] != ']') {
				if (lvl->exit_count >= exit_cap) {
					exit_cap *= 2;
					lvl->exits = (typeof(lvl->exits))realloc(lvl->exits, exit_cap * sizeof(lvl->exits[0]));
					if (!lvl->exits) goto error;
				}

				if (!json_parse_object_start(&p)) break;
				memset(&lvl->exits[lvl->exit_count], 0, sizeof(lvl->exits[0]));

				while (p.pos < p.len && p.text[p.pos] != '}') {
					json_skip_whitespace(&p);
					if (p.text[p.pos] == '"') p.pos++;
					else break;

					char ekey[32] = {0};
					size_t ekey_len = 0;
					while (p.pos < p.len && p.text[p.pos] != '"' && ekey_len < sizeof(ekey)-1) {
						ekey[ekey_len++] = p.text[p.pos++];
					}
					p.pos++; /* closing " */

					json_skip_whitespace(&p);
					p.pos++; /* : */

					double val = 0;
					if (strcmp(ekey, "x") == 0) {
						json_parse_number(&p, &val);
						lvl->exits[lvl->exit_count].x = (float)val;
					} else if (strcmp(ekey, "y") == 0) {
						json_parse_number(&p, &val);
						lvl->exits[lvl->exit_count].y = (float)val;
					} else if (strcmp(ekey, "z") == 0) {
						json_parse_number(&p, &val);
						lvl->exits[lvl->exit_count].z = (float)val;
					} else if (strcmp(ekey, "radius") == 0) {
						json_parse_number(&p, &val);
						lvl->exits[lvl->exit_count].radius = (float)val;
					} else if (strcmp(ekey, "next_level_id") == 0) {
						/* Per-exit override, if a future export ever carries one directly.
						 * The real, current export instead carries a single top-level
						 * next_level_id applied to every exit -- see the post-parse fixup
						 * below. */
						json_parse_number(&p, &val);
						lvl->exits[lvl->exit_count].next_level_id = (uint32_t)val;
					} else {
						json_skip_value(&p);
					}

					json_skip_whitespace(&p);
					if (p.pos < p.len && p.text[p.pos] == ',') p.pos++;
				}

				if (!json_parse_object_end(&p)) break;
				lvl->exit_count++;

				json_skip_whitespace(&p);
				if (p.pos < p.len && p.text[p.pos] == ',') p.pos++;
			}

			if (!json_parse_array_end(&p)) {
				snprintf(err, err_len, "Unterminated exits array");
				goto error;
			}
		} else {
			/* Skip unknown fields */
			json_skip_whitespace(&p);
			if (p.pos < p.len && p.text[p.pos] == '"') {
				/* String */
				p.pos++;
				while (p.pos < p.len && p.text[p.pos] != '"') p.pos++;
				p.pos++;
			} else if (p.pos < p.len && p.text[p.pos] == '[') {
				/* Array: skip to closing ] */
				int depth = 1;
				p.pos++;
				while (p.pos < p.len && depth > 0) {
					if (p.text[p.pos] == '[') depth++;
					else if (p.text[p.pos] == ']') depth--;
					p.pos++;
				}
			} else if (p.pos < p.len && p.text[p.pos] == '{') {
				/* Object: skip to closing } */
				int depth = 1;
				p.pos++;
				while (p.pos < p.len && depth > 0) {
					if (p.text[p.pos] == '{') depth++;
					else if (p.text[p.pos] == '}') depth--;
					p.pos++;
				}
			} else {
				/* Number, bool, null */
				while (p.pos < p.len && p.text[p.pos] != ',' && p.text[p.pos] != '}') p.pos++;
			}
		}

		json_skip_whitespace(&p);
		if (p.pos < p.len && p.text[p.pos] == ',') p.pos++;
	}

	if (!json_parse_object_end(&p)) {
		snprintf(err, err_len, "Expected object end");
		goto error;
	}

	if (!found_name || !found_walls) {
		snprintf(err, err_len, "Missing required fields (name, walls)");
		goto error;
	}

	/* The real export carries next_level_id once, at the top level, applying to every exit
	 * (there is no per-exit override in practice) -- fill in any exit that didn't parse its own. */
	if (found_next_level_id) {
		for (uint32_t i = 0; i < lvl->exit_count; i++) {
			if (lvl->exits[i].next_level_id == 0) {
				lvl->exits[i].next_level_id = (uint32_t)next_level_id_val;
			}
		}
	}

	return lvl;

error:
	level_free(lvl);
	return NULL;
}

Level* level_load_from_iduna(const char *iduna_host, int iduna_port,
                             uint32_t level_id,
                             char *error_out, size_t error_len) {
	/* The plain /api/v1/shankpit-levels/:id endpoint is the NOCK registry's editor-facing route
	 * and 404s for a public fetch -- packages/world/level_boxes.h (SHANKPIT's own already-working
	 * loader) hits /export, confirmed live against local IDUNA (level 12 = nextown). */
	char path[256];
	snprintf(path, sizeof(path), "/api/v1/shankpit-levels/%u/export", level_id);

	char resp[65536]; /* 64KB response buffer */
	int status = 0;
	if (http_get_json(iduna_host, iduna_port, path, NULL, resp, sizeof(resp), &status) != 0) {
		snprintf(error_out, error_len, "HTTP request failed to %s:%d%s", iduna_host, iduna_port, path);
		return NULL;
	}

	if (status != 200) {
		snprintf(error_out, error_len, "HTTP %d from %s:%d", status, iduna_host, iduna_port);
		return NULL;
	}

	Level *lvl = parse_level_json(resp, strlen(resp), error_out, error_len);
	/* The real export has no top-level "id" field -- stamp the id the caller actually asked for. */
	if (lvl) lvl->id = level_id;
	return lvl;
}

Level* level_load_from_file(const char *path, char *error_out, size_t error_len) {
	FILE *f = fopen(path, "rb");
	if (!f) {
		snprintf(error_out, error_len, "Cannot open file: %s", path);
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *json = (char*)malloc(size + 1);
	if (!json) {
		snprintf(error_out, error_len, "Memory allocation failed");
		fclose(f);
		return NULL;
	}

	if (fread(json, 1, size, f) != size) {
		snprintf(error_out, error_len, "Read error");
		free(json);
		fclose(f);
		return NULL;
	}
	json[size] = '\0';
	fclose(f);

	Level *lvl = parse_level_json(json, size, error_out, error_len);
	free(json);
	return lvl;
}

void level_free(Level *lvl) {
	if (!lvl) return;
	free(lvl->walls);
	free(lvl->spawners);
	free(lvl->characters);
	free(lvl->exits);
	free(lvl->materials);
	free(lvl);
}

Level* level_load_from_pcm(const char *pcm_data, size_t pcm_len, char *error_out, size_t error_len) {
	(void)pcm_data;
	(void)pcm_len;
	snprintf(error_out, error_len, "PCM loading not yet implemented");
	return NULL;
}

#endif /* LEVEL_LOADER_IMPL */
