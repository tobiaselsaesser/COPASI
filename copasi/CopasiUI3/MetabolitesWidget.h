#ifndef METABOLITES_WIDGET_H
#define METABOLITES_WIDGET_H

//#include <qtable.h>
#include "MyTable.h"

class CModel;
class MetabolitesWidget : public QWidget
{
		Q_OBJECT
protected:
	MyTable *table;
	CModel *mModel;
	

public:
	MetabolitesWidget(QWidget *parent, const char * name=0, WFlags f=0);
	void loadMetabolites(CModel *model);
	void setFocus();
	//void mousePressEvent ( QMouseEvent * me);
	void resizeEvent( QResizeEvent * re);
	
protected slots:
    virtual void slotTableClicked( int row, int col, int button, const QPoint & mousePos );
	virtual void slotTableSelectionChanged();
};

#endif
