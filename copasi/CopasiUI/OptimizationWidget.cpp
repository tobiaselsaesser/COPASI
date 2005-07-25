/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/CopasiUI/Attic/OptimizationWidget.cpp,v $
   $Revision: 1.72 $
   $Name:  $
   $Author: anuragr $ 
   $Date: 2005/07/25 22:12:50 $
   End CVS Header */

#include <qfiledialog.h>
#include <qvariant.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qframe.h>
#include <qlineedit.h>
#include <qtable.h>
#include <qapplication.h>

#include "copasi.h"

#include "copasiui3window.h"
#include "OptimizationWidget.h"
#include "listviews.h"
#include "CReportDefinitionSelect.h"
#include "OptimizationItemWidget.h"
#include "ObjectBrowserDialog.h"
#include "ObjectBrowserItem.h"
#include "CCopasiSelectionDialog.h"
#include "CScanContainerWidget.h"
#include "qtUtilities.h"
#include "qtUtilities.h"
#include "CProgressBar.h"

//#include "CReportDefinitionSelect.h"
#include "utilities/CCopasiException.h"
#include "utilities/CopasiTime.h"
#include "optimization/COptTask.h"
#include "optimization/COptProblem.h"
#include "optimization/COptMethod.h"
#include "optimization/COptItem.h"
#include "model/CModel.h"
#include "report/CKeyFactory.h"
#include "CopasiDataModel/CCopasiDataModel.h"

