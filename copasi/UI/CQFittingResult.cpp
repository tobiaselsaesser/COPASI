// Begin CVS Header
//   $Source: /fs/turing/cvs/copasi_dev/cvs_admin/addHeader,v $
//   $Revision: 1.13 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2010/03/21 15:21:13 $
// End CVS Header

// Copyright (C) 2011 - 2010 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

/*
 *  CQFittingResult.cpp
 *  Created by Paul on 4/2/10.
 */

#include "CQFittingResult.h"

#include "copasi.h"

#include "q3header.h"
#include "qregexp.h"

#include "UI/CopasiFileDialog.h"
#include "UI/CQMessageBox.h"

#include "CopasiDataModel/CCopasiDataModel.h"
#include "parameterFitting/CFitTask.h"
#include "parameterFitting/CFitProblem.h"
#include "parameterFitting/CFitItem.h"
#include "parameterFitting/CExperimentSet.h"
#include "parameterFitting/CExperiment.h"
#include "report/CCopasiRootContainer.h"
#include "commandline/CLocaleString.h"
#include "model/CModel.h"

#include "UI/qtUtilities.h"

/*
 *  Constructs a CQFittingResult which is a child of 'parent', with the
 *  name 'name'.'
 */
CQFittingResult::CQFittingResult(QWidget* parent, const char* name)
    : CopasiWidget(parent, name)
{
  setupUi(this);

  init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
CQFittingResult::~CQFittingResult()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void CQFittingResult::languageChange()
{
  retranslateUi(this);
}

void CQFittingResult::init()
{
  size_t i, imax;

  // Set up the parameters table
  mpParameters->setNumCols(mpParameters->numCols() + 1);
  mpParameters->horizontalHeader()->setLabel(mpParameters->numCols() - 1, tr("Parameter"));
  mpParameters->setNumCols(mpParameters->numCols() + 1);
  mpParameters->horizontalHeader()->setLabel(mpParameters->numCols() - 1, tr("Value"));
  mpParameters->setNumCols(mpParameters->numCols() + 1);
  mpParameters->horizontalHeader()->setLabel(mpParameters->numCols() - 1, tr("Std. Deviation"));
  mpParameters->setNumCols(mpParameters->numCols() + 1);
  mpParameters->horizontalHeader()->setLabel(mpParameters->numCols() - 1, tr("Coeff. of Variation [%]"));
  mpParameters->setNumCols(mpParameters->numCols() + 1);
  mpParameters->horizontalHeader()->setLabel(mpParameters->numCols() - 1, tr("Gradient"));
  mpParameters->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, 0, 0, mpParameters->sizePolicy().hasHeightForWidth()));
  mpParameters->setNumRows(0);
  mpParameters->setNumCols(5);
  mpParameters->setReadOnly(true);

  for (i = 0, imax = mpParameters->numCols(); i != imax; i++)
    mpParameters->adjustColumn((int) i);

  // Set up the experiments table
  mpExperiments->setNumCols(mpExperiments->numCols() + 1);
  mpExperiments->horizontalHeader()->setLabel(mpExperiments->numCols() - 1, tr("Experiment"));
  mpExperiments->setNumCols(mpExperiments->numCols() + 1);
  mpExperiments->horizontalHeader()->setLabel(mpExperiments->numCols() - 1, tr("Objective Value"));
  mpExperiments->setNumCols(mpExperiments->numCols() + 1);
  mpExperiments->horizontalHeader()->setLabel(mpExperiments->numCols() - 1, tr("Root Mean Square"));
  mpExperiments->setNumCols(mpExperiments->numCols() + 1);
  mpExperiments->horizontalHeader()->setLabel(mpExperiments->numCols() - 1, tr("Error Mean"));
  mpExperiments->setNumCols(mpExperiments->numCols() + 1);
  mpExperiments->horizontalHeader()->setLabel(mpExperiments->numCols() - 1, tr("Error Mean Std. Deviation"));
  mpExperiments->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, 0, 0, mpExperiments->sizePolicy().hasHeightForWidth()));
  mpExperiments->setNumRows(0);
  mpExperiments->setNumCols(5);
  mpExperiments->setReadOnly(true);

  for (i = 0, imax = mpExperiments->numCols(); i != imax; i++)
    mpExperiments->adjustColumn((int) i);

  // Set up the experiments table
  mpValues->setNumCols(mpValues->numCols() + 1);
  mpValues->horizontalHeader()->setLabel(mpValues->numCols() - 1, tr("Fitted Value"));
  mpValues->setNumCols(mpValues->numCols() + 1);
  mpValues->horizontalHeader()->setLabel(mpValues->numCols() - 1, tr("Objective Value"));
  mpValues->setNumCols(mpValues->numCols() + 1);
  mpValues->horizontalHeader()->setLabel(mpValues->numCols() - 1, tr("Root Mean Square"));
  mpValues->setNumCols(mpValues->numCols() + 1);
  mpValues->horizontalHeader()->setLabel(mpValues->numCols() - 1, tr("Error Mean"));
  mpValues->setNumCols(mpValues->numCols() + 1);
  mpValues->horizontalHeader()->setLabel(mpValues->numCols() - 1, tr("Error Mean Std. Deviation"));
  mpValues->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, 0, 0, mpValues->sizePolicy().hasHeightForWidth()));
  mpValues->setNumRows(0);
  mpValues->setNumCols(5);
  mpValues->setReadOnly(true);

  for (i = 0, imax = mpValues->numCols(); i != imax; i++)
    mpValues->adjustColumn((int) i);

  mpCorrelations->setLegendEnabled(false);
  mpFisherInformation->setLegendEnabled(false);

