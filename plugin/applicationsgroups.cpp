#include "applicationsgroups.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include <QDebug>

ApplicationsGroups::ApplicationsGroups(QObject *parent) : QObject(parent)
{
}

QString ApplicationsGroups::getApplicationGroupId(QString applicationId)
{
    int i = m_groupIdGroupIndex.value(applicationId, -1);
    if (i == -1)
        return QString();
    else
        return m_groupsIds[i];
}

int ApplicationsGroups::groupsCount()
{
    return m_groupsIds.length();
}

QString ApplicationsGroups::groupId(int index)
{
    if (index >= 0 && index < m_groupsIds.length())
        return m_groupsIds[index];
    else
        return QString();
}

QString ApplicationsGroups::groupNameByIndex(int index)
{
    if (index >= 0 && index < m_groupsNames.length())
        return m_groupsNames[index];
    else
        return QString();

}

QString ApplicationsGroups::groupNameById(QString id)
{
    int i = m_groupIdGroupIndex.value(id, -1);
    return groupNameByIndex(i);
}

QString ApplicationsGroups::applicationGroupId(QString applicationId)
{
    int i = m_applicationIdGroupIndex.value(applicationId, -1);
    if (i == -1)
        return QString();
    else
        return m_groupsIds[i];
}

QStringList ApplicationsGroups::groupIds()
{
    return m_groupsIds;
}

QStringList ApplicationsGroups::groupApplications(QString groupId)
{
    QStringList applicationsIds;
    int groupIndex = m_groupsIds.indexOf(groupId);
    if (groupIndex == -1)
        return applicationsIds;

    for (QString applicationId : m_applicationIdGroupIndex.keys()) {
        if (m_applicationIdGroupIndex.value(applicationId) == groupIndex)
            applicationsIds << applicationId;
    }

    return applicationsIds;
}

QString ApplicationsGroups::rawRelations() const
{
    return m_rawRelations;
}

QString ApplicationsGroups::rawGroupInfo() const
{
    return m_rawGroupInfo;
}

void ApplicationsGroups::setRawRelations(QString relations)
{
    if (m_rawRelations == relations)
        return;

    m_rawRelations = relations;
    refreshCache();

    emit rawRelationsChanged(relations);
}

void ApplicationsGroups::setRawGroupInfo(QString rawGroupInfo)
{
    if (m_rawGroupInfo == rawGroupInfo)
        return;

    m_rawGroupInfo = rawGroupInfo;
    refreshCache();

    emit rawGroupInfoChanged(rawGroupInfo);
}

void ApplicationsGroups::refreshCache()
{
    m_applicationIdGroupIndex.clear();
    m_groupIdGroupIndex.clear();
    m_groupsIds.clear();
    m_groupsNames.clear();

    parseRawGroupInfo();
    qDebug() << "Parsed groups " << m_groupsIds;

    parseRelations();
    qDebug() << "Parsed relation " << m_applicationIdGroupIndex.keys();
}

bool ApplicationsGroups::parseRawGroupInfo()
{
    QJsonParseError error;
    QJsonDocument groupsDocument = QJsonDocument::fromJson(m_rawGroupInfo.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError) {
        if (!groupsDocument.isObject())
            return false;

        QJsonObject groupsObject = groupsDocument.object();
        for (QString groupId : groupsObject.keys()) {
            QJsonValue groupValue = groupsObject.value(groupId);
            if (!groupValue.isObject())
                continue;

            QJsonObject group = groupValue.toObject();
            if (group.contains("display")) {
                QString display = group.value("display").toString();

                m_groupIdGroupIndex.insert(groupId, m_groupsIds.length());

                m_groupsIds << groupId;
                m_groupsNames << display;
            }
        }
    } else {
        qWarning() << error.errorString();
        return false;
    }
    return true;
}

bool ApplicationsGroups::parseRelations()
{
    QJsonParseError error;
    QJsonDocument relationsDocument = QJsonDocument::fromJson(m_rawRelations.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError) {
        if (!relationsDocument.isObject())
            return false;

        QJsonObject relationObject = relationsDocument.object();
        for (QString applicationId : relationObject.keys()) {
            QJsonValue relationValue = relationObject.value(applicationId);
            QString groupId = relationValue.toString();

            int groupIndex = m_groupIdGroupIndex.value(groupId, -1);
            if (groupIndex != -1) {
                m_applicationIdGroupIndex.insert(applicationId, groupIndex);
            } else {
                qWarning() << "Referencing non existent group id: " <<
                              groupId << " while reading group settings.";
            }

        }
    } else {
        qWarning() << error.errorString();
        return false;
    }
    return true;
}