OptimizationWidget::OptimizationWidget(QWidget* parent, const char* name, WFlags f)
    : CopasiWidget(parent, name, f), pParent(parent)
{
  if (!name)
    setName("OptimizationWidget");
  setSizePolicy(QSizePolicy((QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, sizePolicy().hasHeightForWidth()));

  optimizationWidgetLayout = new QGridLayout(this, 30, 12, 0, 1, "optimizationWidgetLayout");

  nameLayout = new QHBoxLayout(this);
  nameLabel = new QLabel(this, "nameLabel");
  nameLayout->addWidget(nameLabel, 0, 0);

  taskExecCheck = new QCheckBox(this, "taskExecCheck");
  nameLayout->addWidget(taskExecCheck, 2, Qt::AlignRight);

  optimizationWidgetLayout->addMultiCellLayout(nameLayout, 0, 0, 1, 10);

  expressionTextLayout = new QHBoxLayout(this, 0, 1, "expressionTextLayout");

  expressionEditlabel = new QLabel(this, "expressionEditlabel");
  expressionTextLayout->addWidget(expressionEditlabel, 0, 0);

  expressionText = new QLineEdit(this, "expressionText");
  expressionText->setFrameShape(QLineEdit::LineEditPanel);
  expressionText->setFrameShadow(QLineEdit::Sunken);
  expressionTextLayout->addWidget(expressionText);

  selectParameterButton = new QPushButton(this, "selectParameterButton");
  expressionTextLayout->addWidget(selectParameterButton);

  optimizationWidgetLayout->addMultiCellLayout(expressionTextLayout, 3, 3, 1, 10);

  constraintLayout = new QGridLayout(this, 14, 10, 1, 1, "constraintLayout");

  paramsGroupBox = new QGroupBox(this, "paramsGroupBox");

  constraintLayout->addWidget(paramsGroupBox, 0, 0);

  buttonsLayout = new QHBoxLayout(0, 0, 6, "buttonsLayout");

  confirmButton = new QPushButton(this, "confirmButton");
  buttonsLayout->addWidget(confirmButton);

  runButton = new QPushButton(this, "runButton");
  buttonsLayout->addWidget(runButton);

  cancelButton = new QPushButton(this, "cancelButton");
  buttonsLayout->addWidget(cancelButton);

  reportButton = new QPushButton(this, "reportButton");
  buttonsLayout->addWidget(reportButton);

  outputAssistantButton = new QPushButton(this, "outputAssistantButton");
  buttonsLayout->addWidget(outputAssistantButton);

  constraintLayout->addLayout(buttonsLayout, 1, 0);

  optimizationWidgetLayout->addMultiCellLayout(constraintLayout, 16, 30, 1, 11);

  AddTaskButton = new QPushButton(this, "AddTaskButton");
  AddTaskButton->setAutoMask(TRUE);
  AddTaskButton->setOn(FALSE);

  optimizationWidgetLayout->addMultiCellWidget(AddTaskButton, 11, 14, 8, 10);

  methodGroupBoxLayout = new QVBoxLayout(this);

  methodLabel = new QLabel(this, "methodLabel");
  methodGroupBoxLayout->addWidget(methodLabel);

  methodCombo = new QComboBox(FALSE, this, "methodCombo");
  methodGroupBoxLayout->addWidget(methodCombo);

  parameterTable = new QTable(this, "parameterTable");
  parameterTable->setNumRows(3);
  parameterTable->setNumCols(1);
  methodGroupBoxLayout->addWidget(parameterTable);

  optimizationWidgetLayout->addMultiCellLayout(methodGroupBoxLayout, 5, 13, 1, 5);

  typeGroupBoxLayout = new QGridLayout(this);
  typeGroupBoxLayout->setAlignment(Qt::AlignTop);

  typeLabel = new QLabel(this, "typeLabel");
  typeGroupBoxLayout->addWidget(typeLabel , 0, 0);

  steadystateCheck = new QCheckBox(this, "steadystateCheck");
  typeGroupBoxLayout->addWidget(steadystateCheck, 2, 0);

  timeCheck = new QCheckBox(this, "timeCheck");
  typeGroupBoxLayout->addWidget(timeCheck, 4, 0);

  optimizationWidgetLayout->addMultiCellLayout(typeGroupBoxLayout, 5, 10, 8, 10);

  languageChange();
  resize(QSize(581, 478).expandedTo(minimumSizeHint()));
  clearWState(WState_Polished);

  // tab order

  setTabOrder(expressionText, steadystateCheck);
  setTabOrder(steadystateCheck, timeCheck);
  setTabOrder(timeCheck, parameterTable);
  setTabOrder(parameterTable, confirmButton);
  setTabOrder(confirmButton, cancelButton);
  setTabOrder(cancelButton, reportButton);
  setTabOrder(reportButton, outputAssistantButton);

  //scrollview

  Layoutparams = new QHBoxLayout(paramsGroupBox, 0, 6, "Layout24");
  scrollview = new CScanContainerWidget(paramsGroupBox);
  Layoutparams->addWidget(scrollview);

  mObject = NULL;

  methodCombo->insertItem("Steepest Descent");
  methodCombo->insertItem("Random Search");
  methodCombo->insertItem("Random Search (PVM)");
  methodCombo->insertItem("Simulated Annealing");
  methodCombo->insertItem("Genetic Algorithm");
  methodCombo->insertItem("Evolutionary Program2");
  methodCombo->insertItem("Hybrid GA/SA");
  methodCombo->insertItem("Genetic Algorithm SR");

  // signals and slots connections

  // * connect the combo box with the respective slot
  connect(methodCombo, SIGNAL(activated(int)), this, SLOT(changeMethod(int)));

  connect(timeCheck, SIGNAL(clicked()), this, SLOT(slotTimechecked()));
  connect(steadystateCheck, SIGNAL(clicked()), this, SLOT(slotSteadystatechecked()));
  connect(taskExecCheck, SIGNAL(clicked()), this, SLOT(slottaskExecCheck()));

  // connect the buttons with the respective slots
  connect(runButton, SIGNAL(clicked()), this, SLOT(runOptimizationTask()));
  connect(AddTaskButton , SIGNAL(clicked()), this, SLOT(slotAddItem()));
  connect(selectParameterButton , SIGNAL(clicked()), this, SLOT(slotChooseObject()));
  connect(confirmButton, SIGNAL(clicked()), this, SLOT(slotConfirm()));
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(CancelChangeButton()));
  connect(reportButton, SIGNAL(clicked()), this, SLOT(ReportDefinitionClicked()));

  //reportDefinitionButton->setEnabled(false);

  // for the default option
  timeCheck->setChecked(true);
  taskExecCheck->setChecked(false);
}

