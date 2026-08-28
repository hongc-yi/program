# ESP32-C3 WiFi/NTP 校时固件（阶段二）

配套手表端 `BSP/ESP_LINK`（USART2，PA3 RX，115200 8N1）。
本工程负责 WiFi/NTP 自动校时、离线软时钟和 ESP32 → STM32 单向同步。

## 目录

```
esp32-c3/
├── ovwatch_link/
│   └── ovwatch_link.ino   ← Arduino 主程序
└── README.md              ← 本文件
```

> Arduino 规定 .ino 必须放在同名文件夹里，所以烧录时选的是 `ovwatch_link/ovwatch_link.ino`。

## 接线（2 根杜邦线）

| ESP32-C3 supermini | → | 扩展板 H1 |
|--------------------|---|-----------|
| GPIO21 (TX)        | → | Pin13 (D5 / PA3 / USART2_RX) |
| GND                | — | Pin2 (GND) |

GPIO20 不接线。H1 Pin12 实测为 PA0/KEY，接线会造成按键误触发。

供电：开发期各插各的 USB（STM32 用核心板 USB2 口，ESP32 用自身 Type-C）。

## 烧录步骤

1. Arduino IDE → 文件 → 首选项 → 附加开发板管理器网址：
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
2. 工具 → 开发板 → 开发板管理器 → 搜 `esp32` 安装（装过可跳过）
3. 工具 → 开发板 → `ESP32C3 Dev Module`
4. 打开 `ovwatch_link/ovwatch_link.ino` → Type-C 连 ESP32 → 上传

## 使用与预期输出

USB 串口监视器 **115200**，行结尾选「换行符」。正常应看到：

```text
=== OV-Watch ESP32-C3 链路联调 ===
[TX] TIME,14,35,20
[STM] TIME_OK
[TX] TIME,14,35,21     ← 每5秒一次（软时钟在走）
[STM] TIME_OK
```

同时手表表盘时间会跳到该值并走秒。WiFi 连接后会自动使用 NTP 校准，默认时区为 UTC+8。

监视器指令：

| 输入 | 效果 |
|------|------|
| `ping` | 发 PING，手表回 `[STM] PONG`——链路最先验证的一步 |
| `set 15:42:30` | 设置软时钟并立即同步到手表 |
| `time` | 查看 ESP32 当前软时钟 |
| `wifi` | 查看 WiFi 连接、RSSI 与 IP |
| `ntp` | 立即执行一次 NTP 校准 |

协议约定见手表端 `BSP/ESP_LINK/esp_link.h` 头注释：有效对时回 `TIME_OK`，PING 回 `PONG`，其余回 `ERR`。

## 排障速查

| 现象 | 检查 |
|------|------|
| 完全无 `[STM]` 回执 | TX/RX 是否交叉（GPIO20→Pin12、GPIO21→Pin13）；是否共地；波特率双方都 115200 |
| 回执乱码 | 共地？两边串口都是 115200 8N1？ |
| 偶尔失联后自动恢复 | 正常——手表端已做 ORE 自愈（esp_link.c 错误回调重挂接收），正是为此设计的 |
| 回 `ERR` | 检查发送的行格式：`TIME,h,m,s` 数字范围 0-23/0-59/0-59 |
| ESP32 烧录失败 | 按住 BOOT 再插 USB 进下载模式 |

## 命令行编译/上传（本机唯一可用方式）

> 为什么不用 Arduino IDE：IDE 的构建目录和 ESP32 核心都位于 `C:\Users\红尘\...`，中文用户名会让 ESP32 的 `ld.exe` 链接失败（路径乱码）。因此本机统一用 `arduino-cli` + 纯英文目录（`D:\arduino15` 数据目录、`D:\ovwatch_build` 构建目录）编译上传，已验证可用。

**最省事：双击 `build_upload.cmd`**（自动编译→自动找 COM 口→上传）。它等价于：

```bat
set CLI=R:\arduino-ide\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe
set CFG=D:\arduino15\cli.yaml
set FQBN=esp32:esp32:esp32c3:CDCOnBoot=cdc
set BUILD=D:\ovwatch_build

"%CLI%" --config-file "%CFG%" compile --fqbn %FQBN% "ovwatch_link" --build-path "%BUILD%"
"%CLI%" --config-file "%CFG%" upload -p COM6 --fqbn %FQBN% --input-dir "%BUILD%"
```

要点：

- `CDCOnBoot=cdc` 必须带上，否则 `Serial` 不走 USB，vofa/串口助手看不到输出
- 板子若卡在下载模式（连不上），按住 BOOT 插 USB 后再上传
- 数据目录 `D:\arduino15` 是原 `C:\Users\红尘\AppData\Local\Arduino15` 的**完整拷贝**（含已装好的 esp32 3.3.11），不要删
- 以后改代码后重新跑一遍即可；增量编译，通常十几秒

## 后续阶段占位

- 阶段二：`WiFi.begin()` + NTP → 周期校准本软时钟（替换手动 set），其余逻辑不变
- 阶段三：I2S(INMP441) 录音上传云端 STT → AI 回复文本经 `StmSerial` 下发新协议行（待与手表端约定）
