#include "gateway.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>
#include <QTimer>
#include <QCryptographicHash>
#include <QDateTime>
#include <QMessageAuthenticationCode>
#include <QLocale>
#include "configmgr.h"

GateWay::GateWay(QObject *parent)
    : QObject{parent}
    ,_networkAccessMgr(new QNetworkAccessManager(this))
{
}

GateWay::~GateWay()
{
    if (_ttsWebSocket != nullptr) {
        _ttsWebSocket->abort();
        _ttsWebSocket->deleteLater();
        _ttsWebSocket = nullptr;
    }
    if (_ttsTimeoutTimer != nullptr) {
        _ttsTimeoutTimer->stop();
        _ttsTimeoutTimer->deleteLater();
        _ttsTimeoutTimer = nullptr;
    }
}

void GateWay::postStream(const QUrl &url, const QByteArray &data, ReqId id)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply* reply = _networkAccessMgr->post(request, data);

    auto contentBuffer = std::make_shared<QString>();
    auto accumulated = std::make_shared<QString>();

    connect(reply, &QNetworkReply::readyRead, this, [this, reply, contentBuffer, accumulated]() {
        QByteArray chunk = reply->readAll();
        *contentBuffer += QString::fromUtf8(chunk);

        while (true) {
            int newlinePos = contentBuffer->indexOf('\n');
            if (newlinePos == -1) break;

            QString line = contentBuffer->left(newlinePos).trimmed();
            *contentBuffer = contentBuffer->mid(newlinePos + 1);

            if (line.isEmpty()) continue;

            QJsonDocument jsonDoc = QJsonDocument::fromJson(line.toUtf8());
            if (jsonDoc.isNull()) continue;

            QJsonObject jsonObj = jsonDoc.object();
            if (jsonObj.contains("done") && jsonObj["done"].toBool()) {
                qDebug().noquote() << "流式LLM完成，总长度:" << accumulated->length();
                return;
            }

            QString delta = jsonObj["message"].toObject()["content"].toString();
            if (!delta.isEmpty()) {
                *accumulated += delta;
                emit signal_receive_llm_stream(delta, false);
            }
        }
    });

    connect(reply, &QNetworkReply::finished, this, [this, reply, contentBuffer, accumulated, id]() {
        if (!contentBuffer->isEmpty()) {
            qDebug() << "postStream finished: 缓冲区残留数据:" << *contentBuffer;
            QStringList lines = contentBuffer->split('\n', Qt::SkipEmptyParts);
            for (const QString &line : lines) {
                QString trimmedLine = line.trimmed();
                if (trimmedLine.isEmpty()) continue;
                QJsonDocument jsonDoc = QJsonDocument::fromJson(trimmedLine.toUtf8());
                if (jsonDoc.isNull()) continue;
                QJsonObject jsonObj = jsonDoc.object();
                QString delta = jsonObj["message"].toObject()["content"].toString();
                if (!delta.isEmpty()) {
                    *accumulated += delta;
                    emit signal_receive_llm_stream(delta, false);
                }
            }
        }

        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "流式LLM网络错误:" << reply->errorString()
                       << "HTTP状态码:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
                       << "响应体:" << reply->readAll();
        }

        qDebug() << "postStream finished: accumulated长度=" << accumulated->length()
                 << "id=" << id << "error=" << reply->error();

        emit signal_receive_llm_stream("", true);

        if (!accumulated->isEmpty()) {
            qDebug() << "发送TTS请求，文本长度:" << accumulated->length();
            sendttsMessage(*accumulated);
        } else {
            qWarning() << "LLM流式回复为空，跳过TTS请求！"
                       << "网络错误:" << reply->errorString()
                       << "HTTP状态码:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
                       << "请求URL:" << reply->url().toString();
        }

        reply->deleteLater();
    });
}

void GateWay::sendllmMessage(const QString &text, ReqId id, const QJsonArray& chatHistory)
{
    ConfigMgr &config = ConfigMgr::instance();

    QJsonObject rootObj;
    rootObj["model"] = config.llmModelName();
    rootObj["stream"] = true;
    rootObj["think"] = false;

    QJsonArray messageArray;

    QString systemPrompt = config.buildFullSystemPrompt();
    if (!systemPrompt.isEmpty()) {
        QJsonObject systemMsgObj;
        systemMsgObj["role"] = "system";
        systemMsgObj["content"] = systemPrompt;
        messageArray.append(systemMsgObj);
    }

    for (const QJsonValue &msg : chatHistory) {
        QJsonObject historyMsg = msg.toObject();
        QJsonObject msgObj;
        QString role = historyMsg["role"].toString();
        if (role == "User") {
            msgObj["role"] = "user";
        } else if (role == "Assistant") {
            msgObj["role"] = "assistant";
        } else {
            continue;
        }
        msgObj["content"] = historyMsg["content"].toString();
        messageArray.append(msgObj);
    }

    QJsonObject messageObj;
    messageObj["role"] = "user";
    messageObj["content"] = text;
    messageArray.append(messageObj);

    rootObj["messages"] = messageArray;
    QJsonDocument jsonDoc(rootObj);
    QByteArray data = jsonDoc.toJson(QJsonDocument::Indented);

    qDebug() << "发送LLM请求（流式），消息数量:" << messageArray.size();
    postStream(QUrl(config.llmServerUrl()), data, id);
}