OptimizationWidget::~OptimizationWidget()
{}

void OptimizationWidget::CancelChangeButton()
{
  loadOptimization();
}

void OptimizationWidget::CheckBoxClicked()
{}

void OptimizationWidget::runOptimizationTask()
{
  COptTask* optimizationTask =
    dynamic_cast< COptTask * >(GlobalKeys.get(optimizationTaskKey));

  if (!optimizationTask) return;

  // save the state of the widget
  saveOptimization();

  static_cast<CopasiUI3Window *>(qApp->mainWidget())->autoSave();
  static_cast<CopasiUI3Window *>(qApp->mainWidget())->suspendAutoSave(true);

  optimizationTask->initialize(NULL);

  setCursor(Qt::WaitCursor);

  CProgressBar* tmpBar = new CProgressBar();
  optimizationTask->setCallBack(tmpBar);

  try
    {
      optimizationTask->process();
    }
  catch (CCopasiException Exception)
    {
      std::cout << std::endl << "exception in optimization task" << std::endl;
      //TODO: message box
    }

  //should be renamed?

  optimizationTask->restore();

  tmpBar->finish(); pdelete(tmpBar);

  protectedNotify(ListViews::STATE, ListViews::CHANGE,
                  CCopasiDataModel::Global->getModel()->getKey());

  unsetCursor();
  static_cast<CopasiUI3Window *>(qApp->mainWidget())->suspendAutoSave(false);

  return;
}

bool OptimizationWidget::loadOptimization()
{
  COptTask* optimizationTask =
    dynamic_cast< COptTask * >(GlobalKeys.get(optimizationTaskKey));

  if (!optimizationTask) return false;

  COptProblem *optimizationProblem = dynamic_cast<COptProblem *>(optimizationTask->getProblem());
  if (!optimizationProblem) return false;

  mModel = optimizationProblem->getModel();

  scrollview->clearWidgetList();

  //scrollview->updateFromWidgetList();

  COptMethod *optimizationMethod = dynamic_cast<COptMethod*>(optimizationTask->getMethod());
  if (!optimizationMethod) return false;
  // for GA
  methodCombo->setCurrentText(FROM_UTF8(CCopasiMethod::SubTypeName[optimizationMethod->getSubType()]));
  parameterTable->setNumRows(optimizationMethod->size());

  unsigned C_INT32 i;
  for (i = 0; i < optimizationMethod->size(); ++i)
    {
      QHeader *rowHeader = parameterTable->verticalHeader();
      rowHeader->setLabel(i, FROM_UTF8(optimizationMethod->getName(i)));
      CCopasiParameter::Type Type;
      QString value = getParameterValue(optimizationMethod, i, &Type);
      QTableItem * pItem = new QTableItem (parameterTable, QTableItem::Always, value);
      parameterTable->setItem(i, 0, pItem);
    }

  // the optimization items

  if (*optimizationProblem->getValue("Steady-State").pSTRING != "")
    {
      timeCheck->setChecked(false);
      steadystateCheck->setChecked(true);
    }
  else
    {
      timeCheck->setChecked(true);
      steadystateCheck->setChecked(false);
    }

  unsigned C_INT32 imax = optimizationProblem->getOptItemSize();
  for (i = 0; i < imax; ++i)

    {
      OptimizationItemWidget * tmp;
      const COptItem& item = optimizationProblem->getOptItem(i);
      tmp = new OptimizationItemWidget(scrollview);
      tmp->initFromOptItem(item);
      scrollview->addWidget(tmp);
    }

  // load the expression

  std::string objFunc = optimizationProblem->getObjectiveFunction();

  i = 0;

  std::string out_str = "";
  while (i < objFunc.length())
    {
      if (objFunc[i] == '<')
        {
          i++;
          std::string objectName = "";

          while (objFunc[i] != '>' && i < objFunc.length())
            {
              if (objFunc[i] == '\\')
                objectName += objFunc[i++];

              objectName += objFunc[i];
              i++;
            }

          CCopasiObjectName temp_CN(objectName);
          CCopasiObject * temp_object = const_cast<CCopasiObject *>(RootContainer.getObject(temp_CN));
          out_str += "<" + temp_object->getObjectDisplayName() + ">";
          parseList.push_back(temp_object);
          continue;
        }

      else if (objFunc[i] == '>')
        {
          //do nothing
        }

      else
        {
          out_str += objFunc[i];
        }

      i++;
    }

  expressionText->setText(FROM_UTF8(out_str));

  // load the list of objects

  return true;
}

