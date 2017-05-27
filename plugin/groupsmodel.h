#ifndef GROUPSMODEL_H
#define GROUPSMODEL_H


#include <QMap>
#include <QMultiMap>
#include <QList>
#include <QObject>
#include <QString>
#include <QAbstractListModel>

#include "abstractentry.h"
#include "appsmodel.h"

class GroupsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString json READ json WRITE setJson NOTIFY jsonChanged)
    Q_PROPERTY(QList<AbstractEntry *> apps READ apps WRITE setApps NOTIFY appsChanged)

public:
    explicit GroupsModel(QObject *parent = 0);

    QString json();
    QList<AbstractEntry *> apps();
    QStringList groupsIds();
    Q_INVOKABLE AppsModel* groupApps(QString groupId);


    virtual QHash<int,QByteArray> roleNames() const Q_DECL_OVERRIDE;

    virtual int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
    virtual int columnCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;

    Q_INVOKABLE QVariant get(int index) const;
    Q_INVOKABLE QStringList appGroups(QString appId) const;
    Q_INVOKABLE QString groupName(QString groupId) const;

Q_SIGNALS:
    void jsonChanged(QString json);
    void appsChanged(QList<AbstractEntry *> apps);

public Q_SLOTS:
    void refresh();
    void setApps(QList<AbstractEntry *> apps);
    void setJson(QString json);

    void newGroup(QString id);
    void removeGroup(QString id);
    void addAppToGroup(QString groupId, QString appId);
    void removeAppFromGroup(QString groupId, QString appId);

private:
    void parseJson();
    void updateJson();
    int indexOfApp(QString appId);
    QString m_json;
    QList<AbstractEntry *> m_apps;

    QStringList m_groups; // groupsId
    QMultiMap<QString, AbstractEntry *> m_groupsApps; // groupId, groupMembers
    QMap<QString, QString> m_groupsName; // groupId, groupName
    QMultiMap <QString, QString> m_appsGroups; // appId, groupId;

    QMap<QString, AppsModel*> m_groupAppsModel;
};

#endif // GROUPSMODEL_H
