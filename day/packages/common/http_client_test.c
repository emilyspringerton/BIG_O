/* http_client_test.c — hermetic regression coverage for http_client.h's chunked-transfer decode
 * (http_headers_has_chunked_encoding / http_dechunk). Added 2026-09-20 after a real, live bug:
 * IDUNA's GET /api/v1/shankpit-levels/:id/export sends Transfer-Encoding: chunked (Go net/http's
 * default when a handler doesn't set an explicit Content-Length), and this client's hand-rolled
 * HTTP parser used to hand the raw chunked body -- hex chunk-size lines and all -- straight to
 * the JSON parser, which failed with "Expected object start" on every real call. No network
 * needed here: both functions are pure text transforms, so this is exercised directly, not via
 * a live IDUNA fetch (level_loader_test.c / the live main() integration cover that separately). */
#include "http_client.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
	/* Test 1: detects chunked encoding in a real-shaped header block */
	{
		const char headers[] = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nTransfer-Encoding: chunked\r\n\r\n";
		assert(http_headers_has_chunked_encoding(headers, strlen(headers)) == 1);
		printf("✓ Test 1: detects chunked encoding\n");
	}

	/* Test 2: does not false-positive on a plain Content-Length response */
	{
		const char headers[] = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: 42\r\n\r\n";
		assert(http_headers_has_chunked_encoding(headers, strlen(headers)) == 0);
		printf("✓ Test 2: no false positive on Content-Length\n");
	}

	/* Test 3: case-insensitive (real servers vary casing) */
	{
		const char headers[] = "HTTP/1.1 200 OK\r\ntransfer-encoding: CHUNKED\r\n\r\n";
		assert(http_headers_has_chunked_encoding(headers, strlen(headers)) == 1);
		printf("✓ Test 3: case-insensitive match\n");
	}

	/* Test 4: decodes a real multi-chunk body ("{\"a\":1," + "\"b\":22}", 7 bytes each) */
	{
		const char chunked[] = "7\r\n{\"a\":1,\r\n7\r\n\"b\":22}\r\n0\r\n\r\n";
		char out[64];
		size_t n = http_dechunk(chunked, out, sizeof(out));
		assert(n == 14);
		assert(strcmp(out, "{\"a\":1,\"b\":22}") == 0);
		printf("✓ Test 4: multi-chunk decode\n");
	}

	/* Test 5: single chunk, exact real shape IDUNA sends for a small body ("{\"ok\":true}" is 11 bytes) */
	{
		const char chunked[] = "b\r\n{\"ok\":true}\r\n0\r\n\r\n";
		char out[64];
		size_t n = http_dechunk(chunked, out, sizeof(out));
		assert(n == 11);
		assert(strcmp(out, "{\"ok\":true}") == 0);
		printf("✓ Test 5: single-chunk decode\n");
	}

	/* Test 6: output buffer smaller than the decoded body truncates cleanly (no overrun, no
	 * crash) instead of writing past out_cap -- same best-effort posture as the surrounding
	 * recv()-truncation handling this header already documents. */
	{
		const char chunked[] = "a\r\n0123456789\r\n0\r\n\r\n";
		char out[6];
		size_t n = http_dechunk(chunked, out, sizeof(out));
		assert(n == 5);
		assert(strcmp(out, "01234") == 0);
		printf("✓ Test 6: output truncation is safe\n");
	}

	printf("\nAll tests passed! http_client.h chunked-decode is ready.\n");
	return 0;
}
