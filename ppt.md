# Talk-Oneone 学期汇报 PPT 提纲

---

## 第 1 页 — 封面

- **项目名称**: Talk-Oneone — 基于 Qt6/QML 的 AI 虚拟伴侣桌面应用
- **副标题**: 集成大语言模型与语音合成的智能对话系统
- **姓名 / 学号 / 班级 / 日期**

---

## 第 2 页 — 项目背景与目标

- **背景**:
  - 大语言模型（LLM）快速发展，Ollama 等工具让本地部署成为可能
  - 语音合成（TTS）技术成熟，讯飞等云服务提供高质量中文语音
  - 桌面端 AI 助手/伴侣类应用有实际使用场景
- **目标**:
  - 开发一款跨平台桌面 AI 聊天应用，支持**文字对话 + 语音播报**
  - 实现**流式响应**（边生成边显示，体验接近 ChatGPT）
  - 支持**多会话管理**、**AI 人设定制**、**云服务监控**
  - 采用 **MySQL 持久化**，保证数据不丢失
- **适用场景**: 日常陪伴、语言练习、情感倾诉

---

## 第 3 页 — 系统总体架构

- 用一张架构图展示四层结构:

```
┌─────────────────────────────────────────┐
│  QML 表现层 (Main.qml + ChatPage /      │
│  SettingPage / StatePage)               │
├─────────────────────────────────────────┤
│  C++ 业务逻辑层 (Application 单例)       │
│  - 会话管理 / 消息路由 / 状态检测       │
├─────────────────────────────────────────┤
│  C++ 基础设施层                          │
│  ConfigMgr   GateWay    AudioMgr        │
│  (配置)      (HTTP+WS)  (音频播放)      │
│  DbManager   Logger                     │
│  (MySQL)     (日志)                     │
├──────────┬──────────┬───────────────────┤
│ MySQL 8.4│ Ollama   │ 讯飞 TTS 云服务   │
│ (Docker) │ (本地LLM)│ (WebSocket)       │
└──────────┴──────────┴───────────────────┘
```

- 简要说明各层职责和数据流向

---

## 第 4 页 — 技术栈

| 分类 | 技术 | 说明 |
|------|------|------|
| 语言 | C++17 + QML + JavaScript | 前后端分离的 Qt 架构 |
| UI 框架 | Qt 6.8 Quick / Quick Controls 2 | FluentWinUI3 风格 |
| 构建 | CMake 3.16+ | 跨平台构建系统 |
| 数据库 | MySQL 8.4 (Docker) | 三表：config / conversations / messages |
| LLM | Ollama REST API | 本地部署，NDJSON 流式输出 |
| TTS | 讯飞流式 WebSocket API | HMAC-SHA256 鉴权，PCM 16kHz 音频 |
| 音频 | Qt Multimedia (QAudioSink) | PCM 16bit / 16000Hz / 单声道回放 |
| 容器 | Docker Compose | 一键启动 MySQL 服务 |

---

## 第 5 页 — 核心功能一览

| 功能 | 实现 |
|------|------|
| **流式 LLM 聊天** | HTTP POST → NDJSON 逐行解析 → 逐 token 刷新 UI |
| **自动语音播报** | LLM 回复完成后自动触发讯飞 TTS → 本地 PCM 回放 |
| **多会话管理** | UUID 标识会话，侧边栏列表，支持切换/删除 |
| **AI 人设定制** | 自定义 AI 名称、性格描述、系统提示词模板 |
| **发音人定制** | 5 种预设 + 自定义，可调速/音调/音量 |
| **服务状态监控** | Ollama + TTS 实时状态，30 秒自动刷新 |
| **持久化存储** | 全部配置和聊天记录写入 MySQL，重启不丢失 |
| **优雅降级** | 数据库不可用时仍可运行（无持久化），Ollama 离线时给出提示 |

---

## 第 6 页 — 数据库设计

- 展示 init.sql 中的三张表：

```
┌──────────────────────────────────────┐
│ config                               │
│ ──────────────────────────────────   │
│ config_key VARCHAR(128) PK           │
│ config_value TEXT                    │
│ updated_at TIMESTAMP                 │
│                                     │
│ 示例: "AI/aiName" → "雅萱"          │
│       "iFlytek/voiceName" → "xiaoyan"│
└──────────────────────────────────────┘

┌──────────────────────────────────────┐
│ conversations                        │
│ ──────────────────────────────────   │
│ id CHAR(36) PK  (UUID)              │
│ title VARCHAR(255)                   │
│ created_at / updated_at TIMESTAMP    │
└──────────┬───────────────────────────┘
           │ 1:N (CASCADE DELETE)
┌──────────▼───────────────────────────┐
│ messages                             │
│ ──────────────────────────────────   │
│ id BIGINT AUTO_INCREMENT PK          │
│ conversation_id CHAR(36) FK          │
│ role VARCHAR(32)  ('User'/'Assistant')│
│ content TEXT                         │
│ created_at TIMESTAMP(3) (毫秒精度)   │
└──────────────────────────────────────┘
```

