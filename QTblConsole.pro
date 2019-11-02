TEMPLATE = subdirs

SUBDIRS += \
    console-app \
    filesystem \
    tbl \
    test
    
filesystem.file = filesystem.pro

console-app.depends += tbl
test.depends += tbl
