#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QIcon>
#include <QQmlContext>
#include <QDir>
#include "application.h"
#include "configmgr.h"
#include "logger.h"
#include "dbmanager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/img/icon/favicon.ico"));
    app.setApplicationName("Talk-Oneone");

    // 初始化日志系统（写入项目根目录的 log 目录下）
    Logger::instance().init(QDir::currentPath() + "/log");

    // 初始化数据库连接（在业务单例之前，确保 ConfigMgr 构造时能读到 DB 配置）
    DbManager::instance().openConnection();

    QQmlApplicationEngine engine;

    Application application(&app);
    application.initializeFromDatabase();

    engine.rootContext()->setContextProperty("app", &application);
    engine.rootContext()->setContextProperty("configMgr", &ConfigMgr::instance());

    QQuickStyle::setStyle("FluentWinUI3");
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("Talk-Oneone", "Main");

    return app.exec();
}
