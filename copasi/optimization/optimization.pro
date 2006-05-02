######################################################################
# $Revision: 1.20 $ $Author: shoops $ $Date: 2006/05/02 20:32:32 $  
######################################################################

LIB = optimization

include(../lib.pri)
include(../common.pri)

# Input
HEADERS += COptMethod.h \
#           COptMethodEP2.h \
           COptMethodGA.h \
           COptMethodHookeJeeves.h \
#           COptMethodHGASA.h \
#           COptMethodSA.h \
           COptProblem.h \
           CRandomSearch.h \
           COptMethodGASR.h \
           COptMethodLevenbergMarquardt.h \
           COptMethodSRES.h \
           COptMethodSteepestDescent.h \
		       COptMethodEP.h \
#           CRandomSearchMaster.h \
           CRealProblem.h \
           COptFunction.h \
           COptTask.h \ 
           COptItem.h \
           FminBrent.h
           
SOURCES += COptMethod.cpp \
#           COptMethodEP2.cpp \
           COptMethodGA.cpp \
           COptMethodHookeJeeves.cpp \
#           COptMethodHGASA.cpp \
#           COptMethodSA.cpp \
           COptProblem.cpp \
           CRandomSearch.cpp \
           COptMethodGASR.cpp \
           COptMethodLevenbergMarquardt.cpp \
           COptMethodSRES.cpp \
           COptMethodSteepestDescent.cpp \
		       COptMethodEP.cpp \
#           CRandomSearchMaster.cpp \
           CRealProblem.cpp \
           COptFunction.cpp \
           COptTask.cpp \
           COptItem.cpp \
           FminBrent.cpp
