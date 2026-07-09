#include "dbmanager.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

DbManager& DbManager::instance()
{
    static DbManager inst;
    return inst;
}

DbManager::DbManager()
    : _connectionName(CONNECTION_NAME)
{
}

DbManager::~DbManager()
{
    closeConnection();
}

bool DbManager::openConnection()
{
    if (_connected) return true;

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", _connectionName);
    db.setHostName(DB_HOST);
    db.setPort(DB_PORT);
    db.setDatabaseName(DB_NAME);
    db.setUserName(DB_USER);
    db.setPassword(DB_PASS);
    db.setConnectOptions("MYSQL_OPT_CONNECT_TIMEOUT=3");

    if (db.open()) {
        _connected = true;
        qDebug() << "DbManager: MySQL connected successfully to" << DB_HOST << ":" << DB_PORT;
        emit connectedChanged();
        return true;
    }

    qWarning() << "DbManager: Failed to connect to MySQL:" << db.lastError().text();
    qWarning() << "DbManager: App will run without database persistence.";
    QSqlDatabase::removeDatabase(_connectionName);
    _connected = false;
    return false;
}

void DbManager::closeConnection()
{
    if (!_connected) return;

    {
        QSqlDatabase db = QSqlDatabase::database(_connectionName, false);
        if (db.isOpen()) {
            db.close();
        }
    }
    QSqlDatabase::removeDatabase(_connectionName);
    _connected = false;
    emit connectedChanged();
}

QSqlDatabase DbManager::database() const
{
    return QSqlDatabase::database(_connectionName, false);
}