#ifdef COPASI_CROSSVALIDATION
  // Set up the cross validation table
  mpCrossValidations->setNumCols(mpCrossValidations->numCols() + 1);
  mpCrossValidations->horizontalHeader()->setLabel(mpCrossValidations->numCols() - 1, tr("CV Experiment"));
  mpCrossValidations->setNumCols(mpCrossValidations->numCols() + 1);
  mpCrossValidations->horizontalHeader()->setLabel(mpCrossValidations->numCols() - 1, tr("Objective Value"));
  mpCrossValidations->setNumCols(mpCrossValidations->numCols() + 1);
  mpCrossValidations->horizontalHeader()->setLabel(mpCrossValidations->numCols() - 1, tr("Root Mean Square"));
  mpCrossValidations->setNumCols(mpCrossValidations->numCols() + 1);
  mpCrossValidations->horizontalHeader()->setLabel(mpCrossValidations->numCols() - 1, tr("Error Mean"));
  mpCrossValidations->setNumCols(mpCrossValidations->numCols() + 1);
  mpCrossValidations->horizontalHeader()->setLabel(mpCrossValidations->numCols() - 1, tr("Error Mean Std. Deviation"));
  mpCrossValidations->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, 0, 0, mpExperiments->sizePolicy().hasHeightForWidth()));
  mpCrossValidations->setNumRows(0);
  mpCrossValidations->setNumCols(5);
  mpCrossValidations->setReadOnly(true);

  for (i = 0, imax = mpExperiments->numCols(); i != imax; i++)
    mpCrossValidations->adjustColumn(i);

  // Set up the experiments table
  mpCrossValidationValues->setNumCols(mpCrossValidationValues->numCols() + 1);
  mpCrossValidationValues->horizontalHeader()->setLabel(mpCrossValidationValues->numCols() - 1, tr("CV Fitted Value"));
  mpCrossValidationValues->setNumCols(mpCrossValidationValues->numCols() + 1);
  mpCrossValidationValues->horizontalHeader()->setLabel(mpCrossValidationValues->numCols() - 1, tr("Objective Value"));
  mpCrossValidationValues->setNumCols(mpCrossValidationValues->numCols() + 1);
  mpCrossValidationValues->horizontalHeader()->setLabel(mpCrossValidationValues->numCols() - 1, tr("Root Mean Square"));
  mpCrossValidationValues->setNumCols(mpCrossValidationValues->numCols() + 1);
  mpCrossValidationValues->horizontalHeader()->setLabel(mpCrossValidationValues->numCols() - 1, tr("Error Mean"));
  mpCrossValidationValues->setNumCols(mpCrossValidationValues->numCols() + 1);
  mpCrossValidationValues->horizontalHeader()->setLabel(mpCrossValidationValues->numCols() - 1, tr("Error Mean Std. Deviation"));
  mpCrossValidationValues->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)7, (QSizePolicy::SizeType)5, 0, 0, mpValues->sizePolicy().hasHeightForWidth()));
  mpCrossValidationValues->setNumRows(0);
  mpCrossValidationValues->setNumCols(5);
  mpCrossValidationValues->setReadOnly(true);

  for (i = 0, imax = mpValues->numCols(); i != imax; i++)
    mpCrossValidationValues->adjustColumn(i);

