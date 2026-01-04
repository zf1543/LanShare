#include "peermodel.h"

PeerModel::PeerModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int PeerModel::rowCount(const QModelIndex &) const
{
    return m_peers.size();
}

QVariant PeerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();
    return m_peers.at(index.row());
}

void PeerModel::addPeer(const QString &ip)
{
    if (m_peerSet.contains(ip))
        return;   // 🔴 已存在则忽略

    beginInsertRows(QModelIndex(), m_peers.size(), m_peers.size());
    m_peers.append(ip);
    m_peerSet.insert(ip);
    endInsertRows();
}

QString PeerModel::getPeerIp(int row) const
{
    if (row < 0 || row >= m_peers.size())
        return QString();
    return m_peers.at(row);
}

void PeerModel::clear()
{
    beginResetModel();
    m_peers.clear();
    m_peerSet.clear();
    endResetModel();
}
