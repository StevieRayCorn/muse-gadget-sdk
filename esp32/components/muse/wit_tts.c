#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "driver/i2s_std.h"
#include "wit_tts.h"

#define TAG "WIT_TTS"
#define BUFFER 1024

// Callback or function executed when the agent's text response is ready
void play_wit_tts_audio(char *agent_text, wit_tts_cb on_wit_tts_cb)
{
    char url[128];
    snprintf(url, sizeof(url), "https://api.wit.ai/synthesize?v=20240304");

    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 5000,
        .crt_bundle_attach = esp_crt_bundle_attach, // Enables standard CA certificate verification
    };

    // Clean agent_text to prevent malformed JSON
    char clean_text[BUFFER];
    int j = 0;
    for (int i = 0; agent_text[i] != '\0' && j < sizeof(clean_text) - 1; i++)
    {
        char c = agent_text[i];
        if (c != '"' && c != '\\' && c != '\n' && c != '\r')
        {
            clean_text[j++] = c;
        }
        else
        {
            clean_text[j++] = ' '; // Replace problematic characters with a space
        }
    }
    clean_text[j] = '\0';

    esp_http_client_handle_t client = esp_http_client_init(&config);

    // Set headers (Authorization bearer token & content type)
    esp_http_client_set_header(client, "Accept", "audio/mpeg");
    esp_http_client_set_header(client, "Authorization", "Bearer ");
    esp_http_client_set_header(client, "Content-Type", "application/json");

    // Format the text body payload
    char post_data[BUFFER + 64];
    snprintf(post_data, sizeof(post_data), "{\"q\":\"%s\",\"voice\":\"wit$Remi\"}", clean_text);
    // ESP_LOGW(TAG, "POST body: %s", post_data);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    // Open connection
    esp_err_t err = esp_http_client_open(client, strlen(post_data));
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return;
    }

    // 2. CRITICAL: Actually write the POST body data to the socket!
    int wlen = esp_http_client_write(client, post_data, strlen(post_data));
    if (wlen < 0)
    {
        ESP_LOGE(TAG, "Failed to write POST data");
        esp_http_client_cleanup(client);
        return;
    }

    int content_length = esp_http_client_fetch_headers(client);
    int status_code = esp_http_client_get_status_code(client);
    ESP_LOGI(TAG, "Wit.ai HTTP Status: %d, Content Length: %i", status_code, content_length);

    // Buffer to read incoming audio stream chunks
    int read_len;
    char read_buffer[512];
    // size_t bytes_written = 0;

    while (1)
    {
        read_len = esp_http_client_read(client, read_buffer, sizeof(read_buffer));
        if (on_wit_tts_cb)
        {
            on_wit_tts_cb((uint8_t *)read_buffer, read_len);
        }
        if (read_len < 0)
        {
            ESP_LOGE(TAG, "Read failed from HTTP stream");
            break;
        }
        else if (read_len == 0)
        {
            // End of stream
            break;
        }
    }

    esp_http_client_cleanup(client);
    ESP_LOGI(TAG, "Finished playing TTS audio stream.");
}