bool OptimizationWidget::slotAddItem()
{
  if (expressionText->text().length() > 0)
    {
      OptimizationItemWidget * tmp;
      tmp = new OptimizationItemWidget(scrollview);
      scrollview->addWidget(tmp);
      return true;
    }

  else
    return false;
}

bool OptimizationWidget::saveOptimization()
{
  COptTask* optimizationTask =
    dynamic_cast< COptTask * >(GlobalKeys.get(optimizationTaskKey));
  if (!optimizationTask) return false;

  COptProblem *optimizationProblem = dynamic_cast<COptProblem *>(optimizationTask->getProblem());
  if (!optimizationProblem) return false;

  COptMethod *optimizationMethod = dynamic_cast<COptMethod*>(optimizationTask->getMethod());
  if (!optimizationMethod) return false;

  unsigned C_INT32 i;
  QTableItem * pItem;
  QString value, strname;

  for (i = 0; i < optimizationMethod->size(); i++)
    {
      pItem = parameterTable->item(i, 0);
      value = pItem->text();
      setParameterValue(optimizationMethod, i, value);
    }

  const std::vector<QWidget*> & widgetList = scrollview->getWidgetList();

  // clearing up the COptItems in COptProblem
  C_INT32 optItemSize = optimizationProblem->getOptItemSize(), numItem;

  for (numItem = optItemSize - 1; numItem >= 0;--numItem)
    optimizationProblem->removeOptItem(numItem);

  unsigned C_INT32 imax = widgetList.size();

  for (i = 0; i < imax; ++i)
    {
      const OptimizationItemWidget* tmp = dynamic_cast<OptimizationItemWidget*>(widgetList[i]);
      if (tmp)
        {
          tmp->saveToOptItem(optimizationProblem);
          continue;
        }
    }

  optItemSize = optimizationProblem->getOptItemSize();
  //CCopasiDataModel::Global->getModel()->compileIfNecessary();
  // optimizationProblem->setInitialState(CCopasiDataModel::Global->getModel()->getInitialState());

  saveExpression(); // save objective function

  if (timeCheck->isChecked())
    {
      optimizationProblem->setValue("Time-Course", (*CCopasiDataModel::Global->getTaskList())["Time-Course"]->getKey());
      optimizationProblem->setValue("Steady-State", std::string(""));
    }
  else
    {
      optimizationProblem->setValue("Time-Course", std::string(""));
      optimizationProblem->setValue("Steady-State", (*CCopasiDataModel::Global->getTaskList())["Steady-State"]->getKey());
    }

  if (taskExecCheck->isChecked())
    {
      optimizationTask->setScheduled(true);
    }

  else
    {
      optimizationTask->setScheduled(false);
    }

  return true;
}

void OptimizationWidget::ReportDefinitionClicked()
{
  CReportDefinitionSelect* pSelectDlg = new CReportDefinitionSelect(pParent);
  COptTask* optimizationTask = (COptTask*)(CCopasiContainer*)GlobalKeys.get(optimizationTaskKey);
  assert(optimizationTask);

  pSelectDlg->setReport(&(optimizationTask->getReport()));
  pSelectDlg->loadReportDefinitionVector();
  if (pSelectDlg->exec () == QDialog::Rejected)
    {
      return;
    }

  delete pSelectDlg;
}

