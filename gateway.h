#ifndef GATEWAY_H
#define GATEWAY_H

#include <QObject>
#include <QWebSocket>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QJsonArray>

enum ReqId{
    CHAT_LLM
};

enum ErrorCode{
    ERROR_NETWORK,
    SUCCESS,
};

class GateWay : public QObject
{
    Q_OBJECT
public:
    explicit GateWay(QObject *parent = nullptr);
    ~GateWay();
    void sendllmMessage(const QString& text, ReqId id, const QJsonArray& chatHistory = QJsonArray());
    void postStream(const QUrl &url, const QByteArray &data, ReqId id);
    void sendttsMessage(const QString& text);
signals:
    void signal_tts_finished(const QByteArray &data);
    void signal_receive_llm(const QString &context);
    void signal_receive_llm_stream(const QString &delta, bool isDone);
private:
    QNetworkAccessManager* _networkAccessMgr;

    QWebSocket* _ttsWebSocket = nullptr;
    QByteArray _ttsAudioBuffer;
    QTimer* _ttsTimeoutTimer = nullptr;
    bool _ttsCompleted = false;
    void finishTtsWithError();
};

#endif // GATEWAY_H
