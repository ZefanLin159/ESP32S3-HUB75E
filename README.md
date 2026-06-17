# 像素屏音乐伴侣 (Pixel Music Companion)

基于 ESP32-S3 的离线音乐播放器 + 64×64 HUB75E 像素屏展示设备。

## 功能特性（当前 Phase 1 MVP）

- 使用 ESP-IDF v5.1.2 + FreeRTOS 构建。
- 多 Agent 架构：Main Controller、Audio、Display、Storage、HAL。
- SD 卡挂载，自动扫描 `/MUSIC` 与 `/IMAGE`。
- WAV 音频播放，MP3 预留桩（后续接入解码器）。
- 实时频谱可视化（FFT 占位实现，后续替换为真实 FFT）。
- 3 按键交互：短按切歌/暂停，长按调音量/休眠，组合键进入设置。
- HUB75E 64×64 像素屏驱动桩（可编译，后续实现 DMA/PIO 扫描）。
- 所有硬件引脚通过 `idf.py menuconfig` 可配置。

## 目录结构

```
.
├── main/                 # 主程序与各 Agent
│   ├── app_main.c
│   ├── main_controller.c/.h
│   ├── audio_agent.c/.h
│   ├── display_agent.c/.h
│   ├── storage_agent.c/.h
│   ├── message_broker.c/.h
│   └── hal/              # 硬件抽象层
├── components/           # 可复用组件
│   ├── audio_decoder/
│   ├── spectrum_analyzer/
│   ├── hub75e_driver/
│   └── ui_animations/
├── partitions.csv        # Flash 分区表
├── sdkconfig.defaults    # 默认 SDK 配置
└── docs/ARCHITECTURE.md  # 架构说明
```

## 构建步骤

1. 安装 ESP-IDF v5.1.2（Windows 用户可使用 [ESP-IDF Tools Installer](https://dl.espressif.com/dl/esp-idf/)）。
2. 打开 ESP-IDF 命令行，进入本项目目录：

```bash
cd E:/thinking/ESP32S3-HUB75E
idf.py set-target esp32s3
idf.py menuconfig
```

3. 在 menuconfig 中配置 GPIO 引脚：
   - `Pixel Music Companion Configuration`：按键、SD 卡 SPI、I2S 引脚。
   - `HUB75E Display Driver`：HUB75E 数据/地址/控制引脚。
4. 编译并烧录：

```bash
idf.py build
idf.py -p COMx flash monitor
```

## 默认引脚说明

> **注意**：默认引脚仅供参考，实际接线时请根据你的硬件修改 `menuconfig`。

| 功能 | 默认 GPIO |
|------|----------|
| 左键 | 0 |
| 右键 | 1 |
| 确认键 | 2 |
| SD MOSI | 11 |
| SD MISO | 13 |
| SD SCLK | 12 |
| SD CS | 10 |
| I2S BCLK | 4 |
| I2S WS | 5 |
| I2S DOUT | 6 |
| I2S DIN (麦克风) | 7 |
| HUB75E 数据/地址 | 见 Kconfig |

## 按键操作

| 按键 | 短按 | 长按 | 组合 |
|------|------|------|------|
| 左键 | 上一曲 | 音量减 | OK+左 = 菜单 |
| 右键 | 下一曲 | 音量加 | OK+右 = 设置 |
| 确认键 | 播放/暂停 | 休眠/唤醒 (3秒) | - |

## 后续计划

- Phase 2：图片显示、设置菜单、多种频谱样式、情感化 UI。
- Phase 3：稳定性测试、功耗优化、SD 卡热插拔。

## 协议

MIT License
