######################################################################
# Communi: qtdocktile.pri
######################################################################

DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD
QMAKE_CLEAN += $$PWD/*~

HEADERS += $$PWD/qtdocktile.h
HEADERS += $$PWD/qtdocktile_p.h

SOURCES += $$PWD/qtdocktile.cpp

mac:include(mac/mac.pri)