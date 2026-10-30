#pragma once

typedef void (*wit_tts_cb)(const uint8_t *data, size_t len);

void play_wit_tts_audio(char *agent_text, wit_tts_cb on_wit_tts_cb);