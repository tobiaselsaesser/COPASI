/*********************************************************************
**  $ CopasiUI/ReactionsWidget1.cpp                 
**  $ Author  : Mudita Singhal
**
** This file is used to create the GUI FrontPage for the  information 
** obtained from the data model about the Moiety----It is Basically 
** the Second level of Reactions.
**********************************************************************/

#include "ReactionsWidget1.h"
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qtoolbar.h>
#include <qwidget.h>
#include <qframe.h>
#include <qcheckbox.h> 
#include "listviews.h"
#include "Model/CChemEqElement.h" 
/* 
 *  Constructs a ReactionsWidget which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 */

ReactionsWidget1::ReactionsWidget1( QWidget *parent, const char * name, WFlags f )
    : QWidget(parent, name, f)

{	
	//This is to make the Main Frame of the page
	//The Main layout used is the Vertical Layout
	QVBoxLayout *vboxLayout = new QVBoxLayout(this, 0 );
	Frame1 = new QFrame( this, "Frame1" );
	Frame1->setFrameShape( QFrame::Box );
    Frame1->setFrameShadow( QFrame::Plain);
	vboxLayout->addWidget(Frame1);
	QVBoxLayout *vboxLayout1 = new QVBoxLayout(Frame1, 0 );
	vboxLayout1->addSpacing(1);
   


	//This Frame had to be added because of the border around the frame
	//The grid Layout is used for this frame
	Frame3 = new QFrame( Frame1, "Frame3" );
	vboxLayout1->addWidget(Frame3);
	QGridLayout *gridLayout1 = new QGridLayout(Frame3, 0 );
	
	
	//Frame for Ist Row
	Frame4a = new QFrame(Frame3, "Frame4a" );
	gridLayout1->addWidget(Frame4a,0,0,0);
	QHBoxLayout *hBoxLayout4a = new QHBoxLayout( Frame4a, 0 );
	hBoxLayout4a->addSpacing(15);
	
	TextLabel1 = new QLabel( "Name", Frame4a );
    hBoxLayout4a->addWidget(TextLabel1);
	hBoxLayout4a->addSpacing(75);

    LineEdit1 = new QLineEdit( "",Frame4a  );
	hBoxLayout4a->addWidget(LineEdit1);
    hBoxLayout4a->addSpacing(250);
	

	//Frame for 2nd Row
	Frame4b = new QFrame(Frame3, "Frame4b" );
   gridLayout1->addWidget(Frame4b,1,0,0);
	QHBoxLayout *hBoxLayout4b = new QHBoxLayout( Frame4b, 0 );
	hBoxLayout4b->addSpacing(15);
	
	TextLabel2 = new QLabel( "Chemical Reaction", Frame4b );
    hBoxLayout4b->addWidget(TextLabel2);
	hBoxLayout4b->addSpacing(17);

    LineEdit2 = new QLineEdit( "",Frame4b  );
	hBoxLayout4b->addWidget(LineEdit2);
	hBoxLayout4b->addSpacing(50);


	//Frame for 3rd Row
	Frame4c = new QFrame(Frame3, "Frame4c" );
    gridLayout1->addWidget(Frame4c,2,0,0);
	QHBoxLayout *hBoxLayout4c = new QHBoxLayout( Frame4c, 0 );
	hBoxLayout4c->addSpacing(15);

	Line2 = new QFrame( Frame4c, "Line2" );
    Line2->setGeometry( QRect( 80, 10, 91, 31 ) ); 
    Line2->setProperty( "frameShape", (int)QFrame::HLine );
    Line2->setFrameShadow( QFrame::Sunken );
    Line2->setFrameShape( QFrame::HLine );
	hBoxLayout4c->addWidget(Line2);




	//Frame for 4th Row
	Frame4d = new QFrame( Frame3, "Frame4d" );
    gridLayout1->addWidget(Frame4d,3,0,0);
	QHBoxLayout *hBoxLayout4d = new QHBoxLayout( Frame4d, 0 );
	hBoxLayout4d->addSpacing(15);
	
	
	TextLabel3= new QLabel( "Kinetics",Frame4d);
    hBoxLayout4d->addWidget( TextLabel3);
  	hBoxLayout4d->addSpacing(65);
	
	ComboBox1 = new QComboBox( Frame4d, "ComboBox1" );
    hBoxLayout4d ->addWidget(ComboBox1);
	ComboBox1->setFixedSize(225,20);
	hBoxLayout4d->addSpacing(50);
	
	checkBox=new QCheckBox ( Frame4d, "checkBox" );
	
	hBoxLayout4d->addWidget(checkBox);
	hBoxLayout4d->addSpacing(1);
	
	
	TextLabel4= new QLabel( "Reversible",Frame4d);
    hBoxLayout4d->addWidget( TextLabel4);
    hBoxLayout4d->addSpacing(20);
	

	newKinetics = new QPushButton("&New Kinetics", Frame4d);
	hBoxLayout4d->addWidget(newKinetics);
	hBoxLayout4d->addSpacing(20);




	//Frame for 5th Row
	Frame4e = new QFrame(Frame3, "Frame4e" );
	Frame4e->setGeometry( QRect( 0, 0, 740, 120 )); 
    gridLayout1->addMultiCellWidget(Frame4e,4,5,0,0,0);

	TextLabel5 = new QLabel( Frame4e, "Symbol Definition" );
    TextLabel5->setGeometry( QRect( 10, 40, 90, 50 )); 
    TextLabel5->setText( trUtf8( "Symbol Definition" ) );
    
    table = new QTable( Frame4e, "tblsymbol" );
    table->setGeometry( QRect( 130, 10, 160, 154 )   ); 
    
	table->sortColumn (0, TRUE, TRUE);
	table->setFixedSize(300,150);
	table->setFocusPolicy(QWidget::WheelFocus);
	table->setColumnWidth ( 0, 200 );




	//Frame for 6th Row
	Frame4f = new QFrame(Frame3, "Frame4f" );
    gridLayout1->addWidget(Frame4f,6,0,0);
	QHBoxLayout *hBoxLayout4f = new QHBoxLayout( Frame4f, 0 );
	hBoxLayout4f->addSpacing(15);

	Line1 = new QFrame( Frame4f, "Line1" );
    Line1->setGeometry( QRect( 180, 110, 291, 31 ) ); 
    Line1->setProperty( "frameShape", (int)QFrame::HLine );
    Line1->setFrameShadow( QFrame::Sunken );
    Line1->setFrameShape( QFrame::HLine );
	hBoxLayout4f->addWidget(Line1);





	
	
	//Frame for 7th Row
	Frame4g = new QFrame(Frame3, "Frame4g" );
    gridLayout1->addWidget(Frame4g,7,0,0);
	QHBoxLayout *hBoxLayout4g = new QHBoxLayout( Frame4g, 0 );
	hBoxLayout4g->addSpacing(15);
	
	TextLabel6 = new QLabel( "Flux", Frame4g);
    hBoxLayout4g->addWidget(TextLabel6);
	hBoxLayout4g->addSpacing(84);

    LineEdit3 = new QLineEdit( "",Frame4g);
	hBoxLayout4g->addWidget(LineEdit3);
	LineEdit3->setEnabled(false);
	hBoxLayout4g->addSpacing(250);
		


	
	//for the commit and cancel buttons
        
	Frame4h= new QFrame( Frame3, "Frame4h" );
    gridLayout1->addWidget(Frame4h,8,0,0);
	QHBoxLayout *hBoxLayout4h = new QHBoxLayout( Frame4h, 0 );
	hBoxLayout4h->addSpacing(15);

	
	commitChanges = new QPushButton("&Commit Changes", Frame4h);
	cancelChanges = new QPushButton("&Cancel Changes", Frame4h);
	hBoxLayout4h->addWidget(commitChanges);
	hBoxLayout4h->addSpacing(15);
	hBoxLayout4h->addWidget(cancelChanges);
	hBoxLayout4h->addSpacing(15);
   
}


