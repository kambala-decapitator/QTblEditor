TEMPLATE = subdirs

SUBDIRS += \
    console-app \
    filesystem \
    tbl
    
filesystem.file = filesystem.pro

console-app.depends += tbl
