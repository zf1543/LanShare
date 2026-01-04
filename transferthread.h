#ifndef TRANSFERTHREAD_H
#define TRANSFERTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QFile>

// 用于发送文件的线程，避免主界面卡死
class SendThread : public QThread
{
    Q_OBJECT
公共:
    SendThread(QString ip, int port, QString filePath, QObject *parent = nullptr);

protected:
    void run() override;

signals:
    void progressUpdated(int percent);
    void logMessage(QString msg);
    void finished(bool success);

private:
    QString m_ip;         ///< 目标主机IP地址
    int m_port;           ///< 目标主机端口号
    QString m_filePath;   ///< 待发送文件的本地绝对路径
};

#endif // TRANSFERTHREAD_H

