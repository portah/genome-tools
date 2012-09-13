######################################################################
# pro file for complexity plot Wed Oct 19 14:59:16 2011
######################################################################

TEMPLATE = app
TARGET   = ReadsCounting

CONFIG   += console warn_on release
QT       -= gui
QT       += sql


DEFINES     += _SQL_     \
               D_USE_BAM \
	       _NORMAL


!win32{

DEFINES        += _APPNAME=\\\"$$TARGET\\\"
LIBS           += -lm -lz ../../thirdparty/bamtools/libbamtools.a
QMAKE_CXXFLAGS += -Werror -std=c++0x

lib_bamtools.commands = cd ../../thirdparty/bamtools/; qmake; $(MAKE) -j 8
QMAKE_EXTRA_TARGETS   = lib_bamtools
PRE_TARGETDEPS        = lib_bamtools

OBJECTS_DIR = GeneratedFiles
UI_DIR      = GeneratedFiles
MOC_DIR     = GeneratedFiles
RCC_DIR     = GeneratedFiles

}

win32{

DEFINES        += _APPNAME=\"$$TARGET\"
LIBS           += -lbamtools

}




DEPENDPATH +=  . \
               ./src \
               ../global \
	       ../../thirdparty/bamtools \
               ../../thirdparty/boost

INCLUDEPATH += . \
               ./src \
               ../global \
               ../../thirdparty/bamtools \
               ../../thirdparty/boost



HEADERS     +=  ../global/Arguments.hpp \
		../global/Reads.hpp \
		./src/Threads.hpp \
		./src/ReadsCounting.hpp
		
               
               
SOURCES     += \
		../global/Arguments.cpp \
		../global/Reads.cpp \
                ./src/Threads.cpp \
                ./src/ReadsCounting.cpp \
                ./src/main.cpp


QMAKE_CLEAN += $$TARGET logfile.log *~ ./src/*~ *.txt ../global/*~
