#include "groupsmodel.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

GroupsModel::GroupsModel(QObject *parent) : QAbstractListModel(parent), m_json("{}")
{

}

QList<AbstractEntry *> GroupsModel::apps()
{
    return m_apps;
}

QStringList GroupsModel::groupsIds()
{
    return m_groups;
}

AppsModel *GroupsModel::groupApps(QString groupId)
{
    if (m_groups.contains(groupId)) {
        AppsModel * groupApps = new AppsModel(m_groupsApps.values(groupId), false, this);
        return groupApps;
    } else
        return NULL;
}

QHash<int, QByteArray> GroupsModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    roles[Qt::DecorationRole] = "decoration";
    roles[Qt::UserRole] = "id";
    return roles;
}

int GroupsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_groups.length();
}

int GroupsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant GroupsModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_groups.length() )
        return QVariant();

    QString groupId = m_groups.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        return m_groupsName.value(groupId);
        break;
    case Qt::UserRole:
        return groupId;

    default:
        return QVariant();
        break;
    }
}

bool GroupsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() < 0 || index.row() >= m_groups.length() )
        return false;

    QString groupId = m_groups.at(index.row());

    if (role == Qt::DisplayRole) {
        m_groupsName.insert(groupId, value.toString());
        dataChanged(index, index);

        updateJson();
        return true;
    }
    return false;
}

QVariant GroupsModel::get(int index) const
{
    if (index < 0 || index >= m_groups.length())
        return QVariant();

    QString groupId = m_groups.at(index);
    QVariantMap group;
    group.insert("id", groupId);
    group.insert("name", m_groupsName.value(groupId));
    for (AbstractEntry* entry : m_groupsApps.values(groupId))
        group.insertMulti("apps", entry->id());

    return group;
}

QStringList GroupsModel::appGroups(QString appId) const
{
    return m_appsGroups.values(appId);
}

QString GroupsModel::groupName(QString groupId) const
{
    return m_groupsName.value(groupId);
}


void GroupsModel::setApps(QList<AbstractEntry *> apps) {
    m_apps = apps;
    emit appsChanged(m_apps);

    refresh();
}

void GroupsModel::refresh() {
    beginResetModel();

    m_groups.clear();
    m_groupsName.clear();
    m_groupsApps.clear();
    m_appsGroups.clear();

    parseJson();

    endResetModel();
}


QString GroupsModel::json() {
    return m_json;
}

void GroupsModel::setJson(QString json) {
    m_json = json;
    emit jsonChanged(m_json);

    refresh();
}

void GroupsModel::newGroup(QString id)
{
    int index = m_groups.indexOf(id);
    if (index == -1) {
        beginInsertRows(QModelIndex(), m_groups.length(), m_groups.length());

        m_groups.append(id);
        m_groupsName.insert(id, "New Group");

        endInsertRows();
    }

    updateJson();
}

void GroupsModel::removeGroup(QString id)
{
    int index = m_groups.indexOf(id);
    if (index != -1) {
        beginRemoveRows(QModelIndex(), index, index);

        m_groups.removeAll(id);
        m_groupsApps.remove(id);
        m_groupsName.remove(id);

        endRemoveRows();
    }

    updateJson();
}

void GroupsModel::addAppToGroup(QString groupId, QString appId)
{
    int index = m_groups.indexOf(groupId);
    if (index != -1) {
        int appIndex = indexOfApp(appId);

        if (appIndex != -1) {
            m_groupsApps.insertMulti(groupId, m_apps.at(appIndex));
            m_appsGroups.insertMulti(appId, groupId);

            dataChanged(createIndex(index, 0), createIndex(index, 0));
        }
    }

    updateJson();
}

void GroupsModel::removeAppFromGroup(QString groupId, QString appId)
{
    int index = m_groups.indexOf(groupId);
    if (index != -1) {
        int i = indexOfApp(appId);

        m_groupsApps.remove(groupId, m_apps.at(i));
        m_appsGroups.remove(appId, groupId);

        dataChanged(createIndex(index, 0), createIndex(index, 0));
    }

    updateJson();
}

void GroupsModel::parseJson()
{
    QJsonDocument groupsDocument = QJsonDocument::fromJson(m_json.toLocal8Bit());
    if (!groupsDocument.isArray()) {
        qWarning() << "Bad formated json";
        return;
    }

    QJsonArray groups = groupsDocument.array();
    for (QJsonValue group : groups) {
        if (!group.isObject()) {
            qWarning() << "Bad formated json";
            continue;
        }

        QJsonObject groupObject = group.toObject();
        QJsonValue groupId = groupObject.value("id");
        QJsonValue groupName = groupObject.value("name");
        QJsonValue groupApps = groupObject.value("apps");

        if (groupId.isString() && groupName.isString()) {
            m_groups.append(groupId.toString());
            m_groupsName.insert(groupId.toString(), groupName.toString());

            if (groupApps.isArray()) {
                QList<AbstractEntry *> apps;
                for (QJsonValue appId : groupApps.toArray()) {
                    if (appId.isString()) {
                        int index = indexOfApp(appId.toString());
                        if (index != -1) {
                            m_appsGroups.insertMulti(m_apps.at(index)->id(), groupId.toString());
                            m_groupsApps.insertMulti(groupId.toString(), m_apps.at(index));
                            apps.append(m_apps.at(index));
                        }
                    }
                }
            }
        }

    }
}

void GroupsModel::updateJson()
{
    QJsonArray groupsRoot;
    for (int i = 0; i < m_groups.length(); i ++ ) {
        QJsonObject group;
        QString groupId = m_groups.at(i);

        group.insert("id", groupId);
        group.insert("name", m_groupsName.value(groupId));

        QJsonArray groupApps;
        for (AbstractEntry * entry : m_groupsApps.values(groupId))
            groupApps.append(entry->id());

        group.insert("apps", groupApps);

        groupsRoot.append(group);
    }

    QJsonDocument groupsDocument(groupsRoot);
    m_json = groupsDocument.toJson();

    emit jsonChanged(m_json);
}

int GroupsModel::indexOfApp(QString appId)
{
    if (m_apps.length() == 0)
        qWarning() << "No apps where add to the groups model.";

    // TODO: improve search method with a binary search
    int i = 0;
    for (AbstractEntry * entry : m_apps) {
        if (entry->id().compare(appId) == 0)
            return i;
        i ++;
    }

    return -1;
}
