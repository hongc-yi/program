# OV-Watch AI 语音助手连接指南

> 供电：USB 直插核心板 USB2 口。  
> 只需焊 H1 和 H2 各一个排母（2 分钟），其余全部插线。

---

## 一、唯一需要烙铁的步骤

在扩展板底部焊两个排母：

| 位置 | 焊什么 | 耗时 |
|------|--------|------|
| H1（底部 2×9 焊盘） | **2×9P 排母** | 1 分钟 |
| H2（左下角 4 孔） | **1×4P 排母** | 1 分钟 |

焊完就可以把烙铁收起来了，之后全靠插线。

---

## 二、认识你的模块

### ESP32-C3 supermini

```
         ┌──────────────┐
   5V ── │5V         GND│ ── GND
  3V3 ── │3V3       EN │
  GND ── │GND       IO0│
         │IO1       IO2│
         │IO3       IO4│ ← INMP441 SD
         │IO5       IO6│ ← INMP441 SCK / WS
         │IO7       IO8│
  H1 ←── │IO20 RX IO21 TX│ ──→ H1
         └──────────────┘
              [TYPE-C ← 烧录用，日常空着]
```

### INMP441 麦克风

```
┌─────────────────────────┐
│  INMP441                │
│  [黑色金属方块朝上]      │
│  L/R  WS  SCK  SD  VDD  GND │
└─────────────────────────┘
```

### AHT21 温湿度（已焊排针版）

```
┌──────┐
│ VCC  │
│ GND  │
│ SCL  │
│ SDA  │
└──────┘
```

---

## 三、线材分配规则

| 场景 | 用什么线 |
|------|---------|
| H1/H2 排母 ↔ 模块排针 | **公对母杜邦线**（公头插排母，母口套模块排针） |
| ESP32 ↔ INMP441（都在面包板上） | **硬跳线**（两头公，直接捅面包板孔） |

---

## 四、分步接线

### 第 1 步：ESP32 + INMP441 插上面包板

把 ESP32 和 INMP441 插在面包板上（排针直接捅进面包板孔）。留出间距方便走线。

### 第 2 步：INMP441 ↔ ESP32（6 根硬跳线，全在面包板内）

| INMP441 | → | ESP32 |
|---------|---|-------|
| VDD | → | 3V3 |
| GND | → | GND |
| SD | → | GPIO4 |
| SCK | → | GPIO5 |
| WS | → | GPIO6 |
| L/R | → | GND |

> 全部用硬跳线，距离短，贴着面包板走，整洁。

### 第 3 步：ESP32 ↔ H2（3 根公对母杜邦线）

| 线 | 公头插 | 母口套 |
|----|-------|--------|
| STM32 TX | H2 Pin3（PC6 / USART6_TX） | ESP32 GPIO20（UART1_RX） |
| STM32 RX | H2 Pin4（PC7 / USART6_RX） | ESP32 GPIO21（UART1_TX） |
| GND | H2 Pin2（GND） | ESP32 GND |

> TX 和 RX 必须交叉连接。当前工程保留 USART1 的 PA9/PA10 作为调试输出，PB15/PB2 不用于串口。

### 第 4 步：AHT21 ↔ H1（4 根公对母杜邦线）

| 线 | 公头插 | 母口套 |
|----|-------|--------|
| 红 | H1 Pin1（3V3） | AHT21 VCC |
| 黑 | H1 Pin2（GND） | AHT21 GND |
| 黄 | H1 Pin10（PC4 / SCL） | AHT21 SCL |
| 绿 | H1 Pin9（PC5 / SDA） | AHT21 SDA |

> AHT21 使用独立 PC4/PC5 软件 I2C，不占用触摸屏 PB6/PB7，也不占用 USART6 PC6/PC7。

### 第 5 步：供电

核心板 USB2 口（CH340 那个 TYPE-C）插充电头或充电宝。**ESP32 也从核心板取电**：ESP32 的 5V 和 GND 不用单独接——它自己就是通过自身 TYPE-C 烧录完后从核心板供电的逻辑不对……

实际上 ESP32 日常运行时需要供电。有两种方式：

- **方式 A（省事）**：ESP32 的 TYPE-C 口另插一根 USB 线供电，和 STM32 各供各的
- **方式 B（整洁）**：从面包板拉 5V 和 GND 给 ESP32。但目前没有电池电路，面包板上没有 5V

> ⚠️ 初期开发和调试直接用方式 A：STM32 插一个充电头，ESP32 插另一个（或电脑 USB），各供各的。等以后加了电池再统一供电。

---

## 五、全系统连接一览