void GateWay::sendttsMessage(const QString &text)
{
    ConfigMgr &config = ConfigMgr::instance();

    const QString appId = config.iflytekAppId();
    const QString apiKey = config.iflytekApiKey();
    const QString apiSecret = config.iflytekApiSecret();
    const QString voiceName = config.iflytekVoiceName();

    if (appId.isEmpty() || apiKey.isEmpty() || apiSecret.isEmpty()) {
        qWarning() << "讯飞 TTS 凭证未配置，跳过TTS请求";
        emit signal_tts_finished(QByteArray());
        return;
    }

    if (_ttsWebSocket) {
        _ttsWebSocket->abort();
        _ttsWebSocket->deleteLater();
        _ttsWebSocket = nullptr;
    }
    if (_ttsTimeoutTimer) {
        _ttsTimeoutTimer->stop();
        _ttsTimeoutTimer->deleteLater();
        _ttsTimeoutTimer = nullptr;
    }
    _ttsAudioBuffer.clear();
    _ttsCompleted = false;

    QDateTime nowUtc = QDateTime::currentDateTimeUtc();
    QLocale enUS(QLocale::English, QLocale::UnitedStates);
    QString dateStr = enUS.toString(nowUtc, "ddd, dd MMM yyyy HH:mm:ss") + " GMT";

    const QString host = "tts-api.xfyun.cn";
    QString signatureRaw = QString("host: %1\ndate: %2\nGET /v2/tts HTTP/1.1")
                               .arg(host, dateStr);

    QByteArray hmac = QMessageAuthenticationCode::hash(
        signatureRaw.toUtf8(), apiSecret.toUtf8(), QCryptographicHash::Sha256);
    QString signature = QString::fromUtf8(hmac.toBase64());

    QString authRaw = QString(
        "api_key=\"%1\", algorithm=\"hmac-sha256\", "
        "headers=\"host date request-line\", signature=\"%2\"")
                          .arg(apiKey, signature);

    QString authorization = QString::fromUtf8(authRaw.toUtf8().toBase64());
    QString encodedDate = QString::fromUtf8(QUrl::toPercentEncoding(dateStr));

    QUrl wsUrl(QString("wss://%1/v2/tts").arg(host));
    wsUrl.setQuery(QString("authorization=%1&date=%2&host=%3")
                       .arg(authorization, encodedDate, host));

    qDebug() << "讯飞TTS WebSocket 连接中... 发音人:" << voiceName
             << "文本长度:" << text.length();

    _ttsWebSocket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);

    auto onError = [this]() {
        if (!_ttsCompleted) {
            _ttsCompleted = true;
            qWarning() << "讯飞TTS 失败，通知 UI 恢复";
            emit signal_tts_finished(QByteArray());
        }
    };

    _ttsTimeoutTimer = new QTimer(this);
    _ttsTimeoutTimer->setSingleShot(true);
    connect(_ttsTimeoutTimer, &QTimer::timeout, this, [this, onError]() {
        qWarning() << "讯飞TTS 连接超时（10秒）";
        if (_ttsWebSocket) {
            _ttsWebSocket->abort();
        }
        onError();
    });
    _ttsTimeoutTimer->start(10000);

    connect(_ttsWebSocket, &QWebSocket::connected, this,
            [this, appId, voiceName, text]() {
        _ttsTimeoutTimer->stop();

        qDebug() << "讯飞TTS WebSocket 已连接，发送合成请求";

        QJsonObject business;
        business["aue"] = "raw";
        business["auf"] = "audio/L16;rate=16000";
        business["vcn"] = voiceName;
        business["speed"] = ConfigMgr::instance().iflytekSpeed();
        business["volume"] = ConfigMgr::instance().iflytekVolume();
        business["pitch"] = ConfigMgr::instance().iflytekPitch();
        business["tte"] = "UTF8";

        QJsonObject common;
        common["app_id"] = appId;

        QJsonObject data;
        data["text"] = QString::fromUtf8(text.toUtf8().toBase64());
        data["status"] = 2;

        QJsonObject root;
        root["common"] = common;
        root["business"] = business;
        root["data"] = data;

        QJsonDocument doc(root);
        _ttsWebSocket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
    });

    connect(_ttsWebSocket, &QWebSocket::textMessageReceived, this,
            [this, onError](const QString &message) {
        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
        if (doc.isNull()) {
            qWarning() << "讯飞TTS 收到非法JSON:" << message.left(200);
            return;
        }

        QJsonObject obj = doc.object();
        int code = obj["code"].toInt(-1);
        int status = obj["data"].toObject()["status"].toInt(0);

        if (code != 0) {
            qWarning() << "讯飞TTS API 错误: code=" << code
                       << "message=" << obj["message"].toString()
                       << "sid=" << obj["sid"].toString();
            onError();
            _ttsWebSocket->close();
            return;
        }

        QString audioB64 = obj["data"].toObject()["audio"].toString();
        if (!audioB64.isEmpty()) {
            _ttsAudioBuffer.append(QByteArray::fromBase64(audioB64.toUtf8()));
        }

        if (status == 2) {
            if (!_ttsCompleted && !_ttsAudioBuffer.isEmpty()) {
                _ttsCompleted = true;
                qDebug() << "讯飞TTS 合成完成，总音频大小:" << _ttsAudioBuffer.size();
                emit signal_tts_finished(_ttsAudioBuffer);
            } else if (!_ttsCompleted) {
                onError();
            }
            _ttsWebSocket->close();
        }
    });

    connect(_ttsWebSocket, &QWebSocket::errorOccurred, this, onError);

    connect(_ttsWebSocket, &QWebSocket::disconnected, this, [this, onError]() {
        qDebug() << "讯飞TTS WebSocket 断开";
        onError();
        _ttsWebSocket->deleteLater();
        _ttsWebSocket = nullptr;
    });

    _ttsWebSocket->open(wsUrl);
}

void GateWay::finishTtsWithError()
{
    if (!_ttsCompleted) {
        _ttsCompleted = true;
        emit signal_tts_finished(QByteArray());
    }
}
