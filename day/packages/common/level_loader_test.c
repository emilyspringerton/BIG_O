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

	/* Test 3: Error handling */
	{
		const char json[] = "{\"id\": 1}";
		char err[256];
		Level *lvl = parse_level_json(json, strlen(json), err, sizeof(err));
		assert(lvl == NULL);
		printf("✓ Test 3: Error handling\n");
	}

	/* Test 4: Realistic nextown data */
	{
		const char json[] = "{\"id\": 12, \"name\": \"nextown\", \"walls\": [{\"x\": 360.91, \"y\": 100, \"z\": 490.66, \"sx\": 432, \"sy\": 320, \"sz\": 555, \"r\": 0.6, \"g\": 0.6, \"b\": 0.65}]}";
		char err[256];
		Level *lvl = parse_level_json(json, strlen(json), err, sizeof(err));
		assert(lvl != NULL);
		assert(lvl->id == 12);
		assert(lvl->wall_count == 1);
		assert(lvl->walls[0].sx == 432.0f);
		level_free(lvl);
		printf("✓ Test 4: Realistic nextown data\n");
	}

	printf("\nAll tests passed! level_loader.h is ready to integrate.\n");
	return 0;
}
