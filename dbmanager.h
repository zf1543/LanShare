#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QString>
#include <QSqlDatabase>

class DbManager
{
公共:
    static DbManager& instance();
    bool init();
    void addHistory(const QString &type, const QString &filename, const QString &targetIp);
    QStringList getAllHistory();

private:
// 禁用拷贝构造和赋值运算符，防止单例被拷贝
    DbManager();
    QSqlDatabase m_db;
};

#endif // DBMANAGER_H


