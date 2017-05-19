import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.0
import QtQuick.Dialogs 1.2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.plasmoid 2.0

import org.nomad.private.menu 1.0 ;

import '../code/tools.js' as Tools

Item {
    property alias cfg_groupsData: rawGroupDataEdit.text;
    property alias cfg_relations: rawRelationsEdit.text;

    property var groupsDataJSON : plasmoid.configuration.groupsData
    property var relationsJSON : plasmoid.configuration.relations

    //    property var groupList: []
    //    property var apps: []

    Component.onCompleted: {
        loadGroupList()
    }

    function loadGroupList() {
        var groupsData = JSON.parse(groupsDataJSON)
        groupList.clear()
        for (var k in groupsData) {
            var group = groupsData[k]
            groupList.append(group)
        }
    }

    function saveGroupList() {
        var newGroupsData = {}
        for (var i = 0; i < groupList.count; i++) {
            var group = groupList.get(i)
            newGroupsData[group.id] = group;
        }

        rawGroupDataEdit.text = JSON.stringify(newGroupsData)
    }


    ListModel {
        id: groupList

        function addGroup() {
            var newGroup = {
                id: Tools.randomId(),
                display: i18n("New group")
            }

            append(newGroup)

            groupListView.model = groupList

            saveGroupList()
        }

        function eraseGroup(index) {
            remove(index)

            groupListView.currentIndex = -1
            groupListView.model = groupList

            saveGroupList()
        }
    }

    ListModel {
        id: apps
        property var groupId: groupListView.currentIndex != -1 ? groupList.get(groupListView.currentIndex).id : "";
        onGroupIdChanged: loadApps()

        function loadApps() {
            var relations = []
            if (rawRelationsEdit.text == "")
                relations = JSON.parse(relationsJSON);
            else
                relations = JSON.parse(rawRelationsEdit.text);

            apps.clear()
            for (var i = 0; i < launchers.rowCount(); i ++) {
                var index = launchers.index(i, 0);
                if (relations[launchers.data(index, Qt.UserRole)] === groupId) {
                    var item = {
                        display: launchers.data(index, Qt.DisplayRole),
                        decoration: launchers.data(index, Qt.DecorationRole),
                        tooltTip: launchers.data(index, Qt.ToolTipRole),
                        id: launchers.data(index, Qt.UserRole)
                    }
                    apps.append(item)
                }
            }
        }

        function saveApps() {
            var relations = JSON.parse(rawRelationsEdit.text);
            var newRelations = {};
            for (var i = 0; i < apps.count; i ++) {
                var app = apps.get(i)
                newRelations[app.id] = groupId;
            }

            for (var appId in relations) {
                if (relations[appId] !== groupId)
                    newRelations[appId] = relations[appId]
            }

            rawRelationsEdit.text = JSON.stringify(newRelations);
        }

        function addApplication(app) {
            apps.append(app)

            saveApps()
            loadApps()
        }
        function removeApp(index) {
            remove(index)

            saveApps()
        }
    }

    Launchers {
        id: launchers

        property var apps: []
        onModelReset: listApps()


        function listApps() {
            var newApps = [];
            for (var i = 0; i < launchers.rowCount(); i ++) {
                var index = launchers.index(i, 0);
                var item = {
                    display: launchers.data(index, Qt.DisplayRole),
                    decoration: launchers.data(index, Qt.DecorationRole),
                    tooltTip: launchers.data(index, Qt.ToolTipRole),
                    id: launchers.data(index, Qt.UserRole)
                }
                newApps.push(item);
            }
            launchers.apps = newApps
        }
    }

    GridLayout {
        anchors.fill: parent
        anchors.rightMargin: 18
        columns: 2

        PlasmaExtras.ScrollArea {
            Layout.rowSpan: 2
            Layout.minimumWidth: 200
            Layout.fillHeight: true

            Rectangle {
                anchors.fill: parent
                color: "white"

                border.color: "lightgray"
                border.width: 1
            }

            ListView {
                id: groupListView
                anchors.fill: parent

                model: groupList
                delegate: PlasmaComponents.ListItem {
                    height: 40
                    Label {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 12
                        text: display
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked:  groupListView.currentIndex = index;
                    }
                }

                highlightFollowsCurrentItem: true
                highlight: Rectangle {
                    color: "lightblue"
                    opacity: 0.3
                }

                RowLayout {
                    anchors {
                        bottom: parent.bottom
                        right: parent.right
                    }
                    PlasmaComponents.ToolButton {
                        iconName: "list-remove-symbolic"
                        onClicked: groupList.eraseGroup(groupListView.currentIndex)
                    }

                    PlasmaComponents.ToolButton {
                        iconName: "list-add-symbolic"
                        onClicked: groupList.addGroup()
                    }

                }
            }
        }

        TextField {
            id: groupTitle
            Layout.fillWidth: true
            placeholderText: i18n("Group title")
            text: groupListView.currentIndex != -1 ? groupList.get(groupListView.currentIndex).display : ""
            onTextChanged: {
                if (groupListView.currentIndex != -1 ) {
                    groupList.get(groupListView.currentIndex).display = text
                    saveGroupList()
                }
            }
        }

        PlasmaExtras.ScrollArea {
            id: appsListViewScroll
            Layout.fillWidth: true
            Layout.fillHeight: true

            Rectangle {
                anchors.fill: parent
                color: "white"

                border.color: "lightgray"
                border.width: 1

                Popup {
                    id: newAppPopup
                    y: appsListViewScroll.height - groupListView.height
                    width: appsListViewScroll.width
                    height: groupListView.height
                    focus: true
                    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

                    PlasmaExtras.ScrollArea {
                        anchors.fill: parent
                        ListView {
                            id: newAppPopupListView
                            model: launchers

                            highlightFollowsCurrentItem: true
                            highlight: Rectangle {
                                color: "lightblue"
                                opacity: 0.3
                            }

                            delegate: PlasmaComponents.ListItem {
                                height: 40
                                RowLayout {
                                    anchors.fill: parent

                                    PlasmaCore.IconItem {
                                        Layout.fillHeight: true
                                        Layout.minimumWidth: 32
                                        Layout.maximumWidth: Layout.minimumWidth

                                        source: icon
                                    }

                                    PlasmaComponents.Label {
                                        Layout.fillHeight: true
                                        Layout.fillWidth: true
                                        text: name
                                    }

                                    MouseArea {
                                        anchors.fill: parent

                                        hoverEnabled: true
                                        onContainsMouseChanged: {
                                            if (containsMouse) {
                                                newAppPopupListView.currentIndex = index
                                            }
                                        }
                                        onClicked: {
                                            var app = {
                                                id: model.id,
                                                display: model.name,
                                                decoration: model.icon
                                            }

                                            apps.addApplication(app)
                                            newAppPopup.close()
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            ListView {
                id: appsListView
                anchors.fill: parent

                model: apps
                delegate: PlasmaComponents.ListItem {
                    height: 40
                    RowLayout {
                        anchors.fill: parent
                        PlasmaCore.IconItem {
                            Layout.fillHeight: true
                            Layout.minimumWidth: 32
                            Layout.maximumWidth: Layout.minimumWidth

                            source: decoration
                        }

                        PlasmaComponents.Label {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            text: display
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: appsListView.currentIndex = index
                        }
                    }
                }

                highlightFollowsCurrentItem: true
                highlight: Rectangle {
                    color: "lightblue"
                    opacity: 0.3
                }

                RowLayout {
                    anchors {
                        bottom: parent.bottom
                        right: parent.right
                    }
                    PlasmaComponents.ToolButton {
                        iconName: "list-remove-symbolic"
                        onClicked: apps.removeApp(appsListView.currentIndex)
                    }

                    PlasmaComponents.ToolButton {
                        iconName: "list-add-symbolic"

                        onClicked: newAppPopup.open()
                    }
                }
            }
        }

        TextField {
            id: rawGroupDataEdit
            visible: false
        }

        TextField {
            id: rawRelationsEdit
            visible: false
            Layout.fillWidth: true
        }
    }
}