//************* CCopasiWidget interface *******************************

bool OptimizationWidget::enter(const std::string & key)
{
  COptTask* optimizationTask =
    dynamic_cast< COptTask * >(GlobalKeys.get(key));
  if (!optimizationTask) return false;

  mObject = NULL; // invalidate the variable
  parseList.clear();

  optimizationTaskKey = key;

  //pSteadyStateWidget->enter(SteadyStateKey);
  //pTrajectoryWidget->enter(TrajectoryKey);

  // ((COptProblem*)(optimizationTask->getProblem()))->createDebugOptimization(dataModel->getModel());

  loadOptimization();

  return true;
}

bool OptimizationWidget::leave()
{
  COptTask* optimizationTask =
    dynamic_cast< COptTask * >(GlobalKeys.get(optimizationTaskKey));
  if (!optimizationTask) return false;

  return saveOptimization();
}

bool OptimizationWidget::update(ListViews::ObjectType objectType, ListViews::Action C_UNUSED(action), const std::string & C_UNUSED(key))
{
  if (mIgnoreUpdates) return true;

  switch (objectType)
    {
    case ListViews::MODEL:
      // check if there is a list of Report Defs
      /*CReportDefinitionVector* pReportDefinitionVector;
      pReportDefinitionVector = CCopasiDataModel::Global->getReportDefinitionList();
      if (pReportDefinitionVector)
        reportDefinitionButton->setEnabled(true);*/

      break;
    default:
      break;
    }
  return true;
}

std::string OptimizationWidget::getKey()
{
  return optimizationTaskKey;
}

bool OptimizationWidget::changeMethod(int /* index */)
{
  saveOptimization();

  COptTask* optimizationTask =
    dynamic_cast< COptTask * >(GlobalKeys.get(optimizationTaskKey));

  CCopasiMethod::SubType Type =
    CCopasiMethod::TypeNameToEnum((const char *) methodCombo->currentText().utf8());
  optimizationTask->setMethodType(Type);

  loadOptimization();
  return true;
}

void OptimizationWidget::languageChange()
{
  setCaption(tr("Optimization"));
  timeCheck->setText(tr("Time Course"));
  steadystateCheck->setText(tr("Steady State"));
  AddTaskButton->setText(tr("Add"));
  methodLabel->setText(tr("Method\n\n"));
  typeLabel->setText(tr("Type \n\n"));
  nameLabel->setText(tr("<h2>Optimization</h2>"));
  taskExecCheck->setText(tr("Task Executable"));
  confirmButton->setText(tr("confirm"));
  runButton->setText(tr("run"));
  cancelButton->setText(tr("cancel"));
  reportButton->setText(tr("report"));
  outputAssistantButton->setText(tr("output assistant"));
  selectParameterButton->setText(tr("..."));
  expressionEditlabel->setText(tr("Expression"));
  expressionText->setText(QString::null);
}

/* function - slotChooseObject
 * Description -  parsing */ 
/* Author - stupe   */

