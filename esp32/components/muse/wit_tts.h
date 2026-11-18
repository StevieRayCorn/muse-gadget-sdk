#pragma once

/* ctx is passed through to every callback: the caller uses it to tell a stale
 * fetch (from a turn that has since ended) from the live one. A call with
 * len == 0 signals the end of the stream (data is NULL then); it is delivered
 * exactly once, on success and on failure alike, so the caller can finish the
 * message instead of waiting on audio that will never arrive. */
typedef void (*wit_tts_cb)(void *ctx, const uint8_t *data, size_t len);

void play_wit_tts_audio(char *agent_text, wit_tts_cb on_wit_tts_cb, void *ctx);
