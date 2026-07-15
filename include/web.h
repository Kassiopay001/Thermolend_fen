#pragma once

#include <Arduino.h>

static uint8_t fw_ver[3] = {1, 1, 0};	// Версия прошивки

#define WEB_SERVER_PORT 80

#define API_PATH_STATUS "/api/status"
#define API_PATH_VERSION "/api/version"
#define API_PATH_RUNTIME "/api/runtime"
#define API_PATH_RELAY "/api/relay"
#define API_PATH_ANALOG "/api/analog"
#define API_PATH_SETTINGS "/api/settings"
#define API_PATH_SETTINGS_ALGORITHM "/api/settings/algorithm"

#define API_PATH_WIFI_SCAN "/api/wifi/scan"
#define API_PATH_WIFI_CONNECT "/api/wifi/connect"
#define API_PATH_WIFI_STATUS "/api/wifi/status"
#define API_PATH_WIFI_FORGET "/api/wifi/forget"

#define API_PATH_UPDATE "/api/update"
#define API_PATH_UPDATE_FROM_SERVER "/api/update/server"
#define API_PATH_UPDATE_PROGRESS "/api/update/progress"

// Путь к бинарнику прошивки для обновления "с сервера" — задаётся заранее,
// пользователь в веб-интерфейсе этот адрес не редактирует.
#define FIRMWARE_UPDATE_URL "http://ml-systems.ru/firmwares_laguna/firmware_lcd.bin"

#define SESSION_COOKIE_NAME "session"

#define OTA_HOSTNAME "esp32-relay-controller"

#define LOGO_DATA_URI \
  "data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAGgAAAB4CAMAAAApFEcIAAAAwFBMVEWJvgD8jgDyXgCJvgAA/wD5lAB6pwC0agCNxQC4uABVVQD9jwA/PwCa1wAAAAD+kQCLwQCJvgD+jgD+fgD/qgCqqgD//wB+vQD+kQCIvACLwgD8jQB//wB/fwCLwgD+kQCJvgCLwgD9jwD/AAD9kAD+kQCLwgD7igCq/wCKwQCIuAD+jwD+kAD9kABVqgCIwQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADATSEZAAAAMHRSTlOkWARWARAIAxAEA5AE/wD+/fz8AgMDAQfPK9AxAgKtsdmQqwGQcm8UA08VzzBNAy9xCiUkAAAHh0lEQVR42rVb6XraOhAVhpC0vVcGL2KJTUKNgaSp8/5v1xnZ8iKNsGQS/8jylfhIc2bTHJVxr0dcOX/b7+ecbxZ+f8m8Pp1y/nlYwXPcyV++CQje/H5crdZrhDpvuMi+BSiv+M8zIEgY+LJ/A+j8y4GQnPlebaeGO+w8qGLOVtsdOhgJBT8f352pcgKqRI+c4XNe8jz7IqAc1nzeN+QYD1J1FV8AtJCRs7Lg4CYPnwB1N5AiZ/j6lT9VbIyczVmDwTcP2MKf50ue5ZOBkJw3nRz45fXVdAykaiMmAS02Tb7RV398v54J+2FUXScAITm6S6v3weKPhP2QqsoTKBN8eaaceS4tdDXjV2al808rVcyHHAzP+kVCfWI9/ECzEDcgJGd3GPPiLOvl2B6WjSp2uxho5GyoD+q+f1zyRTUKBJZZnok8QGaaa+2X+qbwswZVjKjUNDlU7hREflK7z24ANZFjWO34ZE0xUA6NjNu4urABwaueqPAYqW9YQ6hImPOcBALLLOcr2uAjFduMbWk9IXgcCw2oNTZR2MZ7EHHtZ6t14ztcJPAlGQBZyXHtqjDE5/suQ8CfiUfOn8sLL5IWqKoDwiBUVjTX5qPqkhY6Qgqvv5zCcFtyMKAEAnLyieSQkY6+k8K7gzIMwwig/oL9BGcpVanrfCMqv65X5i5IdtcYtjPbhvIBqJdngGJmpVYR59fytlRlKZLzAhBRjQTfThfOGOEDMt8s+JQnU+T0HgQsmd7WND6Tikk4SHw8a14+gGJG96R8Jin8YQT4199taD5RxIgmDX3m8tDFmvOTNOQQSP0dresCmaLPwLLKoA4A5ycoJDlRNGTHBIJ8k/MqjetlQQDMZAC4k1OgS0chidMHOkIYSJ/5pbxSBoATVUVMWq23uw7oFVNeofnMyY0qJOek7QYX+rGlgN7FQgx9RgZAPEoV1IJLGRpWw9xDAm1gYdqy5OdHqBJNvtHJKR94nPyxAf2mrHyLqiIZ5Jv2gaTDYy7cgbo/SyzkXChy5NIKnvsBKUPEVL6xkJPEOfcG6pcVF3LQfbD8gwuPA0VaiBtUITkfOjlRExBQLv7bQb/lABTqa9WiCr49nCx+I3uVPeM/nIA+AsL6KqogckhyZvVusPv6XwJtHYBMf2qpqrMutQoRq+Z9dXQEesb0hSSEOlUfROQgOWi1vOsPXU33DDaiVo6vPBGeAvBx3u8PPYBaLiIj9xPkBPHwZMUgRzsDSe/SnTiK9O1A4ujIUZ2OJ5CMF2k/MpLDxmotOV1D5QsE9gOoMrS0AtIN4/xqNu+uQB+8S22J2Q0ocmRgVcSxF9zbH6jrb3RyIFUEcD6njr24o8etL5DMoANXb8nZWMZsB0egZx4bNWGQldClJTnmmK3dUTAFqFfl8DOzoCOHHBoepgO1dbspuXCyqubEifzuHbVUyWKA5Mz1GcNgR+kdQHVWQsKIqQm692tvR/cBIVUFFTn1wWo3AEruAuIFTmsocqB33w1Ml/jHETFmM8b6G55Vn95A1h01cxJqznTN9B39mQyUmZMfdeqF02jGP4ccTQXCgQBJzhInkjg6230F0OI6prl8DVBq5ht93t0BrSc7A5DzdCZmBTU5XwYkUpMcQunrA6HpHn0LH0XOilBZ7gRK6XwjyRkOcu4Coip1Q06lnzh1II8uiNIG7DLsdCBK7VjZp2zeQFENZCFnb51P51rAjgHJLrdIoVJT5Jy5VbvxA2oObldCuKmLgV2NGgKNHi2xkXqgpagRfS3n/Xp0G0h2OJvEUA/WLophPigTNqAlAL1Iqz3QwsY8H9NA+0BnO0dZzEs4G/wY6vxOUmHH0brVABZFHFBA+yXfZBJG193UCHQzPo6WQHUAVDhgJbL3erWHdz3FppzWaIQuynsNJEtHN8nUd4T7/TQFsXWdbzKnuwTSvdWYeTjw0ObeOG/XrXZDDyOAZHaC7TyUN4a36xVRDJzI6WU7gEmI6Tez3h/wuhbRlydicuDB1rarF00xyH3u5NDTqHpHc/oc5aWHjYwK5SmUkddWvC/jdIMCg5xmUsVSi5AvY8FLB1FTqkjfTomDFUYU63VdqaXk4kXO5UQNVp7lvzGj/VB62COMzWNnHYQipz27i1YebWt2q4fVMp2cvwh3cvQxYjh7VEtlwxZR6WFqKKg27kQOOYMNdAm7oaqvobYzsiLwJUfNCrupck+Uzyr+hOSIfqKSlb24QRVJDjEnH1wzgLqWYuT0Y0ENmWmqbJFjTv6HFycWORfi8tscbkozuEWORcsw75yIYEb5DyxRBCY5v4jdSAMULtd1rEYP4gE5Fys5sdO9IKsb9cSJvhxu5hvXC0i0+qSG6LfSGs3ljStVlGTTUhWTNUdZrfC8JEbpHKpfpskJS5Kc8ft1hObZUEVJPjKDJNMu8kkVd0sOvCNS1Zt+NRF16ZKiylNCdbhsGTSuTo/xFTkO+j276+7Abe3U90Isxt9sS+7GQ7hnjnU6MO93OJLjd2mZyEq38s0d17CNrHQz39x1sVzPSrJxKb7lqryq2vIpH/2upDDfDr6mCl064N/3vwzq+rAlS+gI0D98ZiOeShKtFQAAAABJRU5ErkJggg=="

