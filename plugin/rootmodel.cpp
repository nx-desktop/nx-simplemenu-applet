/***************************************************************************
 *   Copyright (C) 2014-2015 by Eike Hein <hein@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "rootmodel.h"
#include "actionlist.h"
#include "favoritesmodel.h"
//#include "recentappsmodel.h"
//#include "recentdocsmodel.h"
//#include "recentcontactsmodel.h"
#include "systemmodel.h"

#include <QCollator>
#include <QDebug>

#include <KLocalizedString>

GroupEntry::GroupEntry(AppsModel *parentModel, const QString &name,
    const QString &iconName, AbstractModel *childModel)
: AbstractGroupEntry(parentModel)
, m_name(name)
, m_iconName(iconName)
, m_childModel(childModel)
{
    // FIXME TODO HACK
    if (childModel != parentModel->favoritesModel()) {
        QObject::connect(parentModel, &RootModel::cleared, childModel, &AbstractModel::deleteLater);
    }

    QObject::connect(childModel, &AbstractModel::countChanged,
        [parentModel, this] { if (parentModel) { parentModel->entryChanged(this); } }
    );
}

QString GroupEntry::name() const
{
    return m_name;
}

QIcon GroupEntry::icon() const
{
    return QIcon::fromTheme(m_iconName, QIcon::fromTheme("unknown"));
}

bool GroupEntry::hasChildren() const
{
    return m_childModel && m_childModel->count() > 0;
}

AbstractModel *GroupEntry::childModel() const
{
    return m_childModel;
}

RootModel::RootModel(QObject *parent) : AppsModel(QString(), parent)
, m_favorites(new FavoritesModel(this))
, m_systemModel(nullptr)
, m_showAllSubtree(true)
, m_showRecentApps(false)
, m_showRecentDocs(false)
, m_showRecentContacts(false)
, m_recentAppsModel(0)
, m_recentDocsModel(0)
, m_recentContactsModel(0)
, m_appletInterface(0)
, m_allAppsModel(0)
, m_groupsModel(0)
{
    connect(&m_groupsModel, &GroupsModel::jsonChanged, this, &RootModel::refresh);
}

RootModel::~RootModel()
{
}

QVariant RootModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_entryList.count()) {
        return QVariant();
    }

    if (role == Kicker::HasActionListRole || role == Kicker::ActionListRole) {
        const AbstractEntry *entry = m_entryList.at(index.row());

        if (entry->type() == AbstractEntry::GroupType) {
            /*
            const GroupEntry *group = static_cast<const GroupEntry *>(entry);
            AbstractModel *model = group->childModel();

            if (model == m_recentAppsModel
                || model == m_recentDocsModel
                || model == m_recentContactsModel) {
                if (role ==  Kicker::HasActionListRole) {
                    return true;
                } else if (role == Kicker::ActionListRole) {
                    QVariantList actionList;
                    actionList << model->actions();
                    actionList << Kicker::createSeparatorActionItem();
                    actionList << Kicker::createActionItem(i18n("Hide %1",
                        group->name()), "hideCategory");
                    return actionList;
                }
            }
            */
        }
    }

    return AppsModel::data(index, role);
}

bool RootModel::trigger(int row, const QString& actionId, const QVariant& argument)
{
    const AbstractEntry *entry = m_entryList.at(row);

    if (entry->type() == AbstractEntry::GroupType) {
        /* if (actionId == "hideCategory") {
            AbstractModel *model = entry->childModel();

            if (model == m_recentAppsModel) {
                setShowRecentApps(false);

                return true;
            } else if (model == m_recentDocsModel) {
                setShowRecentDocs(false);

                return true;
            } else if (model == m_recentContactsModel) {
                setShowRecentContacts(false);

                return true;
            }
        } else */ if (entry->childModel()->hasActions()) {
            return entry->childModel()->trigger(-1, actionId, QVariant());
        }
    }

    return AppsModel::trigger(row, actionId, argument);
}

bool RootModel::showRecentApps() const
{
    return m_showRecentApps;
}

void RootModel::setShowRecentApps(bool show)
{
    if (show != m_showRecentApps) {
        m_showRecentApps = show;

        refresh();

        emit showRecentAppsChanged();
    }
}

