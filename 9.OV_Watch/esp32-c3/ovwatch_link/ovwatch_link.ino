/*
 * OV-Watch + ESP32-C3 阶段二固件：WiFi/NTP 自动校时
 * -------------------------------------------------
 * 硬件接线（单向）：
 *   ESP32 GPIO21 (TX) -> H1 Pin13 (D5/PA3/USART2_RX)
 *   ESP32 GND         -> H1 Pin2 (GND)
 *   GPIO20 悬空；H1 Pin12 是 PA0/KEY，禁止接线。
 *
 * NTP 成功后校准本地软时钟，每 5 秒向手表发送 TIME,h,m,s。
 * WiFi 断开时软时钟继续运行，后台自动重连并重新校准。
 */

#include <HardwareSerial.h>
#include <WiFi.h>
#include <time.h>

#define STM_TX_PIN             21
#define SYNC_PERIOD_MS         5000UL
#define WIFI_RETRY_MS          30000UL
#define NTP_RETRY_MS           5000UL
#define NTP_RESYNC_MS          3600000UL
#define LED_PIN                8

static const char WIFI_SSID[] = "tytxdy";
static const char WIFI_PASSWORD[] = "lty20120712";
static const char NTP_SERVER_1[] = "ntp.aliyun.com";
static const char NTP_SERVER_2[] = "pool.ntp.org";
static const long UTC_OFFSET_SECONDS = 8L * 3600L;

HardwareSerial StmSerial(1);

/* 本地软时钟 */
static uint32_t clockSeconds = 14UL * 3600UL + 35UL * 60UL + 20UL; /* 开机默认 */
static uint32_t clockAnchorMs = 0;

static uint32_t lastWifiAttemptMs = 0;
static uint32_t lastNtpAttemptMs = 0;
static uint32_t lastNtpSyncMs = 0;
static bool wifiWasConnected = false;

static uint32_t elapsedSeconds(void)
{
  return (millis() - clockAnchorMs) / 1000UL;
}

static void applyClock(uint32_t seconds)
{
  clockSeconds = seconds % 86400UL;
  clockAnchorMs = millis();
}

static void stmSendLine(const char *line)
{
  StmSerial.print(line);
  StmSerial.print('\n');
  Serial.print("[TX] ");
  Serial.println(line);
}

/* 把当前软时钟按 TIME 协议发给手表 */
static void sendTimeToWatch(void)
{
  uint32_t t = (clockSeconds + elapsedSeconds()) % 86400UL;
  char buf[32];
  snprintf(buf, sizeof(buf), "TIME,%lu,%lu,%lu",
           (unsigned long)((t / 3600UL) % 24UL),
           (unsigned long)((t / 60UL) % 60UL),
           (unsigned long)(t % 60UL));
  stmSendLine(buf);
}

