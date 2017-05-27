#ifndef APPLICATIONSGROUPS_H
#define APPLICATIONSGROUPS_H

#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>

/***
  * Provides utility funcitions to read the groups information settings.
  * The settings are provided in JSON format.
  */
class ApplicationsGroups : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString rawRelations READ rawRelations WRITE setRawRelations NOTIFY rawRelationsChanged)
    Q_PROPERTY(QString rawGroupInfo READ rawGroupInfo WRITE setRawGroupInfo NOTIFY rawGroupInfoChanged)
public:
    explicit ApplicationsGroups(QObject *parent = 0);

    Q_INVOKABLE QString getApplicationGroupId(QString applicationId);

    Q_INVOKABLE int groupsCount();
    Q_INVOKABLE int newGroup(QString name);
    Q_INVOKABLE bool removeGroup(int index);
    Q_INVOKABLE QString groupId(int index);
    Q_INVOKABLE QString groupNameByIndex(int index);
    Q_INVOKABLE void setGroupName(int index, QString name);
    Q_INVOKABLE QString groupNameById(QString id);
    Q_INVOKABLE QString applicationGroupId(QString applicationId);
    Q_INVOKABLE QStringList groupIds();
    Q_INVOKABLE QStringList groupApplications(QString groupId);
    Q_INVOKABLE bool addApplicationToGroup(QString applicationId, QString groupId);
    Q_INVOKABLE bool removeApplicationFromGroup(QString applicationId, QString groupId);


    QString rawRelations() const;
    QString rawGroupInfo() const;

public Q_SLOTS:
    void setRawRelations(QString rawRelations);
    void setRawGroupInfo(QString rawGroupInfo);

Q_SIGNALS:
    void rawRelationsChanged(QString rawRelations);
    void rawGroupInfoChanged(QString rawGroupInfo);

private:
    void refreshCache();
    bool parseRawGroupInfo();
    bool parseRawRelations();

    void refreshRawData();

    QString m_rawRelations;
    QString m_rawGroupInfo;

    QStringList m_groupsIds;
    QStringList m_groupsNames;
    QMap<QString, int> m_groupIdGroupIndex;
    QMap<QString, int> m_applicationIdGroupIndex;

};

#endif // APPLICATIONSGROUPS_H
