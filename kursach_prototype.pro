TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../usr/local/lib64/release/ -lpapi
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../usr/local/lib64/debug/ -lpapi
else:unix: LIBS += -L$$PWD/../../../../usr/local/lib64/ -lpapi

INCLUDEPATH += $$PWD/../../../../usr/local/lib64
DEPENDPATH += $$PWD/../../../../usr/local/lib64

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../../../usr/local/lib64/release/libpapi.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../../../usr/local/lib64/debug/libpapi.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../../../usr/local/lib64/release/papi.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../../../usr/local/lib64/debug/papi.lib
else:unix: PRE_TARGETDEPS += $$PWD/../../../../usr/local/lib64/libpapi.a