/*This function is used to connect this class to the listviews 
    class to basically choose the right widget to display   */
int ReactionsWidget1::isName(QString setValue)
{

	if (mModel == NULL)
	{ 
		return 0;
	}
	

	CCopasiVectorNS < CReaction > & reactions = mModel->getReactions();

		
		CReaction *reactn1;
		reactn1 = reactions[(string) setValue];

		
	if(reactn1 !=NULL)
	{	
		loadName(setValue);
		return 1;
	}
	else
		return 0;
}





/*This function is to load the model for the reactions*/
void ReactionsWidget1::loadReactions(CModel *model)
{
	if (model != NULL)
	{
		mModel = model;
	}
}





/* This function loads the reactions widget when its name is 
   clicked in the tree   */
void ReactionsWidget1::loadName(QString setValue)
{
	
    if (mModel == NULL)
	{ return;
	}
	
	
		CCopasiVectorNS < CReaction > & reactions = mModel->getReactions();
		C_INT32 noOfReactionsRows = reactions.size();
		
		CReaction *reactn;
		CChemEq * chemEq;
		CFunction *function;


		reactn = reactions[(string)setValue];
		

		LineEdit1->setText(reactn->getName().c_str());

		chemEq = & reactn->getChemEq();
	    LineEdit2->setText(chemEq->getChemicalEquation().c_str());
			
		LineEdit3->setText(QString::number(reactn->getFlux()));

	
		for (C_INT32  j = 0; j < noOfReactionsRows ; j++)
		{
		   int m=-1;
		   reactn = reactions[j];
		   function = &reactn->getFunction();
		   ComboBox1->insertItem(function->getName().c_str(),m);

		   if(reactn->isReversible() == TRUE)
			{
				checkBox->setChecked(TRUE);
			}

			
	   table->setNumCols( 1 );
	   QHeader *tableHeader1 = table->horizontalHeader();
	   QHeader *tableHeader2 = table->verticalHeader();
       CCopasiVector < CReaction::CId2Metab> & react3 = reactn->getId2Modifiers();
	   if(	react3.size()!=0)
	   {
	     table->setNumRows( 5 );
	     tableHeader2->setLabel(3,"ParameterName");
         tableHeader2->setLabel(4,"ParameterValue");
	   }
	   else
	   { 
	    table->setNumRows( 4 );
	    tableHeader2->setLabel(2,"ParameterName");
        tableHeader2->setLabel(3,"ParameterValue");
	   };

	  //Setting table headers

	  tableHeader1->setLabel(0, "Value");
	  tableHeader2->setLabel(0,"Substrates");
      tableHeader2->setLabel(1,"Products");
	  table->setColumnWidth (0, 200); 
	
	  QString overall2="{";
	  QString overall4="}";


//CCopasiVector < CReaction::CId2Metab > & react1 = reactn->getId2Substrates();
		const CCopasiVector < CChemEqElement > &react1 = reactn->getChemEq().getSubstrates();
	  	for ( int k = 0;k <react1.size(); ++k );
		{
	    QString overall1=react1[k]->getMetaboliteName().c_str();
	    QString & overall1_2=overall1.operator+=(overall2) ;
		QString overall3=react1[k]->getCompartmentName().c_str();
		QString & overall3_4=overall3.operator+=(overall4) ;
		QString & overall=overall1_2.operator+=(overall3_4) ;
		//for the combo box
		QStringList comboEntries1;
		comboEntries1= overall;
		QComboTableItem * item = new QComboTableItem(table, comboEntries1, FALSE );
		table->setItem( 0, 0, item );
	  }
	


	  


	}

}

///end of all the functions
