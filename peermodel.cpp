#include "peermodel.h"

PeerModel::PeerModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int PeerModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_peers.count();
}

QVariant PeerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole) {
        return m_peers.at(index.row()); // 显示 IP 地址
    }
    return QVariant();
}

void PeerModel::addPeer(const QString &ip)
{
    if (m_peers.contains(ip)) return;

    beginInsertRows(QModelIndex(), m_peers.count(), m_peers.count());
    m_peers.append(ip);
    endInsertRows();
}

QString PeerModel::getPeerIp(int row)
{
    if (row < 0 || row >= m_peers.count()) return "";
    return m_peers.at(row);
}

void PeerModel::clear()
{
    beginResetModel();
    m_peers.clear();
    endResetModel();
}
