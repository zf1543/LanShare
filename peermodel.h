#ifndef PEERMODEL_H
#define PEERMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include <QSet>

class PeerModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit PeerModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void addPeer(const QString &ip);
    QString getPeerIp(int row) const;
    void clear();

private:
    QStringList m_peers;
    QSet<QString> m_peerSet;   // 🔴 用于去重
};

#endif
