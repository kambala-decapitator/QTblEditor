win32 {
  CONFIG(debug, debug|release): outLibDir = $${libTarget}/debug
  else:                         outLibDir = $${libTarget}/release
}
else: outLibDir = $$libTarget

outLibPath = $${OUT_PWD}/../$${outLibDir}
LIBS *= -L$${outLibPath} -l$${libTarget}

INCLUDEPATH *= $$baseDir
DEPENDPATH *= $$baseDir
