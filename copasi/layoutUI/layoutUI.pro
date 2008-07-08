# Begin CVS Header
#   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/layoutUI/layoutUI.pro,v $
#   $Revision: 1.21 $
#   $Name:  $
#   $Author: shoops $
#   $Date: 2008/07/08 16:06:25 $
# End CVS Header

# Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
# Properties, Inc., EML Research, gGmbH, University of Heidelberg,
# and The University of Manchester.
# All rights reserved.

# Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
# Properties, Inc. and EML Research, gGmbH.
# All rights reserved.

LIB = layoutUI

include(../lib.pri)
include(../common.pri)

CONFIG += qt

HEADERS = CQGLNetworkPainter.h \
          CArrow.h \
          CGraphNode.h \
          CGraphCurve.h \
          CQLayoutMainWindow.h \
          CSimSummaryInfo.h \
          CDataEntity.h \
          CVisParameters.h \
          CQCopasiLayoutWidget.h \
          CQCurrentValueTable.h \
          BezierCurve.h \
          RGTextureSpec.h \
          CLabel.h


SOURCES = CQLayoutMainWindow.cpp \
          CArrow.cpp \
          CGraphNode.cpp \
          CGraphCurve.cpp \
          CQGLNetworkPainter.cpp \
          CSimSummaryInfo.cpp \
          CDataEntity.cpp \
          CVisParameters.cpp \
          CQCopasiLayoutWidget.cpp \
          CQCurrentValueTable.cpp \
          BezierCurve.cpp \
          RGTextureSpec.cpp \
          CLabel.cpp



FORMS = ParaPanel.ui \
        NodeSizePanel.ui \
        FontChooser.ui

       # CQCopasiLayoutWidget.ui

INCLUDEPATH += $${QWT_PATH}/include

DISTFILES += layoutUI.vcproj
