#include "application.h"
#include "configmgr.h"
#include "dbmanager.h"
#include <QDateTime>
#include <QUuid>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

Application::Application(QObject *parent)
    : QObject{parent}
    ,_audioMgr(new AudioMgr(this))
    ,_gateWay(new GateWay(this))
{
    // TTS 完成 → 播放 → 播放完成
    connect(_gateWay,&GateWay::signal_tts_finished,_audioMgr,&AudioMgr::slot_tts_finished);
    connect(_audioMgr,&AudioMgr::signal_tts_playback_finished,this,&Application::signal_tts_playback_finished);

    // LLM 流式回复信号
    auto accumulated = std::make_shared<QString>();
    connect(_gateWay, &GateWay::signal_receive_llm_stream, this, [this, accumulated](const QString &delta, bool isDone) {
        if (isDone) {
            if (!accumulated->isEmpty()) {
                addMessageToHistory("Assistant", *accumulated);
                *accumulated = QString();
            }
            emit signal_receive_llm_stream("", true);
        } else {
            *accumulated += delta;
            emit signal_receive_llm_stream(delta, false);
        }
    });

    // LLM 回复信号，添加到历史并转发
    connect(_gateWay,&GateWay::signal_receive_llm,this,[this](const QString &content){
        addMessageToHistory("Assistant", content);
        emit signal_receive_llm(content);
    });

    // 启动时自动检测 Ollama 状态
    checkOllamaStatus();
}

Application::~Application()
{
    saveCurrentConversation();
}

void Application::initializeFromDatabase()
{
    loadConversationList();
}

void Application::sendChatMessage(QString text)
{
    addMessageToHistory("User", text);
    QJsonArray historyWithoutLast = _chatHistory;
    if (!historyWithoutLast.isEmpty()) {
        historyWithoutLast.removeLast();
    }
    _gateWay->sendllmMessage(text, ReqId::CHAT_LLM, historyWithoutLast);
}

// 历史对话管理
void Application::createNewConversation()
{
    saveCurrentConversation();

    _currentConversationId = generateConversationId();
    _chatHistory = QJsonArray();

    QJsonObject newConv;
    newConv["id"] = _currentConversationId;
    newConv["title"] = "新对话";
    newConv["createTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    newConv["updateTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    _conversationList.prepend(newConv);

    // Persist empty conversation row to DB
    if (DbManager::instance().isConnected()) {
        QSqlDatabase db = DbManager::instance().database();
        QSqlQuery query(db);
        query.prepare("INSERT INTO conversations (id, title) VALUES (?, ?)");
        query.addBindValue(_currentConversationId);
        query.addBindValue("新对话");
        if (!query.exec()) {
            qWarning() << "Application::createNewConversation: insert failed:" << query.lastError().text();
        }
    }

    emit currentConversationIdChanged();
    emit chatHistoryChanged();
    emit conversationListChanged();
}

void Application::loadConversation(const QString &conversationId)
{
    saveCurrentConversation();

    if (!DbManager::instance().isConnected()) {
        _chatHistory = QJsonArray();
    } else {
        QSqlDatabase db = DbManager::instance().database();
        QSqlQuery query(db);
        query.prepare("SELECT role, content, created_at FROM messages "
                      "WHERE conversation_id = ? ORDER BY created_at ASC");
        query.addBindValue(conversationId);

        QJsonArray history;
        if (query.exec()) {
            while (query.next()) {
                QJsonObject msg;
                msg["role"]      = query.value(0).toString();
                msg["content"]   = query.value(1).toString();
                msg["timestamp"] = query.value(2).toDateTime().toString(Qt::ISODate);
                history.append(msg);
            }
        } else {
            qWarning() << "Application::loadConversation: query failed:" << query.lastError().text();
        }
        _chatHistory = history;
    }

    _currentConversationId = conversationId;
    emit currentConversationIdChanged();
    emit chatHistoryChanged();
}

void Application::deleteConversation(const QString &conversationId)
{
    if (DbManager::instance().isConnected()) {
        QSqlDatabase db = DbManager::instance().database();
        QSqlQuery query(db);
        query.prepare("DELETE FROM conversations WHERE id = ?");
        query.addBindValue(conversationId);
        if (!query.exec()) {
            qWarning() << "Application::deleteConversation: delete failed:" << query.lastError().text();
        }
    }

    loadConversationList();

    if (_currentConversationId == conversationId) {
        if (_conversationList.isEmpty()) {
            createNewConversation();
        } else {
            loadConversation(_conversationList[0].toObject()["id"].toString());
        }
    }

    emit conversationListChanged();
}

void Application::addMessageToHistory(const QString &role, const QString &content)
{
    if (_currentConversationId.isEmpty()) {
        createNewConversation();
    }

    QJsonObject msg;
    msg["role"] = role;
    msg["content"] = content;
    msg["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    _chatHistory.append(msg);

    // Update conversation title from first user message
    if (_chatHistory.size() == 1 && role == "User") {
        for (int i = 0; i < _conversationList.size(); ++i) {
            QJsonObject conv = _conversationList[i].toObject();
            if (conv["id"].toString() == _currentConversationId) {
                QString title = content.left(20);
                if (content.length() > 20) title += "...";
                conv["title"] = title;
                conv["updateTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
                _conversationList[i] = conv;
                break;
            }
        }
        emit conversationListChanged();
    }

    emit chatHistoryChanged();
    saveCurrentConversation();
}

void Application::checkOllamaStatus()
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QUrl llmBaseUrl(ConfigMgr::instance().llmServerUrl());
    QUrl statusUrl;
    statusUrl.setScheme(llmBaseUrl.scheme());
    statusUrl.setHost(llmBaseUrl.host());
    statusUrl.setPort(llmBaseUrl.port());
    statusUrl.setPath("/api/tags");
    QNetworkRequest request(statusUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, manager]() {
        bool online = (reply->error() == QNetworkReply::NoError);
        _ollamaOnline = online;
        QString info = online ? "Ollama 服务正常运行" : reply->errorString();

        if (online) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonArray models = doc.object()["models"].toArray();
            info = QString("已加载 %1 个模型").arg(models.size());
        }

        emit ollamaStatusChanged(online, info);
        reply->deleteLater();
        manager->deleteLater();
    });
}