#endif // COPASI_CROSSVALIDATION

#ifndef COPASI_CROSSVALIDATION
  pdelete(mpCrossValidations);
  pdelete(mpCrossValidationValues);
#endif // not COPASI_CROSSVALIDATION
}

bool CQFittingResult::update(ListViews::ObjectType /* objectType */,
                             ListViews::Action /* action */,
                             const std::string & /* key */)
{
  // :TODO:
  return true;
}

bool CQFittingResult::leave()
{
  // :TODO:
  return true;
}

bool CQFittingResult::enterProtected()
{
  assert(CCopasiRootContainer::getDatamodelList()->size() > 0);
  CCopasiDataModel* pDataModel = (*CCopasiRootContainer::getDatamodelList())[0];
  assert(pDataModel != NULL);
  mpTask =
    dynamic_cast<CFitTask *>((*pDataModel->getTaskList())["Parameter Estimation"]);

  if (!mpTask) return false;

  mpProblem = dynamic_cast<const CFitProblem *>(mpTask->getProblem());

  if (!mpProblem) return false;

  mpMain->load(mpProblem);

  if (mpProblem->getCalculateStatistics())
    {
      mpTabWidget->setTabEnabled(mpExperiments, true);
      mpTabWidget->setTabEnabled(mpValues, true);
      mpTabWidget->setTabEnabled(mpCorrelations, true);
      mpTabWidget->setTabEnabled(mpFisherInformation, true);
    }
  else
    {
      mpTabWidget->setTabEnabled(mpExperiments, false);
      mpTabWidget->setTabEnabled(mpValues, false);
      mpTabWidget->setTabEnabled(mpCorrelations, false);
      mpTabWidget->setTabEnabled(mpFisherInformation, false);

#ifdef COPASI_CROSSVALIDATION
      mpTabWidget->setTabEnabled(mpCrossValidations, false);
      mpTabWidget->setTabEnabled(mpCrossValidationValues, false);
#endif // COPASI_CROSSVALIDATION

    }

  size_t i, imax;

  // Loop over the optimization items
  const std::vector< COptItem * > & Items = mpProblem->getOptItemList();
  const CVector< C_FLOAT64 > & Solutions = mpProblem->getSolutionVariables();
  const CVector< C_FLOAT64 > & StdDeviations = mpProblem->getVariableStdDeviations();
  const CVector< C_FLOAT64 > & Gradients = mpProblem->getVariableGradients();

  imax = Items.size();

  if (mpProblem->getFunctionEvaluations() == 0)
    imax = 0;

  mpParameters->setNumRows((int) imax);

  for (i = 0; i != imax; i++)
    {
      const CCopasiObject *pObject =
        pDataModel->getDataObject(Items[i]->getObjectCN());

      if (pObject)
        {
          std::string Experiments =
            static_cast<CFitItem *>(Items[i])->getExperiments();

          if (Experiments != "")
            Experiments = "; {" + Experiments + "}";

          mpParameters->setText((int) i, 0, FROM_UTF8(pObject->getObjectDisplayName() + Experiments));
        }
      else
        mpParameters->setText((int) i, 0, "Not Found");

      const C_FLOAT64 & Solution = Solutions[i];
      mpParameters->setText((int) i, 1, QString::number(Solution));
      const C_FLOAT64 & StdDeviation = StdDeviations[i];
      mpParameters->setText((int) i, 2, QString::number(StdDeviation));
      mpParameters->setText((int) i, 3, QString::number(fabs(100.0 * StdDeviation / Solution)));
      mpParameters->setText((int) i, 4, QString::number(Gradients[i]));
    }

  for (i = 0, imax = mpParameters->numCols(); i != imax; i++)
    mpParameters->adjustColumn((int) i);

  // Loop over the experiments
  const CExperimentSet & Experiments = mpProblem->getExperiementSet();

  imax = Experiments.getExperimentCount();

  if (mpProblem->getFunctionEvaluations() == 0)
    imax = 0;

  mpExperiments->setNumRows((int) imax);

  for (i = 0; i != imax; i++)
    {
      const CExperiment & Experiment = * Experiments.getExperiment(i);
      mpExperiments->setText((int) i, 0, FROM_UTF8(Experiment.getObjectName()));
      mpExperiments->setText((int) i, 1, QString::number(Experiment.getObjectiveValue()));
      mpExperiments->setText((int) i, 2, QString::number(Experiment.getRMS()));

      mpExperiments->setText((int) i, 3, QString::number(Experiment.getErrorMean()));
      mpExperiments->setText((int) i, 4, QString::number(Experiment.getErrorMeanSD()));
    }

  for (i = 0, imax = mpExperiments->numCols(); i != imax; i++)
    mpExperiments->adjustColumn((int) i);

  // Loop over the dependent objects
  imax = Experiments.getDependentObjects().size();

  if (mpProblem->getFunctionEvaluations() == 0)
    imax = 0;

  mpValues->setNumRows((int) imax);

  for (i = 0; i != imax; i++)
    {
      const CCopasiObject * pObject = Experiments.getDependentObjects()[i];

      if (pObject)
        mpValues->setText((int) i, 0, FROM_UTF8(pObject->getObjectDisplayName()));
      else
        mpValues->setText((int) i, 0, "Not Found");

      mpValues->setText((int) i, 1, QString::number(Experiments.getDependentObjectiveValues()[i]));
      mpValues->setText((int) i, 2, QString::number(Experiments.getDependentRMS()[i]));

      mpValues->setText((int) i, 3, QString::number(Experiments.getDependentErrorMean()[i]));
      mpValues->setText((int) i, 4, QString::number(Experiments.getDependentErrorMeanSD()[i]));
    }

  for (i = 0, imax = mpValues->numCols(); i != imax; i++)
    mpValues->adjustColumn((int) i);

  // Fill correlation matrix
  imax = Items.size();

  if (mpProblem->getFunctionEvaluations() == 0)
    imax = 0;

  CColorScaleBiLog * tcs = new CColorScaleBiLog();
  mpCorrelations->setColorCoding(tcs);
  mpCorrelations->setColorScalingAutomatic(true);
  mpCorrelations->setArrayAnnotation(&mpProblem->getCorrelations());

  tcs = new CColorScaleBiLog();
  mpFisherInformation->setColorCoding(tcs);
  mpFisherInformation->setColorScalingAutomatic(true);
  mpFisherInformation->setArrayAnnotation(&mpProblem->getFisherInformation());

#ifdef COPASI_CROSSVALIDATION
  bool Enable = (mpProblem->getCrossValidationSet().getExperimentCount() > 0);

  mpTabWidget->setTabEnabled(mpCrossValidations, Enable);
  mpTabWidget->setTabEnabled(mpCrossValidationValues, Enable);

  // Loop over the cross validation
  const CCrossValidationSet & CrossValidations = mpProblem->getCrossValidationSet();

  imax = CrossValidations.getExperimentCount();

  if (mpProblem->getFunctionEvaluations() == 0)
    imax = 0;

  mpCrossValidations->setNumRows(imax);

  for (i = 0; i != imax; i++)
    {
      const CExperiment & Experiment = * CrossValidations.getExperiment(i);
      mpCrossValidations->setText(i, 0, FROM_UTF8(Experiment.getObjectName()));
      mpCrossValidations->setText(i, 1, QString::number(Experiment.getObjectiveValue()));
      mpCrossValidations->setText(i, 2, QString::number(Experiment.getRMS()));

      mpCrossValidations->setText(i, 3, QString::number(Experiment.getErrorMean()));
      mpCrossValidations->setText(i, 4, QString::number(Experiment.getErrorMeanSD()));
    }

  for (i = 0, imax = mpCrossValidations->numCols(); i != imax; i++)
    mpCrossValidations->adjustColumn(i);

  // Loop over the dependent objects
  imax = CrossValidations.getDependentObjects().size();

  if (mpProblem->getFunctionEvaluations() == 0)
    imax = 0;

  mpCrossValidationValues->setNumRows(imax);

  for (i = 0; i != imax; i++)
    {
      const CCopasiObject * pObject = CrossValidations.getDependentObjects()[i];

      if (pObject)
        mpCrossValidationValues->setText(i, 0, FROM_UTF8(pObject->getObjectDisplayName()));
      else
        mpCrossValidationValues->setText(i, 0, "Not Found");

      mpCrossValidationValues->setText(i, 1, QString::number(CrossValidations.getDependentObjectiveValues()[i]));
      mpCrossValidationValues->setText(i, 2, QString::number(CrossValidations.getDependentRMS()[i]));

      mpCrossValidationValues->setText(i, 3, QString::number(CrossValidations.getDependentErrorMean()[i]));
      mpCrossValidationValues->setText(i, 4, QString::number(CrossValidations.getDependentErrorMeanSD()[i]));
    }

  for (i = 0, imax = mpCrossValidationValues->numCols(); i != imax; i++)
    mpCrossValidationValues->adjustColumn(i);

#endif // COPASI_CROSSVALIDATION

  return true;
}

