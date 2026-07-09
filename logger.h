#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QMutex>

class Logger : public QObject
{
    Q_OBJECT
public:
    static Logger& instance();
    void init(const QString &logDir);

private:
    Logger() = default;
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    static void messageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg);

    QFile _logFile;
    QTextStream _stream;
    QMutex _mutex;
};

#endif // LOGGER_H