// Общая шапка (лого) и навигация — вставляется во все защищённые страницы.
#define PAGE_HEADER R"navblk(<header class="brand">
<img class="brand-logo" src=")navblk" LOGO_DATA_URI R"navblk(" alt="MLSystems" />
<div class="brand-text"><span class="brand-name">MLSystems</span><span class="brand-tagline">Панель управления</span></div>
</header>
<nav class="topnav">
<a href="/info" class="navlink" data-path="/info"><svg class="navicon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="10"></circle><line x1="12" y1="16" x2="12" y2="12"></line><line x1="12" y1="8" x2="12.01" y2="8"></line></svg><span>Инфо</span></a>
<a href="/connection" class="navlink" data-path="/connection"><svg class="navicon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M9 2v6M15 2v6"></path><path d="M6 8h12v4a6 6 0 0 1-6 6 6 6 0 0 1-6-6V8Z"></path><path d="M12 18v4"></path></svg><span>Подключение</span></a>
<a href="/settings" class="navlink" data-path="/settings"><svg class="navicon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M14.7 6.3a1 1 0 0 0 0 1.4l1.6 1.6a1 1 0 0 0 1.4 0l3.77-3.77a6 6 0 0 1-7.94 7.94l-6.91 6.91a2.12 2.12 0 0 1-3-3l6.91-6.91a6 6 0 0 1 7.94-7.94l-3.76 3.76z"></path></svg><span>Настройки</span></a>
<a href="/wifi" class="navlink" data-path="/wifi"><svg class="navicon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M2 8.5a16 16 0 0 1 20 0"></path><path d="M5 12a11 11 0 0 1 14 0"></path><path d="M8.5 15.5a6 6 0 0 1 7 0"></path><circle cx="12" cy="19" r="1.3" fill="currentColor" stroke="none"></circle></svg><span>Wi-Fi сеть</span></a>
<a href="/update" class="navlink" data-path="/update"><svg class="navicon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M12 4v14"></path><path d="M6 12l6 6 6-6"></path><path d="M4 20h16"></path></svg><span>Прошивка</span></a>
<a href="/access" class="navlink" data-path="/access"><svg class="navicon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><rect x="5" y="11" width="14" height="9" rx="2"></rect><path d="M8 11V7a4 4 0 0 1 8 0v4"></path></svg><span>Доступ</span></a>
<a href="/io" class="navlink" data-path="/io"><svg class="navicon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M13 2 4 14h6l-1 8 9-12h-6l1-8Z"></path></svg><span>Тест</span></a>
<a href="/logout" class="navlink" data-path="/logout"><svg class="navicon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M9 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h4"></path><path d="M16 17l5-5-5-5"></path><path d="M21 12H9"></path></svg><span>Выход</span></a>
</nav>
)navblk"