void OptimizationWidget::slotChooseObject()
{
  std::string mPrevExpr;
  std::string mStr;
  std::string parseMe;
  CCopasiObject* mpObj;
  mModel = CCopasiDataModel::Global->getModel();
  CCopasiObjectName CN;

  CCopasiObject* tmpObject = mObject;
  // open a selection dialog with single selection mode
  CCopasiSelectionDialog* browseDialog = new CCopasiSelectionDialog(this);

  browseDialog->setModel(mModel);

  browseDialog->setSingleSelection(true);
  //browseDialog->enableExpertMode(false);
  std::vector<CCopasiObject*>* selection = new std::vector<CCopasiObject*>();
  if (mObject)
    selection->push_back(mObject);
  browseDialog->setOutputVector(selection);

  if (browseDialog->exec() == QDialog::Accepted && selection->size() != 0)
    {
      mObject = selection->at(0);
      if (mObject)
        {
          parseList.push_back(mObject);
          mPrevExpr = (const char *) expressionText->text().utf8();
          parseMe = CRegisteredObjectName(mObject->getCN());
          mPrevExpr = mPrevExpr.append("<");
          mPrevExpr = mPrevExpr.append(mObject->getObjectDisplayName());
          mPrevExpr = mPrevExpr.append(">");
          //CN = CCopasiObjectName(mObject->getObjectName());
          mpObj = (CCopasiObject*)CCopasiContainer::Root->getObject(parseMe);
          //parseMe =
          //mpObj = (CCopasiObject*)CCopasiContainer::Root->getObject(CN.getRemainder());
          C_FLOAT64 value = *(C_FLOAT64*)mObject->getReference();
          //value = *(C_FLOAT64*)mpObj->getReference();
          /*/mStr = mObject->getObjectDisplayName();
          //const QString mPrevExpr(mStr.c_str());
          //expressionText->remove(expressionText->len
          //expressionText->append(mPrevExpr.truncate(mPrevExpr.lenght()-1));*/

          //mPrevExpr = mPrevExpr.append(mObject->getCN());
          expressionText->setText(FROM_UTF8(mPrevExpr));
          //rowCount = expressionText->numLines();
          //expressionText->setCursorPosition(rowCount ,1000);

          if (mObject != tmpObject)
            {
              if (mObject->isValueDbl())
                {
                  value = *(C_FLOAT64*)mObject->getReference();
                  //lineEditMin->setText(QString::number(value*0.5));
                  //lineEditMax->setText(QString::number(value*2));
                }
            }
        }

      //else
      //expressionName->setText("");
    }

  else
    {}}

/* function - slotParseExpression
 * Description -  parsing */ 
/* Author - stupe   */

bool OptimizationWidget::saveExpression()
{
  std::string DisplayName = "";
  std::string InfixCN = "";

  std::string InfixDispayName = (const char *)expressionText->text().utf8();
  std::vector<CCopasiObject *>::iterator it = parseList.begin();

  for (unsigned int i = 0; i < InfixDispayName.length(); i++)
    {
      InfixCN += InfixDispayName[i];
      DisplayName = "";

      if (InfixDispayName[i] == '<')
        {
          i++;
          while (i < InfixDispayName.length() && InfixDispayName[i] != '>')
            {
              if (InfixDispayName[i] == '\\') // '\' is an escape character.
                DisplayName += InfixDispayName[i++];

              DisplayName += InfixDispayName[i++];
            }

          it = parseList.begin();
          while (it < parseList.end())
            {
              if ((*it)->getObjectDisplayName() == DisplayName)
                {
                  InfixCN += (*it)->getCN();
                  break;
                }

              it++;
            }

          InfixCN += ">";
        }
    }

  COptTask* optimizationTask =
    dynamic_cast< COptTask * >(GlobalKeys.get(optimizationTaskKey));

  if (!optimizationTask) return false;

  COptProblem *optimizationProblem = dynamic_cast<COptProblem *>(optimizationTask->getProblem());
  if (!optimizationProblem) return false;

  optimizationProblem->setObjectiveFunction(InfixCN);

  // :TODO: need to handle errors.
  return true;
}

void OptimizationWidget::slotSteadystatechecked()
{
  timeCheck->setChecked(false);
}

void OptimizationWidget::slotTimechecked()
{
  steadystateCheck->setChecked(false);
}

void OptimizationWidget::slotConfirm()
{
  saveOptimization();
}

void OptimizationWidget::slottaskExecCheck()
{
  COptTask* optimizationTask =
    dynamic_cast< COptTask * >(GlobalKeys.get(optimizationTaskKey));
  if (!optimizationTask) return;

  if (taskExecCheck->isChecked())
    {
      optimizationTask->setScheduled(true);
    }
  else
    {
      optimizationTask->setScheduled(false);
    }
}

//***********************************************************
