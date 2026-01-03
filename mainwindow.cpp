#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "transferthread.h"
#include "dbmanager.h"
#include <QFileDialog>
#include <QNetworkDatagram>
#include <QMessageBox>
#include <QHostInfo>
#include <QDir>
#include <QCoreApplication> // 获取程序运行目录

const quint16 UDP_PORT = 45454;
const quint16 TCP_START_PORT = 45455;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    ， ui(new Ui::MainWindow)
    ， m_peerModel(new PeerModel(this))
    ， m_udpSocket(new QUdpSocket(this))
    ， m_tcpServer(new QTcpServer(this))
    ， m_myTcpPort(0)
{
    ui->setupUi(this);

    // 设置窗口图标
    this->setWindowIcon(QIcon(":/logo.png"));

    // 1. 数据库初始化
    if(DbManager::instance().init()) {
        log("系统已就绪，历史记录已加载。");
        QStringList history = DbManager::instance().getAllHistory();
        if(!history.isEmpty()) {
            log("--- 最近传输记录 ---");
            for(const QString &h : history) log(h);
            log("--------------------");
        }
    }

    // 2. 设置 Model/View
    ui->listViewPeers->setModel(m_peerModel);

    // 3. 初始化 UDP (自动发现)
    m_udpSocket->bind(UDP_PORT, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    connect(m_udpSocket, &QUdpSocket::readyRead, this, &MainWindow::processPendingDatagrams);

    // 4. 初始化 TCP Server (自动解决端口冲突)
    if(m_tcpServer->listen(QHostAddress::Any, TCP_START_PORT)) {
        m_myTcpPort = TCP_START_PORT;
        log("服务启动成功，监听端口: " + QString::number(m_myTcpPort));
    } else if (m_tcpServer->listen(QHostAddress::Any, TCP_START_PORT + 1)) {
        m_myTcpPort = TCP_START_PORT + 1;
        log("端口自动切换为: " + QString::number(m_myTcpPort));
    } else {
        log("错误: 服务启动失败 (端口被占用)");
    }

    connect(m_tcpServer, &QTcpServer::newConnection, this, &MainWindow::onNewConnection);

    // 启动时自动刷新一次
    on_btnRefresh_clicked();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::log(const QString &msg)
{
    QString time = QDateTime::currentDateTime().toString("[HH:mm:ss] ");
    ui->textEditLog->append(time + msg);
}

// === 按钮 ===
void MainWindow::on_btnAbout_clicked()
{
    QString info = "软件名称：局域网文件快传系统 (LanShare v1.0)\n\n"
                   "开发者信息：\n"
                   "姓名：钟帆\n"
                   "学号：2023414300128\n\n"
                   "基于 Qt 6 C++ Widget 开发。";
    QMessageBox::about(this, "关于软件", info);
}

// === UDP 发现 ===
void MainWindow::on_btnRefresh_clicked()
{
    m_peerModel->clear();
    sendUdpBroadcast();
    log("正在刷新用户列表...");
}

void MainWindow::sendUdpBroadcast()
{
    QString msg = "LAN_SHARE|" + QString::number(m_myTcpPort);
    QByteArray data = msg.toUtf8();
    m_udpSocket->writeDatagram(data, QHostAddress::Broadcast, UDP_PORT);
}

void MainWindow::processPendingDatagrams()
{
    while (m_udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_udpSocket->receiveDatagram();
        QString msg = QString::fromUtf8(datagram.data());

        if (msg.startsWith("LAN_SHARE|")) {
            QStringList parts = msg.split("|");
            if (parts.length() == 2) {
                QString peerPort = parts[1];
                QString senderIp = datagram.senderAddress().toString();
                if(senderIp.startsWith("::ffff:")) senderIp = senderIp.mid(7);

                if (peerPort.toUShort() != m_myTcpPort && !senderIp.isEmpty()) {
                    m_peerModel->addPeer(senderIp + ":" + peerPort);
                }
            }
        }
    }
}

// === 发送文件 ===
void MainWindow::on_btnSelectFile_clicked()
{
    // 支持多选文件
    m_selectedFilePaths = QFileDialog::getOpenFileNames(this, "选择要发送的文件(可多选)");

    if (!m_selectedFilePaths.isEmpty()) {
        QString text = QString("已选 %1 个文件").arg(m_selectedFilePaths.count());
        if(m_selectedFilePaths.count() == 1) {
            text = "已选: " + QFileInfo(m_selectedFilePaths.first()).fileName();
        }
        ui->btnSelectFile->setText(text);
        ui->btnSend->setEnabled(true);
        log("准备发送 " + QString::number(m_selectedFilePaths.count()) + " 个文件...");
    }
}

void MainWindow::on_btnSend_clicked()
{
    QModelIndex index = ui->listViewPeers->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "提示", "请先选择一个接收者！");
        return;
    }

    QString targetString = m_peerModel->getPeerIp(index.row());
    QStringList parts = targetString.split(":");
    if (parts.length() != 2) return;

    QString targetIp = parts[0];
    int targetPort = parts[1].toInt();

    ui->btnSend->setEnabled(false); // 暂时锁定按钮

    // 批量发送逻辑
    for (const QString &filePath : m_selectedFilePaths) {
        log("正在启动发送任务: " + QFileInfo(filePath).fileName() + " -> " + targetIp);

        SendThread *thread = new SendThread(targetIp, targetPort, filePath, this);

        // 进度条只显示最后一个文件的进度（简化处理）
        if (filePath == m_selectedFilePaths.last()) {
            connect(thread, &SendThread::progressUpdated, ui->progressBar, &QProgressBar::setValue);
            connect(thread, &SendThread::finished, this, [this](){
                ui->btnSend->setEnabled(true); // 全部发完（大概率）后恢复按钮
                ui->progressBar->setFormat("发送任务结束");
            });
        }

        connect(thread, &SendThread::logMessage, this, &MainWindow::log);
        connect(thread, &SendThread::finished, thread, &QThread::deleteLater);

        // 记录数据库
        DbManager::instance().addHistory("发送", QFileInfo(filePath).fileName(), targetIp);

        thread->start();

        // 稍微延时，避免瞬间并发过高
        QThread::msleep(200);
    }
}