static const char LOGIN_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>Вход</title>
  <link rel="stylesheet" href="/style.css" />
</head>
<body>
  <main class="login-page">
    <div class="login-box">
      <div class="brand brand-centered">
        <img class="brand-logo" src=")rawliteral" LOGO_DATA_URI R"rawliteral(" alt="MLSystems" />
        <span class="brand-name">MLSystems</span>
      </div>

      <form class="panel login-form" method="POST" action="/login">
        <h1>Вход</h1>
        <label>Логин</label>
        <input type="text" name="username" autocomplete="username" required />
        <label>Пароль</label>
        <input type="password" name="password" autocomplete="current-password" required />
        <button type="submit">Войти</button>
      </form>
    </div>
  </main>
</body>
</html>
)rawliteral";

static const char INFO_HTML[] PROGMEM = R"page(
<!DOCTYPE html>
<html lang="ru">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>Инфо — MLSystems</title>
  <link rel="stylesheet" href="/style.css" />
</head>
<body>
)page" PAGE_HEADER R"page(
  <main>
    <section class="panel">
      <h2>Наработка</h2>
      <p id="runtimeText">Загрузка...</p>
    </section>

    <section class="panel">
      <h2>Циклы</h2>
      <p>Полных циклов: <span id="fullCycles">—</span></p>
      <p>Неполных циклов: <span id="partialCycles">—</span></p>
    </section>

    <section class="panel">
      <p>Версия прошивки: <span id="fwVer"></span></p>
    </section>
  </main>

  <script src="/script.js"></script>
</body>
</html>
)page";

static const char CONNECTION_HTML[] PROGMEM = R"page(
<!DOCTYPE html>
<html lang="ru">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>Подключение — MLSystems</title>
  <link rel="stylesheet" href="/style.css" />
</head>
<body>
)page" PAGE_HEADER R"page(
  <main>
    <section class="panel">
      <h2>Входы</h2>
      <table class="wiring-table">
        <thead><tr><th>Вход</th><th>Назначение</th></tr></thead>
        <tbody>
          <tr><td><span class="wiring-badge">DI1</span></td><td>Аварийная остановка</td></tr>
          <tr><td><span class="wiring-badge">DI2</span></td><td>Кнопка старт/стоп</td></tr>
          <tr><td><span class="wiring-badge">DI3</span></td><td>Кнопка вкл/выкл светомузыка</td></tr>
          <tr><td><span class="wiring-badge">DI4</span></td><td>Кнопка вкл/выкл хромотерапии</td></tr>
          <tr><td><span class="wiring-badge">DI5</span></td><td>Кнопка вкл/выкл нагрев</td></tr>
          <tr><td><span class="wiring-badge">DI6</span></td><td>Кнопка понижения потока воздуха</td></tr>
          <tr><td><span class="wiring-badge">DI7</span></td><td>Датчик присутствия</td></tr>
          <tr><td><span class="wiring-badge">DI8</span></td><td>Не используется</td></tr>
        </tbody>
      </table>
    </section>

    <section class="panel">
      <h2>Выходы</h2>
      <table class="wiring-table">
        <thead><tr><th>Канал</th><th>Назначение</th></tr></thead>
        <tbody>
          <tr><td><span class="wiring-badge">CH1</span></td><td>Управление ароматизацией</td></tr>
          <tr><td><span class="wiring-badge">CH2</span></td><td>Подсветка кнопки старт/стоп</td></tr>
          <tr><td><span class="wiring-badge">CH3</span></td><td>Подсветка кнопки вкл/выкл светомузыка</td></tr>
          <tr><td><span class="wiring-badge">CH4</span></td><td>Подсветка кнопки вкл/выкл хромотерапии</td></tr>
          <tr><td><span class="wiring-badge">CH5</span></td><td>Подсветка кнопки вкл/выкл нагрев</td></tr>
          <tr><td><span class="wiring-badge">CH6</span></td><td>Управление светомузыкой</td></tr>
          <tr><td><span class="wiring-badge">CH7</span></td><td>Управление хромотерапией</td></tr>
          <tr><td><span class="wiring-badge">CH8</span></td><td>Управление нагревом</td></tr>
        </tbody>
      </table>
    </section>

    <section class="panel">
      <h2>Аналоговый выход</h2>
      <table class="wiring-table">
        <thead><tr><th>Выход</th><th>Назначение</th></tr></thead>
        <tbody>
          <tr><td><span class="wiring-badge">AO1</span></td><td>Управление скоростью потока воздуха</td></tr>
        </tbody>
      </table>
    </section>
  </main>

  <script src="/script.js"></script>
</body>
</html>
)page";

static const char SETTINGS_HTML[] PROGMEM = R"page(
<!DOCTYPE html>
<html lang="ru">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>Настройки — MLSystems</title>
  <link rel="stylesheet" href="/style.css" />
