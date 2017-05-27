/***************************************************************************
 *   Copyright (C) 2015 by Eike Hein <hein@kde.org>                        *
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

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0

import "../code/tools.js" as Tools

MouseArea {
    id: item

    width: GridView.view.cellWidth
    height: icon.height + label.height + units.smallSpacing * 2

    signal actionTriggered(string actionId, variant actionArgument)
    signal aboutToShowActionMenu(variant actionMenu)
    signal aboutToShowSubgroup(var groupModel)

    property bool showLabel: true

    property int itemIndex: model.index
    property url url: model.url != undefined ? model.url : ""
    property bool pressed: false
    property bool hasActionList: ((model.favoriteId != null)
        || (("hasActionList" in model) && (model.hasActionList == true)))
    property Item view: GridView.view
    property Item menu: actionMenu

    property var childrenModel: GridView.view.model.modelForRow(index);
    property var isGroup: childrenModel !== null

    Accessible.role: Accessible.MenuItem
    Accessible.name: model.display

    acceptedButtons: Qt.LeftButton | Qt.RightButton

    onPressed: {
        if (mouse.buttons & Qt.RightButton) {
            if (hasActionList) {
                openActionMenu(item, mouse.x, mouse.y);
            }
        } else {
            pressed = true;
        }
    }

    onReleased: {
        if (isGroup)
            aboutToShowSubgroup(childrenModel)
        else {
            if (pressed && GridView.view.currentItem == item) {
                GridView.view.model.trigger(index, "", null);

                if ("toggle" in root) {
                    root.toggle();
                } else {
                    root.visible = false;
                }
            }

            pressed = false;
        }
    }

    onAboutToShowActionMenu: {
        var actionList = hasActionList ? model.actionList : [];
        Tools.fillActionMenu(actionMenu, actionList, GridView.view.model.favoritesModel, model.favoriteId);
    }

    onActionTriggered: {
        Tools.triggerAction(GridView.view.model, model.index, actionId, actionArgument);
    }

    function openActionMenu(visualParent, x, y) {
        aboutToShowActionMenu(actionMenu);
        actionMenu.visualParent = visualParent;
        actionMenu.open(x, y);
    }

    ActionMenu {
        id: actionMenu

        onActionClicked: {
            actionTriggered(actionId, actionArgument);
        }
    }

    Item {
        id: icon
        y: showLabel ? (2 * highlightItemSvg.margins.top) : 0

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: showLabel ? undefined : parent.verticalCenter

        width: iconSize
        height: width

        PlasmaCore.IconItem {
            id: icon_simple
            anchors.fill: parent
            animated: false
            usesPlasmaTheme: view.usesPlasmaTheme

            source: model.decoration
            visible: !isGroup
        }

        Rectangle {
            anchors.fill: parent
            visible: icon_simple.visible == false

            color: "darkgray"
            border.color: "gray"
            border.width: 1
            radius: 5

            Flow {
                anchors.fill: parent
                anchors.margins: 4
                spacing: 4
                clip: true


                Repeater {
                    model: item.childrenModel
                    delegate: PlasmaCore.IconItem {
                        height: iconSize / 4
                        width: iconSize / 4

                        animated: false
                        usesPlasmaTheme: view.usesPlasmaTheme
                        source: decoration
                    }
                }
            }
        }
    }

    PlasmaComponents.Label {
        id: label

        visible: showLabel

        anchors {
            top: icon.bottom
            topMargin: units.smallSpacing
            left: parent.left
            leftMargin: highlightItemSvg.margins.left
            right: parent.right
            rightMargin: highlightItemSvg.margins.right
        }

        horizontalAlignment: Text.AlignHCenter

        elide: Text.ElideRight
        wrapMode: Text.NoWrap

        text: model.display
    }

    Keys.onPressed: {
        if (event.key == Qt.Key_Menu && hasActionList) {
            event.accepted = true;
            openActionMenu(item);
        } else if ((event.key == Qt.Key_Enter || event.key == Qt.Key_Return)) {
            event.accepted = true;
            if (isGroup)
                aboutToShowSubgroup(childrenModel)
            else {
                GridView.view.model.trigger(index, "", null);

                if ("toggle" in root) {
                    root.toggle();
                } else {
                    root.visible = false;
                }
            }
        }
    }
}
