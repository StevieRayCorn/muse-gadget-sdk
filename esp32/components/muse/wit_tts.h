#pragma once

#include <stdbool.h>

/* ctx is passed through to every callback: the caller uses it to tell a stale
 * fetch (from a turn that has since ended) from the live one. A call with
 * len == 0 signals the end of the stream (data is NULL then); it is delivered
 * exactly once, on success and on failure alike, so the caller can finish the
 * message instead of waiting on audio that will never arrive. */
typedef void (*wit_tts_cb)(void *ctx, const uint8_t *data, size_t len);

void play_wit_tts_audio(char *agent_text, wit_tts_cb on_wit_tts_cb, void *ctx);

/* Fetch one piece of text without signalling the end of the stream. Returns
 * true when the whole MP3 arrived, false when the request failed part-way
 * (connection, write, HTTP error, or a read failure). The caller signals the
 * end itself, once, after the last piece — or after a failed one, so the
 * message ends instead of hanging. */
bool play_wit_tts_audio_part(char *agent_text, wit_tts_cb on_wit_tts_cb, void *ctx);
