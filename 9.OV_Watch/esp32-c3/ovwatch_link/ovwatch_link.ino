/*
 * OV-Watch × ESP32-C3 串口链路联调固件（阶段一：链路验证 + 手动对时）
 * ------------------------------------------------------------------
 * 硬件接线（与 D:\learn\program\target\OV_Watch连接指南.md 一致）：
 *   ESP32 GPIO20 (RX) <- H1 Pin12 (D4/PA2/USART2_TX)
 *   ESP32 GPIO21 (TX) -> H1 Pin13 (D5/PA3/USART2_RX)
 *   ESP32 GND         -> H1 Pin2 (GND，必须共地)
 *
 * 烧录：Arduino IDE -> 开发板选 ESP32C3 Dev Module -> Type-C 上传
 * 调试：USB 串口监视器 115200（行结尾选"换行符"）
 *
 * 行为：
 *   - 每 5 秒向手表发送 TIME,h,m,s（本地软时钟走秒），手表回 TIME_OK
 *   - 手表所有回执实时转发到 USB 监视器，前缀 [STM]
 *
 * 监视器指令：
 *   ping          发 PING 测链路（手表回 PONG）
 *   set 15:42:30  设置软时钟并立即同步到手表
 *
 * TODO(阶段二)：WiFi 连接 + NTP 校准软时钟后自动对时；
 *               阶段三：INMP441 录音 + 云端语音 + AI 回复文本下行。
 */

#include <HardwareSerial.h>

#define STM_RX_PIN     20
#define STM_TX_PIN     21
#define SYNC_PERIOD_MS 5000UL
#define LED_PIN        8            /* supermini 板载 LED（启动闪一下 + 心跳） */

HardwareSerial StmSerial(1);

/* 本地软时钟 */
static uint32_t clockSeconds = 14UL * 3600UL + 35UL * 60UL + 20UL; /* 开机默认 */
static uint32_t clockAnchorMs = 0;

static char stmLine[96];
static uint8_t stmLen = 0;

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

  if(cmd.startsWith("time")) {
    uint32_t t = (clockSeconds + elapsedSeconds()) % 86400UL;
    char buf[12];
    snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu",
             (unsigned long)(t / 3600UL), (unsigned long)((t / 60UL) % 60UL),
             (unsigned long)(t % 60UL));
    Serial.print("[CLK] ");
    Serial.println(buf);
    return;
  }

  Serial.println("[HLP] 指令: ping | set HH:MM:SS | time");
}

void setup()
{
  Serial.begin(115200);                       /* USB 调试口 */
  StmSerial.begin(115200, SERIAL_8N1, STM_RX_PIN, STM_TX_PIN);
  clockAnchorMs = millis();
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);      /* 启动点亮，验证固件在运行 */

  Serial.println();
  Serial.println("=== OV-Watch ESP32-C3 链路联调 ===");
  Serial.println("指令: ping | set HH:MM:SS | time");

  sendTimeToWatch();                          /* 开机先同步一次 */
}

void loop()
{
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

  /* 手表回执 -> USB 监视器 */
  while(StmSerial.available() > 0) {
    char c = (char)StmSerial.read();
    if(c == '\n' || c == '\r') {
      if(stmLen > 0) {
        stmLine[stmLen] = '\0';
        Serial.print("[STM] ");
        Serial.println(stmLine);
        stmLen = 0;
      }
    } else if(stmLen < sizeof(stmLine) - 1) {
      stmLine[stmLen++] = c;
    } else {
      stmLen = 0;                             /* 超长丢弃 */
    }
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
