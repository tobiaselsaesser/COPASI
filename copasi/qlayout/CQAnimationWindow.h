// Copyright (C) 2017 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and University of
// of Connecticut School of Medicine.
// All rights reserved.

// Copyright (C) 2013 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#ifndef QANIMATION_WINDOW_H
#define QANIMATION_WINDOW_H

#include <QMainWindow>
#include <QtCore/QSharedPointer>

#include <UI/CWindowInterface.h>

#include <layout/CLayoutState.h>
#include <copasi/qlayout/ui_CQAnimationWindow.h>

class CQLayoutScene;
class CQCopasiAnimation;
class CDataModel;
class QCloseEvent;
class QMenu;
class CLayout;
class CQLayoutThread;

class CQAnimationWindow : public CWindowInterface, public Ui::CQAnimationWindow
{
  Q_OBJECT
public:
  CQAnimationWindow(CLayout* layout, CDataModel* dataModel = NULL);
  CQAnimationWindow();
  virtual ~CQAnimationWindow();
  void setScene(CQLayoutScene* scene, CDataModel* dataModel);
  void setAnimation(CQCopasiAnimation* animation, CDataModel* dataModel);
  virtual QMenu *getWindowMenu() const;
public slots:
  void slotShowStep(int);
  void slotExportImage();
  void slotEditSettings();
  void slotSwitchAnimation();
  void slotRandomizeLayout();
  void slotAutoLayout();
  void slotStopLayout();
  void slotLayoutStateChanged(QSharedPointer<CLayoutState> state);
private:
  void init();
  void toggleUI(bool isPlaying);
protected:
  virtual void closeEvent(QCloseEvent *closeEvent);

  CQLayoutScene* mpScene;
  CDataModel* mpModel;
  QMenu* mpWindowMenu;
  CQCopasiAnimation* mAnimation;
  CQLayoutThread* mpLayoutThread;
  CLayout* mpCopy;
};

#endif // QANIMATION_WINDOW_H
