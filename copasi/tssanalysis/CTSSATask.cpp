// Copyright (C) 2010 - 2015 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 - 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/**
 * CTSSATask class.
 *
 * This class implements a time scale separation analysis task which is comprised of a
 * of a problem and a method. Additionally calls to the reporting
 * methods are done when initialized.
 *
 */

#include <string>

#include "copasi.h"

#include "CTSSATask.h"
#include "CTSSAProblem.h"
#include "CTSSAMethod.h"
#include "math/CMathContainer.h"
#include "report/CKeyFactory.h"
#include "report/CReport.h"
#include "utilities/CProcessReport.h"
#include "utilities/CCopasiException.h"
#include  "CopasiDataModel/CCopasiDataModel.h"

#define XXXX_Reporting

bool tfle(const C_FLOAT64 & d1, const C_FLOAT64 & d2)
{return (d1 <= d2);}

bool tfl(const C_FLOAT64 & d1, const C_FLOAT64 & d2)
{return (d1 < d2);}

bool tble(const C_FLOAT64 & d1, const C_FLOAT64 & d2)
{return (d1 >= d2);}

bool tbl(const C_FLOAT64 & d1, const C_FLOAT64 & d2)
{return (d1 > d2);}

// static
const CTaskEnum::Method CTSSATask::ValidMethods[] =
{
  CTaskEnum::tssILDM,
  CTaskEnum::tssILDMModified,
  CTaskEnum::tssCSP,
  CTaskEnum::UnsetMethod
};

CTSSATask::CTSSATask(const CCopasiContainer * pParent,
                     const CTaskEnum::Task & type):
  CCopasiTask(pParent, type),
  mTimeSeriesRequested(true),
  mTimeSeries(),
  mpTSSAProblem(NULL),
  mpTSSAMethod(NULL),
  mContainerState(),
  mpContainerStateTime(NULL)
{
  mpProblem = new CTSSAProblem(this);
  mpMethod = createMethod(CTaskEnum::tssILDM);

  CCopasiParameter * pParameter = mpMethod->getParameter("Integrate Reduced Model");

  if (pParameter != NULL)
    mUpdateMoieties = pParameter->getValue< bool >();
  else
    mUpdateMoieties = false;
}

CTSSATask::CTSSATask(const CTSSATask & src,
                     const CCopasiContainer * pParent):
  CCopasiTask(src, pParent),
  mTimeSeriesRequested(src.mTimeSeriesRequested),
  mTimeSeries(),
  mpTSSAProblem(NULL),
  mpTSSAMethod(NULL),
  mContainerState(),
  mpContainerStateTime(NULL)
{
  mpProblem =
    new CTSSAProblem(*static_cast< CTSSAProblem * >(src.mpProblem), this);

  mpMethod = createMethod(src.mpMethod->getSubType());
  * mpMethod = * src.mpMethod;
  mpMethod->elevateChildren();

  this->add(mpMethod, true);

  CCopasiParameter * pParameter = mpMethod->getParameter("Integrate Reduced Model");

  if (pParameter != NULL)
    mUpdateMoieties = pParameter->getValue< bool >();
  else
    mUpdateMoieties = false;
}

CTSSATask::~CTSSATask()
{}

bool CTSSATask::updateMatrices()
{
  assert(mpProblem != NULL && mpMethod != NULL);

  assert(dynamic_cast<CTSSAProblem *>(mpProblem) != NULL);

  if (!mpMethod->isValidProblem(mpProblem)) return false;

  CTSSAMethod * pMethod = dynamic_cast<CTSSAMethod*>(mpMethod);

  if (!pMethod) return false;

  pMethod->predifineAnnotation();

  return true;
}

bool CTSSATask::initialize(const OutputFlag & of,
                           COutputHandler * pOutputHandler,
                           std::ostream * pOstream)
{
  assert(mpProblem && mpMethod);

  mpTSSAProblem = dynamic_cast<CTSSAProblem *>(mpProblem);
  assert(mpTSSAProblem);

  mpTSSAMethod = dynamic_cast<CTSSAMethod *>(mpMethod);
  assert(mpTSSAMethod);

  mpTSSAMethod->setProblem(mpTSSAProblem);

  bool success = mpMethod->isValidProblem(mpProblem);

  CCopasiParameter * pParameter = mpMethod->getParameter("Integrate Reduced Model");

  if (pParameter != NULL)
    mUpdateMoieties = pParameter->getValue< bool >();
  else
    mUpdateMoieties = false;

  // Handle the time series as a regular output.
  mTimeSeriesRequested = mpTSSAProblem->timeSeriesRequested();

  if (pOutputHandler != NULL)
    {
      if (mTimeSeriesRequested)
        {
          mTimeSeries.allocate(mpTSSAProblem->getStepNumber());
          pOutputHandler->addInterface(&mTimeSeries);
        }
      else
        {
          mTimeSeries.clear();
        }
    }

  mpTSSAMethod->predifineAnnotation();

  success &= CCopasiTask::initialize(of, pOutputHandler, pOstream);

  mContainerState.initialize(mpContainer->getState(mUpdateMoieties));
  mpContainerStateTime = mContainerState.array() + mpContainer->getTimeIndex();

  return success;
}

