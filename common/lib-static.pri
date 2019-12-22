include(lib.pri)

win32:!win32-g++: libFilename = $${libTarget}.lib
else:             libFilename = lib$${libTarget}.a

PRE_TARGETDEPS *= $${outLibPath}/$${libFilename}
