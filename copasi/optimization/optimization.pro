######################################################################
# $Revision: 1.10 $ $Author: shoops $ $Date: 2005/02/08 16:33:04 $  
######################################################################

LIB = optimization

include(../lib.pri)
include(../common.pri)

# Input
HEADERS += COptMethod.h \
           COptMethodEP2.h \
           COptMethodGA.h \
           COptMethodHGASA.h \
           COptMethodSA.h \
           COptProblem.h \
           CRandomSearch.h \
           CRandomSearchMaster.h \
           CRealProblem.h \
           COptFunction.h \
		   COptTask.h \ 
           COptItem.h
           
SOURCES += COptMethod.cpp \
           COptMethodEP2.cpp \
           COptMethodGA.cpp \
           COptMethodHGASA.cpp \
           COptMethodSA.cpp \
           COptProblem.cpp \
           CRandomSearch.cpp \
           CRandomSearchMaster.cpp \
           CRealProblem.cpp \
           COptFunction.cpp \
		   COptTask.cpp \
           COptItem.cpp