- 说明：config 表用 key-value 模式替代传统 QSettings INI 文件；conversations + messages 替代 JSON 文件存储

---

## 第 7 页 — 关键技术实现

**1. LLM 流式响应（NDJSON 解析）**
- Ollama 返回 `{ "message": { "content": "你好" }, "done": false }` 逐行推送
- C++ `QNetworkReply::readyRead` 信号驱动，按 `\n` 拆行解析
- 每收到一个 token 立即通过 `signal_receive_llm_stream` 传递到 QML 刷新 UI

**2. 讯飞 TTS WebSocket 鉴权**
- 生成 RFC 7231 GMT 时间戳 → 构建签名串 → HMAC-SHA256 签名 → Base64 编码 → 拼接 authorization 参数 → 再次 Base64
- WebSocket 连接后发送 JSON 请求帧，接收 Base64 PCM 音频分片

**3. 优雅降级设计**
- `DbManager::openConnection()` 失败时仅 `qWarning()`，返回 false
- 各模块在执行 SQL 前均检查 `isConnected()`，DB 不可用时静默跳过

**4. 信号/槽链式调用**
- `GateWay → Application → AudioMgr → Application → QML` 五段链路清晰可追踪

---

## 第 8 页 — UI 界面展示

- **主聊天页 (ChatPage)**:
  - 欢迎语 + 渐入动画
  - 聊天气泡（用户蓝底白字右侧 / AI 白底黑字左侧）
  - 消息入场滑入动画
  - 底部输入区 + 发送按钮（Enter 发送 / Shift+Enter 换行）
  - LLM 流式打字动画 + TTS 播放状态指示

- **侧边栏 (Drawer)**:
  - 三大导航入口：新聊天 / 个性化定制 / 服务器状态
  - 历史对话列表（高亮当前对话 / 悬停显示删除按钮）

- **设置页 (SettingPage)**:
  - 用户信息 / AI 角色 / 服务器 / 讯飞 TTS 四大卡片
  - 发音人下拉 + 语速/音调/音量滑块 + 测试按钮

- **服务状态页 (StatePage)**:
  - 两个状态卡片，脉冲指示灯，30 秒自动刷新

---

## 第 9 页 — 项目亮点与难点

**亮点:**
- 完整的**桌面端 LLM 应用**，从 UI 到后端到数据库全栈实现
- **流式响应**显著提升用户体验（等待感降低）
- **自动 TTS 串联**：LLM 回复完立即语音播报，无需手动触发
- **优雅降级**：DB 不可用时应用仍可运行
- 代码整洁，**~3400 行**，单例/信号槽/RAII 等 C++ 惯用法应用得当
- Docker 一键部署 MySQL，开发环境简单

**难点攻克:**
- 讯飞 TTS WebSocket 鉴权链路复杂（两次 Base64 + HMAC-SHA256 + RFC 7231 时间戳）
- NDJSON 流式解析需要处理缓冲区跨行/粘包
- Qt6 QML 与 C++ 的类型桥接（QJsonArray ↔ ListModel）
- MySQL 与 Qt6 QMYSQL 驱动的认证兼容（`mysql_native_password`）

---

## 第 10 页 — 总结与展望

**已完成:**
- 文字聊天（流式）+ 语音播报 + 多会话管理 + MySQL 持久化 + 服务监控

**可扩展方向:**
- **语音输入**（ASR 语音识别 → 文本），实现真正的语音对话闭环
- **多模态**（图片理解 / 图片生成）
- **角色扮演增强**：更丰富的 persona 系统（情绪状态、记忆图谱）
- **Web 端 / 移动端**：将 QML 移植到 Qt for Android/iOS 或改用 Web 技术栈
- **云端部署**：Ollama + MySQL 上云，支持多用户
- **国际化**：英文 / 多语言 UI

**致谢**

---

## 附：建议的 PPT 演示流程

| 页码 | 时间 | 要点 |
|------|------|------|
| 1-2 | 1 min | 封面 + 背景 |
| 3-4 | 2 min | 架构图 + 技术栈 |
| 5 | 1 min | 功能一览 |
| 6 | 1 min | 数据库设计 |
| 7 | 2 min | 关键技术（流式LLM + TTS 鉴权 + 降级） |
| 8 | 2 min | UI 截图演示（最好现场跑一下） |
| 9 | 1 min | 亮点与难点 |
| 10 | 1 min | 总结与展望 |
| **合计** | **~11 min** | |