</head>
<body>
)page" PAGE_HEADER R"page(
  <main>
    <section class="panel">
      <h2>Настройки алгоритма</h2>
      <form id="algorithmForm" class="settings-form">
        <div class="settings-actions">
          <button type="button" id="algorithmResetBtn" class="btn-secondary">Сбросить</button>
          <button type="submit">Сохранить</button>
        </div>

        <div class="settings-table">
          <div class="settings-label"><label for="di2Cycle">Длина цикла, сек (60-300)</label></div>
          <div class="settings-control"><input type="number" id="di2Cycle" name="di2_cycle" class="input-narrow" min="60" max="300" step="1" required /></div>

          <div class="settings-label"><label for="ch1Imp">Ароматизация, длительность импульса, сек (0-30)</label></div>
          <div class="settings-control"><input type="number" id="ch1Imp" name="ch1_imp" class="input-narrow" min="0" max="30" step="1" required /></div>

          <div class="settings-label"><label for="ao1Max">Ограничение вентилятора, % (1-100)</label></div>
          <div class="settings-control"><input type="number" id="ao1Max" name="ao1_max" class="input-narrow" min="1" max="100" step="1" required /></div>

          <div class="settings-label"><label for="ao1F1Time">1 фаза обдува, длительность, сек (1-30)</label></div>
          <div class="settings-control"><input type="number" id="ao1F1Time" name="ao1_f1_time" class="input-narrow" min="1" max="30" step="1" required /></div>

          <div class="settings-label"><label for="ao1F1Pwr">1 фаза обдува, мощность, % (1-100)</label></div>
          <div class="settings-control"><input type="number" id="ao1F1Pwr" name="ao1_f1_pwr" class="input-narrow" min="1" max="100" step="1" required /></div>

          <div class="settings-label"><label for="ao1F2Time">2 фаза обдува, длительность, сек (1-30)</label></div>
          <div class="settings-control"><input type="number" id="ao1F2Time" name="ao1_f2_time" class="input-narrow" min="1" max="30" step="1" required /></div>

          <div class="settings-label"><label for="ao1F2Pwr">2 фаза обдува, мощность, % (1-100)</label></div>
          <div class="settings-control"><input type="number" id="ao1F2Pwr" name="ao1_f2_pwr" class="input-narrow" min="1" max="100" step="1" required /></div>

          <div class="settings-label"><label for="ao1F3Time">3 фаза обдува, длительность, сек (1-30)</label></div>
          <div class="settings-control"><input type="number" id="ao1F3Time" name="ao1_f3_time" class="input-narrow" min="1" max="30" step="1" required /></div>

          <div class="settings-label"><label for="ao1F3Pwr">3 фаза обдува, мощность, % (1-100)</label></div>
          <div class="settings-control"><input type="number" id="ao1F3Pwr" name="ao1_f3_pwr" class="input-narrow" min="1" max="100" step="1" required /></div>

          <div class="settings-label"><label for="ao1DelayOff">Остановка обдува, сек (1-30)</label></div>
          <div class="settings-control"><input type="number" id="ao1DelayOff" name="ao1_delay_off" class="input-narrow" min="1" max="30" step="1" required /></div>

          <div class="settings-label"><label for="ch6Inp">Длительность импульса управления подсветкой, мсек (200-2000)</label></div>
          <div class="settings-control"><input type="number" id="ch6Inp" name="ch6_inp" class="input-narrow" min="200" max="2000" step="1" required /></div>

          <div class="settings-label"><label for="ch6Delay">Длительность паузы управления подсветкой, мсек (200-2000)</label></div>
          <div class="settings-control"><input type="number" id="ch6Delay" name="ch6_delay" class="input-narrow" min="200" max="2000" step="1" required /></div>

          <div class="settings-label"><label for="di7On">Датчик присутствия</label></div>
          <div class="settings-control"><input type="checkbox" id="di7On" name="di7_on" /></div>

          <div class="settings-label"><label for="presenceOffDelaySec">Задержка отключения по присутствию, сек (1-300)</label></div>
          <div class="settings-control"><input type="number" id="presenceOffDelaySec" name="presence_off_delay_sec" class="input-narrow" min="1" max="300" step="1" required /></div>
        </div>
      </form>
      <p id="algorithmMsg"></p>
    </section>
  </main>

  <script src="/script.js"></script>
</body>
</html>
)page";

static const char WIFI_HTML[] PROGMEM = R"page(
<!DOCTYPE html>
<html lang="ru">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>Wi-Fi сеть — MLSystems</title>
  <link rel="stylesheet" href="/style.css" />
</head>
<body>
)page" PAGE_HEADER R"page(
  <main>
    <section class="panel">
      <h2>Подключение к Wi-Fi</h2>
      <p id="wifiStatus">Загрузка статуса...</p>
      <button id="forgetBtn" hidden>Забыть сеть</button>
      <button id="scanBtn">Найти сети</button>
      <div id="networksList" class="wifi-list"></div>
    </section>
  </main>

  <script src="/script.js"></script>
</body>
</html>
)page";

static const char UPDATE_HTML[] PROGMEM = R"page(
<!DOCTYPE html>
<html lang="ru">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>Обновление прошивки — MLSystems</title>
  <link rel="stylesheet" href="/style.css" />
</head>
<body>
)page" PAGE_HEADER R"page(
  <main>
    <section class="panel">
      <h2>Обновление прошивки</h2>

      <div class="update-block">
        <h3>Загрузить файл с телефона/компьютера</h3>
        <input type="file" id="fwFile" accept=".bin" />
        <button id="fwUploadBtn">Обновить</button>
        <div class="progress"><div class="progress-bar" id="fwUploadBar"></div></div>
        <p id="fwUploadPercent"></p>
      </div>

      <div class="update-block">
        <h3>Обновить с сервера</h3>
        <button id="fwServerBtn">Скачать и установить</button>
        <div class="progress"><div class="progress-bar" id="fwServerBar"></div></div>
        <p id="fwServerPercent"></p>
      </div>
    </section>
  </main>

  <script src="/script.js"></script>
</body>
</html>
)page";

static const char ACCESS_HTML[] PROGMEM = R"page(
<!DOCTYPE html>
<html lang="ru">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>Доступ — MLSystems</title>
  <link rel="stylesheet" href="/style.css" />
