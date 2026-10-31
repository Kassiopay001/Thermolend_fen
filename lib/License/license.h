#pragma once
#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// Проверка без интернета: подпись в NVS соответствует заводскому MAC
bool license_check(void);

// Получение подписи с сервера (нужен интернет), сохраняет в NVS
esp_err_t license_activate(void);

// Части license_activate() по отдельности — для запроса через свой транспорт
// (например, Ethernet W5500, где esp_http_client не работает):
// адрес запроса для этого устройства и приём ответа сервера (проверка + запись в NVS)
esp_err_t license_get_url(char *url, size_t size);
esp_err_t license_save(const char *hex, int len);

#ifdef __cplusplus
}
#endif
