# 架构设计

## Agent 划分

```
┌─────────────────────────────────────┐
│         Main Controller             │
│  (状态机、按键映射、模式切换)         │
└─────────────────────────────────────┘
                  │
    ┌─────────────┼─────────────┐
    │             │             │
┌───▼────┐  ┌─────▼─────┐  ┌────▼─────┐
│ Audio  │  │  Display  │  │ Storage  │
│ Agent  │  │   Agent   │  │  Agent   │
└───┬────┘  └─────┬─────┘  └────┬─────┘
    │             │             │
    └─────────────┼─────────────┘
                  │
        ┌─────────▼─────────┐
        │ Hardware Abstraction │
        │  (HAL: buttons, mic, │
        │   audio out, power)   │
        └─────────────────────┘
```

## 任务与优先级

| 任务 | 优先级 | 核心 | 周期 | 说明 |
|------|--------|------|------|------|
| audio_task | 12 | 1 | 实时 | 解码 + I2S 输出 |
| display_task | 10 | 1 | 33 ms | 渲染 + 刷屏 |
| controller_task | 8 | 0 | 事件驱动 | 状态机 + 消息分发 |
| button_scan | 5 | 0 | 20 ms | 按键采样与消抖 |
| app_main | 1 | 0 | - | 初始化 + 心跳 |

## 消息总线

所有 Agent 通过同一个 FreeRTOS Queue 交换事件：

```c
typedef struct {
    message_type_t type;
    int32_t param;
    void *data;
    size_t data_len;
} message_t;
```

主要消息类型：

- `MSG_BUTTON_*`：按键事件。
- `MSG_PLAY_NEXT` / `MSG_PLAY_PREV` / `MSG_PLAY_PAUSE`：播放控制。
- `MSG_VOLUME_UP` / `MSG_VOLUME_DOWN`：音量调节。
- `MSG_NEW_SPECTRUM_DATA`：频谱数据已更新。
- `MSG_STORAGE_SCAN_DONE`：SD 卡扫描完成。
- `MSG_AUDIO_TRACK_STARTED` / `MSG_AUDIO_TRACK_FINISHED`：曲目生命周期。

## 数据流

1. 用户短按右键 → `hal_buttons` 发送 `MSG_PLAY_NEXT`。
2. `main_controller` 接收后转发给 `audio_agent`。
3. `audio_agent` 从 `storage_agent` 获取下一曲路径，通过 `audio_decoder` 解码。
4. PCM 数据通过 `hal_audio_output` 输出，并喂给 `spectrum_analyzer`。
5. `audio_agent` 发送 `MSG_NEW_SPECTRUM_DATA`。
6. `display_agent` 读取频谱数据，调用 `ui_animations` 渲染到 HUB75E 帧缓冲。

## 扩展指南

- 新增解码器：在 `components/audio_decoder/` 实现 `audio_decoder_ops_t` 并在 `audio_decoder_open()` 中根据扩展名分发。
- 新增频谱样式：在 `display_agent.c` 的 `render_music_screen()` 中增加样式切换逻辑。
- 新增 Agent：创建独立源文件，向 `message_broker` 注册消息处理。
