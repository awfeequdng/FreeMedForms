TARGET = Webcam
TEMPLATE = lib
LIBS += -lopencv_video -lopencv_highgui

DEFINES += WEBCAM_LIBRARY
BUILD_PATH_POSTFIXE = FreeMedForms

include(../fmf_plugins.pri)
include(webcam_dependencies.pri)

HEADERS += \
    webcamplugin.h\
    webcam_exporter.h\
    webcamconstants.h \
    webcamphotoprovider.h \
    qopencvwidget.h
        
SOURCES += \
    webcamplugin.cpp \
    webcamphotoprovider.cpp \
    qopencvwidget.cpp

OTHER_FILES = Webcam.pluginspec

#FREEMEDFORMS_SOURCES=%FreeMedFormsSources%
#IDE_BUILD_TREE=%FreeMedFormsBuild%

PROVIDER = FreeMedForms

TRANSLATIONS += $${SOURCES_TRANSLATIONS_PATH}/webcam_fr.ts \
                $${SOURCES_TRANSLATIONS_PATH}/webcam_de.ts \
                $${SOURCES_TRANSLATIONS_PATH}/webcam_es.ts

