/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/CopasiUI/Attic/MetaboliteSymbols.h,v $
   $Revision: 1.15 $
   $Name:  $
   $Author: anuragr $ 
   $Date: 2004/11/11 21:14:07 $
   End CVS Header */

/****************************************************************************
 **  $ CopasiUI/MetaboliteSymbols.h               
 **  $ Author  : Mudita Singhal
 **  
 ** This is the header file for the Metabolite Symbols
 *****************************************************************************/

#ifndef METABOLITE_SYMBOLS_H
#define METABOLITE_SYMBOLS_H

#include <qtable.h>
#include <qpushbutton.h>

//#include "MyTable.h"
#include "copasi.h"
#include "copasiWidget.h"

class CMathModel;

class MetaboliteSymbols : public CopasiWidget
  {
    Q_OBJECT

  protected:
    CMathModel * mModel;
    QTable * table;
    QPushButton *btnOK;
    QPushButton *btnCancel;

  signals:
    void name(const QString &);

  public:

    MetaboliteSymbols(QWidget *parent, const char * name = 0, WFlags f = 0);
    void resizeEvent(QResizeEvent * re);

    virtual bool update(ListViews::ObjectType objectType, ListViews::Action action, const std::string & key);
    virtual bool enter(const std::string & key = "");

  protected slots:
    virtual void slotBtnOKClicked();
    virtual void slotBtnCancelClicked();
    virtual void slotTableSelectionChanged();

  private:
    void loadMetaboliteSymbols(CMathModel *model);
  };

#endif