</head>
<body>
)page" PAGE_HEADER R"page(
  <main>
    <section class="panel">
      <h2>Точка доступа Wi-Fi</h2>
      <form id="wifiForm" class="settings-form">
        <label>SSID точки доступа</label>
        <input type="text" name="ap_ssid" placeholder="оставить как есть" />
        <label>Пароль точки доступа</label>
        <input type="password" name="ap_password" placeholder="оставить как есть" />
        <button type="submit">Сохранить Wi-Fi</button>
      </form>
    </section>

    <section class="panel">
      <h2>Логин и пароль входа</h2>
      <form id="authForm" class="settings-form">
        <label>Логин</label>
        <input type="text" name="auth_user" placeholder="оставить как есть" />
        <label>Новый пароль</label>
        <input type="password" name="auth_password" placeholder="оставить как есть" />
        <button type="submit">Сохранить логин/пароль</button>
      </form>
      <p id="settingsMsg"></p>
    </section>
  </main>

  <script src="/script.js"></script>
</body>
</html>
)page";

static const char IO_HTML[] PROGMEM = R"page(
<!DOCTYPE html>
<html lang="ru">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0" />
  <title>Тест входов/выходов — MLSystems</title>
  <link rel="stylesheet" href="/style.css" />
</head>
<body>
)page" PAGE_HEADER R"page(
  <main>
    <section class="panel">
      <h2>Дискретные входы</h2>
      <div id="inputs" class="grid"></div>
    </section>

    <section class="panel">
      <h2>Дискретные выходы</h2>
      <div id="relays" class="grid"></div>
    </section>

    <section class="panel">
      <h2>Аналоговый выход</h2>
      <div class="analog-control">
        <div class="analog-label">
          <span>AO1</span>
          <span id="analogValue">0</span>
        </div>
        <input type="range" id="analogSlider" min="0" max="255" step="1" value="0" />
      </div>
    </section>
  </main>

  <script src="/script.js"></script>
</body>
</html>
)page";

static const char STYLE_CSS[] PROGMEM = R"rawliteral(
:root {
  --bg: #101418;
  --panel: #181d24;
  --border: #2a313b;
  --text: #e6e9ed;
  --accent: #3b82f6;
  --on: #22c55e;
  --off: #4b5563;
}

* {
  box-sizing: border-box;
}

body {
  margin: 0;
  font-family: system-ui, sans-serif;
  background: var(--bg);
  color: var(--text);
}

header {
  padding: 16px 24px;
  border-bottom: 1px solid var(--border);
}

.brand {
  display: flex;
  align-items: center;
  gap: 12px;
}

.brand-logo {
  display: block;
  width: 40px;
  height: auto;
}

.brand-text {
  display: flex;
  flex-direction: column;
  line-height: 1.25;
}

.brand-name {
  font-size: 18px;
  font-weight: 700;
  letter-spacing: 0.3px;
}

.brand-tagline {
  font-size: 12px;
  opacity: 0.65;
}

.topnav {
  display: flex;
  flex-wrap: wrap;
  gap: 4px;
  padding: 8px 16px;
  background: var(--panel);
  border-bottom: 1px solid var(--border);
}

.navlink {
  display: flex;
  align-items: center;
  gap: 6px;
  padding: 8px 12px;
  border-radius: 8px;
  color: var(--text);
  text-decoration: none;
  font-size: 13px;
  opacity: 0.7;
}

.navlink:hover {
  opacity: 1;
  background: var(--bg);
}

.navlink.active {
  opacity: 1;
  color: var(--accent);
  background: var(--bg);
}

.navicon {
  width: 18px;
  height: 18px;
  flex-shrink: 0;
}

main {
  padding: 24px;
  display: flex;
  flex-direction: column;
  gap: 24px;
}

.panel {
  background: var(--panel);
  border: 1px solid var(--border);
  border-radius: 12px;
  padding: 16px 20px;
}

.grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(110px, 1fr));
  gap: 12px;
  margin-top: 12px;
}

.cell {
  border: 1px solid var(--border);
  border-radius: 10px;
  padding: 12px;
  text-align: center;
}

.cell .label {
  font-size: 13px;
  opacity: 0.7;
}

.cell .state {
  margin-top: 8px;
  font-weight: 600;
}

.state.on {
  color: var(--on);
}

.state.off {
  color: var(--off);
}

.analog-control {
  margin-top: 12px;
  max-width: 320px;
}

.analog-label {
  display: flex;
  justify-content: space-between;
  font-size: 14px;
  margin-bottom: 8px;
}

.analog-label span:last-child {
  font-weight: 600;
  color: var(--accent);
  font-variant-numeric: tabular-nums;
}

input[type="range"] {
  width: 100%;
  accent-color: var(--accent);
}

.wiring-table {
  width: 100%;
  border-collapse: collapse;
  margin-top: 12px;
}

.wiring-table th,
.wiring-table td {
  text-align: left;
  padding: 10px 12px;
  border-bottom: 1px solid var(--border);
}

.wiring-table th {
  font-size: 12px;
  text-transform: uppercase;
  letter-spacing: 0.04em;
  opacity: 0.6;
  font-weight: 600;
}

.wiring-table tbody tr:last-child td {
  border-bottom: none;
}

.wiring-badge {
  display: inline-block;
  padding: 3px 10px;
  border-radius: 999px;
  background: rgba(59, 130, 246, 0.15);
  color: var(--accent);
  font-weight: 600;
  font-size: 13px;
  font-variant-numeric: tabular-nums;
  white-space: nowrap;
}

button.relay-btn {
  margin-top: 10px;
  width: 100%;
  padding: 6px 0;
  border: none;
  border-radius: 8px;
  background: var(--accent);
  color: #fff;
  cursor: pointer;
}

