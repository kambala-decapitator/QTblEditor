win32:!g++: libFilename = $${libTarget}.lib
else:       libFilename = lib$${libTarget}.a

win32 {
  CONFIG(debug, debug|release): outLibDir = $${libTarget}/debug
  else:                         outLibDir = $${libTarget}/release
}
else: outLibDir = $$libTarget

outLibPath = $${OUT_PWD}/../$${outLibDir}
LIBS *= -L$${outLibPath} -l$${libTarget}
PRE_TARGETDEPS *= $${outLibPath}/$${libFilename}

INCLUDEPATH *= $$baseDir
DEPENDPATH *= $$baseDir
