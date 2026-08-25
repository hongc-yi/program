# OV-Watch 项目现状（截至 2026-08-13）

## 一句话

把开源智能手表固件 OV-Watch v2.4.4 跑在自己的 STM32F411 开发板上，正在加「AI 语音助手 + 温湿度」功能。

## 硬件现状

| 部件 | 状态 |
|------|------|
| STM32F411CEU6 核心板 | ✅ 已有，含 32.768kHz RTC 晶振、25MHz 主晶振、CH340N 串口、双 Type-C |
| 摄像头+LCD 扩展板 | ✅ 已有，已与核心板对插（所有排针埋死） |
| 1.69 寸触摸屏 P169H002-CTP | ✅ 已有，已用 hex 烧录验证，触屏滑动正常 |
| 可用外接接口 | 仅 **H1（底部2×9P）** 和 **H2（左下角1×4P）** |
| ESP32-C3 + INMP441 + AHT21 | 🛒 已定清单（¥26-47），待购买 |
| 电池 | ❌ 暂用 USB 供电，电池方案砍掉 |

## 固件现状

- 已烧录 OV-Watch v2.4.4 hex，正常开机、触屏可用
- 已知问题：时间断电丢失（无VBAT）、秒表按 KEY 卡死、部分小游戏未实现——后两个是固件 bug，以后修

## 接线方案（当前固件方案）

```
H2 Pin3 (PC6 / USART6_TX) -> ESP32-C3 GPIO20 (UART1_RX)
H2 Pin4 (PC7 / USART6_RX) <- ESP32-C3 GPIO21 (UART1_TX)
H2 Pin2 (GND)              -> ESP32-C3 GND
H1 Pin1 (3V3)              -> AHT21 VCC
H1 Pin2 (GND)              -> AHT21 GND
H1 Pin10 (PC4 / SCL)       -> AHT21 SCL
H1 Pin9 (PC5 / SDA)        -> AHT21 SDA
ESP32 ↔ INMP441：面包板内硬跳线（3V3/GND/GPIO4=SD/GPIO5=SCK/GPIO6=WS/LR→GND）
供电：STM32 插 USB2 口；ESP32 开发期用自身 Type-C
```

USART1 PA9/PA10 继续保留给调试输出；H1 PB15/PB2 不用于当前串口方案。

- H1、H2 已计划焊排母（2×9P、1×4P），其余免焊
- 线材：公对母杜邦线（H1/H2↔模块）+ 面包板硬跳线（面包板内）

## 固件待改（到货后）

1. `BSP/AHT21/AHT21.c`：I2C 脚 PC9/PA8 → PC7/PC6
2. `User/Func/Inc/HWDataAccess.h`：`HW_USE_AHT21` 置 1
3. ESP32 固件：Arduino IDE 烧录（语音→云端STT→AI→串口回传→LVGL显示）

## 相关文档（D:\learn\program\target\）

- `OV_Watch购物清单.md` — 8 样元件清单
- `OV_Watch连接指南.md` — 分步接线 + 固件修改
- `开发板信息\OV_Watch开发板完整信息.md` — 引脚全表 + H1/H2 布局
