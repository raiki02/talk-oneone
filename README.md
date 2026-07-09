# Talk-Oneone (你的女孩)

一个基于 Qt 6 QML 开发的 AI 虚拟伴侣桌面应用程序，集成了大语言模型 (LLM) 和语音合成 (TTS) 功能，支持文字聊天和语音对话两种交互模式。

## 功能特性

### 核心功能
- **文字聊天**: 与 AI 进行文字对话，支持实时流式响应
- **语音合成**: AI 通过讯飞在线 TTS 自动朗读回复内容
- **对话历史**: 自动保存对话记录，支持多会话管理
- **上下文记忆**: 同一对话中 AI 能够记住之前的对话内容，实现连贯的长对话
- **个性化定制**: 可自定义 AI 角色名称、性格、系统提示词等

### 语音合成 (TTS)
- 基于讯飞在线流式 TTS API
- 支持多种预设发音人（青年女声、萝莉、磁性男声、东北女声、童声）
- 支持自定义发音人名称
- 可调节语速、音调、音量

### 服务状态监控
- 实时监控 Ollama、讯飞 TTS 服务状态
- 可视化服务状态面板

### 界面特性
- 现代化 UI 设计，简洁美观
- 响应式布局，自适应窗口大小
- 平滑动画效果，提升用户体验
- 侧边栏历史对话管理

## 技术架构

### 前端
- **Qt 6.8+** - 跨平台 GUI 框架
- **QML** - 声明式 UI 开发
- **Qt Quick Controls 2** - 现代化控件库

### 后端服务
- **Ollama** - 本地大语言模型服务 (默认端口: 11434)
  - 支持 Qwen3、Llama3 等多种模型
  - 支持对话历史上下文传递
- **讯飞 TTS** - 云端语音合成服务
  - WebSocket 流式传输
  - HMAC-SHA256 鉴权

### 通信协议
- **WebSocket** - 讯飞 TTS 流式音频传输
- **HTTP REST API** - LLM 服务调用

## 项目结构

```
Talk-Oneone/
├── main.cpp              # 应用程序入口
├── Main.qml              # 主界面
├── application.cpp/h     # 应用程序核心逻辑
│                         # - 对话历史管理
│                         # - 服务状态检测
├── audiomgr.cpp/h        # 音频管理
│                         # - TTS 音频播放
├── configmgr.cpp/h       # 配置管理 (单例模式)
│                         # - 用户/AI 角色设置
│                         # - 服务器地址配置
│                         # - 讯飞 TTS 凭据
├── gateway.cpp/h         # 网络通信网关
│                         # - HTTP LLM 流式请求
│                         # - WebSocket TTS 连接
├── CMakeLists.txt        # CMake 构建配置
├── res.qrc               # Qt 资源文件
├── Components/           # QML 组件
│   ├── ChatMessageListModel.qml  # 聊天消息模型
│   ├── HistoryListModel.qml      # 历史对话模型
│   ├── MessageBubble.qml         # 消息气泡组件
│   └── SideButton.qml            # 侧边栏按钮
├── Pages/                # QML 页面
│   ├── ChatPage.qml      # 文字聊天页面
│   ├── VoiceChatPage.qml # 语音对话页面
│   ├── SettingPage.qml   # 设置页面
│   └── StatePage.qml     # 服务状态页面
├── js/                   # JavaScript 工具函数
│   └── addListModel.js
└── img/icon/             # 图标资源
```

## 环境要求

### 开发环境
- Qt 6.8 或更高版本
- CMake 3.16+
- MSVC 2022 (Windows) 或 GCC/Clang (Linux/macOS)

### 运行时依赖
- **Ollama** - 本地 LLM 服务

## 构建说明

### Windows (MSVC)

```bash
# 创建构建目录
mkdir build
cd build

# 配置项目 (确保 Qt 在 PATH 中)
cmake .. -G "Ninja" -DCMAKE_PREFIX_PATH="C:/Qt/6.10.0/msvc2022_64"

# 编译
cmake --build .
```

### 使用 Qt Creator

1. 打开 `CMakeLists.txt` 作为项目文件
2. 选择合适的 Qt Kit (Qt 6.8+ MSVC 2022 64-bit)
3. 点击构建并运行

## 配置说明

### 默认配置

| 配置项 | 默认值 |
|--------|--------|
| 用户名称 | 用户 |
| AI 名称 | 雅萱 |
| LLM 服务地址 | http://10.242.115.43:11434/api/chat |
| LLM 模型 | qwen3.5:0.8b |
| TTS 发音人 | xiaoyan (青年女声) |

### 自定义配置

所有配置项均可在应用程序的"个性化定制"页面中修改，包括：

- **用户信息**: 用户名称、用户描述
- **AI 角色**: AI 名称、性格描述、系统提示词
- **服务器配置**: LLM 服务地址、模型名称
- **讯飞 TTS**: APPID、APIKey、APISecret、发音人、语速/音调/音量

## 后端服务部署

### Ollama

```bash
# 安装 Ollama
# Windows: 从 https://ollama.ai 下载安装程序

# 下载并运行模型
ollama run qwen3.5:0.8b
```

### 讯飞 TTS

1. 注册讯飞开放平台账号 (console.xfyun.cn)
2. 创建应用，获取 APPID、APIKey、APISecret
3. 在应用的"个性化定制"→"讯飞 TTS 设置"中填入凭据

## 使用说明

1. **启动后端服务**: 确保 Ollama 服务已启动
2. **运行应用程序**: 启动 Talk-Oneone 应用
3. **检查服务状态**: 在"服务器状态"页面确认 Ollama 在线
4. **配置 TTS**: 在"个性化定制"页面填入讯飞 TTS 凭据
5. **开始对话**: 
   - **文字聊天**: 在首页输入框中输入文字，按回车或点击发送
   - **语音对话**: 点击侧边栏"语音对话"，输入文字后 AI 会用语音回复

### 使用技巧

- **上下文记忆**: 在同一对话中，AI 会记住之前的对话内容
- **新建对话**: 点击顶部"+"按钮创建新对话，清空上下文重新开始
- **历史对话**: 在侧边栏可以查看和切换历史对话
- **发音人切换**: 在语音对话页面可快速切换讯飞 TTS 发音人

## 开发者信息

- **组织**: BeyondESH
- **域名**: beyondesh.top
- **应用名称**: Talk-Oneone

<img src="https://1.beyondesh.top/wechat.jpg" style="zoom: 50%;" />

## 许可证

本项目仅供学习和研究使用，无开发者本人许可禁止商用

## 更新日志

### v0.3 (2025-12)
-  **变更**: 移除 FunASR/CosyVoice 本地 ASR/TTS 服务，改用讯飞在线 TTS
-  **变更**: 移除麦克风录音和语音识别功能
-  **优化**: TTS 使用讯飞 WebSocket 流式接口，延迟更低
-  **优化**: 对话上下文记忆功能
-  **修复**: WebSocket 连接使用 127.0.0.1 替代 localhost，解决 IPv6 兼容性问题

### v0.1 (初始版本)
- 实现文字聊天功能
- 实现语音对话功能
- 添加服务状态监控
- 添加个性化设置功能
