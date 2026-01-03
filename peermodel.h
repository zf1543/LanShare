#ifndef PEERMODEL_H
#define PEERMODEL_H

#include <QAbstractListModel>
#include <QStringList>

class PeerModel : public QAbstractListModel
{
    Q_OBJECT

公共:
    explicit PeerModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void addPeer(const QString &ip);
    QString getPeerIp(int row);
    void clear();

private:
    QStringList m_peers;
};

#endif // PEERMODEL_H