static bool syncClockFromNtp(void)
{
  struct tm timeInfo;
  if(!getLocalTime(&timeInfo, 1000)) {
    Serial.println("[NTP] 尚未获取到时间");
    return false;
  }

  applyClock((uint32_t)timeInfo.tm_hour * 3600UL +
             (uint32_t)timeInfo.tm_min * 60UL +
             (uint32_t)timeInfo.tm_sec);
  lastNtpSyncMs = millis();
  Serial.printf("[NTP] 校准成功: %04d-%02d-%02d %02d:%02d:%02d\n",
                timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday,
                timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
  sendTimeToWatch();
  return true;
}

static void serviceNetwork(void)
{
  uint32_t now = millis();
  bool connected = WiFi.status() == WL_CONNECTED;

  if(!connected) {
    if(wifiWasConnected) {
      Serial.println("[WiFi] 连接断开，软时钟继续运行");
      wifiWasConnected = false;
      lastWifiAttemptMs = now;
    }

    wl_status_t status = WiFi.status();
    if(status == WL_IDLE_STATUS || status == WL_DISCONNECTED ||
       status == WL_NO_SSID_AVAIL || status == WL_CONNECT_FAILED ||
       status == WL_CONNECTION_LOST ||
       (lastWifiAttemptMs != 0 && now - lastWifiAttemptMs >= WIFI_RETRY_MS)) {
      if(lastWifiAttemptMs == 0 || now - lastWifiAttemptMs >= WIFI_RETRY_MS) {
        lastWifiAttemptMs = now;
        Serial.printf("[WiFi] 正在连接 %s...\n", WIFI_SSID);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      }
    }
    return;
  }

  if(!wifiWasConnected) {
    wifiWasConnected = true;
    lastNtpAttemptMs = 0;
    Serial.print("[WiFi] 已连接，IP: ");
    Serial.println(WiFi.localIP());
    configTime(UTC_OFFSET_SECONDS, 0, NTP_SERVER_1, NTP_SERVER_2);
  }

  if(lastNtpSyncMs == 0 || now - lastNtpSyncMs >= NTP_RESYNC_MS) {
    if(lastNtpAttemptMs == 0 || now - lastNtpAttemptMs >= NTP_RETRY_MS) {
      lastNtpAttemptMs = now;
      syncClockFromNtp();
    }
  }
}

/* 解析并执行监视器指令 */
static void handleConsole(const String &cmd)
{
  if(cmd.length() == 0) return;

  if(cmd.equalsIgnoreCase("ping")) {
    stmSendLine("PING");
    return;
  }

  if(cmd.startsWith("set ")) {
    int h = -1, m = -1, s = -1;
    if(sscanf(cmd.c_str() + 4, "%d:%d:%d", &h, &m, &s) == 3 &&
       h >= 0 && h < 24 && m >= 0 && m < 60 && s >= 0 && s < 60) {
      applyClock((uint32_t)h * 3600UL + (uint32_t)m * 60UL + (uint32_t)s);
      Serial.println("[CLK] 已设置，正在同步...");
      sendTimeToWatch();
      return;
    }
    Serial.println("[ERR] 格式: set HH:MM:SS");
    return;
  }

  if(cmd.equalsIgnoreCase("time")) {
    uint32_t t = (clockSeconds + elapsedSeconds()) % 86400UL;
    char buf[12];
    snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu",
             (unsigned long)(t / 3600UL), (unsigned long)((t / 60UL) % 60UL),
             (unsigned long)(t % 60UL));
    Serial.print("[CLK] ");
    Serial.println(buf);
    return;
  }

  if(cmd.equalsIgnoreCase("wifi")) {
    if(WiFi.status() == WL_CONNECTED) {
      Serial.printf("[WiFi] 已连接 %s, RSSI: %d dBm, IP: ", WIFI_SSID, WiFi.RSSI());
      Serial.println(WiFi.localIP());
    } else {
      Serial.printf("[WiFi] 未连接，状态码: %d\n", WiFi.status());
    }
    return;
  }

  if(cmd.equalsIgnoreCase("ntp")) {
    if(WiFi.status() != WL_CONNECTED) {
      Serial.println("[NTP] WiFi 未连接");
    } else {
      syncClockFromNtp();
    }
    return;
  }

  Serial.println("[HLP] 指令: ping | set HH:MM:SS | time | wifi | ntp");
}

void setup()
{
  Serial.begin(115200);
  StmSerial.begin(115200, SERIAL_8N1, -1, STM_TX_PIN);
  clockAnchorMs = millis();
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);

  Serial.println();
  Serial.println("=== OV-Watch ESP32-C3 WiFi/NTP 自动校时 ===");
  Serial.println("指令: ping | set HH:MM:SS | time | wifi | ntp");

  sendTimeToWatch();
  serviceNetwork();
}

void loop()
{
  serviceNetwork();

  /* 周期对时 */
  static uint32_t lastSync = 0;
  if(millis() - lastSync >= SYNC_PERIOD_MS) {
    lastSync = millis();
    sendTimeToWatch();
  }

  /* 心跳 LED：亮 100ms / 灭 900ms */
  static uint32_t lastLed = 0;
  static uint8_t ledOn = 1;
  uint32_t now = millis();
  if((ledOn && (now - lastLed >= 100UL)) || (!ledOn && (now - lastLed >= 900UL))) {
    ledOn = !ledOn;
    lastLed = now;
    digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
  }

  /* USB 监视器指令 */
  static String pending = "";
  while(Serial.available() > 0) {
    char c = (char)Serial.read();
    if(c == '\n' || c == '\r') {
      handleConsole(pending);
      pending = "";
    } else {
      pending += c;
    }
  }
}
