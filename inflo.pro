SUB_PROJECT_NAME = INFLO
BASE_DIR = ../../../..

include (../../config_type.incl)

SOURCES += \
INFLODescription.cxx \
INFLOExtension.cxx \
INFLOPlugin.cxx 

HEADERS += \
INFLODescription.h \
INFLOExtension.h \
INFLOPlugin.h \
INFLOUtil.h 

TRANSLATIONS += \
inflo_de.ts \
inflo_es.ts \
inflo_fa.ts \
inflo_fr.ts \
inflo_it.ts \
inflo_jp.ts \
inflo_pt.ts \
inflo_ru.ts \
inflo_zh.ts

macx {
    DESTDIR = .
	TARGET = $${BIN_PATH}/Aimsun.app/Contents/PlugIns/inflo
    QMAKE_LFLAGS_SONAME  = -Wl,-install_name,@executable_path/../PlugIns/
}else{
	linux {
	    DESTDIR = .
		TARGET = $${BIN_PATH}/plugins/inflo
	}else{
		TARGET = ../$${PLG_PATH}/inflo
	}
}

#-------------------------------------------------------------------------------------

QT += sql xml opengl network

TEMPLATE = lib
VERSION = 8.0.0

CONFIG += dll qt thread exceptions rtti

win32 {
	DEFINES += _INFLODLL_DEF
}


ANG_HOME = ../..

INCLUDEPATH += $$PTH_INCL $$ANG_HOME/core/ang_kernel $$ANG_HOME/core/ang_kernel/gkstats $$ANG_HOME/core/ang_gui $${UI_HEADERS_DIR}/../ang_gui $$ANG_HOME/core/ANG_SSTATS $$ANG_HOME/plugins/$${A2SIM_INCL}/V5.2/A2MODELS/SDK $$ANG_HOME/plugins/$${A2SIM_INCL}/V5.2/A2Kernel
INCLUDEPATH += $$ANG_HOME/plugins/$${A2SIM_INCL} $${UI_HEADERS_DIR}/../ANG_NEWSIM $$ANG_HOME/plugins/$${A2SIM_INCL}/V5.2/A2Kernel/AThread $$ANG_HOME/plugins/$${A2SIM_INCL}/V5.2/ACONTROL $$ANG_HOME/core/ANG_DTA
INCLUDEPATH += $$ANG_HOME/core/ang_kernel/nspmanager $$ANG_HOME/core/random
INCLUDEPATH += $$GIBASE_INCL
DEPENDPATH += $$INCLUDEPATH

LIBS += $$STL_LIBS

win32 {
	LIBS += -L$${BIN_PATH} -langgui8 -langkernel8 -la2kernel8 -lsstats8
	LIBS += -L$${PLG_PATH} -laimsunmicroplugin8 -ldtakernel8 -lacontrol8
	LIBS += -L$${SLP_PATH} -lathread -lrandom
} macx {
	QT += network
	INCLUDEPATH += $$VPX_INCL
	LIBS += $$QWT_LIBS $$QSCINDILLA_LIBS $$VPX_LIB
	LIBS += -L$${BIN_PATH}/Aimsun.app/Contents/Frameworks -langgui -langkernel -lsstats
	LIBS += -L$${BIN_PATH}/Aimsun.app/Contents/PlugIns -la2kernel -lacontrol -laimsunmicroplugin -ldtakernel
	LIBS += -L$${SLP_PATH} -lathread -lrandom
} linux {
# poner luego $$QSCINDILLA_LIBS
	LIBS += $$QWT_LIBS $$GDAL_LIBS
	LIBS += -L$${BIN_PATH} -langgui -langkernel -lsstats
	LIBS += -L$${BIN_PATH}/plugins -laimsunmicroplugin -la2kernel -lacontrol -ldtakernel
	LIBS += -L$${SLP_PATH} -lathread -lrandom
}

PRE_TARGETDEPS += $${SLP_PATH}/$${STATICLIBPREFIX}athread.$${STATICLIBSUFFIX}

#-------------------------------------------------------------------------------------