void Application::checkTtsStatus()
{
    bool hasCredentials = !ConfigMgr::instance().iflytekAppId().isEmpty()
                       && !ConfigMgr::instance().iflytekApiKey().isEmpty()
                       && !ConfigMgr::instance().iflytekApiSecret().isEmpty();
    _ttsOnline = hasCredentials;
    QString info = hasCredentials ? "讯飞 TTS 服务已配置" : "讯飞 TTS 未配置凭据";
    emit ttsStatusChanged(hasCredentials, info);
}

QJsonArray Application::chatHistory() const
{
    return _chatHistory;
}

QJsonArray Application::conversationList() const
{
    return _conversationList;
}

QString Application::currentConversationId() const
{
    return _currentConversationId;
}

void Application::setCurrentConversationId(const QString &id)
{
    if (_currentConversationId != id) {
        loadConversation(id);
    }
}

void Application::saveCurrentConversation()
{
    if (_currentConversationId.isEmpty() || _chatHistory.isEmpty()) {
        return;
    }

    if (!DbManager::instance().isConnected()) {
        qWarning() << "Application::saveCurrentConversation: DB not connected, skipping save.";
        return;
    }

    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);

    db.transaction();

    // Delete existing messages for this conversation (full replace)
    query.prepare("DELETE FROM messages WHERE conversation_id = ?");
    query.addBindValue(_currentConversationId);
    query.exec();

    // Insert all current messages
    query.prepare("INSERT INTO messages (conversation_id, role, content, created_at) "
                  "VALUES (?, ?, ?, ?)");
    for (const QJsonValue &val : _chatHistory) {
        QJsonObject msg = val.toObject();
        query.addBindValue(_currentConversationId);
        query.addBindValue(msg["role"].toString());
        query.addBindValue(msg["content"].toString());
        query.addBindValue(msg["timestamp"].toString());
        if (!query.exec()) {
            qWarning() << "Application::saveCurrentConversation: insert message failed:" << query.lastError().text();
        }
    }

    // Upsert conversation metadata (title from in-memory list)
    QString title = "新对话";
    for (const QJsonValue &val : _conversationList) {
        QJsonObject conv = val.toObject();
        if (conv["id"].toString() == _currentConversationId) {
            title = conv["title"].toString();
            break;
        }
    }
    query.prepare("INSERT INTO conversations (id, title) VALUES (?, ?) "
                  "ON DUPLICATE KEY UPDATE title = VALUES(title), updated_at = CURRENT_TIMESTAMP");
    query.addBindValue(_currentConversationId);
    query.addBindValue(title);
    if (!query.exec()) {
        qWarning() << "Application::saveCurrentConversation: upsert conversation failed:" << query.lastError().text();
    }

    db.commit();
}

void Application::loadConversationList()
{
    if (!DbManager::instance().isConnected()) {
        _conversationList = QJsonArray();
        return;
    }

    QSqlDatabase db = DbManager::instance().database();
    QSqlQuery query(db);
    query.prepare("SELECT id, title, created_at, updated_at FROM conversations ORDER BY updated_at DESC");

    QJsonArray list;
    if (query.exec()) {
        while (query.next()) {
            QJsonObject conv;
            conv["id"]         = query.value(0).toString();
            conv["title"]      = query.value(1).toString();
            conv["createTime"] = query.value(2).toDateTime().toString(Qt::ISODate);
            conv["updateTime"] = query.value(3).toDateTime().toString(Qt::ISODate);
            list.append(conv);
        }
    } else {
        qWarning() << "Application::loadConversationList: query failed:" << query.lastError().text();
    }
    _conversationList = list;
}

QString Application::generateConversationId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void Application::testTts()
{
    _gateWay->sendttsMessage("你好，我是你的AI助手，很高兴认识你！");
}
