#include "dbmanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>
#include <QCoreApplication>

DbManager& DbManager::instance()
{
    static DbManager instance;
    return instance;
}

DbManager::DbManager()
{
}

bool DbManager::init()
{
    
    // 连接 SQLite 数据库
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    // 数据库文件会生成在运行目录
    m_db.setDatabaseName(QCoreApplication::applicationDirPath() + "/transfer_log.db");

    if (!m_db.open()) {
        qDebug() << "Error: connection with database failed";
        return false;
    }

    // 创建表结构
    QSqlQuery query;
    bool success = query.exec("CREATE TABLE IF NOT EXISTS transfer_history ("
                              "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                              "timestamp TEXT, "
                              "type TEXT, "
                              "filename TEXT, "
                              "remote_ip TEXT)");
    return success;
}

void DbManager::addHistory(const QString &type, const QString &filename, const QString &targetIp)
{
    QSqlQuery query;
    query.prepare("INSERT INTO transfer_history (timestamp, type, filename, remote_ip) "
                  "VALUES (:time, :type, :file, :ip)");

    query.bindValue(":time", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    query.bindValue(":type", type);
    query.bindValue(":file", filename);
    query.bindValue(":ip", targetIp);

    if(!query.exec()) {
        qDebug() << "Add history failed: " << query.lastError();
    }
}

QStringList DbManager::getAllHistory()
{
    QStringList list;
    QSqlQuery query("SELECT * FROM transfer_history ORDER BY id DESC LIMIT 50");
    while (query.next()) {
        QString record = QString("[%1] %2: %3 (对方: %4)")
                             。arg(query.value("timestamp").toString())
                             。arg(query.value("type").toString())
                             。arg(query.value("filename").toString())
                             。arg(query.value("remote_ip").toString());
        list.append(record);
    }
    return list;
}

