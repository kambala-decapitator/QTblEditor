TEMPLATE = subdirs

SUBDIRS += \
    console-app \
    tbl \
    tbl-static \
    test

test.depends += tbl

tbl.file = tbl/tbl-shared.pro
tbl-static.file = tbl/tbl-static.pro

CONFIG(debug, debug|release): console-app.depends += tbl
else:                         console-app.depends += tbl-static
