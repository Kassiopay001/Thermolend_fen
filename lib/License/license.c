#include <stdio.h>
#include <stdlib.h>
#include "license.h"
#include "esp_mac.h"
#include "esp_http_client.h"
#include "nvs.h"
#include "monocypher.h"	// Ed25519 (RFC 8032), вместо libsodium

#define NVS_NS   "sys"
#define NVS_KEY  "key"
#define FW_STAMP_URL "http://ml-systems.ru"	// Корень сайта, хвост в PATH_ENC

// Хвост адреса "/stamp.php?id=" в перемешанном виде (XOR с KEY_B).
// strings его не покажет. Пересчитать под свой путь/ключ — маленьким скриптом.
#define KEY_B 0x5a
static const uint8_t PATH_ENC[14] = {
    0x75, 0x29, 0x2e, 0x3b, 0x37, 0x2a, 0x74,
    0x2a, 0x32, 0x2a, 0x65, 0x33, 0x3e, 0x67
};

// Публичный ключ: открыть в браузере http://ТВОЙ_САЙТ/stamp.php?k
// и вставить сюда полученную строку вместо нулей
static const uint8_t PUB_KEY[32] = {0x4a, 0x1b, 0x9f, 0x2d, 0x2f, 0x95, 0x0b, 0xe7, 0x2c, 0x60, 0x54, 0xf3, 0xdf, 0x4b, 0x53, 0x72, 0x19, 0x1d, 0x4d, 0x90, 0xf9, 0x63, 0xa3, 0x09, 0xf6, 0x65, 0x73, 0x92, 0xdf, 0x66, 0x16, 0x73};

// Проверка без интернета. true = устройство активировано
bool license_check(void)
{
	// return true; // Заглушка для тестов, пока нет лицензии. Убрать после получения лицензии.
	
    uint8_t mac[6], sig[64];
    size_t len = sizeof(sig);
    nvs_handle_t h;

    if (esp_efuse_mac_get_default(mac) != ESP_OK) return false;
    if (nvs_open(NVS_NS, NVS_READONLY, &h) != ESP_OK) return false;
    esp_err_t err = nvs_get_blob(h, NVS_KEY, sig, &len);
    nvs_close(h);
    if (err != ESP_OK || len != sizeof(sig)) return false;

    return crypto_ed25519_check(sig, PUB_KEY, mac, sizeof(mac)) == 0;
}

// Полный адрес запроса подписи: FW_STAMP_URL + "/stamp.php?id=" + MAC.
// Вынесено из license_activate(), чтобы тот же адрес можно было запросить и через
// другой сетевой стек (например, W5500), где esp_http_client не работает.
esp_err_t license_get_url(char *url, size_t size)
{
    uint8_t mac[6];
    char path[15];

    if (esp_efuse_mac_get_default(mac) != ESP_OK) return ESP_FAIL;

    // собрать хвост "/stamp.php?id=" обратно
    for (int i = 0; i < 14; i++) path[i] = PATH_ENC[i] ^ KEY_B;
    path[14] = 0;

    snprintf(url, size, "%s%s%02x%02x%02x%02x%02x%02x",
             FW_STAMP_URL, path,
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return ESP_OK;
}

// Ответ сервера (128 hex-символов) -> проверка подписи -> NVS. ESP_OK = активировано
esp_err_t license_save(const char *hex, int len)
{
    uint8_t mac[6], sig[64];

    if (len != 128) return ESP_FAIL;
    if (esp_efuse_mac_get_default(mac) != ESP_OK) return ESP_FAIL;

    // --- hex -> 64 байта подписи ---
    for (int i = 0; i < 64; i++) {
        char b[3] = { hex[2 * i], hex[2 * i + 1], 0 }, *e;
        sig[i] = (uint8_t)strtoul(b, &e, 16);
        if (*e) return ESP_FAIL;
    }

    // --- проверка подписи перед сохранением ---
    if (crypto_ed25519_check(sig, PUB_KEY, mac, sizeof(mac)) != 0)
        return ESP_FAIL;

    // --- запись в NVS ---
    nvs_handle_t h;
    esp_err_t err = nvs_open(NVS_NS, NVS_READWRITE, &h);
    if (err != ESP_OK) return err;
    err = nvs_set_blob(h, NVS_KEY, sig, sizeof(sig));
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    return err;
}

// Активация через сервер (нужен интернет, только через сетевой стек ESP). ESP_OK = активировано
esp_err_t license_activate(void)
{
    char url[128], hex[129] = {0};
    int n = 0;

    if (license_get_url(url, sizeof(url)) != ESP_OK) return ESP_FAIL;

    // --- HTTP GET ---
    esp_http_client_config_t cfg = { .url = url, .timeout_ms = 10000 };
    esp_http_client_handle_t c = esp_http_client_init(&cfg);
    if (!c) return ESP_FAIL;
    if (esp_http_client_open(c, 0) == ESP_OK) {
        esp_http_client_fetch_headers(c);
        if (esp_http_client_get_status_code(c) == 200)
            n = esp_http_client_read_response(c, hex, 128);
    }
    esp_http_client_cleanup(c);

    return license_save(hex, n);
}
