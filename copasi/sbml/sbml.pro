
LIB = sbmlimport

include(../lib.pri)
include(../common.pri)

contains(BUILD_OS, WIN32) {
  DEFINES += LIBSBML_STATIC
}

# Input
HEADERS += StdException.h \
           ConverterASTNode.h \
           SBMLImporter.h \
           SBMLExporter.h
           
           
           
SOURCES += StdException.cpp \
           ConverterASTNode.cpp \
           SBMLImporter.cpp \
           SBMLExporter.cpp


