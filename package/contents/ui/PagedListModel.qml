import QtQuick 2.0

/**
  Split a list model into pages with no more tan "pageSize" elements.
*/
QtObject {
    id: root
    property int pageSize: 20
    property var items : []

    function count() {
        return Math.ceil(items.length / pageSize);
    }

    function page(index) {
        if (index >= 0 < count())
            return p.pagesCache[index];

        return p.createEmptyModel;
    }

    property QtObject p : QtObject {
        property var pagesCache: []

        function rebuildCache () {
            print("REBUILDING PAGES CACHE", items.length)
//            pagesCache = []
            for (var i = 0; i < pagesCache.length; i ++)
                pagesCache[i].clear()

            for (var i = 0; i < items.length; i ++) {
                var page = Math.floor(i / pageSize);

                var pageModel = pagesCache[page];
                if (pageModel === undefined) {
                    pageModel = createEmptyModel()
                    pageModel.pageIdx = page

                    pagesCache.push(pageModel)
                }
                var item = items[i]
                print("ITEM ", item.id, item.decoration)
                pageModel.append(item);
            }
        }

        function createEmptyModel() {
            var emptyModel = Qt.createQmlObject(
                        'import QtQuick 2.0;
                        ListModel {
                            property var favoritesModel: rootModel.favoritesModel
                            property int pageIdx;
                        }',
                        root)
            return emptyModel;
        }
    }

    onItemsChanged: p.rebuildCache()
}
