#ifndef TRANSFERTHREAD_H
#define TRANSFERTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QFile>

// 用于发送文件的线程，避免主界面卡死
class SendThread : public QThread
{
    Q_OBJECT
public:
    SendThread(QString ip, int port, QString filePath, QObject *parent = nullptr);

protected:
    void run() override;

signals:
    void progressUpdated(int percent);
    void logMessage(QString msg);
    void finished(bool success);

private:
    QString m_ip;
    int m_port;
    QString m_filePath;
};

#endif // TRANSFERTHREAD_H
