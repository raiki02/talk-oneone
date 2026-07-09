#include "logger.h"
#include <QDateTime>
#include <QDir>
#include <iostream>

Logger &Logger::instance()
{
    static Logger inst;
    return inst;
}

Logger::~Logger()
{
    if (_logFile.isOpen()) {
        _mutex.lock();
        _stream << "\n=== 日志结束 " << QDateTime::currentDateTime().toString(Qt::ISODate) << " ===\n";
        _stream.flush();
        _mutex.unlock();
        _logFile.close();
    }
}

void Logger::init(const QString &logDir)
{
    QDir dir;
    if (!dir.exists(logDir)) {
        dir.mkpath(logDir);
    }

    QString logPath = logDir + "/talk_oneone_"
                      + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".log";
    _logFile.setFileName(logPath);
    if (_logFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        _stream.setDevice(&_logFile);
        _stream << "=== Talk-Oneone 日志 " << QDateTime::currentDateTime().toString(Qt::ISODate) << " ===\n";
        _stream.flush();
    }

    // 注册全局消息处理器
    qInstallMessageHandler(messageHandler);

    qDebug() << "日志系统初始化完成，日志文件:" << logPath;
}

void Logger::messageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg)
{
    Logger &logger = instance();
    QMutexLocker locker(&logger._mutex);

    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString level;
    switch (type) {
    case QtDebugMsg:    level = "DEBUG"; break;
    case QtWarningMsg:  level = "WARN "; break;
    case QtCriticalMsg: level = "CRIT "; break;
    case QtFatalMsg:    level = "FATAL"; break;
    default:            level = "INFO "; break;
    }

    // 文件名只保留最后一段
    QString file = QString::fromUtf8(ctx.file);
    int lastSlash = file.lastIndexOf('/');
    if (lastSlash >= 0) file = file.mid(lastSlash + 1);

    QString line = QString("[%1] [%2] %3:%4 %5\n")
                       .arg(timestamp, level, file)
                       .arg(ctx.line)
                       .arg(msg);

    // 写入文件
    if (logger._stream.device()) {
        logger._stream << line;
        logger._stream.flush();
    }

    // 同时输出到 stderr（终端可见）
    std::cerr << line.toStdString() << std::flush;

    if (type == QtFatalMsg) {
        abort();
    }
}
