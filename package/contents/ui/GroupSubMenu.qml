import QtQuick 2.4
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.plasma.private.kicker 0.1 as Kicker

Popup {
    id: submenuRoot
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    padding: 0
    property int previousSelectedItemIndex: 0

    function show(groupItem, members) {
        previousSelectedItemIndex = pageList.currentItem.itemGrid.currentIndex;
        visible = true
        groupGridView.model = members;

        var relativeItemPos = parent.mapFromItem(groupItem, 0, 0);
        groupGridView.focus = true;
        groupGridView.currentIndex = 0;

        if (relativeItemPos.y < (pageList.height / 2) ) {
            y = relativeItemPos.y + groupItem.height
            height = pageListScrollArea.height - (groupItem.y + groupItem.height);
        } else {
            y = pageListScrollArea.y
            height = groupItem.y;
        }

        submenuRoot.open()
    }

    function hide() {
        submenuRoot.visible = false
        pageList.currentItem.itemGrid.focus = true;
        pageList.currentItem.itemGrid.forceActiveFocus()
        pageList.currentItem.itemGrid.currentIndex = previousSelectedItemIndex;
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            submenuRoot.hide()
            mouse.accepted = true
        }
        preventStealing: true
    }
    
    background: Rectangle { color: "lightblue" }
    ItemGridView {
        id: groupGridView

        anchors.fill: parent

        cellWidth: cellSize
        cellHeight: cellSize

        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff

        dragEnabled: (index == 0)

        onCurrentIndexChanged: {
            if (currentIndex != -1) {
                pageListScrollArea.focus = true;
            }
        }

        onKeyNavUp: {
            currentIndex = -1;
            submenuRoot.hide()
        }

        onActiveFocusChanged: if (!activeFocus) hide()

        Keys.onPressed: {
            if (event.key == Qt.Key_Escape) {
                event.accepted = true;
                hide()
            }
        }
    }
}