.login-page {
  min-height: 100vh;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 24px;
}

.login-box {
  width: 100%;
  max-width: 320px;
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 20px;
}

.brand-centered {
  flex-direction: column;
  gap: 8px;
}

.brand-centered .brand-logo {
  width: 64px;
}

.brand-centered .brand-name {
  font-size: 20px;
}

.login-form {
  width: 100%;
  display: flex;
  flex-direction: column;
  gap: 10px;
}

.settings-form {
  display: flex;
  flex-direction: column;
  gap: 8px;
  margin-bottom: 16px;
}

.login-form input,
.settings-form input {
  padding: 8px 10px;
  border-radius: 8px;
  border: 1px solid var(--border);
  background: var(--bg);
  color: var(--text);
}

.login-form button,
.settings-form button {
  margin-top: 6px;
  padding: 8px 0;
  border: none;
  border-radius: 8px;
  background: var(--accent);
  color: #fff;
  cursor: pointer;
}

.settings-actions {
  display: flex;
  gap: 10px;
  margin-bottom: 4px;
}

.settings-actions button {
  flex: 1;
  margin-top: 0;
  padding: 8px 16px;
}

.settings-actions .btn-secondary {
  background: transparent;
  border: 1px solid var(--border);
  color: var(--text);
}

.settings-table {
  display: grid;
  grid-template-columns: 1fr auto;
}

.settings-table .settings-label,
.settings-table .settings-control {
  display: flex;
  align-items: center;
  padding: 10px 0;
  border-bottom: 1px solid var(--border);
}

.settings-table .settings-label {
  padding-right: 12px;
}

.settings-table .settings-control {
  padding-left: 12px;
  border-left: 1px solid var(--border);
  justify-content: center;
}

.settings-table .settings-label:nth-last-child(2),
.settings-table > .settings-control:last-child {
  border-bottom: none;
}

.settings-table label {
  margin: 0;
}

.settings-table input[type="checkbox"] {
  width: 18px;
  height: 18px;
  margin: 0;
  accent-color: var(--accent);
}

.settings-table input[type="number"].input-narrow {
  width: 4.5em;
  flex: none;
  padding: 6px 8px;
  text-align: center;
}

.update-block {
  padding: 14px 0;
  border-bottom: 1px solid var(--border);
}

.update-block:last-child {
  border-bottom: none;
}

.update-block h3 {
  margin: 0 0 10px;
  font-size: 15px;
}

.update-block input[type="file"] {
  display: block;
  margin-bottom: 10px;
  font-size: 13px;
  color: var(--text);
}

#scanBtn,
#fwUploadBtn,
#fwServerBtn,
#forgetBtn {
  display: block;
  margin-bottom: 10px;
  padding: 8px 16px;
  border: none;
  border-radius: 8px;
  background: var(--accent);
  color: #fff;
  cursor: pointer;
}

#forgetBtn[hidden] {
  display: none;
}

.progress {
  margin-top: 10px;
  height: 10px;
  border-radius: 6px;
  background: var(--bg);
  border: 1px solid var(--border);
  overflow: hidden;
}

.progress-bar {
  height: 100%;
  width: 0%;
  background: var(--accent);
  transition: width 0.2s ease;
}

.wifi-list {
  display: flex;
  flex-direction: column;
  gap: 8px;
  margin-top: 12px;
}

.wifi-net {
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 10px;
  padding: 10px 12px;
  border: 1px solid var(--border);
  border-radius: 10px;
  flex-wrap: wrap;
}

.wifi-net-info {
  display: flex;
  align-items: center;
  gap: 8px;
  min-width: 0;
}

.wifi-net-name {
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
  max-width: 180px;
}

.wifi-connect-form {
  display: flex;
  gap: 6px;
  align-items: center;
}

.wifi-connect-form input {
  padding: 6px 8px;
  border-radius: 6px;
  border: 1px solid var(--border);
  background: var(--bg);
  color: var(--text);
  width: 120px;
}

.wifi-connect-form button {
  padding: 6px 14px;
  border: none;
  border-radius: 8px;
  background: var(--accent);
  color: #fff;
  cursor: pointer;
}
)rawliteral";

static const char SCRIPT_JS[] PROGMEM = R"rawliteral(
document.querySelectorAll('.navlink').forEach((a) => {
  if (a.dataset.path === window.location.pathname) {
    a.classList.add('active');
  }
});

const fwVerEl = document.getElementById('fwVer');
if (fwVerEl) {
  fetch('/api/version')
    .then((res) => res.json())
    .then((v) => {
      let text = v.major + '.' + v.minor;
      if (v.patch !== 0) text += '.' + v.patch; // последний 0 не показываем
      fwVerEl.textContent = text;
    });
}

const runtimeTextEl = document.getElementById('runtimeText');
if (runtimeTextEl) {
  fetch('/api/runtime')
    .then((res) => res.json())
    .then((d) => {
      const hours = Math.floor(d.minutes / 60);
      const mins = d.minutes % 60;
      runtimeTextEl.textContent = hours + ' часов ' + mins + ' минут';
      document.getElementById('fullCycles').textContent = d.fullCycles;
      document.getElementById('partialCycles').textContent = d.partialCycles;
    });
}

const inputsEl = document.getElementById('inputs');
const relaysEl = document.getElementById('relays');
const analogSlider = document.getElementById('analogSlider');
const analogValueEl = document.getElementById('analogValue');

let status = { inputs: [], relays: [], analog: 0 };

