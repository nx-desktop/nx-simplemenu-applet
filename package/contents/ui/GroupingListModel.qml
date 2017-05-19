import QtQuick 2.0

QtObject {
    property var groupsDataJSON : plasmoid.configuration.groupsData
    property var relationsJSON : plasmoid.configuration.relations
    property var items: [];
    property var groupedItems: [];

    onItemsChanged: p.buildGroups()
    onGroupsDataJSONChanged: p.buildGroups()
    onRelationsJSONChanged: p.buildGroups()

    property QtObject p : QtObject {

        function buildGroups() {            
            var groupsData = JSON.parse(groupsDataJSON);
            var relations = JSON.parse(relationsJSON);
            var groups = {};

            var newGroupedItems = []

            for (var i in items) {
                var item = items[i]
                item['type'] = 'item'; // Differente siple items from group items
                var groupId = relations[item.id];
                if (groupId) {
                    var group = groups[groupId];
                    if (!group) {
                        group = groupsData[groupId];
                        group['members'] = []
                        group['type'] = 'group'
                    }
                    group.members.push(item);

                    groups[groupId] = group;
                } else
                    newGroupedItems.push(item);
            }

            for (var groupId in groups) {
                var group = groups[groupId];
                newGroupedItems.push(groups[groupId])
            }

            groupedItems = newGroupedItems;
        }

    }

}