void CQFittingResult::slotSave(void)
{
  C_INT32 Answer = QMessageBox::No;
  QString fileName;

  while (Answer == QMessageBox::No)
    {
      fileName =
        CopasiFileDialog::getSaveFileName(this, "Save File Dialog",
                                          QString::null, "TEXT Files (*.txt);;All Files (*.*);;", "Save to");

      if (fileName.isEmpty()) return;

      if (!fileName.endsWith(".txt") &&
          !fileName.endsWith(".")) fileName += ".txt";

      fileName = fileName.remove(QRegExp("\\.$"));

      Answer = checkSelection(fileName);

      if (Answer == QMessageBox::Cancel) return;
    }

  std::ofstream file(CLocaleString::fromUtf8(TO_UTF8(fileName)).c_str());

  if (file.fail()) return;

  size_t i, imax;

  // The global result and statistics
  file << "Objective Value\tRoot Mean Square\tStandard Deviation" << std::endl;
  file << mpProblem->getSolutionValue() << "\t";
  file << mpProblem->getRMS() << "\t";
  file << mpProblem->getStdDeviation() << std::endl;

  file << "Function Evaluations\tCPU Time [s]\tEvaluations/second [1/s]" << std::endl;
  const unsigned C_INT32 & FunctionEvaluations = mpProblem->getFunctionEvaluations();
  const C_FLOAT64 & ExecutionTime = mpProblem->getExecutionTime();
  file << FunctionEvaluations << "\t";
  file << ExecutionTime << "\t";
  file << FunctionEvaluations / ExecutionTime << std::endl;

  // Set up the parameters table
  file << std::endl << "Parameters:" << std::endl;
  file << "Parameter\tValue\tStd. Deviation\tCoeff. of Variation [%]\tGradient" << std::endl;

  // Loop over the optimization items
  const std::vector< COptItem * > & Items = mpProblem->getOptItemList();
  const CVector< C_FLOAT64 > & Solutions = mpProblem->getSolutionVariables();
  const CVector< C_FLOAT64 > & StdDeviations = mpProblem->getVariableStdDeviations();
  const CVector< C_FLOAT64 > & Gradients = mpProblem->getVariableGradients();

  imax = Items.size();

  if (mpProblem->getFunctionEvaluations() == 0)
    imax = 0;

  assert(CCopasiRootContainer::getDatamodelList()->size() > 0);
  CCopasiDataModel* pDataModel = (*CCopasiRootContainer::getDatamodelList())[0];
  assert(pDataModel != NULL);

  for (i = 0; i != imax; i++)
    {
      const CCopasiObject *pObject =
        pDataModel->getDataObject(Items[i]->getObjectCN());

      if (pObject)
        {
          std::string Experiments =
            static_cast<CFitItem *>(Items[i])->getExperiments();

          if (Experiments != "")
            Experiments = "; {" + Experiments + "}";

          file << pObject->getObjectDisplayName() << Experiments << "\t";
        }
      else
        file << "Not Found\t";

      const C_FLOAT64 & Solution = Solutions[i];
      file << Solution << "\t";
      const C_FLOAT64 & StdDeviation = StdDeviations[i];
      file << StdDeviation << "\t";
      file << fabs(100.0 * StdDeviation / Solution) << "\t";
      file << Gradients[i] << std::endl;
    }

  // Set up the experiments table
  file << std::endl << "Experiments:" << std::endl;
  file << "Experiment\tObjective Value\tRoot Mean Square\tError Mean\tError Mean Std. Deviation" << std::endl;

  // Loop over the experiments
  const CExperimentSet & Experiments = mpProblem->getExperiementSet();

  imax = Experiments.getExperimentCount();

  if (mpProblem->getFunctionEvaluations() == 0)
    imax = 0;

  for (i = 0; i != imax; i++)
    {
      const CExperiment & Experiment = * Experiments.getExperiment(i);
      file << Experiment.getObjectName() << "\t";
      file << Experiment.getObjectiveValue() << "\t";
      file << Experiment.getRMS() << "\t";
      file << Experiment.getErrorMean() << "\t";
      file << Experiment.getErrorMeanSD() << std::endl;
    }

  // Set up the fitted values table
  file << std::endl << "Fitted Values:" << std::endl;
  file << "Fitted Value\tObjective Value\tRoot Mean Square\tError Mean\tError Mean Std. Deviation" << std::endl;

  // Loop over the fitted values objects
  imax = Experiments.getDependentObjects().size();

  if (mpProblem->getFunctionEvaluations() == 0)
    imax = 0;

  for (i = 0; i != imax; i++)
    {
      const CCopasiObject * pObject = Experiments.getDependentObjects()[i];

      if (pObject)
        file << pObject->getObjectDisplayName() << "\t";
      else
        file << "Not Found\t";

      file << Experiments.getDependentObjectiveValues()[i] << "\t";
      file << Experiments.getDependentRMS()[i] << "\t";
      file << Experiments.getDependentErrorMean()[i] << "\t";
      file << Experiments.getDependentErrorMeanSD()[i] << std::endl;
    }

  file << std::endl;

  // Save the parameter correlations
  file << mpProblem->getCorrelations() << std::endl;

  // Save the Fisher information
  file << mpProblem->getFisherInformation() << std::endl;

#ifdef COPASI_CROSSVALIDATION
  const CCrossValidationSet & CrossValidations = mpProblem->getCrossValidationSet();
  imax = CrossValidations.getExperimentCount();

  if (mpProblem->getFunctionEvaluations() == 0)
    imax = 0;

  if (imax)
    {
      // Set up the cross validations table
      file << std::endl << "Cross Validations:" << std::endl;
      file << "Cross Validation Experiment\t Objective Value\tRoot Mean Square\tError Mean\tError Mean Std. Deviation" << std::endl;

      // Loop over the cross validations
      for (i = 0; i != imax; i++)
        {
          const CExperiment & Experiment = * CrossValidations.getExperiment(i);
          file << Experiment.getObjectName() << "\t";
          file << Experiment.getObjectiveValue() << "\t";
          file << Experiment.getRMS() << "\t";
          file << Experiment.getErrorMean() << "\t";
          file << Experiment.getErrorMeanSD() << std::endl;
        }
    }

  imax = CrossValidations.getDependentObjects().size();

  if (mpProblem->getFunctionEvaluations() == 0)
    imax = 0;

  if (imax)
    {
      // Set up the fitted values table
      file << std::endl << "Cross Validation Fitted Values:" << std::endl;
      file << "CV Fitted Value\tObjective Value\tRoot Mean Square\tError Mean\tError Mean Std. Deviation" << std::endl;

      // Loop over the fitted values objects
      for (i = 0; i != imax; i++)
        {
          const CCopasiObject * pObject = CrossValidations.getDependentObjects()[i];

          if (pObject)
            file << pObject->getObjectDisplayName() << "\t";
          else
            file << "Not Found\t";

          file << CrossValidations.getDependentObjectiveValues()[i] << "\t";
          file << CrossValidations.getDependentRMS()[i] << "\t";
          file << CrossValidations.getDependentErrorMean()[i] << "\t";
          file << CrossValidations.getDependentErrorMeanSD()[i] << std::endl;
        }
    }

  file << std::endl;
#endif // COPASI_CROSSVALIDATION
}

void CQFittingResult::slotUpdateModel()
{
  // Loop over the optimization items
  const std::vector< COptItem * > & Items = mpProblem->getOptItemList();
  std::vector<COptItem * >::const_iterator it = Items.begin();
  std::vector<COptItem * >::const_iterator end = Items.end();

  const C_FLOAT64 * pTmp;

  pTmp = mpProblem->getSolutionVariables().array();

  for (; it != end; ++it, pTmp++)
    {
      (*(*it)->COptItem::getUpdateMethod())(*pTmp);
      (*it)->setStartValue(*pTmp);
    }

  mpProblem->getModel()->updateInitialValues();
}