// === 接收文件 (已修改：保存到 ReceivedFiles 文件夹) ===
void MainWindow::onNewConnection()
{
    QTcpSocket *clientSocket = m_tcpServer->nextPendingConnection();
    connect(clientSocket, &QTcpSocket::readyRead, this, &MainWindow::onReadClient);
    connect(clientSocket, &QTcpSocket::disconnected, clientSocket, &QTcpSocket::deleteLater);
}

void MainWindow::onReadClient()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;

    static qint64 expectedSize = 0;
    static QString fileName = "";
    static QFile *file = nullptr;
    static qint64 receivedSize = 0;

    // 解析头部
    if (receivedSize == 0) {
        if (clientSocket->bytesAvailable() < 4) return;
        QDataStream stream(clientSocket);
        stream >> fileName >> expectedSize;

        // === 修改重点：保存到程序目录下的 ReceivedFiles 文件夹 ===
        QString dirPath = QCoreApplication::applicationDirPath() + "/ReceivedFiles";
        QDir dir(dirPath);
        if (!dir.exists()) {
            dir.mkpath("."); // 如果文件夹不存在，自动创建
        }

        
        QString savePath = dir.filePath(fileName);

        // 自动重命名 (避免覆盖)
        int counter = 1;
        QFileInfo fileInfo(savePath);
        while (fileInfo.exists()) {
            QString newName = fileInfo.baseName() + "_" + QString::number(counter++) + "." + fileInfo.suffix();
            savePath = dir.filePath(newName);
            fileInfo.setFile(savePath);
        }

        file = new QFile(savePath);
        if(!file->open(QIODevice::WriteOnly)) {
            log("错误: 无法创建文件 " + savePath);
            // 失败清理
            delete file; file = nullptr;
            receivedSize = 0; fileName = ""; expectedSize = 0;
            clientSocket->disconnectFromHost();
            return;
        }

        log("正在接收文件: " + fileName);
        log("保存路径: " + savePath); // 在日志里告诉你存在哪了

        ui->progressBar->setValue(0);
        ui->progressBar->setFormat("正在接收... %p%");
    }

    // 写入数据
    if (file && file->isOpen()) {
        QByteArray data = clientSocket->readAll();
        file->write(data);
        receivedSize += data.size();

        if (expectedSize > 0) {
            ui->progressBar->setValue((receivedSize * 100) / expectedSize);
        }

        if (receivedSize >= expectedSize) {
            file->close();
            log("接收成功！");
            ui->progressBar->setFormat("接收成功！100%");

            // 记录数据库
            QString senderIp = clientSocket->peerAddress().toString().replace("::ffff:", "");
            DbManager::instance().addHistory("接收", fileName, senderIp);

            // 清理状态
            delete file; file = nullptr;
            receivedSize = 0; fileName = ""; expectedSize = 0;
            clientSocket->disconnectFromHost();
        }
    }
}





