#include "accountlistmodel.h"

#include "room.h"

AccountListModel::AccountListModel(QObject* parent)
    : QAbstractListModel(parent) {}

void AccountListModel::setController(Controller* value) {
  if (m_controller != value) {
    beginResetModel();
    m_connections.clear();

    m_controller = value;

    for (auto c : m_controller->connections()) {
      connectConnectionSignals(c);
      m_connections.append(c);
    };

    connect(m_controller, &Controller::connectionAdded, this,
            [=](Connection* conn) {
              if (!conn) {
              }
              beginInsertRows(QModelIndex(), m_connections.count(),
                              m_connections.count());
              connectConnectionSignals(conn);
              m_connections.append(conn);
              endInsertRows();
            });
    connect(m_controller, &Controller::connectionDropped, this,
            [=](Connection* conn) {
              qDebug() << "Dropping connection" << conn->userId();
              if (!conn) {
                qDebug() << "Trying to remove null connection";
                return;
              }
              conn->disconnect(this);
              const auto it =
                  std::find(m_connections.begin(), m_connections.end(), conn);
              if (it == m_connections.end())
                return;  // Already deleted, nothing to do
              const int row = it - m_connections.begin();
              beginRemoveRows(QModelIndex(), row, row);
              m_connections.erase(it);
              endRemoveRows();
            });
    emit controllerChanged();
  }
}

QVariant AccountListModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) return QVariant();

  if (index.row() >= m_connections.count()) {
    qDebug() << "AccountListModel, something's wrong: index.row() >= "
                "m_users.count()";
    return QVariant();
  }
  auto m_connection = m_connections.at(index.row());
  if (role == NameRole) {
    return m_connection->user()->displayname();
  }
  if (role == AccountIDRole) {
    return m_connection->user()->id();
  }
  if (role == AvatarRole) {
    if (!m_connection->user()->avatarUrl().isEmpty())
      return m_connection->user()->avatar(64);
    return QImage();
  }
  if (role == ConnectionRole) {
    return QVariant::fromValue(m_connection);
  }

  return QVariant();
}

int AccountListModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) return 0;

  return m_connections.count();
}

void AccountListModel::connectConnectionSignals(Connection* conn) {
  connect(conn->user(), &User::avatarChanged, this, [=] {
    const auto it = std::find(m_connections.begin(), m_connections.end(), conn);
    if (it == m_connections.end()) {
      return;
    }
    const auto idx = index(it - m_connections.begin());
    emit dataChanged(idx, idx, {AvatarRole});
  });
}

QHash<int, QByteArray> AccountListModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[NameRole] = "name";
  roles[AccountIDRole] = "accountID";
  roles[AvatarRole] = "avatar";
  roles[ConnectionRole] = "connection";
  return roles;
}
