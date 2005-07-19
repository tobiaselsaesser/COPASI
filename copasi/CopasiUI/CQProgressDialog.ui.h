/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/CopasiUI/Attic/CQProgressDialog.ui.h,v $
   $Revision: 1.4 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2005/07/19 21:00:53 $
   End CVS Header */

#include <qapplication.h>
#include <qtimer.h>

bool CQProgressDialog::insertProgressItem(CQProgressItem * pItem)
{
  if (mItemCount == 0) mpLine->show();

  CQProgressDialogLayout->removeItem(mpSpacer);
  CQProgressDialogLayout->addWidget(pItem);
  CQProgressDialogLayout->addItem(mpSpacer);

  pItem->show();
  qApp->processEvents();

  mItemCount++;
  return true;
}

bool CQProgressDialog::removeProgressItem(CQProgressItem * pItem)
{
  CQProgressDialogLayout->remove(pItem);

  mItemCount--;
  if (mItemCount == 0) mpLine->hide();

  qApp->processEvents();
  return true;
}

void CQProgressDialog::init()
{
  mpLine->hide();
  mpBtnPause->hide();
  mpBtnContinue->hide();

  mItemCount = 0;

  mPause = false;
  mProceed = true;

  mpTimer = new QTimer(this);
  mpTimer->start(1500, true);
  connect(mpTimer, SIGNAL(timeout()), this, SLOT(timerShow()));

  return;
}

void CQProgressDialog::btnContinuePressed()
{mPause = false;}

void CQProgressDialog::btnPausePressed()
{mPause = true;}

void CQProgressDialog::btnStopPressed()
{mProceed = false;}

void CQProgressDialog::timerShow()
{this->show();}
