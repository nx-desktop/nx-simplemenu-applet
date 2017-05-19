import QtQuick 2.4
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.plasma.private.kicker 0.1 as Kicker

Item {
    id: submenuRoot

    property int previousSelectedItemIndex: 0

    function show(groupItem, members) {
        previousSelectedItemIndex = pageList.currentItem.itemGrid.currentIndex;
        visible = true
        groupGridView.model = members;

        groupGridView.focus = true;
        groupGridView.currentIndex = 0;
        if (groupItem.y > pageList.height / 2) {
            groupView.y = 0
            groupView.height = groupItem.y
        } else {
            groupView.y = groupItem.y + groupItem.height
            groupView.height = pageList.height - groupView.y
        }
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
    
    Rectangle {
        id: groupView
        
        anchors.left: parent.left
        anchors.right: parent.right
        
        color: "lightblue"
        
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
}
