#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QTcpServer>
#include "peermodel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

公共:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
// ================== 网络发现（UDP） ==================
// 第一阶段实现：基于 UDP 广播的在线设备发现
// 后续可拆分为独立 DeviceDiscoverer 模块


private slots:
    void on_btnRefresh_clicked();
    void on_btnSelectFile_clicked();
    void on_btnSend_clicked();
    void on_btnAbout_clicked(); // 新增：关于按钮点击事件

    void processPendingDatagrams();
    void onNewConnection();
    void onReadClient();

private:
    Ui::MainWindow *ui;
    PeerModel *m_peerModel;
    QUdpSocket *m_udpSocket;
    QTcpServer *m_tcpServer;
    QStringList m_selectedFilePaths;
    quint16 m_myTcpPort;

    void log(const QString &msg);
    void sendUdpBroadcast();
};
#endif // MAINWINDOW_H