bool RootModel::showAllSubtree() const
{
    return m_showAllSubtree;
}

void RootModel::setShowAllSubtree(bool show)
{
    if (m_showAllSubtree != show) {
        m_showAllSubtree = show;

        refresh();

        emit showAllSubtreeChanged();
    }
}

bool RootModel::showRecentDocs() const
{
    return m_showRecentDocs;
}

void RootModel::setShowRecentDocs(bool show)
{
    if (show != m_showRecentDocs) {
        m_showRecentDocs = show;

        refresh();

        emit showRecentDocsChanged();
    }
}

bool RootModel::showRecentContacts() const
{
    return m_showRecentContacts;
}

void RootModel::setShowRecentContacts(bool show)
{
    if (show != m_showRecentContacts) {
        m_showRecentContacts = show;

        refresh();

        emit showRecentContactsChanged();
    }
}

QObject* RootModel::appletInterface() const
{
    return m_appletInterface;
}

void RootModel::setAppletInterface(QObject* appletInterface)
{
    if (m_appletInterface != appletInterface) {
        m_appletInterface = appletInterface;

        refresh();

        emit appletInterfaceChanged();
    }
}

AbstractModel* RootModel::favoritesModel()
{
    return m_favorites;
}

AbstractModel* RootModel::systemFavoritesModel()
{
    if (m_systemModel) {
        return m_systemModel->favoritesModel();
    }

    return nullptr;
}

QObject *RootModel::allAppsModel() const
{
    return m_allAppsModel;
}

GroupsModel *RootModel::groupsModel()
{
    return &m_groupsModel;
}

ApplicationsGroups *RootModel::applicationsGroups()
{
    return &m_applicationsGroupsCache;
}

void RootModel::refresh()
{
    if (!m_appletInterface) {
        return;
    }

    beginResetModel();

    AppsModel::refreshInternal();

    m_recentAppsModel = nullptr;
    m_recentDocsModel = nullptr;
    m_recentContactsModel = nullptr;

    if (m_showAllSubtree) {
        QHash<QString, AbstractEntry *> appsHash;

        foreach (const AbstractEntry *groupEntry, m_entryList) {
            AbstractModel *model = groupEntry->childModel();

            for (int i = 0; i < model->count(); ++i) {
                GroupEntry *subGroupEntry = static_cast<GroupEntry*>(model->index(i, 0).internalPointer());
                AbstractModel *subModel = subGroupEntry->childModel();

                for (int i = 0; i < subModel->count(); ++i) {
                    AbstractEntry *abstractEntry = static_cast<AbstractEntry *>(subModel->index(i, 0).internalPointer());
                    if (abstractEntry->type() == AbstractEntry::RunnableType) {
                        AppEntry *appEntry = static_cast<AppEntry*>(abstractEntry);

                        if (appEntry->name().isEmpty()) {
                            continue;
                        }
                        appsHash.insert(appEntry->service()->menuId(), appEntry);
                    }

                    if (abstractEntry->type() == AbstractEntry::GroupType) {
                        GroupEntry *groupEntry = static_cast<GroupEntry *>(abstractEntry);

                        if (groupEntry->name().isEmpty()) {
                            continue;
                        }
                        appsHash.insert(groupEntry->id(), groupEntry);
                    }
                }
            }
        }

        AppsModel *allModel = nullptr;
        allModel = new AppsModel(appsHash.values(), false, this);
        allModel->setDescription(QStringLiteral("KICKER_ALL_UNPAGED_MODEL")); // Intentionally no i18n.

        m_allAppsModel = allModel;
        emit allAppsModelChanged(m_allAppsModel);

        QList<AbstractEntry *> pages = pageEntries(appsHash.values());

        AppsModel *allPagedModel = nullptr;
        allPagedModel = new AppsModel(pages, true, this);
        allPagedModel->setDescription(QStringLiteral("KICKER_ALL_MODEL")); // Intentionally no i18n.

        if (allPagedModel) {
            m_entryList.prepend(new GroupEntry(this, i18n("All Applications"), QString(), allPagedModel));
        }

        AppsModel *allGRoupedModel = nullptr;
        QList<AbstractEntry *> groupedEntries = groupEntries(appsHash.values());

        QList<AbstractEntry *> pagedGroupedEntries = pageEntries(groupedEntries);

        allGRoupedModel = new AppsModel(pagedGroupedEntries, true, this);
        allGRoupedModel->setDescription(QStringLiteral("KICKER_ALL_GROUPED_MODEL")); // Intentionally no i18n.

        if (allGRoupedModel) {
            m_entryList.prepend(new GroupEntry(this, i18n("All Applications"), QString(), allGRoupedModel));
        }
    }

    if (m_showSeparators) {
        m_entryList.prepend(new SeparatorEntry(this));
        ++m_separatorCount;
    }


/*
    if (m_showRecentContacts) {
        m_recentContactsModel = new RecentContactsModel(this);
        m_entryList.prepend(new GroupEntry(this, i18n("Recent Contacts"), QString(), m_recentContactsModel));
    }

    if (m_showRecentDocs) {
        m_recentDocsModel = new RecentDocsModel(this);
        m_entryList.prepend(new GroupEntry(this, i18n("Recent Documents"), QString(), m_recentDocsModel));
    }

    if (m_showRecentApps) {
        m_recentAppsModel = new RecentAppsModel(this);
        m_entryList.prepend(new GroupEntry(this, i18n("Recent Applications"), QString(), m_recentAppsModel));
    }
*/

    m_systemModel = new SystemModel(this);

    endResetModel();

    m_favorites->refresh();

    emit systemFavoritesModelChanged();
    emit countChanged();
    emit separatorCountChanged();

    emit refreshed();
}

