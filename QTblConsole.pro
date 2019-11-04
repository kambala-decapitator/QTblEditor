TEMPLATE = subdirs

SUBDIRS += \
    console-app \
    tbl \
    test

console-app.depends += tbl
test.depends += tbl
