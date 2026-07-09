#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QSqlDatabase>

class DbManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged FINAL)

public:
    static DbManager& instance();

    bool isConnected() const { return _connected; }

    bool openConnection();
    void closeConnection();
    QSqlDatabase database() const;

signals:
    void connectedChanged();

private:
    DbManager();
    ~DbManager();
    DbManager(const DbManager&) = delete;
    DbManager& operator=(const DbManager&) = delete;

    bool _connected = false;
    QString _connectionName;

    static constexpr const char* DB_HOST = "127.0.0.1";
    static constexpr int   DB_PORT = 3306;
    static constexpr const char* DB_NAME = "talkoneone";
    static constexpr const char* DB_USER = "user";
    static constexpr const char* DB_PASS = "user_pass";
    static constexpr const char* CONNECTION_NAME = "talk_oneone_mysql";
};

#endif // DBMANAGER_H
