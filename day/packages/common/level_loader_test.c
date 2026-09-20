/* level_loader_test.c — unit tests for level_loader.h JSON parsing */
#define LEVEL_LOADER_IMPL
#include "level_loader.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
	/* Test 1: Parse minimal valid level JSON */
	{
		const char json[] = "{\"id\": 12, \"name\": \"nextown\", \"walls\": [{\"x\": 100, \"y\": 50, \"z\": 200, \"sx\": 50, \"sy\": 100, \"sz\": 50, \"r\": 0.6, \"g\": 0.6, \"b\": 0.65}]}";
		char err[256];
		Level *lvl = parse_level_json(json, strlen(json), err, sizeof(err));
		assert(lvl != NULL);
		assert(lvl->id == 12);
		assert(strcmp(lvl->name, "nextown") == 0);
		assert(lvl->wall_count == 1);
		assert(lvl->walls[0].x == 100.0f);
		level_free(lvl);
		printf("✓ Test 1: Minimal JSON parse\n");
	}

	/* Test 2: Multiple walls */
	{
		const char json[] = "{\"id\": 1, \"name\": \"test\", \"walls\": [{\"x\": 0, \"y\": 0, \"z\": 0, \"sx\": 1, \"sy\": 1, \"sz\": 1, \"r\": 0.5, \"g\": 0.5, \"b\": 0.5}, {\"x\": 10, \"y\": 0, \"z\": 0, \"sx\": 1, \"sy\": 1, \"sz\": 1, \"r\": 0.5, \"g\": 0.5, \"b\": 0.5}]}";
		char err[256];
		Level *lvl = parse_level_json(json, strlen(json), err, sizeof(err));
		assert(lvl != NULL);
		assert(lvl->wall_count == 2);
		assert(lvl->walls[1].x == 10.0f);
		level_free(lvl);
		printf("✓ Test 2: Multiple walls\n");
	}

	/* Test 3: Error handling — a level with no name and no walls is genuinely invalid */
	{
		const char json[] = "{\"id\": 1}";
		char err[256];
		Level *lvl = parse_level_json(json, strlen(json), err, sizeof(err));
		assert(lvl == NULL);
		printf("✓ Test 3: Error handling\n");
	}

	/* Test 4: Real shankpit-levels /export shape — no top-level "id" field (confirmed live
	 * against local IDUNA, level 12 = nextown, 2026-09-20). Parsing must not require "id". */
	{
		const char json[] = "{\"version\":1,\"name\":\"nextown\",\"width\":333,\"height\":300,\"depth\":333,"
			"\"ground_plane_enabled\":true,\"ground_plane_squares\":1111,"
			"\"walls\":[{\"id\":1,\"x\":360.91,\"y\":100,\"z\":490.66,\"sx\":432,\"sy\":320,\"sz\":555,\"r\":0.6,\"g\":0.6,\"b\":0.65,\"friction\":0.8}],"
			"\"spawners\":[{\"id\":1,\"x\":567.62,\"y\":3.81,\"z\":-227.88,\"yaw\":0,\"team\":-1}],"
			"\"level_exits\":[{\"x\":175,\"y\":0,\"z\":550,\"radius\":4}],"
			"\"next_level_id\":19,"
			"\"materials\":[{\"name\":\"brick\",\"shader_name\":\"standard\",\"specular\":0.04,\"shininess\":6,\"friction\":0.3}]}";
		char err[256];
		Level *lvl = parse_level_json(json, strlen(json), err, sizeof(err));
		assert(lvl != NULL);
		assert(strcmp(lvl->name, "nextown") == 0);
		assert(lvl->wall_count == 1);
		assert(lvl->walls[0].sx == 432.0f);
		assert(lvl->spawner_count == 1);
		assert(lvl->spawners[0].team == -1);
		assert(lvl->exit_count == 1);
		assert(lvl->exits[0].radius == 4.0f);
		assert(lvl->exits[0].next_level_id == 19);
		level_free(lvl);
		printf("✓ Test 4: Real /export shape (no top-level id, spawners + level_exits + next_level_id)\n");
	}

	printf("\nAll tests passed! level_loader.h is ready to integrate.\n");
	return 0;
}