function renderInputs(states) {
  inputsEl.innerHTML = '';
  states.forEach((state, i) => {
    const cell = document.createElement('div');
    cell.className = 'cell';
    cell.innerHTML = `
      <div class="label">DI${i + 1}</div>
      <div class="state ${state ? 'on' : 'off'}">${state ? 'ВКЛ' : 'ВЫКЛ'}</div>
    `;
    inputsEl.appendChild(cell);
  });
}

function renderRelays(states) {
  relaysEl.innerHTML = '';
  states.forEach((state, i) => {
    const cell = document.createElement('div');
    cell.className = 'cell';
    cell.innerHTML = `
      <div class="label">CH${i + 1}</div>
      <div class="state ${state ? 'on' : 'off'}">${state ? 'ВКЛ' : 'ВЫКЛ'}</div>
      <button class="relay-btn" data-index="${i}">Переключить</button>
    `;
    relaysEl.appendChild(cell);
  });

  relaysEl.querySelectorAll('.relay-btn').forEach((btn) => {
    btn.addEventListener('click', () => {
      const index = Number(btn.dataset.index);
      setRelay(index, !status.relays[index]);
    });
  });
}

function renderAnalog(value) {
  if (!analogSlider) return;
  analogValueEl.textContent = value;
  if (document.activeElement !== analogSlider) {
    analogSlider.value = value;
  }
}

async function fetchStatus() {
  const res = await fetch('/api/status');
  if (res.status === 401) {
    window.location.href = '/login';
    return;
  }
  status = await res.json();
  renderInputs(status.inputs);
  renderRelays(status.relays);
  renderAnalog(status.analog);
}

async function setRelay(index, state) {
  await fetch('/api/relay', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ index, state }),
  });
  fetchStatus();
}

async function setAnalog(value) {
  await fetch('/api/analog', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ value }),
  });
}

if (analogSlider) {
  analogSlider.addEventListener('input', () => {
    analogValueEl.textContent = analogSlider.value;
  });
  analogSlider.addEventListener('change', () => {
    setAnalog(Number(analogSlider.value));
  });
}

if (inputsEl && relaysEl) {
  fetchStatus();
  setInterval(fetchStatus, 2000);
}

async function submitSettings(form) {
  const params = new URLSearchParams();
  new FormData(form).forEach((value, key) => {
    if (value) params.append(key, value);
  });

  const res = await fetch('/api/settings', {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: params.toString(),
  });

  const msg = document.getElementById('settingsMsg');
  msg.textContent = res.ok ? 'Сохранено, перезагрузка...' : 'Ошибка сохранения';
}

const wifiForm = document.getElementById('wifiForm');
const authForm = document.getElementById('authForm');
if (wifiForm) wifiForm.addEventListener('submit', (e) => { e.preventDefault(); submitSettings(wifiForm); });
if (authForm) authForm.addEventListener('submit', (e) => { e.preventDefault(); submitSettings(authForm); });

const algorithmForm = document.getElementById('algorithmForm');
const di7OnCheckbox = document.getElementById('di7On');
const algorithmResetBtn = document.getElementById('algorithmResetBtn');
const algorithmMsg = document.getElementById('algorithmMsg');

// Числовые поля алгоритма: id инпута на странице <-> ключ в JSON API <-> имя form-поля
// при сохранении. Новые числовые настройки достаточно дописать сюда одной строкой.
const algorithmNumberFields = [
  ['presenceOffDelaySec', 'presenceOffDelaySec', 'presence_off_delay_sec'],
  ['di2Cycle', 'di2Cycle', 'di2_cycle'],
  ['ch1Imp', 'ch1Imp', 'ch1_imp'],
  ['ao1Max', 'ao1Max', 'ao1_max'],
  ['ao1F1Time', 'ao1F1Time', 'ao1_f1_time'],
  ['ao1F1Pwr', 'ao1F1Pwr', 'ao1_f1_pwr'],
  ['ao1F2Time', 'ao1F2Time', 'ao1_f2_time'],
  ['ao1F2Pwr', 'ao1F2Pwr', 'ao1_f2_pwr'],
  ['ao1F3Time', 'ao1F3Time', 'ao1_f3_time'],
  ['ao1F3Pwr', 'ao1F3Pwr', 'ao1_f3_pwr'],
  ['ao1DelayOff', 'ao1DelayOff', 'ao1_delay_off'],
  ['ch6Inp', 'ch6Inp', 'ch6_inp'],
  ['ch6Delay', 'ch6Delay', 'ch6_delay'],
];

async function loadAlgorithmSettings() {
  const res = await fetch('/api/settings/algorithm');
  const data = await res.json();
  if (di7OnCheckbox) di7OnCheckbox.checked = data.di7On;
  for (const [elId, jsonKey] of algorithmNumberFields) {
    const el = document.getElementById(elId);
    if (el) el.value = data[jsonKey];
  }
}

if (algorithmForm) {
  loadAlgorithmSettings();

  algorithmForm.addEventListener('submit', async (e) => {
    e.preventDefault();
    const params = new URLSearchParams();
    params.append('di7_on', di7OnCheckbox.checked ? '1' : '0');
    for (const [elId, , formField] of algorithmNumberFields) {
      const el = document.getElementById(elId);
      if (el) params.append(formField, el.value);
    }
    await fetch('/api/settings/algorithm', {
      method: 'POST',
      headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
      body: params.toString(),
    });
    algorithmMsg.textContent = 'Сохранено';
  });

  if (algorithmResetBtn) {
    algorithmResetBtn.addEventListener('click', () => {
      loadAlgorithmSettings();
      algorithmMsg.textContent = 'Сброшено';
    });
  }
}

