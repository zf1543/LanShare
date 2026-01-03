#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QString>
#include <QSqlDatabase>

// 这是一个单例类，专门用来管理数据库
class DbManager
{
public:
    static DbManager& instance();
    bool init();
    void addHistory(const QString &type, const QString &filename, const QString &targetIp);
    QStringList getAllHistory();

private:
    DbManager();
    QSqlDatabase m_db;
};

#endif // DBMANAGER_H