bool CTSSATask::process(const bool & useInitialValues)
{
  //*****

  processStart(useInitialValues);

  //*****

  C_FLOAT64 StepSize = mpTSSAProblem->getStepSize();
  C_FLOAT64 NextTimeToReport;

  const C_FLOAT64 EndTime = *mpContainerStateTime + mpTSSAProblem->getDuration();
  const C_FLOAT64 StartTime = *mpContainerStateTime;

  C_FLOAT64 StepNumber = (mpTSSAProblem->getDuration()) / StepSize;

  bool (*LE)(const C_FLOAT64 &, const C_FLOAT64 &);
  bool (*L)(const C_FLOAT64 &, const C_FLOAT64 &);

  if (StepSize < 0.0)
    {
      LE = &tble;
      L = &tbl;
    }
  else
    {
      LE = &tfle;
      L = &tfl;
    }

  size_t StepCounter = 1;

  C_FLOAT64 outputStartTime = mpTSSAProblem->getOutputStartTime();

  if (StepSize == 0.0 && mpTSSAProblem->getDuration() != 0.0)
    {
      CCopasiMessage(CCopasiMessage::ERROR, MCTSSAProblem + 1, StepSize);
      return false;
    }

  output(COutputInterface::BEFORE);

  bool flagProceed = true;
  C_FLOAT64 handlerFactor = 100.0 / mpTSSAProblem->getDuration();

  C_FLOAT64 Percentage = 0;
  size_t hProcess;

  if (mpCallBack)
    {
      mpCallBack->setName("performing simulation...");
      C_FLOAT64 hundred = 100;
      hProcess = mpCallBack->addItem("Completion",
                                     Percentage,
                                     &hundred);
    }

  //if ((*LE)(outputStartTime, *mpContainerStateTime)) output(COutputInterface::DURING);

  try
    {
      do
        {
          // This is numerically more stable then adding
          // mpTSSAProblem->getStepSize().
          NextTimeToReport =
            StartTime + (EndTime - StartTime) * StepCounter++ / StepNumber;

          flagProceed &= processStep(NextTimeToReport);

          if (mpCallBack)
            {
              Percentage = (*mpContainerStateTime - StartTime) * handlerFactor;
              flagProceed &= mpCallBack->progressItem(hProcess);
            }

          if ((*LE)(outputStartTime, *mpContainerStateTime))
            {
              output(COutputInterface::DURING);
            }
        }
      while ((*L)(*mpContainerStateTime, EndTime) && flagProceed);
    }

  catch (int)
    {
      mpContainer->updateSimulatedValues(mUpdateMoieties);

      if ((*LE)(outputStartTime, *mpContainerStateTime))
        {
          output(COutputInterface::DURING);
        }

      if (mpCallBack) mpCallBack->finishItem(hProcess);

      output(COutputInterface::AFTER);

      CCopasiMessage(CCopasiMessage::EXCEPTION, MCTSSAMethod + 4);
    }

  catch (CCopasiException & Exception)
    {
      mpContainer->updateSimulatedValues(mUpdateMoieties);

      if ((*LE)(outputStartTime, *mpContainerStateTime))
        {
          output(COutputInterface::DURING);
        }

      if (mpCallBack) mpCallBack->finishItem(hProcess);

      output(COutputInterface::AFTER);

      throw CCopasiException(Exception.getMessage());
    }

  if (mpCallBack) mpCallBack->finishItem(hProcess);

  output(COutputInterface::AFTER);

  return true;
}

void CTSSATask::processStart(const bool & useInitialValues)
{
  if (useInitialValues)
    {
      mpContainer->applyInitialValues();
    }

  mContainerState.initialize(mpContainer->getState(mUpdateMoieties));
  mpTSSAMethod->start();

  return;
}

bool CTSSATask::processStep(const C_FLOAT64 & nextTime)
{
  C_FLOAT64 CompareTime = nextTime - 100.0 * fabs(nextTime) * std::numeric_limits< C_FLOAT64 >::epsilon();

  if (*mpContainerStateTime <= CompareTime)
    {
      do
        {
          mpTSSAMethod->step(nextTime - *mpContainerStateTime);

          if (*mpContainerStateTime > CompareTime) break;

          /* Here we will do conditional event processing */

          /* Currently this is correct since no events are processed. */
          CCopasiMessage(CCopasiMessage::EXCEPTION, MCTSSAMethod + 3);
        }
      while (true);

      mpContainer->updateSimulatedValues(mUpdateMoieties);

      return true;
    }

  CompareTime = nextTime + 100.0 * fabs(nextTime) * std::numeric_limits< C_FLOAT64 >::epsilon();

  if (*mpContainerStateTime >= CompareTime)
    {
      do
        {
          mpTSSAMethod->step(nextTime - *mpContainerStateTime);

          if (*mpContainerStateTime < CompareTime) break;

          /* Here we will do conditional event processing */

          /* Currently this is correct since no events are processed. */
          CCopasiMessage(CCopasiMessage::EXCEPTION, MCTSSAMethod + 3);
        }
      while (true);

      mpContainer->updateSimulatedValues(mUpdateMoieties);

      return true;
    }

  // Current time is approximately nextTime;
  return false;
}

bool CTSSATask::restore()
{
  bool success = CCopasiTask::restore();

  if (mUpdateModel)
    {
      mpContainer->updateSimulatedValues(true);
      mpContainer->setInitialState(mpContainer->getState(false));
      mpContainer->updateInitialValues(CModelParameter::ParticleNumbers);
      mpContainer->pushInitialState();
    }

  return success;
}

// virtual
const CTaskEnum::Method * CTSSATask::getValidMethods() const
{
  return CTSSATask::ValidMethods;
}

const CTimeSeries & CTSSATask::getTimeSeries() const
{return mTimeSeries;}
