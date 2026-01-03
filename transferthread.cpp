#include "transferthread.h"
#include <QFileInfo>
#include <QDataStream>
#include <QNetworkProxy> 

SendThread::SendThread(QString ip, int port, QString filePath, QObject *parent)
    : QThread(parent), m_ip(ip), m_port(port), m_filePath(filePath)
{
}

void SendThread::run()
{
    QTcpSocket socket;

    // 禁用系统代理，防止连本地IP时报错 "Proxy type is invalid"
    socket.setProxy(QNetworkProxy::NoProxy);

    socket.connectToHost(m_ip, m_port);

    if (!socket.waitForConnected(3000)) {
        emit logMessage("连接失败: " + socket.errorString());
        emit finished(false);
        return;
    }

    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit logMessage("错误: 无法读取文件");
        emit finished(false);
        return;
    }

    QFileInfo fileInfo(m_filePath);
    QString fileName = fileInfo.fileName();
    qint64 fileSize = fileInfo.size();

    QByteArray header;
    QDataStream stream(&header, QIODevice::WriteOnly);
    stream << fileName << fileSize;

    socket.write(header);
    socket.waitForBytesWritten();

    emit logMessage(QString("开始发送: %1").arg(fileName));

    qint64 totalSent = 0;
    const qint64 bufferSize = 4096;

    while (!file.atEnd()) {
        QByteArray buffer = file.read(bufferSize);
        socket.write(buffer);
        socket.waitForBytesWritten();

        totalSent += buffer.size();
        int percent = (totalSent * 100) / fileSize;
        emit progressUpdated(percent);
    }

    socket.disconnectFromHost();
    socket.waitForDisconnected();
    文件。close();

    emit logMessage("发送完毕！");
    emit finished(true);
}