QList<AbstractEntry *> RootModel::groupEntries(QList<AbstractEntry *> entries)
{
    m_groupsModel.setApps(entries);

    QList<AbstractEntry *> groupsList;
    QList<AbstractEntry *> groupedEntryList;
    QHash<QString, QList<AbstractEntry *>> groups;

    for (QString groupId : m_groupsModel.groupsIds())
        groups.insert(groupId, QList<AbstractEntry *>());

    for (AbstractEntry *entry : entries) {
        QStringList groupsId = m_groupsModel.appGroups(entry->id());

        bool grouped = false;
        for (QString groupId : groupsId) {
            if (!groupId.isEmpty() && groups.contains(groupId)) {
                QList<AbstractEntry *> groupEntries = groups.value(groupId);
                groupEntries.append(entry);

                groups.insert(groupId, groupEntries);
                grouped = true;
            }
        }
        if (!grouped)
            groupedEntryList.append(entry);
    }


    for (QString key : groups.keys()) {
        qDebug() << "group key " << key;
        QList<AbstractEntry *> groupEntries = groups[key];

        QString groupName = m_groupsModel.groupName(key);

        AppsModel *model = new AppsModel(groupEntries, false, this);
        GroupEntry * groupEntry = new GroupEntry(this, groupName, QString(""), model);

        groupsList.append(groupEntry);
        if (groups[key].length() > 0) {
            groupedEntryList.append(groupEntry);
        }
    }

    sortEntries(groupedEntryList);

    sortEntries(groupsList);

    emit groupsModelChanged(&m_groupsModel);

    return groupedEntryList;
}

QList<AbstractEntry *> RootModel::pageEntries(QList<AbstractEntry *> entries)
{
    sortEntries(entries);

    QList<AbstractEntry *> pages;

    int at = 0;
    QList<AbstractEntry *> page;

    foreach(AbstractEntry *entry, entries) {
        page.append(entry);

        if (at == 23) {
            at = 0;
            AppsModel *model = new AppsModel(page, false, this);
            pages.append(new GroupEntry(this, QString(), QString(), model));
            page.clear();
        } else {
            ++at;
        }
    }

    if (page.count()) {
        AppsModel *model = new AppsModel(page, false, this);
        pages.append(new GroupEntry(this, QString(), QString(), model));
    }

    pages.prepend(new GroupEntry(this, QString(), QString(), m_favorites));

    return pages;
}

void RootModel::sortEntries(QList<AbstractEntry *> &entries)
{
    QCollator c;

    std::sort(entries.begin(), entries.end(),
              [&c](AbstractEntry* a, AbstractEntry* b) {
        return c.compare(a->name(), b->name()) < 0;
    });
}

void RootModel::extendEntryList()
{

}