```
扩展板底部：

H2（左下角）                         H1（底部 2×9）
┌──────────────┐                     ┌──────────────────────┐
│ 3V3          │                     │ Pin1 3V3             │──→ AHT21 VCC
│ GND          │──→ ESP32 GND        │ Pin2 GND             │──→ AHT21 GND
│ PC6 / TX     │──→ ESP32 GPIO20 RX  │ Pin9 PC5 / SDA       │──→ AHT21 SDA
│ PC7 / RX     │←── ESP32 GPIO21 TX  │ Pin10 PC4 / SCL      │──→ AHT21 SCL
└──────────────┘                     └──────────────────────┘

H1 的 PB15/PB2 当前不用于串口。

面包板：
┌────────────────────────────┐
│                            │
│  ESP32-C3    INMP441       │
│  ┌──────┐   ┌──────────┐   │
│  │3V3   │──│VDD       │   │ ← 硬跳线
│  │GND   │──│GND  L/R  │   │ ← 硬跳线   L/R→GND 硬跳线
│  │GPIO4 │──│SD        │   │ ← 硬跳线
│  │GPIO5 │──│SCK       │   │ ← 硬跳线
│  │GPIO6 │──│WS        │   │ ← 硬跳线
│  │      │  └──────────┘   │
│  │GPIO21│──公对母──→ H1 PB15 (TX)
│  │GPIO20│──公对母──→ H1 PB2  (RX)
│  └──────┘                 │
│                            │
│  各自 USB 供电（开发期）    │
└────────────────────────────┘
```

---

## 六、H1 / H2 脚位速查

### H1 排母（焊好后，用公头那端插）

```
上排: Pin2=GND  Pin4=PB14  Pin6=PB2   Pin8=PB0   Pin10=PC4  Pin12=PA6  Pin14=PA5  Pin16=PA4  Pin18=PC1
下排: Pin1=3V3  Pin3=PB15  Pin5=PB13  Pin7=PB12  Pin9=PC5   Pin11=PA7  Pin13=PB10 Pin15=PB1  Pin17=PC0
          ↑
    Pin4/Pin5/Pin7 以前留给MPU6050的，现在空着，以后想加随时加
```

### H2 排母（焊好后，上到下）

```
Pin1 = 3V3
Pin2 = GND
Pin3 = PC6
Pin4 = PC7
```

### 当前 OV-Watch 固件实际分配

```text
H2 PC6 = STM32 USART6_TX -> ESP32-C3 GPIO20 (RX)
H2 PC7 = STM32 USART6_RX <- ESP32-C3 GPIO21 (TX)
H1 PC4 = AHT21 SCL
H1 PC5 = AHT21 SDA
```

USART1 的 PA9/PA10 继续保留给调试输出，不接扩展板。
PB15/PB2 当前不用于串口。

---

## 七、固件修改（1 个文件）

只改 AHT21 的 I2C 脚位（因为原来用 PC9/PA8 埋在排针里碰不到）：

### `BSP/AHT21/AHT21.c`

```c
// 改前
iic_bus_t AHT_bus =
{
    .IIC_SDA_PORT = GPIOC,
    .IIC_SCL_PORT = GPIOA,
    .IIC_SDA_PIN  = GPIO_PIN_9,    // PC9
    .IIC_SCL_PIN  = GPIO_PIN_8,    // PA8
};

// 改后
iic_bus_t AHT_bus =
{
    .IIC_SDA_PORT = GPIOC,
    .IIC_SCL_PORT = GPIOC,
    .IIC_SDA_PIN  = GPIO_PIN_7,    // PC7 (H2)
    .IIC_SCL_PIN  = GPIO_PIN_6,    // PC6 (H2)
};
```

### `User/Func/Inc/HWDataAccess.h`

```c
#define HW_USE_AHT21  1   // 原来是 0，启用温湿度
```

> 不改 adc、不改 power.h。电池没接，`HW_USE_BAT` 保持 0。

---

## 八、当前固件映射

```text
USART6：PC6 = TX，PC7 = RX，115200 8N1
AHT21：PC4 = SCL，PC5 = SDA，地址 0x38
USART1：PA9/PA10 保留给调试输出
```

ESP32-C3 到 STM32 的时间协议：

```text
TIME,14,35,20\n
```

STM32 收到有效消息后更新软件时钟。当前已实现 USART6 接收框架，WiFi/NTP 获取仍由 ESP32 固件负责。

## 九、ESP32 固件

ESP32-C3 用自带 TYPE-C 线连电脑，Arduino IDE 烧录：

1. 下载 [Arduino IDE](https://www.arduino.cc/en/software)
2. 文件 → 首选项 → 附加开发板管理器网址：
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
3. 工具 → 开发板 → 开发板管理器 → 搜 `esp32` → 安装
4. 选开发板：`ESP32C3 Dev Module` → 烧录

语音流程：

```
说"小i你好" → INMP441 拾音 → ESP32 I2S 采音频
→ WiFi 上传云端语音识别 → 文字
→ 发给 AI 大模型（带人设）→ AI 回复文字
→ ESP32 串口 TX → H1 PB15 → STM32 USART → LVGL 屏幕显示
```
