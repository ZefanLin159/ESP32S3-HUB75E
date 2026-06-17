# Pixel Music Companion

基于 ESP32-S3 的离线音乐播放器 + 64×64 HUB75E 像素屏展示设备。

## 功能特性

- **ESP-IDF v5.4 + FreeRTOS** 构建，目标芯片 ESP32-S3。
- **多 Agent 架构**：Main Controller、Audio、Display、Storage、HAL 解耦，通过消息总线通信。
- **HUB75E 64×64 像素屏驱动**：基于 `ESP32-HUB75-MatrixPanel-I2S-DMA`，使用 ESP32-S3 LCD_CAM + GDMA，刷新稳定。
- **启动屏三模式轮播**：
  1. 全屏单色彩虹七彩轮播；
  2. 横向彩虹流动渐变；
  3. 七彩向中心收缩径向渐变。
  每 5 秒自动切换一种。
- **SD 卡图片显示（BMP 已支持，JPG/PNG/GIF 预留接口）**：
  - 自动挂载 `/sdcard`，扫描 `/sdcard/IMAGE` 目录；
  - 任意尺寸图片压缩/重采样到 64×64 RGB565；
  - 支持拉伸、等比裁剪填充、等比留白三种适配模式。
- **WAV 音频播放**：通过 I2S 输出（MP3 解码预留桩）。
- **实时频谱可视化**：可配置频谱条样式（当前为占位实现，可替换真实 FFT）。
- **3 按键交互**：短按切歌/切图/暂停，长按调音量/休眠，组合键进入设置。
- **所有硬件引脚** 可通过 `idf.py menuconfig` 配置。

## 目录结构

```
.
├── main/                       # 主程序与各 Agent
│   ├── app_main.c              # 入口与初始化
│   ├── main_controller.c/.h    # 状态机与按键/消息处理
│   ├── audio_agent.c/.h        # 音频播放
│   ├── display_agent.c/.h      # 显示渲染任务
│   ├── storage_agent.c/.h      # SD 卡挂载与文件扫描
│   ├── message_broker.c/.h     # 消息总线
│   └── hal/                    # 硬件抽象层
├── components/                 # 可复用组件
│   ├── audio_decoder/          # WAV/MP3 解码器
│   ├── spectrum_analyzer/      # 频谱分析
│   ├── hub75e_driver/          # HUB75E DMA 驱动封装
│   ├── ui_animations/          # 启动动画与 UI 绘图工具
│   └── image_processor/        # 图片解码与 64x64 缩放
├── partitions.csv              # Flash 分区表
├── sdkconfig.defaults          # 默认 SDK 配置
├── scripts/build_flash.ps1     # Windows 一键编译烧录脚本
└── docs/ARCHITECTURE.md        # 架构说明
```

## 硬件需求

- ESP32-S3 开发板（建议带 8 MB PSRAM）。
- 64×64 HUB75E 全彩 LED 点阵屏。
- SD 卡模块（SPI 模式），可选。
- 三个轻触按键（左、右、确认）。
- I2S DAC / 功放模块（可选，用于音频播放）。

## 默认引脚

> 默认引脚仅供参考，实际接线请通过 `idf.py menuconfig` 修改。

| 功能 | 默认 GPIO |
|------|----------|
| 左键 | 0 |
| 右键 | 1 |
| 确认键 | 2 |
| SD MOSI | 40 |
| SD MISO | 41 |
| SD SCLK | 42 |
| SD CS | 45 |
| I2S BCLK | 6 |
| I2S WS | 7 |
| I2S DOUT | 38 |
| I2S DIN (麦克风) | 3 |
| HUB75E R1 | 35 |
| HUB75E G1 | 36 |
| HUB75E B1 | 37 |
| HUB75E R2 | 14 |
| HUB75E G2 | 12 |
| HUB75E B2 | 13 |
| HUB75E A | 8 |
| HUB75E B | 18 |
| HUB75E C | 5 |
| HUB75E D | 17 |
| HUB75E E | 21 |
| HUB75E LAT | 4 |
| HUB75E OE | 15 |
| HUB75E CLK | 16 |

## 构建与烧录

1. 安装 ESP-IDF v5.4（Windows 用户可使用 [ESP-IDF Tools Installer](https://dl.espressif.com/dl/esp-idf/)）。
2. 打开 ESP-IDF PowerShell，进入项目目录：

```powershell
cd E:/thinking/ESP32S3-HUB75E
```

3. 一键编译并烧录到 COM8（可修改 `scripts/build_flash.ps1` 中的串口号）：

```powershell
.\scripts\build_flash.ps1
```

或手动：

```bash
idf.py set-target esp32s3
idf.py menuconfig
idf.py build
idf.py -p COMx flash monitor
```

## SD 卡使用

1. SD 卡格式化为 FAT32/exFAT。
2. 在根目录创建文件夹：
   - `/MUSIC` — 存放 `.wav` / `.mp3` 音乐文件。
   - `/IMAGE` — 存放 `.bmp` 图片文件（JPG/PNG/GIF 后续支持）。
3. 插入 SD 卡后上电，系统会自动挂载并扫描；无 SD 卡时保持在启动屏轮播，不会异常。

## 图片处理

任意尺寸图片会被压缩/重采样到 64×64 RGB565：

- **最近邻采样**：默认，速度快。
- **双线性插值**：预留接口，后续可开启（质量更好，CPU 更高）。
- **适配模式**：
  - `STRETCH`：直接拉伸到 64×64；
  - `COVER`：等比裁剪填充（默认）；
  - `CONTAIN`：等比缩放，黑边填充。

## 按键操作

| 按键 | 短按 | 长按 | 组合 |
|------|------|------|------|
| 左键 | 音乐：上一曲 / 图片：上一张 | 音量减 | OK+左 = 菜单 |
| 右键 | 音乐：下一曲 / 图片：下一张 | 音量加 | OK+右 = 设置 |
| 确认键 | 播放/暂停 | 休眠/唤醒 (3秒) | - |

## 模式说明

- **BOOT**：上电默认，显示三模式彩虹轮播。
- **MUSIC**：播放 SD 卡 `/MUSIC` 音乐并显示频谱。
- **PHOTO**：浏览 SD 卡 `/IMAGE` 图片，左右键切换。
- **SETTINGS**：设置菜单（待完善）。

## 后续计划

- 接入 JPG/PNG/GIF 解码器。
- 完善设置菜单与 UI 动效。
- 真实 FFT 频谱与多种频谱样式。
- SD 卡热插拔与稳定性优化。

## 协议

MIT License
