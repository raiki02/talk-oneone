#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include "audiomgr.h"
#include "gateway.h"

class Application : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QJsonArray chatHistory READ chatHistory NOTIFY chatHistoryChanged)
    Q_PROPERTY(QJsonArray conversationList READ conversationList NOTIFY conversationListChanged)
    Q_PROPERTY(QString currentConversationId READ currentConversationId WRITE setCurrentConversationId NOTIFY currentConversationIdChanged)
    Q_PROPERTY(bool ollamaOnline READ ollamaOnline NOTIFY ollamaStatusChanged)
    Q_PROPERTY(bool ttsOnline READ ttsOnline NOTIFY ttsStatusChanged)

public:
    explicit Application(QObject *parent = nullptr);
    ~Application();
    Q_INVOKABLE void sendChatMessage(QString text);

    // 历史对话管理
    Q_INVOKABLE void createNewConversation();
    Q_INVOKABLE void loadConversation(const QString &conversationId);
    Q_INVOKABLE void deleteConversation(const QString &conversationId);
    Q_INVOKABLE void addMessageToHistory(const QString &role, const QString &content);

    // 从数据库加载对话列表（main.cpp 在 DB 连接后调用）
    void initializeFromDatabase();

    // 服务器状态检测
    Q_INVOKABLE void checkOllamaStatus();
    Q_INVOKABLE void checkTtsStatus();

    // 讯飞 TTS 测试
    Q_INVOKABLE void testTts();

    QJsonArray chatHistory() const;
    QJsonArray conversationList() const;
    QString currentConversationId() const;
    void setCurrentConversationId(const QString &id);

    bool ollamaOnline() const { return _ollamaOnline; }
    bool ttsOnline() const { return _ttsOnline; }

signals:
    void signal_receive_llm(const QString &content);
    void signal_receive_llm_stream(const QString &delta, bool isDone);
    void signal_tts_playback_finished();  // TTS播放完成信号
    void chatHistoryChanged();
    void conversationListChanged();
    void currentConversationIdChanged();
    void ollamaStatusChanged(bool online, const QString &info);
    void ttsStatusChanged(bool online, const QString &info);

private:
    AudioMgr* _audioMgr;
    GateWay* _gateWay;

    // 历史对话
    QJsonArray _chatHistory;
    QJsonArray _conversationList;
    QString _currentConversationId;

    bool _ollamaOnline = false;
    bool _ttsOnline = false;

    void saveCurrentConversation();
    void loadConversationList();
    QString generateConversationId();
};

#endif // APPLICATION_H
