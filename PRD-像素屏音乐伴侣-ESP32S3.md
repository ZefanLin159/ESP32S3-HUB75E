# 产品规格文档 (PRD)
## ESP32-S3 像素屏音乐伴侣

**文档版本**: v1.0  
**撰写日期**: 2026-01-17  
**产品名称**: 像素屏音乐伴侣 (Pixel Music Companion)  
**硬件平台**: ESP32-S3  
**目标用户**: 年轻群体，音乐爱好者，情感化礼物场景  

---

## 📋 目录

1. [问题陈述](#1-问题陈述)
2. [产品目标](#2-产品目标)
3. [非目标范围](#3-非目标范围)
4. [用户故事](#4-用户故事)
5. [功能需求](#5-功能需求)
6. [技术架构要求](#6-技术架构要求)
   - 6.1 [开发框架要求](#61-开发框架要求)
   - 6.2 [分时调度系统设计](#62-分时调度系统设计)
   - 6.3 [ESP-IDF 项目配置](#63-esp-idf-项目配置)
   - 6.4 [Agent间通信机制](#64-agent间通信机制)
7. [硬件规格](#7-硬件规格)
8. [用户体验设计](#8-用户体验设计)
9. [成功指标](#9-成功指标)
10. [开放问题](#10-开放问题)
11. [编码架构建议](#11-编码架构建议)
   - 11.1 [为什么需要多Agent架构？](#111-为什么需要多agent架构)
   - 11.2 [推荐的代码目录结构](#112-推荐的代码目录结构)
   - 11.3 [Agent间通信API示例](#113-agent间通信api示例)
   - 11.4 [开发优先级建议](#114-开发优先级建议)
   - 11.5 [Git代码管理规范](#115-git代码管理规范)
12. [风险与缓解措施](#12-风险与缓解措施)
13. [附录](#13-附录)
   - 13.1 [参考资料](#131-参考资料)
   - 13.2 [相关开源项目](#132-相关开源项目)
   - 13.3 [ESP-IDF 版本管理](#133-esp-idf-版本管理)

---

## 1. 问题陈述

**核心问题**：  
现代年轻人的桌面空间缺乏**情感化的音乐陪伴设备**。现有的智能音箱只能"听"，缺乏视觉反馈和情感表达；传统的蓝牙音箱功能单一，无法创造沉浸式的音乐体验。

**目标用户痛点**：
- 想要一个既能播放音乐，又能通过视觉效果营造氛围的桌面设备
- 希望设备能展示个性化的内容（照片、频谱动画）
- 需要一个操作简单、适合作为情感礼物的科技产品

**商业价值**：  
这款产品可以作为**情感化礼物**场景的切入点，通过开源硬件 + 精致软件体验，建立品牌认知度和用户粘性。

**证据基础**：  
- 桌面装饰市场增长（LED像素屏、复古音箱等品类热销）
- 音乐可视化需求（Spotify Canvas、Apple Music动画歌词等受追捧）
- 情感化礼物市场（DIY、定制化产品受欢迎）

---

## 2. 产品目标

### 用户目标
1. **沉浸式音乐体验**：通过实时频谱可视化，让用户"看见"音乐的情感
2. **个性化展示**：支持用户存储和展示个人照片，创造情感连接
3. **简单易用**：3个按键完成所有操作，降低学习成本
4. **氛围营造**：通过灯光和动画效果，适配不同场景（工作、放松、派对）

### 业务目标
1. **情感化定位**：打造"用科技传递情感"的产品形象
2. **开源社区**：通过开源硬件和软件，建立DIY社区和口碑传播
3. **可扩展性**：预留接口，支持后续功能扩展（WiFi联动、APP控制等）

### 成功标准
- 用户每天主动使用时长 > 2小时
- 用户留存率（7日）> 60%
- NPS (净推荐值) > 40

---

## 3. 非目标范围

**本版本明确不包含的功能**：

1. **WiFi/蓝牙联网功能**  
   - 原因：简化硬件设计，降低成本和功耗，专注离线体验  
   - 未来考虑：v2版本可加入

2. **APP远程控制**  
   - 原因：增加开发复杂度，3个按键已覆盖核心交互  
   - 未来考虑：如果市场需求强烈，可作为扩展

3. **多房间同步播放**  
   - 原因：技术复杂度高，不适合第一款产品  
   - 未来考虑：专业音频产品线

4. **录音功能**  
   - 原因：存储和隐私考虑，麦克风仅用于实时频谱分析  
   - 未来考虑：语音助手功能

5. **视频播放**  
   - 原因：64x64像素屏分辨率限制，视频体验不佳  
   - 未来考虑：更高分辨率版本

---

## 4. 用户故事

### 核心用户故事（按优先级排序）

#### P0 - 必须有的功能

**US1: 播放音乐**  
作为用户，我希望插入SD卡后能自动播放音乐，以便享受音乐陪伴。  
- 验收标准：
  - [ ] 插入含MP3/WAV文件的SD卡，设备自动开始播放
  - [ ] 支持循环播放（全部循环、单曲循环、随机播放）
  - [ ] 播放时可显示当前歌曲名称和播放进度（通过像素屏滚动文字）

**US2: 音乐频谱可视化**  
作为用户，我希望播放音乐时能看到实时频谱动画，以便获得沉浸式体验。  
- 验收标准：
  - [ ] 播放时实时显示音乐频谱（刷新率 ≥ 30fps）
  - [ ] 频谱颜色可随音乐节奏变化
  - [ ] 支持多种频谱样式（柱状图、波浪、粒子等）

**US3: 按键交互**  
作为用户，我希望通过3个按键控制设备，以便简单快速地操作。  
- 验收标准：
  - [ ] 左键：上一项（切换歌曲/切换模式/菜单向上）
  - [ ] 右键：下一项（切换歌曲/切换模式/菜单向下）
  - [ ] 确认键：短按确认/播放暂停，长按3秒进入休眠/唤醒
  - [ ] 按键有触觉反馈（可选蜂鸣器或震动）

**US4: 显示图片**  
作为用户，我希望能在像素屏上展示个人照片，以便个性化我的桌面。  
- 验收标准：
  - [ ] 支持BMP/JPG/PNG格式（需转换为64x64像素）
  - [ ] 可设置图片轮播间隔时间
  - [ ] 图片模式下不播放音乐（或音乐继续后台播放）

#### P1 - 应该有但不阻塞发布的功能

**US5: 模式切换**  
作为用户，我希望能在"音乐模式"和"图片模式"之间切换，以便根据不同场景选择展示内容。  
- 验收标准：
  - [ ] 确认键 + 左/右键组合进入模式选择
  - [ ] 模式切换时有动画过渡效果
  - [ ] 记住上次使用的模式，下次开机自动恢复

**US6: 音量调节**  
作为用户，我希望能调节音量，以便适配不同环境。  
- 验收标准：
  - [ ] 长按左/右键调节音量
  - [ ] 音量等级在像素屏上显示（例如：|||·· ）
  - [ ] 音量设置掉电保存

#### P2 - 未来版本考虑

**US7: WiFi联网播放**  
作为用户，我希望能通过网络播放音乐，以便访问更多音乐资源。  

**US8: APP控制**  
作为用户，我希望用手机APP控制设备，以便更方便地管理内容和设置。

---

## 5. 功能需求

### 5.1 音乐播放功能

**需求编号**: FR-001  
**优先级**: P0  
**描述**: 从SD卡读取并播放音频文件

**详细规格**:
- 支持音频格式：MP3 (16-320kbps)、WAV (16-bit PCM)
- 采样率支持：44.1kHz、48kHz
- 播放模式：
  - 全部循环（默认）
  - 单曲循环
  - 随机播放
  - 顺序播放
- 播放控制：
  - 播放/暂停（确认键短按）
  - 上一曲/下一曲（左/右键短按）
  - 音量调节（左/右键长按）
- 音频输出：
  - PWM输出或通过I2S DAC
  - 信噪比 > 80dB
  - 输出功率：2x3W (4Ω喇叭)

**验收标准**:
- [ ] SD卡挂载成功后，自动扫描 `/MUSIC` 目录
- [ ] 音频解码延迟 < 500ms
- [ ] 切换歌曲时间间隔 < 1秒
- [ ] 支持断点续播（掉电记忆）

---

### 5.2 频谱可视化功能

**需求编号**: FR-002  
**优先级**: P0  
**描述**: 实时分析音频信号并生成可视化效果

**详细规格**:
- 频谱分析：
  - FFT点数：256点或512点
  - 频率范围：20Hz - 20kHz
  - 更新频率：≥ 30fps
- 显示效果：
  - 预设3种以上频谱样式（可通过菜单切换）
  - 支持颜色渐变和动态效果
  - 频谱高度随音量自动缩放
- 数据源：
  - 播放音乐时：从音频解码器获取PCM数据
  - 麦克风模式：从麦克风ADC采集环境声音

**验收标准**:
- [ ] 频谱显示无卡顿，刷新率稳定
- [ ] 低频和高频都能清晰显示
- [ ] 麦克风模式下的频谱延迟 < 100ms

---

### 5.3 图片展示功能

**需求编号**: FR-003  
**优先级**: P0  
**描述**: 从SD卡读取并显示图片

**详细规格**:
- 支持图片格式：BMP、JPG (baseline)、PNG (8-bit)
- 分辨率：自动缩放到64x64像素
- 显示模式：
  - 单张显示（手动切换）
  - 轮播模式（可设置间隔时间：5s/10s/30s/60s）
  - 时钟模式（图片 + 时间显示）
- 存储路径：`/IMAGE` 目录

**验收标准**:
- [ ] 图片加载时间 < 500ms
- [ ] 支持至少100张图片轮播
- [ ] 图片过渡效果流畅（淡入淡出或滑动）

---

### 5.4 按键交互功能

**需求编号**: FR-004  
**优先级**: P0  
**描述**: 3个按键完成所有用户交互

**按键定义**:

| 按键 | 短按功能 | 长按功能 | 组合功能 |
|------|----------|----------|----------|
| 左键 | 上一项/上一曲 | 音量减 | 确认+左 = 菜单 |
| 右键 | 下一项/下一曲 | 音量加 | 确认+右 = 设置 |
| 确认键 | 播放/暂停/确认 | 休眠/唤醒 (3秒) | - |

**详细规格**:
- 按键去抖：硬件或软件去抖，响应时间 < 50ms
- 长按判定：> 1秒
- 超低功耗休眠：长按确认键3秒，进入深度睡眠（电流 < 10uA）
- 唤醒方式：任意按键唤醒

**验收标准**:
- [ ] 所有按键操作都有视觉或声音反馈
- [ ] 休眠唤醒时间 < 1秒
- [ ] 误操作率 < 5%（用户测试中）

---

### 5.5 系统设置功能

**需求编号**: FR-005  
**优先级**: P1  
**描述**: 提供系统配置选项

**设置项**:
1. 播放模式（全部循环/单曲循环/随机/顺序）
2. 音量大小（0-100%）
3. 亮度调节（适用于像素屏）
4. 图片轮播间隔（5s/10s/30s/60s）
5. 频谱样式选择（3种以上）
6. 语言选择（中文/英文）
7. 恢复出厂设置

**交互方式**:
- 通过按键进入设置菜单
- 像素屏显示菜单选项（图标 + 文字）
- 设置自动保存到Flash

---

## 6. 技术架构要求

### ⚠️ 关键架构建议：分时调度 + 多Agent编码

**本产品涉及多个实时任务，必须使用分时调度系统，并建议采用多Agent编码架构。**

### 6.1 开发框架要求

**⚠️ 强制要求：必须使用 ESP-IDF 框架开发**

**ESP-IDF 简介**：
- 乐鑫官方物联网开发框架（Espressif IoT Development Framework）
- 基于FreeRTOS，提供完整的API和组件库
- 支持ESP32、ESP32-S3、ESP32-C3等全系列芯片
- 官方文档：https://docs.espressif.com/projects/esp-idf/zh_CN/latest/
- 芯片技术参考手册：https://www.espressif.com.cn/zh-hans

**为什么选择ESP-IDF**：
1. **官方支持**：乐鑫官方维护，稳定性有保障
2. **组件丰富**：内置FreeRTOS、FatFS、SPI驱动、I2S驱动等
3. **工具链完善**：idf.py构建系统、menuconfig配置系统、openOCD调试
4. **社区活跃**：GitHub上有大量开源项目和问题解答
5. **持续更新**：定期发布新版本，修复Bug和增加新功能

**开发环境搭建**：
```bash
# 1. 安装ESP-IDF (Windows用户使用ESP-IDF Tools Installer)
# 下载地址：https://dl.espressif.com/dl/esp-idf/?idf=4.4

# 2. 设置环境变量
# Windows: 运行 ESP-IDF Command Prompt
# Linux/macOS: source $HOME/esp/esp-idf/export.sh

# 3. 创建项目
idf.py create-project pixel_music_companion
cd pixel_music_companion

# 4. 设置目标芯片
idf.py set-target esp32s3

# 5. 配置项目
idf.py menuconfig

# 6. 编译和烧录
idf.py build
idf.py -p COMx flash monitor
```

**关键ESP-IDF组件**（本项目会使用到）：
- `freertos`: 实时操作系统（任务调度、信号量、队列等）
- `fatfs`: FAT文件系统（SD卡挂载）
- `driver/i2s`: I2S音频接口驱动
- `driver/spi`: SPI驱动（SD卡、HUB75E）
- `driver/adc`: ADC驱动（麦克风）
- `driver/gpio`: GPIO驱动（按键、LED）
- `esp_timer`: 高精度定时器（频谱刷新）
- `nvs_flash`: 非易失存储（保存用户设置）

---

### 6.2 分时调度系统设计

**任务划分**（按优先级排序）：

| 任务优先级 | 任务名称 | 执行周期 | 功能描述 | ESP-IDF API |
|-----------|---------|---------|---------|-------------|
| 高 (0) | 音频解码任务 | 实时 | 解码MP3/WAV，输出PCM数据 | `xTaskCreatePinnedToCore()` |
| 高 (1) | 频谱分析任务 | 10ms | FFT分析，生成频谱数据 | `xTimerCreate()` |
| 中 (2) | 显示刷新任务 | 33ms (30fps) | 更新像素屏显示内容 | `xTaskCreate()` |
| 中 (3) | 按键扫描任务 | 20ms | 检测按键事件 | `xTaskCreate()` |
| 低 (4) | SD卡读取任务 | 按需 | 读取音乐/图片文件 | `xTaskCreate()` |
| 低 (5) | 系统监控任务 | 1s | 监控内存、温度、电量等 | `xTaskCreate()` |

**调度策略**:
- 使用ESP-IDF内置的FreeRTOS
- 音频和解码任务使用高优先级（Priority 10-15），确保实时性
- 显示任务使用定时器中断触发，保证帧率稳定
- 按键任务使用轮询或外部中断
- 建议将高实时性任务固定运行在Core 1（Protocol CPU），主控逻辑运行在Core 0（App CPU）

### 6.2 多Agent编码架构建议

**⚠️ 重要提醒：后续编码时，强烈建议将系统分为多个独立Agent（模块），每个Agent由专人负责，通过消息队列或API通信。**

#### 推荐的Agent划分：

```
┌─────────────────────────────────────────────────────┐
│               Main Controller Agent                  │
│  (主控制器：协调各Agent，处理用户交互逻辑)             │
└─────────────────────────────────────────────────────┘
                          │
          ┌───────────────┼───────────────┐
          │               │               │
┌─────────▼──────┐ ┌─────▼─────┐ ┌──────▼────────┐
│ Audio Agent     │ │ Display   │ │ Storage Agent │
│ (音频处理)      │ │ Agent     │ │ (存储管理)    │
│                 │ │ (显示控制) │ │               │
│ - 解码MP3/WAV  │ │ - HUB75E  │ │ - SD卡挂载   │
│ - 频谱分析      │ │   驱动     │ │ - 文件扫描   │
│ - 音量控制      │ │ - 动画渲染 │ │ - 缓存管理   │
└────────────────┘ └───────────┘ └───────────────┘
          │               │               │
          └───────────────┼───────────────┘
                          │
                  ┌───────▼────────┐
                  │ Hardware Abstraction │
                  │ (硬件抽象层)        │
                  │ - 按键驱动          │
                  │ - 麦克风ADC         │
                  │ - I2S/PWM音频输出   │
                  └────────────────────┘
```

#### Agent详细定义：

**Agent 1: Main Controller (主控制器)**
- **职责**：系统协调、用户交互逻辑、模式切换
- **输入**：按键事件、系统状态
- **输出**：控制指令给各Agent
- **技术栈**：FreeRTOS任务，状态机设计模式
- **编码建议**：使用C或C++，事件驱动架构

**Agent 2: Audio Agent (音频处理Agent)**
- **职责**：音频解码、频谱分析、音量控制
- **输入**：SD卡音频文件、麦克风数据
- **输出**：PCM数据流、频谱数据
- **技术栈**：ESP32-I2S、DSP库（FFT）、Madlib或Helix解码器
- **编码建议**：性能关键模块，使用C语言+汇编优化

**Agent 3: Display Agent (显示控制Agent)**
- **职责**：像素屏驱动、动画渲染、UI显示
- **输入**：频谱数据、图片数据、系统状态
- **输出**：HUB75E时序信号
- **技术栈**：PIO（可编程IO）、双缓冲机制、DMA传输
- **编码建议**：使用C++，面向对象设计，支持多种动画效果

**Agent 4: Storage Agent (存储管理Agent)**
- **职责**：SD卡挂载、文件扫描、缓存管理
- **输入**：文件读写请求
- **输出**：文件数据、文件列表
- **技术栈**：FatFS、SPI驱动
- **编码建议**：使用C语言，实现异步读写接口

**Agent 5: Hardware Abstraction (硬件抽象层)**
- **职责**：按键驱动、麦克风ADC、电源管理
- **输入**：硬件中断、ADC数据
- **输出**：标准化硬件接口
- **技术栈**：ESP32硬件驱动、中断处理
- **编码建议**：使用C语言，提供统一API给上层Agent

### 6.3 ESP-IDF 项目配置

**SDK Configuration (`sdkconfig.defaults`)**：

```properties
# ESP32-S3 配置
CONFIG_IDF_TARGET="esp32s3"
CONFIG_ESP32S3_DEFAULT_CPU_FREQ_240=y
CONFIG_ESP32S3_SPIRAM_SUPPORT=y

# FreeRTOS 配置
CONFIG_FREERTOS_UNICORE=n
CONFIG_FREERTOS_MAX_TASK_NAME_LEN=16
CONFIG_FREERTOS_IDLE_TASK_STACKSIZE=1024

# FatFS 配置（SD卡）
CONFIG_FATFS_LFN_STACK=y
CONFIG_FATFS_MAX_LFN=255

# I2S 配置
CONFIG_I2S_ENABLE_PDM=n
CONFIG_I2S_SUPPRESS_WARNING=y

# ADC 配置（麦克风）
CONFIG_ADC_CAL_EFUSE_TP_ENABLE=y

# 优化选项
CONFIG_COMPILER_OPTIMIZATION_LEVEL_RELEASE=y
CONFIG_COMPILER_OPTIMIZATION_ASSERTIONS_DISABLE=n
```

**Partition Table (`partitions.csv`)**：

```csv
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     0x9000,  0x5000,
otadata,  data, ota,     ,        0x2000,
phy_init, data, phy,      ,        0x1000,
factory,  app,  factory,  ,        2M,
storage,  data, spiffs,   ,        2M,     # 存储用户配置
```

**Component 配置 (`Kconfig.projbuild`)**：

```kconfig
menu "Pixel Music Companion Configuration"

    config AUDIO_SAMPLE_RATE
        int "Audio Sample Rate (Hz)"
        range 8000 96000
        default 44100

    config DISPLAY_FPS
        int "Display Refresh Rate (fps)"
        range 10 60
        default 30

    config BUTTON_DEBOUNCE_MS
        int "Button Debounce Time (ms)"
        range 10 200
        default 50

endmenu
```

---

### 6.4 Agent间通信机制

**推荐方案**：
- **消息队列**：用于Agent间异步通信（例如：Main Controller发送"播放下一曲"指令给Audio Agent）
- **共享内存**：用于大数据传输（例如：Audio Agent将PCM数据写入共享缓冲区，Display Agent读取）
- **事件标志组**：用于同步（例如：Display Agent等待新的频谱数据）

**示例通信流程**（播放音乐并显示频谱）：
```
1. 用户按"下一曲" → 按键中断 → Hardware Abstraction
2. Hardware Abstraction → 发送消息给 → Main Controller
3. Main Controller → 发送"播放指令"给 → Audio Agent
4. Audio Agent → 解码音频 → 写入共享缓冲区
5. Audio Agent → 发送"新频谱数据"事件给 → Display Agent
6. Display Agent → 从共享缓冲区读取频谱数据 → 渲染到像素屏
```

---

## 7. 硬件规格

### 7.1 主控芯片
- **型号**：ESP32-S3-WROOM-1
- **Flash**：8MB (QSPI)
- **PSRAM**：8MB (可选，用于帧缓冲)
- **时钟**：240MHz 双核

### 7.2 显示模块
- **接口**：HUB75E (RGB矩阵屏接口)
- **分辨率**：64x64 像素
- **刷新率**：≥ 30fps
- **亮度**：可调（PWM调光）
- **驱动方式**：使用ESP32的PIO或外设接口模拟HUB75E时序

### 7.3 音频模块
- **音频解码**：软件解码（MP3/WAV）
- **DAC输出**：
  - 方案A：I2S接口 + 外部DAC (如MAX98357A)
  - 方案B：ESP32内置DAC + 功放
- **麦克风**：
  - 型号：INMP441 (I2S数字麦克风) 或模拟麦克风 + ADC
  - 采样率：44.1kHz
- **功放**：PAM8403 (3W立体声D类功放)
- **喇叭**：4Ω 3W x2

### 7.4 存储模块
- **SD卡接口**：SPI模式或SDIO模式
- **支持容量**：≤ 32GB (FAT32格式)
- **文件系统集成**：FatFS

### 7.5 交互模块
- **按键**：3x 轻触开关 (带按键帽)
- **LED指示灯**：1x RGB LED (用于状态指示，可选)
- **蜂鸣器**：1x (用于按键音反馈，可选)

### 7.6 电源管理
- **输入电压**：5V (USB-C接口)
- **电池**：可选锂电池 (3.7V 2000mAh) + 充电管理 (TP4056)
- **功耗**：
  - 工作模式：< 5W
  - 休眠模式：< 0.1W
- **电源管理IC**：用于电压转换和电池管理

---

## 8. 用户体验设计

### 8.1 开机动画
- 设备上电后，显示品牌Logo（2-3秒）
- 然后进入主界面（音乐播放或图片展示）

### 8.2 主界面设计

**音乐模式主界面**：
```
┌────────────────────────┐
│  频谱可视化区域          │  ← 占屏幕上半部分 (64x48)
│  (实时动画)            │
├────────────────────────┤
│ 歌曲名: xxxxxx         │  ← 滚动文字
│ 音量: ████░░ 70%      │
│ 模式: 全部循环 [图标]   │
└────────────────────────┘
```

**图片模式主界面**：
```
┌────────────────────────┐
│                        │
│     64x64 图片         │  ← 全屏显示图片
│                        │
└────────────────────────┘
(图片轮播时，底部显示小圆点指示器)
```

### 8.3 菜单设计
- 使用图标 + 文字
- 当前选中项高亮显示
- 支持滚动菜单（如果选项超过一屏）

**设置菜单示例**：
```
⚙ 设置
├─ 🔁 播放模式
├─ 🔊 音量
├─ 💡 亮度
├─ 🖼 图片间隔
├─ 🎨 频谱样式
├─ 🌐 语言
└─ 🏭 恢复出厂
```

### 8.4 情感化设计细节

**⚠️ 特别关注：这款产品用于挽回重要的人，因此在体验设计上需要特别注重情感化细节。**

**建议设计元素**：
1. **开机问候语**：可以自定义显示"Hi, [名字]"或暖心文字
2. **特殊节日模式**：检测系统日期，自动切换主题（例如：情人节、生日）
3. **灯光氛围**：频谱颜色可设置为"温柔模式"（暖色调）或"派对模式"（炫彩）
4. **图片故事模式**：支持为每张图片配一段文字，轮播时滚动显示
5. **晚安模式**：定时休眠，显示"Good Night"动画

---

## 9. 成功指标

### 9.1 产品发布后1个月目标

**采用指标（Leading Indicators）**:
- SD卡使用率：> 80% 的用户存储了个人内容
- 功能使用分布：
  - 音乐模式：> 70% 使用时间
  - 图片模式：> 20% 使用时间
- 按键操作成功率：> 95% (用户能正确完成预期操作)

**满意度指标**:
- NPS (净推荐值)：> 40
- 用户反馈中"情感化"相关正面评价：> 50%

### 9.2 产品发布后3个月目标

**留存指标（Lagging Indicators）**:
- 7日留存率：> 60%
- 30日留存率：> 40%
- 日均使用时长：> 2小时

**口碑指标**:
- 社交媒体分享次数：> 500次 (#产品话题标签)
- 开源社区Star数：> 1000 (如果开源)

### 9.3 技术指标

**性能要求**:
- 音频解码延迟：< 500ms
- 频谱刷新率：稳定 30fps
- 系统启动时间：< 3秒
- 按键响应时间：< 100ms

**稳定性要求**:
- 连续运行72小时无崩溃
- SD卡热插拔成功率：100%
- 内存泄漏：< 1KB/小时

---

## 10. 开放问题

### 需要工程团队回答的问题

1. **ESP32-S3的HUB75E驱动方案**  
   - 使用PIO模拟时序？还是使用外设接口？  
   - 是否需要外接SRAM作为帧缓冲？  
   - 预计刷新率能达到多少fps？

2. **音频解码性能**  
   - ESP32-S3软件解码MP3，最高支持多少比特率？  
   - 是否需要使用硬件加速器（如果有的话）？  
   - 同时解码+频谱分析，CPU占用率预估多少？

3. **功耗优化**  
   - 深度睡眠模式下，哪些外设需要断电？  
   - 预计电池续航时间（如果有电池版本）？

4. **SD卡读取速度**  
   - SPI模式 vs SDIO模式，读取速度差异？  
   - 是否需要实现文件预读取缓存？

### 需要设计团队回答的问题

1. **像素屏的视觉设计**  
   - 频谱动画的具体样式（需要提供设计稿）？  
   - 字体选择（像素字体）？  
   - 图标风格（复古/现代/可爱）？

2. **外壳设计**  
   - 产品尺寸和造型？  
   - 按键布局和手感？  
   - 散热设计（ESP32-S3可能发热）？

### 需要产品决策的问题

1. **是否加入RGB LED氛围灯**？  
   - 可以增加氛围感，但增加成本和功耗

2. **是否支持定时开关机**？  
   - 作为情感礼物，定时播放早安/晚安消息可能很打动人

3. **是否开源软件代码**？  
   - 开源可以增加社区传播，但可能被抄袭

4. **选择哪个代码托管平台**？  
   - **GitHub**：适合开源，社区活跃，但私有仓库需付费
   - **Gitee**：国内访问快，免费私有仓库，适合闭源开发
   - **GitLab自建**：完全掌控，适合企业级需求
   - 建议：如果开源选GitHub，如果闭源选Gitee

5. **单一仓库还是多仓库**？  
   - **单一仓库**：适合小团队（≤5人），集成简单
   - **多仓库**：适合大团队，Agent完全独立，但集成复杂
   - 建议：小团队使用单一仓库 + 目录隔离

---

## 11. 编码架构建议（详细版）

### 11.1 为什么需要多Agent架构？

**传统单线程代码的问题**：
- 音频解码、频谱分析、显示刷新都在主循环中，容易导致卡顿
- 某个模块出错可能影响整个系统
- 代码难以维护和扩展

**多Agent架构的优势**：
- 各模块独立运行，互不干扰
- 便于团队协作（不同的人负责不同的Agent）
- 易于调试（可以单独测试每个Agent）
- 扩展性强（未来增加新功能只需增加新Agent）

### 11.2 推荐的代码目录结构

**⚠️ 必须使用 ESP-IDF 标准项目结构**

```
pixel_music_companion/              # 项目根目录
├── main/                           # 主组件（Application Component）
│   ├── main_controller.c          # Agent 1: 主控制器
│   ├── main_controller.h
│   ├── audio_agent.c             # Agent 2: 音频处理
│   ├── audio_agent.h
│   ├── display_agent.c           # Agent 3: 显示控制
│   ├── display_agent.h
│   ├── storage_agent.c           # Agent 4: 存储管理
│   ├── storage_agent.h
│   ├── hal/                      # Agent 5: 硬件抽象层
│   │   ├── hal_buttons.c
│   │   ├── hal_buttons.h
│   │   ├── hal_mic.c
│   │   ├── hal_mic.h
│   │   ├── hal_audio_output.c
│   │   ├── hal_audio_output.h
│   │   ├── hal_power.c
│   │   └── hal_power.h
│   ├── CMakeLists.txt            # main组件的构建配置
│   └── Kconfig.projbuild        # 项目配置菜单
├── components/                    # 自定义组件目录
│   ├── spectrum_analyzer/        # 频谱分析组件
│   │   ├── spectrum_analyzer.c
│   │   ├── spectrum_analyzer.h
│   │   ├── CMakeLists.txt
│   │   └── Kconfig
│   ├── hub75e_driver/            # HUB75E驱动组件
│   │   ├── hub75e_driver.c
│   │   ├── hub75e_driver.h
│   │   ├── CMakeLists.txt
│   │   └── Kconfig
│   ├── audio_decoder/            # 音频解码组件
│   │   ├── audio_decoder.c
│   │   ├── audio_decoder.h
│   │   ├── CMakeLists.txt
│   │   └── Kconfig
│   └── ui_animations/            # UI动画组件
│       ├── ui_animations.c
│       ├── ui_animations.h
│       ├── CMakeLists.txt
│       └── Kconfig
├── partitions.csv                 # 分区表定义
├── sdkconfig.defaults             # 默认SDK配置
├── CMakeLists.txt                 # 项目构建配置（顶层）
├── Kconfig.projbuild             # 项目配置菜单（顶层）
├── .devcontainer/                 # 开发容器配置（可选）
│   └── devcontainer.json
├── .vscode/                      # VSCode配置（可选）
│   ├── settings.json
│   ├── tasks.json
│   └── launch.json
├── docs/                          # 文档目录
│   ├── ARCHITECTURE.md           # 架构设计文档
│   ├── API_REFERENCE.md          # Agent间API文档
│   ├── CODING_STANDARDS.md       # 编码规范
│   └── ESP_IDF_GUIDE.md         # ESP-IDF开发指南
└── README.md                     # 项目说明文档
```

**ESP-IDF 项目文件说明**：

| 文件/目录 | 说明 | 是否必须 |
|-----------|------|---------|
| `CMakeLists.txt` (顶层) | 项目构建配置，指定项目名称和组件目录 | ✅ 必须 |
| `CMakeLists.txt` (组件) | 组件构建配置，指定源文件和依赖 | ✅ 必须 |
| `Kconfig.projbuild` | 项目级配置菜单（出现在`idf.py menuconfig`） | ❌ 可选 |
| `Kconfig` (组件) | 组件级配置菜单 | ❌ 可选 |
| `partitions.csv` | 分区表定义（Flash布局） | ✅ 必须 |
| `sdkconfig.defaults` | 默认SDK配置（覆盖`menuconfig`默认值） | ✅ 推荐 |
| `main/` | 主组件目录（存放`app_main()`入口） | ✅ 必须 |
| `components/` | 自定义组件目录 | ❌ 可选 |

**构建系统工作原理**：
```bash
# 1. idf.py 读取顶层 CMakeLists.txt
# 2. CMake 递归查找所有 CMakeLists.txt
# 3. 构建所有组件（main + components/*）
# 4. 链接生成固件（partition_table + bootloader + app）
```

### 11.3 Agent间通信API示例

**消息队列定义**（`message_types.h`）：

```c
// 消息类型枚举
typedef enum {
    MSG_PLAY_NEXT,         // 播放下一曲
    MSG_PLAY_PREV,         // 播放上一曲
    MSG_PLAY_PAUSE,        // 播放/暂停
    MSG_VOLUME_UP,         // 音量加
    MSG_VOLUME_DOWN,       // 音量减
    MSG_MODE_SWITCH,       // 模式切换
    MSG_NEW_SPECTRUM_DATA, // 新的频谱数据
    MSG_DISPLAY_UPDATE,     // 更新显示
    // ... 其他消息类型
} message_type_t;

// 消息结构体
typedef struct {
    message_type_t type;
    void* data;  // 可选的数据负载
    int data_len;
} message_t;

// Agent间通信接口
esp_err_t send_message(message_type_t type, void* data, int len);
esp_err_t receive_message(message_t* msg, TickType_t timeout);
```

### 11.4 开发优先级建议

**Phase 1: MVP (最小可行产品) - 预计2周**
1. 搭建ESP32-S3开发环境
2. 实现SD卡挂载和文件扫描
3. 实现音频解码和播放
4. 实现基本的频谱显示（1种样式）
5. 实现3个按键的基本功能

**Phase 2: 体验优化 - 预计2周**
1. 优化频谱动画效果（增加多种样式）
2. 实现图片显示功能
3. 实现设置菜单
4. 优化按键交互体验
5. 加入情感化设计细节

**Phase 3: 稳定性测试 - 预计1周**
1. 压力测试（连续运行72小时）
2. 边界条件测试（SD卡满、文件损坏等）
3. 功耗优化
4. 用户内测和问题修复

### 11.5 Git代码管理规范

**⚠️ 关键要求：必须使用Git进行代码版本管理，确保多Agent协同开发的有序进行。**

#### 代码托管平台选择

| 平台 | 适用场景 | 优势 | 劣势 |
|------|---------|------|--------|
| **GitHub** (Recommended) | 开源项目、国际协作 | 社区活跃、CI/CD集成好、Issues/PR功能强大 | 私有仓库需要付费（团队版） |
| **Gitee** (国内备选) | 国内团队、需要加速访问 | 国内访问快、免费私有仓库 | 社区活跃度不如GitHub |
| **GitLab** (自建) | 企业私有部署 | 完全掌控、免费私有仓库 | 需要自行维护服务器 |
| **Bitbucket** | 小团队、与Jira集成 | 免费小团队、与Atlassian生态集成 | 社区支持较少 |

**推荐方案**：
- 如果**开源**：使用 **GitHub**（建立社区、接受PR、增加曝光）
- 如果**闭源**：使用 **Gitee**（国内访问快）或 **GitHub私有仓库**（如果需要国际化团队）

#### Git工作流策略

**推荐：Feature Branch Workflow（功能分支工作流）**

```
main (生产分支)
  │
  ├─ develop (开发分支)
  │    │
  │    ├─ feature/audio-agent (功能分支：音频Agent)
  │    ├─ feature/display-agent (功能分支：显示Agent)
  │    ├─ feature/storage-agent (功能分支：存储Agent)
  │    ├─ feature/main-controller (功能分支：主控制器)
  │    └─ bugfix/xxx (修复分支)
  │
  └─ release/v1.0 (发布分支)
```

**分支命名规范**：
- `main`：生产主干，始终保持可发布状态
- `develop`：开发主干，集成各功能分支
- `feature/xxx`：新功能开发（从develop分支创建）
- `bugfix/xxx`：Bug修复（从develop分支创建）
- `hotfix/xxx`：紧急生产修复（从main分支创建）
- `release/vX.X`：发布准备分支（从develop分支创建）

#### 提交信息规范（Conventional Commits）

**格式**：`<type>(<scope>): <subject>`

**Type类型**：
- `feat`：新功能
- `fix`：Bug修复
- `docs`：文档更新
- `style`：代码格式（不影响功能）
- `refactor`：重构（不是新功能也不是修复）
- `perf`：性能优化
- `test`：测试相关
- `chore`：构建/工具链相关

**示例**：
```bash
feat(audio-agent): 实现MP3解码功能
fix(display-agent): 修复频谱刷新卡顿问题
docs(README): 更新开发环境搭建说明
test(storage-agent): 增加SD卡挂载单元测试
chore(deps): 升级ESP-IDF到v5.1
```

#### Agent间代码集成策略

**每个Agent独立仓库 vs 单一仓库（Monorepo）**：

| 方案 | 优势 | 劣势 | 推荐场景 |
|------|------|--------|---------|
| **单一仓库** | 代码集成简单、统一版本管理、便于重构 | 仓库体积大、权限管理粗糙 | **推荐**：小团队（≤5人） |
| **多仓库** | 权限精细、独立发布、解耦彻底 | 集成测试复杂、版本依赖管理难 | 大团队、Agent完全独立 |

**推荐方案：单一仓库 + 目录隔离**

```
pixel_music_companion/          # 单一仓库
├── main/                       # 主控制器 + HAL
├── components/
│   ├── audio_agent/           # Audio Agent组件
│   ├── display_agent/         # Display Agent组件
│   ├── storage_agent/        # Storage Agent组件
│   └── ...
├── tests/                     # 集成测试
└── docs/                      # 文档
```

**Agent间API变更管理**：
1. 修改Agent间通信接口时，必须更新 `docs/API_REFERENCE.md`
2. 提交PR前，使用 `git diff` 检查是否破坏向后兼容性
3. 重大API变更需要在 `CHANGELOG.md` 中记录

#### CI/CD自动化流程

**⚠️ 必须使用 ESP-IDF 官方 GitHub Action**

**推荐GitHub Actions配置**（`.github/workflows/ci.yml`）：

```yaml
name: ESP32-S3 CI

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  build:
    name: Build and Test
    runs-on: ubuntu-latest
    
    strategy:
      matrix:
        target: [esp32s3]
        
    steps:
      - name: Checkout code
        uses: actions/checkout@v4
        with:
          submodules: recursive  # 拉取子模块（如有）
      
      - name: Setup ESP-IDF
        uses: espressif/setup-esp-idf@v1
        with:
          esp_idf_version: v5.1.2  # 指定IDF版本
          target: ${{ matrix.target }}
      
      - name: Configure project
        run: |
          idf.py set-target ${{ matrix.target }}
          idf.py menuconfig
      
      - name: Build project
        run: idf.py build
      
      - name: Run unit tests
        run: |
          cd build
          ctest --output-on-failure
      
      - name: Check code style
        run: |
          # 使用ESP-IDF自带的代码检查工具
          idf.py check-code-style
      
      - name: Static analysis (Cppcheck)
        run: |
          cppcheck --enable=all --inconclusive --quiet \
            --suppress=missingIncludeSystem \
            main/ components/
      
      - name: Upload firmware artifacts
        if: success()
        uses: actions/upload-artifact@v4
        with:
          name: firmware-${{ matrix.target }}
          path: build/*.bin
  
  # 低功耗测试（可选）
  power-consumption:
    name: Power Consumption Test
    runs-on: ubuntu-latest
    if: github.event_name == 'push' && github.ref == 'refs/heads/main'
    steps:
      - name: Placeholder for power test
        run: echo "Power consumption test to be implemented"
```

**自动化检查项**：
- [ ] 代码编译通过（多平台）
- [ ] 单元测试覆盖率 > 70%
- [ ] 代码格式检查（clang-format）
- [ ] 静态分析（Cppcheck/Clang-Tidy）
- [ ] Agent间API兼容性检查

#### 代码审查（Code Review）规范

**PR合并要求**：
- [ ] 至少1名核心成员Approve
- [ ] 所有CI检查通过
- [ ] 自测通过（提供测试视频或日志）
- [ ] 重大功能需要更新文档

**审查重点**：
1. **架构一致性**：是否遵循多Agent架构设计？
2. **实时性影响**：是否阻塞高优先级任务（音频解码、显示刷新）？
3. **资源管理**：是否有内存泄漏、栈溢出风险？
4. **代码规范性**：命名、注释、错误处理是否到位？

#### 开源协议选择（如果开源）

| 协议 | 允许 | 要求 | 适用场景 |
|------|------|------|---------|
| **MIT** (Recommended) | 商用、修改、私有化 | 保留版权声明 | 希望最大化传播、降低使用门槛 |
| **GPL v3** | 商用、修改 | 开源衍生代码 | 希望所有改进都回馈社区 |
| **Apache 2.0** | 商用、修改、专利授权 | 保留版权声明、注明变更 | 涉及专利、企业级项目 |

**推荐方案**：使用 **MIT协议**（鼓励更多人使用和改进）

#### .gitignore示例

```gitignore
# ESP-IDF构建输出
build/
sdkconfig
sdkconfig.old

# IDE配置
.vscode/
.idea/
*.swp
*.swo

# 日志和临时文件
*.log
*.tmp
.DS_Store

# 敏感信息
config/secrets.h
*.pem
*.key

# 大文件（ SD卡测试文件等）
sd_card/MUSIC/
sd_card/IMAGE/
!.gitkeep
```

#### Git LFS管理大文件

**问题**：像素屏动画素材、音频测试文件可能很大，会导致Git仓库膨胀。

**解决方案**：使用 **Git LFS**（Large File Storage）

```bash
# 安装Git LFS
git lfs install

# 跟踪大文件类型
git lfs track "*.mp3"
git lfs track "*.wav"
git lfs track "*.png"
git lfs track "*.jpg"

# 提交 .gitattributes
git add .gitattributes
```

---

## 13. 风险与缓解措施

## 12. 风险与缓解措施

### 12.1 技术风险

| 风险 | 影响 | 缓解措施 |
|------|------|---------|
| ESP32-S3性能不足，无法同时解码+频谱分析+显示 | 高 | 使用双核并行处理，或降低频谱刷新率 |
| HUB75E驱动复杂，刷新率达不到30fps | 中 | 使用PIO+DMA，或外接FPGA |
| SD卡读取速度慢，导致音频卡顿 | 中 | 实现环形缓冲区，预读取音频数据 |
| 功耗过高，发热严重 | 中 | 优化代码，使用低功耗模式，改进散热设计 |

### 12.2 产品风险

| 风险 | 影响 | 缓解措施 |
|------|------|---------|
| 用户体验不符合情感化预期 | 高 | 早期用户测试，迭代优化交互设计 |
| 成本过高，定价缺乏竞争力 | 中 | 优化BOM成本，考虑分区版本（标准版/高配版） |
| 市场竞争激烈，缺乏差异化 | 中 | 强化情感化定位，打造独特用户体验 |

---

## 14. 附录

### 14.1 参考资料

**乐鑫官方文档**：
- 乐鑫官网：https://www.espressif.com.cn/zh-hans
- ESP32-S3 技术参考手册：https://www.espressif.com.cn/sites/default/files/documentation/esp32-s3_technical_reference_manual_cn.pdf
- ESP-IDF 编程指南：https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32s3/
- ESP-IDF API 参考：https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32s3/api-reference/index.html
- ESP32-S3 数据手册：https://www.espressif.com.cn/sites/default/files/documentation/esp32-s3_datasheet_cn.pdf
- HUB75E 接口协议文档：https://www.espressif.com.cn/zh-hans/support/documents/technical-documents

**第三方资源**：
- FatFS 文件系统设计指南：http://elm-chan.org/fsw/ff/00index_e.html
- FreeRTOS 实时操作系统教程：https://www.freertos.org/Documentation/RTOS_book.html
- Cmake 构建系统指南：https://cmake.org/cmake/help/latest/guide/tutorial/index.html

### 14.2 相关开源项目

| 项目名称 | 描述 | 链接 |
|---------|------|------|
| ESP32-HUB75-MatrixPanel | ESP32驱动HUB75 LED矩阵屏 | https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA |
| ESP32 MP3 Decoder | MP3解码库（Helix） | https://github.com/espressif/esp-adf-libs/tree/master/decoder |
| Adafruit GFX Library | 图形绘制库（可移植） | https://github.com/adafruit/Adafruit-GFX-Library |
| ESP-IDF Examples | ESP-IDF官方示例 | https://github.com/espressif/esp-idf/tree/master/examples |
| PDM microphone ESP32 | 麦克风驱动示例 | https://github.com/espressif/esp-idf/tree/master/examples/peripherals/i2s |

### 14.3 ESP-IDF 版本管理

**推荐ESP-IDF版本**：`v5.1.2`（稳定版）

**版本选择原则**：
1. 使用官方推荐的稳定版本（非最新版本）
2. 所有开发者必须使用相同版本（在`CMakeLists.txt`中指定）
3. 定期关注安全更新，但避免频繁升级

**版本锁定方法**：
```bash
# 在项目根目录指定IDF版本
echo "v5.1.2" > IDF_VERSION

# 在CMakeLists.txt中添加检查
include($ENV{IDF_PATH}/tools/cmake/idf_version.cmake)
idf_check_version("5.1.2")
```

**升级策略**：
- 主版本升级（v4.x → v5.x）：需要充分测试，至少预留2周
- 次版本升级（v5.0 → v5.1）：相对安全，预留1周测试
- 补丁版本升级（v5.1.1 → v5.1.2）：通常安全，可直接升级

---

## 文档变更记录

| 版本 | 日期 | 修改内容 | 修改人 |
|------|------|---------|--------|
| v1.0 | 2026-01-17 | 初始版本 | 产品通 |
| v1.1 | 2026-01-17 | 补充Git代码管理规范（第11.5章） | 产品通 |

---

**⚠️ 最后提醒**：

1. **分时调度是关键**：请确保开发团队使用RTOS，并合理分配任务优先级
