TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c

LIBS += -L$$PWD/../../../../usr/lib/gcc/x86_64-linux-gnu/7/ -lasan -lubsan
INCLUDEPATH += $$PWD/../../../../usr/lib/gcc/x86_64-linux-gnu/7/include/sanitizer

QMAKE_CFLAGS += -Wall -Wextra -pedantic -O2 -Wshadow -Wformat=2 -Wfloat-equal
QMAKE_CFLAGS += -Wconversion -Wlogical-op -Wshift-overflow=2 -Wduplicated-cond -Wcast-qual
QMAKE_CFLAGS += -Wcast-align -D_GLIBC_DEBUG -D_GLIBC_DEBUG_PEDANTIC -D_FORTIFY_SOURCE=2
QMAKE_CFLAGS += -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fstack-protector

#INCLUDING LIBPAPI ON MY GENTOO SYSTEM
#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../usr/local/lib64/release/ -lpapi
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../usr/local/lib64/debug/ -lpapi
#else:unix: LIBS += -L$$PWD/../../../../usr/local/lib64/ -lpapi

#INCLUDEPATH += $$PWD/../../../../usr/local/lib64
#DEPENDPATH += $$PWD/../../../../usr/local/lib64

#win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../../../usr/local/lib64/release/libpapi.a
#else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../../../usr/local/lib64/debug/libpapi.a
#else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../../../usr/local/lib64/release/papi.lib
#else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../../../usr/local/lib64/debug/papi.lib
#else:unix: PRE_TARGETDEPS += $$PWD/../../../../usr/local/lib64/libpapi.a

#INCLUDING LIBPAPI ON MY UBUNTU SYSTEM
#unix:!macx: LIBS += -L$$PWD/../../../../usr/local/lib/ -lpapi

#INCLUDEPATH += $$PWD/../../../../usr/local/include
#DEPENDPATH += $$PWD/../../../../usr/local/include

#unix:!macx: PRE_TARGETDEPS += $$PWD/../../../../usr/local/lib/libpapi.a