const scanBtn = document.getElementById('scanBtn');
const networksList = document.getElementById('networksList');
const wifiStatusEl = document.getElementById('wifiStatus');
const forgetBtn = document.getElementById('forgetBtn');

async function refreshWifiStatus() {
  const res = await fetch('/api/wifi/status');
  const data = await res.json();
  wifiStatusEl.textContent = data.connected
    ? `Подключено к «${data.ssid}», IP: ${data.ip}`
    : 'Не подключено к внешней сети';

  if (forgetBtn) {
    forgetBtn.hidden = !data.connected;
    forgetBtn.textContent = data.connected ? `Забыть сеть «${data.ssid}»` : 'Забыть сеть';
  }
  return data;
}

if (forgetBtn) {
  forgetBtn.addEventListener('click', async () => {
    forgetBtn.disabled = true;
    await fetch('/api/wifi/forget', { method: 'POST' });
    forgetBtn.disabled = false;
    refreshWifiStatus();
  });
}

function renderNetworkRow(net) {
  const row = document.createElement('div');
  row.className = 'wifi-net';

  const info = document.createElement('div');
  info.className = 'wifi-net-info';

  const name = document.createElement('span');
  name.className = 'wifi-net-name';
  name.textContent = net.ssid || '(скрытая сеть)';
  info.appendChild(name);

  if (net.secure) {
    const lock = document.createElement('span');
    lock.textContent = '🔒';
    info.appendChild(lock);
  }

  row.appendChild(info);

  const form = document.createElement('form');
  form.className = 'wifi-connect-form';

  let pwd = null;
  if (net.secure) {
    pwd = document.createElement('input');
    pwd.type = 'password';
    pwd.placeholder = 'Пароль';
    form.appendChild(pwd);
  }

  const btn = document.createElement('button');
  btn.type = 'submit';
  btn.textContent = 'Подключить';
  form.appendChild(btn);

  form.addEventListener('submit', (e) => {
    e.preventDefault();
    connectToNetwork(net.ssid, pwd ? pwd.value : '');
  });

  row.appendChild(form);
  return row;
}

async function connectToNetwork(ssid, password) {
  const params = new URLSearchParams();
  params.append('ssid', ssid);
  params.append('password', password);
  await fetch('/api/wifi/connect', {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: params.toString(),
  });
  wifiStatusEl.textContent = 'Подключение...';
  if (forgetBtn) forgetBtn.hidden = true;

  let attempts = 0;
  const poll = setInterval(async () => {
    attempts++;
    const data = await refreshWifiStatus();
    if (data.connected || attempts >= 8) {
      clearInterval(poll);
      if (!data.connected) {
        wifiStatusEl.textContent = 'Не удалось подключиться';
      }
    }
  }, 2000);
}

if (scanBtn) {
  scanBtn.addEventListener('click', async () => {
    scanBtn.disabled = true;
    scanBtn.textContent = 'Ищу сети...';
    const res = await fetch('/api/wifi/scan');
    const networks = await res.json();
    networksList.innerHTML = '';
    networks
      .slice()
      .sort((a, b) => b.rssi - a.rssi)
      .forEach((net) => networksList.appendChild(renderNetworkRow(net)));
    scanBtn.disabled = false;
    scanBtn.textContent = 'Найти сети';
  });
}

if (wifiStatusEl) {
  refreshWifiStatus();
}

const fwFile = document.getElementById('fwFile');
const fwUploadBtn = document.getElementById('fwUploadBtn');
const fwUploadBar = document.getElementById('fwUploadBar');
const fwUploadPercent = document.getElementById('fwUploadPercent');

if (fwUploadBtn) {
  fwUploadBtn.addEventListener('click', () => {
    const file = fwFile.files[0];
    if (!file) {
      fwUploadPercent.textContent = 'Сначала выберите файл';
      return;
    }

    const xhr = new XMLHttpRequest();
    xhr.open('POST', '/api/update');
    xhr.upload.onprogress = (e) => {
      if (e.lengthComputable) {
        const percent = Math.round((e.loaded / e.total) * 100);
        fwUploadBar.style.width = percent + '%';
        fwUploadPercent.textContent = percent + '%';
      }
    };
    xhr.onload = () => {
      fwUploadPercent.textContent = xhr.status === 200
        ? 'Готово, перезагрузка...'
        : 'Ошибка обновления';
    };
    xhr.onerror = () => {
      fwUploadPercent.textContent = 'Ошибка соединения';
    };
    fwUploadBtn.disabled = true;
    xhr.send(file);
  });
}

const fwServerBtn = document.getElementById('fwServerBtn');
const fwServerBar = document.getElementById('fwServerBar');
const fwServerPercent = document.getElementById('fwServerPercent');

if (fwServerBtn) {
  let pollTimer = null;

  async function pollServerProgress() {
    const res = await fetch('/api/update/progress');
    const data = await res.json();
    if (data.percent === -2) {
      fwServerPercent.textContent = 'Ошибка обновления';
      clearInterval(pollTimer);
      fwServerBtn.disabled = false;
      return;
    }
    const percent = Math.max(0, data.percent);
    fwServerBar.style.width = percent + '%';
    fwServerPercent.textContent = percent + '%';
    if (percent >= 100) {
      fwServerPercent.textContent = 'Готово, перезагрузка...';
      clearInterval(pollTimer);
    }
  }

  fwServerBtn.addEventListener('click', async () => {
    fwServerBtn.disabled = true;
    fwServerPercent.textContent = 'Запуск...';
    await fetch('/api/update/server', { method: 'POST' });
    pollTimer = setInterval(pollServerProgress, 1000);
  });
}
)rawliteral";

void Web_Init();
void Web_Loop();
void Web_OtaBegin();
