// Copyright (C) 2017 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and University of
// of Connecticut School of Medicine.
// All rights reserved.

// Copyright (C) 2010 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 - 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2002 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include <algorithm>
#include <cmath>

#include "copasi.h"

#include "CSteadyStateMethod.h"
#include "CSteadyStateProblem.h"
#include "CSteadyStateTask.h"

#include "CopasiDataModel/CDataModel.h"
#include "copasi/core/CRootContainer.h"
#include "math/CMathContainer.h"
#include "model/CModel.h"
#include "model/CCompartment.h"
#include "trajectory/CTrajectoryTask.h"
#include "trajectory/CTrajectoryProblem.h"
//#include "trajectory/CTrajectoryMethod.h"
#include "utilities/CCopasiException.h"
#include "utilities/utility.h"
#include "utilities/CProcessReport.h"

#include "lapack/lapackwrap.h"
#include "lapack/blaswrap.h"

CNewtonMethod::CNewtonMethod(const CDataContainer * pParent,
                             const CTaskEnum::Method & methodType,
                             const CTaskEnum::Task & taskType):
  CSteadyStateMethod(pParent, methodType, taskType),
  mIpiv(NULL),
  mpTrajectory(NULL),
  mStartState()
{
  initializeParameter();
}

CNewtonMethod::CNewtonMethod(const CNewtonMethod & src,
                             const CDataContainer * pParent):
  CSteadyStateMethod(src, pParent),
  mIpiv(NULL),
  mpTrajectory(NULL),
  mStartState()
{
  initializeParameter();
}

CNewtonMethod::~CNewtonMethod()
{cleanup();}

void CNewtonMethod::initializeParameter()
{
  CCopasiParameter *pParm;

  assertParameter("Use Newton", CCopasiParameter::Type::BOOL, true);
  assertParameter("Use Integration", CCopasiParameter::Type::BOOL, true);
  assertParameter("Use Back Integration", CCopasiParameter::Type::BOOL, true);
  assertParameter("Accept Negative Concentrations", CCopasiParameter::Type::BOOL, false);
  assertParameter("Iteration Limit", CCopasiParameter::Type::UINT, (unsigned C_INT32) 50);
  assertParameter("Maximum duration for forward integration", CCopasiParameter::Type::UDOUBLE, (C_FLOAT64) 1e9);
  assertParameter("Maximum duration for backward integration", CCopasiParameter::Type::UDOUBLE, (C_FLOAT64) 1e6);
  //assertParameter("Force additional Newton step", CCopasiParameter::Type::BOOL, true);
  //assertParameter("Keep Protocol", CCopasiParameter::Type::BOOL, true);

  // Check whether we have a method with the old parameter names
  if ((pParm = getParameter("Newton.UseNewton")) != NULL)
    {
      setValue("Use Newton", pParm->getValue< bool >());
      removeParameter("Newton.UseNewton");

      if ((pParm = getParameter("Newton.UseIntegration")) != NULL)
        {
          setValue("Use Integration", pParm->getValue< bool >());
          removeParameter("Newton.UseIntegration");
        }

      if ((pParm = getParameter("Newton.UseBackIntegration")) != NULL)
        {
          setValue("Use Back Integration", pParm->getValue< bool >());
          removeParameter("Newton.UseBackIntegration");
        }

      if ((pParm = getParameter("Newton.acceptNegativeConcentrations")) != NULL)
        {
          setValue("Accept Negative Concentrations", pParm->getValue< bool >());
          removeParameter("Newton.acceptNegativeConcentrations");
        }

      if ((pParm = getParameter("Newton.IterationLimit")) != NULL)
        {
          setValue("Iteration Limit", pParm->getValue< unsigned C_INT32 >());
          removeParameter("Newton.IterationLimit");
        }

      removeParameter("Newton.LSODA.RelativeTolerance");
      removeParameter("Newton.LSODA.AbsoluteTolerance");
      removeParameter("Newton.LSODA.AdamsMaxOrder");
      removeParameter("Newton.LSODA.BDFMaxOrder");
      removeParameter("Newton.LSODA.MaxStepsInternal");
    }
}

bool CNewtonMethod::elevateChildren()
{
  CSteadyStateMethod::initializeParameter();
  initializeParameter();
  return true;
}

void CNewtonMethod::cleanup()
{
  if (mIpiv) delete [] mIpiv; mIpiv = NULL;

  pdelete(mpTrajectory);
}

void CNewtonMethod::load(CReadConfig & configBuffer,
                         CReadConfig::Mode C_UNUSED(mode))
{
  if (configBuffer.getVersion() < "4.0")
    {
      C_FLOAT64 Dbl;
      C_INT32 Int;
      bool Bool;

      configBuffer.getVariable("SSStrategy", "C_INT32", &Int, CReadConfig::LOOP);

      switch (Int)
        {
          case 0:
            setValue("Use Newton", true);
            setValue("Use Integration", true);
            setValue("Use Back Integration", false);
            break;

          case 1:
            setValue("Use Newton", false);
            setValue("Use Integration", true);
            setValue("Use Back Integration", false);
            break;

          case 2:
            setValue("Use Newton", true);
            setValue("Use Integration", false);
            setValue("Use Back Integration", false);
            break;

          case 3:
            setValue("Use Newton", false);
            setValue("Use Integration", false);
            setValue("Use Back Integration", true);
            break;

          default:
            fatalError();
            break;
        }

      configBuffer.getVariable("SSBackIntegration", "bool", &Bool);
      setValue("Use Back Integration", Bool);

      configBuffer.getVariable("NewtonLimit", "C_INT32", &Int,
                               CReadConfig::SEARCH);
      setValue("Iteration Limit", Int);

      configBuffer.getVariable("SSResoltion", "C_FLOAT64", &Dbl); //typo is necessary!!
      //setValue("Steady State Resolution", Dbl);
      //setValue("Derivation Resolution", Dbl);
      //setValue("Stability Resolution", Dbl);
      setValue("Resolution", Dbl);
    }
}

//**********************************************************************************

CNewtonMethod::NewtonResultCode CNewtonMethod::doIntegration(bool forward)
{
  C_FLOAT64 iterationFactor = forward ? 10.0 : 2.0;
  C_FLOAT64 maxDuration = forward ? mMaxDurationForward : -mMaxDurationBackward;
  //minimum duration is either hardcoded or equal to maximum duration, whichever is smaller.
  C_FLOAT64 minDuration = forward ? (mMaxDurationForward < 1e-1 ? mMaxDurationForward : 1e-1)
                          : -(mMaxDurationBackward < 1e-2 ? mMaxDurationBackward : 1e-2);

  //progress bar
  size_t hProcess;
  unsigned C_INT32 Step = 0;
  unsigned C_INT32 MaxSteps;
  MaxSteps = (unsigned C_INT32) ceil(log(maxDuration / minDuration) / log(iterationFactor));

  std::string tmpstring = forward ? "forward integrating..." : "backward integrating...";

  if (mpCallBack)
    hProcess = mpCallBack->addItem(tmpstring,
                                   Step,
                                   & MaxSteps);

  //setup trajectory
  CTrajectoryProblem * pTrajectoryProblem = NULL;

  if (mpTrajectory)
    {
      mpTrajectory->setCallBack(mpCallBack);

      pTrajectoryProblem =
        dynamic_cast<CTrajectoryProblem *>(mpTrajectory->getProblem());
      assert(pTrajectoryProblem);
      pTrajectoryProblem->setStepNumber(1);
    }

  bool stepLimitReached = false;
  C_FLOAT64 duration;

  for (duration = minDuration; fabs(duration) <= fabs(maxDuration); duration *= iterationFactor, Step++)
    {
      if (mpCallBack && !mpCallBack->progressItem(hProcess)) break;

      pTrajectoryProblem->setDuration(duration);

      try
        {
          // We must not use useInitialValues = true here as this will interfere with scan continuation.
          mpContainer->setState(mStartState);
          stepLimitReached = !mpTrajectory->process(false); //single step
        }

      catch (CCopasiException & /*Exception*/)
        {
          if (mKeepProtocol)
            mMethodLog << "  Integration with duration " << duration << " failed (Exception).\n\n";

          break;
        }

      if (containsNaN())
        {
          if (mKeepProtocol)
            mMethodLog << "  Integration with duration " << duration << " failed (NaN).\n\n";

          break;
        }

      if (!(mAcceptNegative || allPositive()))
        {
          if (mKeepProtocol)
            mMethodLog << "  Integration with duration " << duration
                       << " resulted in negative concentrations.\n\n";

          break;
        }

      calculateDerivativesX();
      C_FLOAT64 value = targetFunction();

      if (isSteadyState(value))
        {
          if (mpCallBack) mpCallBack->finishItem(hProcess);

          if (mKeepProtocol)
            mMethodLog << "  Integration with duration " << duration
                       << ". Criterium matched by " << value << ".\n\n";

          return CNewtonMethod::found;
        }
      else
        {
          if (mKeepProtocol)
            mMethodLog << "  Integration with duration " << duration
                       << ". Criterium not matched by " << value << ".\n\n";
        }

      if (mUseNewton)
        {
          if (mKeepProtocol) mMethodLog << "  Try Newton's method from this starting point. \n";

          NewtonResultCode returnCode = processNewton();

          if (mKeepProtocol) mMethodLog << "\n";

          // mpParentTask->separate(COutputInterface::DURING);

          if (returnCode == CNewtonMethod::found)
            {
              if (mpCallBack) mpCallBack->finishItem(hProcess);

              return CNewtonMethod::found;
            }
        }

      if (stepLimitReached)
        {
          if (mKeepProtocol)
            mMethodLog << "  Integration with duration " << duration
                       << " reached internal step limit.\n";

          break;
        }
    }

  if (mpCallBack) mpCallBack->finishItem(hProcess);

  return CNewtonMethod::notFound;
}

//**********************************************************************************

CSteadyStateMethod::ReturnCode CNewtonMethod::processInternal()
{
  //clear log
  mMethodLog.str("");

  if (mpCallBack)
    mpCallBack->setName("performing steady state calculation...");

  mStartState = mContainerState;

  NewtonResultCode returnCode;

  // Newton
  if (mUseNewton)
    {
      if (mKeepProtocol) mMethodLog << "Try Newton's method. \n";

      returnCode = processNewton();

      if (returnCode == CNewtonMethod::found)
        return returnProcess(true);
    }

  // forward integration
  if (mUseIntegration)
    {
      if (mKeepProtocol) mMethodLog << "\nTry forward integration. \n";

      returnCode = doIntegration(true); //true means forward

      if (returnCode == CNewtonMethod::found)
        return returnProcess(true);
    }

  // backward integration
  if (mUseBackIntegration)
    {
      if (mKeepProtocol) mMethodLog << "\nTry backward integration. \n";

      returnCode = doIntegration(false); //false means backwards

      if (returnCode == CNewtonMethod::found)
        return returnProcess(true);
    }

  return returnProcess(false);
}

//**************************************************************

CNewtonMethod::NewtonResultCode CNewtonMethod::doNewtonStep(C_FLOAT64 & currentValue)
{
  memcpy(mXold.array(), mpX, mDimension * sizeof(C_FLOAT64));

  // DebugFile << "Iteration: " << k << std::endl;

  calculateJacobian(currentValue, true);

  if (solveAxEqB(*mpJacobian, mH, mdxdt) != 0)
    {
      // We need to check that mH != 0
      C_FLOAT64 * pH = mH.array();
      C_FLOAT64 * pHEnd = pH + mH.size();

      for (; pH != pHEnd; ++pH)
        {
          if (fabs(*pH) > 100.0 * std::numeric_limits< C_FLOAT64 >::epsilon())
            break;
        }

      if (pH == pHEnd)
        {
          if (mKeepProtocol)
            mMethodLog << "    Newton step failed. Jacobian could not be inverted.\n\n";

          return CNewtonMethod::singularJacobian;
        }
    }

  C_FLOAT64 newValue = currentValue * 1.001;

  //repeat till the new max rate is smaller than the old.
  //max 32 times
  size_t i;

  for (i = 0; (i < 32) && !((newValue < currentValue)); i++)
    {
      C_FLOAT64 * pXit = mpX;
      C_FLOAT64 * pXoldIt = mXold.array();
      C_FLOAT64 * pHit = mH.array();
      C_FLOAT64 * pEnd = pHit + mDimension;

      for (; pHit != pEnd; ++pHit, ++pXit, ++pXoldIt)
        {
          *pXit = *pXoldIt - *pHit;
          (*pHit) *= 0.5;
        }

      calculateDerivativesX();
      newValue = targetFunction();

      // mpParentTask->output(COutputInterface::DURING);
    }

  if (i == 32)
    {
      //discard the step
      memcpy(mpX, mXold.array(), mDimension * sizeof(C_FLOAT64));

      calculateDerivativesX();
      currentValue = targetFunction();

      if (mKeepProtocol) mMethodLog << "    Newton step failed. Damping limit exceeded.\n";

      return CNewtonMethod::dampingLimitExceeded;

      //       if (isSteadyState(oldMaxRate) && (mAcceptNegative || allPositive()))
      //         ReturnCode = CNewtonMethod::found;
      //       else if (oldMaxRate < *mpSSResolution)
      //         ReturnCode = CNewtonMethod::notFound;
      //       else
      //         ReturnCode = CNewtonMethod::dampingLimitExceeded;

      //if (mpCallBack) mpCallBack->finish(hProcess);
    }

  if (!(mAcceptNegative || allPositive()))
    {
      if (mKeepProtocol)
        mMethodLog << "    Newton step failed. Negative volume or concentration found.\n\n";

      return CNewtonMethod::negativeValueFound;
    }

  currentValue = newValue; //return the new target value

  if (mKeepProtocol)
    {
      if (i <= 1)
        mMethodLog << "    Regular Newton step.      New value: " << currentValue << "\n";
      else
        mMethodLog << "    Newton step with damping. New value: " << currentValue
                   << " (" << i - 1 << " damping iteration(s))\n";
    }

  return CNewtonMethod::stepSuccesful;
}

//************************************************************

CNewtonMethod::NewtonResultCode CNewtonMethod::processNewton()
{
  NewtonResultCode result = CNewtonMethod::notFound;
  unsigned C_INT32 k;

  k = 0;
  //start progress bar
  size_t hProcess;

  if (mpCallBack)
    hProcess = mpCallBack->addItem("Newton method...",
                                   k,
                                   & mIterationLimit);

  C_FLOAT64 targetValue;

  calculateDerivativesX();
  targetValue = targetFunction();

  {
    if (mKeepProtocol) mMethodLog << "   Starting Newton Iterations...\n";

    for (k = 0; k < mIterationLimit && !isSteadyState(targetValue); k++)
      {
        if (mpCallBack && !mpCallBack->progressItem(hProcess)) break;

        result = doNewtonStep(targetValue);

        if (singularJacobian == result) break;

        if (dampingLimitExceeded == result) break;

        if (negativeValueFound == result) break;
      }
  }

  //check if ss was found. If not make sure the correct return value is set
  if (isSteadyState(targetValue))
    result = CNewtonMethod::found;
  else if (CNewtonMethod::stepSuccesful == result)
    result = CNewtonMethod::iterationLimitExceeded;

  //log
  if (mKeepProtocol)
    {
      if (CNewtonMethod::found == result)
        mMethodLog << "   Success: Target criterium matched by " << targetValue << ".\n";
      else if (CNewtonMethod::dampingLimitExceeded == result)
        mMethodLog << "   Failed: Target criterium not matched after reaching iteration limit. " << targetValue << "\n";
    }

  //do an additional Newton step to refine the result
  if ((CNewtonMethod::found == result) && mForceNewton && targetValue > 0.0)
    {
      bool tmp = true;

      ++k; if (mpCallBack && !mpCallBack->progressItem(hProcess)) tmp = false;

      if (tmp)
        {
          if (mKeepProtocol) mMethodLog << "   Do additional step to refine result...\n";

          result = doNewtonStep(targetValue);

          if (CNewtonMethod::stepSuccesful == result)
            result = CNewtonMethod::found;

          if (CNewtonMethod::singularJacobian == result)
            {
              if (mKeepProtocol) mMethodLog << "   Additional step failed. Old values restored.\n";

              result = CNewtonMethod::found;
            }

          if (CNewtonMethod::dampingLimitExceeded == result)
            {
              if (mKeepProtocol) mMethodLog << "   Additional step failed. Old values restored.\n";

              result = CNewtonMethod::found;
            }

          if (CNewtonMethod::negativeValueFound == result)
            {
              if (mKeepProtocol) mMethodLog << "   Additional step failed. Old values restored.\n";

              result = CNewtonMethod::found;
            }
        }
    }

  //end progress bar
  if (mpCallBack) mpCallBack->finishItem(hProcess);

  return result;
}

void CNewtonMethod::calculateDerivativesX()
{
  mpContainer->updateSimulatedValues(true);
}

bool CNewtonMethod::containsNaN() const
{
  return !mpContainer->isStateValid();
}

bool CNewtonMethod::isSteadyState(C_FLOAT64 value)
{
  if (value > *mpSSResolution)
    return false;

  if (containsNaN())
    return false;

  return true;
}

C_FLOAT64 CNewtonMethod::targetFunction()
{
  // New criterion: We solve Jacobian * x = current rates and compare x with the current state
  // Calculate the Jacobian
  calculateJacobian(*mpSSResolution, true);

  CVector< C_FLOAT64 > Distance;

  if (mpJacobian->numCols() == solveAxEqB(*mpJacobian, Distance, mdxdt))
    {
      return std::numeric_limits< C_FLOAT64 >::infinity();
    }

  // We look at all ODE determined entity and dependent species rates.
  C_FLOAT64 * pDistance = Distance.array();
  C_FLOAT64 * pDistanceEnd = pDistance + Distance.size();
  C_FLOAT64 * pCurrentState = mpX;
  C_FLOAT64 * pAtol = mAtol.array();

  // Assure that all values are updated.
  mpContainer->updateSimulatedValues(true);
  mpContainer->applyUpdateSequence(mUpdateConcentrations);

  const CMathObject * pMathObject = mpContainer->getMathObject(mpContainerStateTime + 1);
  C_FLOAT64 ** ppCompartmentVolume = mCompartmentVolumes.array();
  C_FLOAT64 Number2Quantity = mpContainer->getModel().getNumber2QuantityFactor();

  C_FLOAT64 AbsoluteDistance = 0.0; // Largest relative distance
  C_FLOAT64 RelativeDistance = 0.0; // Total relative distance

  C_FLOAT64 tmp;

  for (; pDistance != pDistanceEnd; ++pDistance, ++pCurrentState, ++pAtol, ++pMathObject, ++ppCompartmentVolume)
    {
      *pAtol = std::max(fabs(*pCurrentState), *pAtol);

      // Prevent division by 0
      tmp = *pDistance / *pAtol;
      RelativeDistance += tmp * tmp;

      tmp = *pDistance;

      if (pMathObject->getEntityType() == CMath::EntityType::Species)
        {
          tmp /= mpContainer->getQuantity2NumberFactor() ***ppCompartmentVolume;
        }

      AbsoluteDistance += tmp * tmp;
    }

  RelativeDistance =
    isnan(RelativeDistance) ? std::numeric_limits< C_FLOAT64 >::infinity() : sqrt(RelativeDistance);
  AbsoluteDistance =
    isnan(AbsoluteDistance) ? std::numeric_limits< C_FLOAT64 >::infinity() : sqrt(AbsoluteDistance);

  C_FLOAT64 TargetValue = std::max(RelativeDistance, AbsoluteDistance);

  return TargetValue;
}

//virtual
bool CNewtonMethod::isValidProblem(const CCopasiProblem * pProblem)
{
  if (!CSteadyStateMethod::isValidProblem(pProblem)) return false;

  if (!mpContainer->isAutonomous() &&
      getValue< bool >("Use Newton"))
    CCopasiMessage(CCopasiMessage::WARNING, MCSteadyState + 1);

  //const CSteadyStateProblem * pP = dynamic_cast<const CSteadyStateProblem *>(pProblem);

  if (!((getValue< bool >("Use Newton"))
        || (getValue< bool >("Use Integration"))
        || (getValue< bool >("Use Back Integration"))))
    {
      //would do nothing
      CCopasiMessage(CCopasiMessage::ERROR, "At least one of the features \n   - UseNewton\n   - UseIntegration\n   - UseBackIntegration\nmust be activated.");
      return false;
    }

  if (getValue< C_FLOAT64 >("Maximum duration for forward integration") <= 0)
    {
      CCopasiMessage(CCopasiMessage::ERROR, "Maximum duration for forward integration needs to be positive.");
      return false;
    }

  if (getValue< C_FLOAT64 >("Maximum duration for backward integration") <= 0)
    {
      CCopasiMessage(CCopasiMessage::ERROR, "Maximum duration for backward integration needs to be positive.");
      return false;
    }

  return true;
}

bool CNewtonMethod::initialize(const CSteadyStateProblem * pProblem)
{
  if (!CSteadyStateMethod::initialize(pProblem)) return false;

  CTrajectoryProblem * pTrajectoryProblem = NULL;
  CTrajectoryMethod * pTrajectoryMethod = NULL;

  cleanup();

  /* Configure Newton */
  mUseNewton = mUseIntegration = mUseBackIntegration = mAcceptNegative
                                 = mForceNewton = mKeepProtocol = false;

  if (getValue< bool >("Use Newton"))
    mUseNewton = true;

  if (getValue< bool >("Use Integration"))
    mUseIntegration = true;

  if (getValue< bool >("Use Back Integration"))
    mUseBackIntegration = true;

  if (getValue< bool >("Accept Negative Concentrations"))
    mAcceptNegative = true;

  mForceNewton = true;
  mKeepProtocol = true;

  mIterationLimit = getValue< unsigned C_INT32 >("Iteration Limit");

  mMaxDurationForward = getValue< C_FLOAT64 >("Maximum duration for forward integration");
  mMaxDurationBackward = getValue< C_FLOAT64 >("Maximum duration for backward integration");

  mpX = mContainerStateReduced.array() + mpContainer->getCountFixedEventTargets() + 1;
  mDimension = mContainerStateReduced.size() - mpContainer->getCountFixedEventTargets() - 1;

  mAtol = mpContainer->initializeAtolVector(*mpSSResolution, false);

  mH.resize(mDimension);
  mXold.resize(mDimension);
  mdxdt.initialize(mDimension, mpContainer->getRate(false).array() + mpContainer->getCountFixedEventTargets() + 1);
  mIpiv = new C_INT [mDimension];

  mCompartmentVolumes.resize(mDimension + mpContainer->getCountDependentSpecies());
  mCompartmentVolumes = NULL;

  if (mUseIntegration || mUseBackIntegration)
    {
      // create an appropriate trajectory task
      CDataModel* pDataModel = getObjectDataModel();
      assert(pDataModel != NULL);
      CTrajectoryTask * pSrc =
        dynamic_cast< CTrajectoryTask * >(&pDataModel->getTaskList()->operator[]("Time-Course"));

      if (pSrc)
        mpTrajectory = new CTrajectoryTask(*pSrc, this);
      else
        mpTrajectory = new CTrajectoryTask(this);

      pTrajectoryProblem =
        dynamic_cast<CTrajectoryProblem *>(mpTrajectory->getProblem());
      assert(pTrajectoryProblem);

      if (mpTrajectory->getMethod()->getSubType() != CTaskEnum::Method::deterministic)
        mpTrajectory->setMethodType(CTaskEnum::Method::deterministic);

      pTrajectoryMethod =
        dynamic_cast<CTrajectoryMethod *>(mpTrajectory->getMethod());
      assert(pTrajectoryMethod);

      pTrajectoryProblem->setStepNumber(1);
      pTrajectoryProblem->setStartInSteadyState(false);

      mpTrajectory->initialize(CCopasiTask::NO_OUTPUT, NULL, NULL);
    }

  //
  CObjectInterface::ObjectSet Requested;
  CMathObject * pMathObject = mpContainer->getMathObject(mpX);
  CMathObject * pMathObjectEnd = pMathObject + mDimension;
  C_FLOAT64 ** ppCompartmentVolume = mCompartmentVolumes.array();

  for (; pMathObject != pMathObjectEnd; ++pMathObject, ++ppCompartmentVolume)
    {
      if (pMathObject->getEntityType() == CMath::EntityType::Species)
        {
          Requested.insert(pMathObject->getCorrespondingProperty());
          *ppCompartmentVolume = (C_FLOAT64 *) mpContainer->getCompartment(pMathObject)->getValuePointer();
        }
    }

  mpContainer->getTransientDependencies().getUpdateSequence(mUpdateConcentrations, CCore::SimulationContext::UseMoieties, mpContainer->getStateObjects(true), Requested, mpContainer->getSimulationUpToDateObjects());

  // Determine the compartment volumes for dependent species;
  pMathObjectEnd += mpContainer->getCountDependentSpecies();

  for (; pMathObject != pMathObjectEnd; ++pMathObject, ++ppCompartmentVolume)
    {
      if (pMathObject->getEntityType() == CMath::EntityType::Species)
        {
          *ppCompartmentVolume = (C_FLOAT64 *) mpContainer->getCompartment(pMathObject)->getValuePointer();
        }
    }

  return true;
}

size_t CNewtonMethod::solveAxEqB(const CMatrix< C_FLOAT64 > & jacobian,
                                 CVector< C_FLOAT64 > & X,
                                 const CVectorCore< const C_FLOAT64 > & B) const
{
  assert(B.size() == jacobian.numCols());

  C_INT M = (C_INT) jacobian.numCols();
  C_INT N = (C_INT) jacobian.numRows();

  if (M == 0 || N == 0 || M != N)
    {
      return M;
    }

  X = * reinterpret_cast<const CVectorCore< C_FLOAT64 > * >(&B);

  C_INT LDA = std::max< C_INT >(1, M);
  C_INT NRHS = 1;

  // We need the transpose of the Jacobian;
  CMatrix< C_FLOAT64 > JT(M, N);
  const C_FLOAT64 * mpJ = jacobian.array();
  C_FLOAT64 * mpJTcolumn = JT.array();
  C_FLOAT64 * mpJTcolumnEnd = mpJTcolumn + M;
  C_FLOAT64 * mpJT = JT.array();
  C_FLOAT64 * mpJTEnd = mpJT + JT.size();

  for (; mpJTcolumn != mpJTcolumnEnd; ++mpJTcolumn)
    {
      mpJT = mpJTcolumn;

      for (; mpJT < mpJTEnd; mpJT += M, ++mpJ)
        {
          if (isnan(*mpJ)) return M;

          *mpJT = *mpJ;
        }
    }

  CVector< C_INT > JPVT(M);
  JPVT = 0;

  C_FLOAT64 RCOND = 100.0 * std::numeric_limits< C_FLOAT64 >::epsilon();

  C_INT RANK = 0;

  CVector< C_FLOAT64 > WORK(1);
  C_INT LWORK = -1;
  C_INT INFO;

  /*
      SUBROUTINE DGELSY(M, N, NRHS, A, LDA, B, LDB, JPVT, RCOND, RANK,
     $                   WORK, LWORK, INFO)
   *
   *  -- LAPACK driver routine (version 3.2) --
   *  -- LAPACK is a software package provided by Univ. of Tennessee,    --
   *  -- Univ. of California Berkeley, Univ. of Colorado Denver and NAG Ltd..--
   *     November 2006
   *
   *     .. Scalar Arguments ..
      INTEGER            INFO, LDA, LDB, LWORK, M, N, NRHS, RANK
      DOUBLE PRECISION   RCOND
   *     ..
   *     .. Array Arguments ..
      INTEGER            JPVT(*)
      DOUBLE PRECISION   A(LDA, * ), B(LDB, * ), WORK(*)
   *     ..
   *
   *  Purpose
   *  =======
   *
   *  DGELSY computes the minimum-norm solution to a real linear least
   *  squares problem:
   *      minimize || A * X - B ||
   *  using a complete orthogonal factorization of A.  A is an M-by-N
   *  matrix which may be rank-deficient.
   *
   *  Several right hand side vectors b and solution vectors x can be
   *  handled in a single call; they are stored as the columns of the
   *  M-by-NRHS right hand side matrix B and the N-by-NRHS solution
   *  matrix X.
   *
   *  The routine first computes a QR factorization with column pivoting:
   *      A * P = Q * [ R11 R12 ]
   *                  [  0  R22 ]
   *  with R11 defined as the largest leading submatrix whose estimated
   *  condition number is less than 1/RCOND.  The order of R11, RANK,
   *  is the effective rank of A.
   *
   *  Then, R22 is considered to be negligible, and R12 is annihilated
   *  by orthogonal transformations from the right, arriving at the
   *  complete orthogonal factorization:
   *     A * P = Q * [ T11 0 ] * Z
   *                 [  0  0 ]
   *  The minimum-norm solution is then
   *     X = P * Z' [ inv(T11)*Q1'*B ]
   *                [        0       ]
   *  where Q1 consists of the first RANK columns of Q.
   *
   *  This routine is basically identical to the original xGELSX except
   *  three differences:
   *    o The call to the subroutine xGEQPF has been substituted by the
   *      the call to the subroutine xGEQP3. This subroutine is a Blas-3
   *      version of the QR factorization with column pivoting.
   *    o Matrix B (the right hand side) is updated with Blas-3.
   *    o The permutation of matrix B (the right hand side) is faster and
   *      more simple.
   *
   *  Arguments
   *  =========
   *
   *  M       (input) INTEGER
   *          The number of rows of the matrix A.  M >= 0.
   *
   *  N       (input) INTEGER
   *          The number of columns of the matrix A.  N >= 0.
   *
   *  NRHS    (input) INTEGER
   *          The number of right hand sides, i.e., the number of
   *          columns of matrices B and X. NRHS >= 0.
   *
   *  A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)
   *          On entry, the M-by-N matrix A.
   *          On exit, A has been overwritten by details of its
   *          complete orthogonal factorization.
   *
   *  LDA     (input) INTEGER
   *          The leading dimension of the array A.  LDA >= max(1,M).
   *
   *  B       (input/output) DOUBLE PRECISION array, dimension (LDB,NRHS)
   *          On entry, the M-by-NRHS right hand side matrix B.
   *          On exit, the N-by-NRHS solution matrix X.
   *
   *  LDB     (input) INTEGER
   *          The leading dimension of the array B. LDB >= max(1,M,N).
   *
   *  JPVT    (input/output) INTEGER array, dimension (N)
   *          On entry, if JPVT(i) .ne. 0, the i-th column of A is permuted
   *          to the front of AP, otherwise column i is a free column.
   *          On exit, if JPVT(i) = k, then the i-th column of AP
   *          was the k-th column of A.
   *
   *  RCOND   (input) DOUBLE PRECISION
   *          RCOND is used to determine the effective rank of A, which
   *          is defined as the order of the largest leading triangular
   *          submatrix R11 in the QR factorization with pivoting of A,
   *          whose estimated condition number < 1/RCOND.
   *
   *  RANK    (output) INTEGER
   *          The effective rank of A, i.e., the order of the submatrix
   *          R11.  This is the same as the order of the submatrix T11
   *          in the complete orthogonal factorization of A.
   *
   *  WORK    (workspace/output) DOUBLE PRECISION array, dimension (MAX(1,LWORK))
   *          On exit, if INFO = 0, WORK(1) returns the optimal LWORK.
   *
   *  LWORK   (input) INTEGER
   *          The dimension of the array WORK.
   *          The unblocked strategy requires that:
   *             LWORK >= MAX(MN+3*N+1, 2*MN+NRHS ),
   *          where MN = min(M, N ).
   *          The block algorithm requires that:
   *             LWORK >= MAX(MN+2*N+N*(N+1), 2*MN+NB*NRHS ),
   *          where NB is an upper bound on the blocksize returned
   *          by ILAENV for the routines DGEQP3, DTZRZF, STZRQF, DORMQR,
   *          and DORMRZ.
   *
   *          If LWORK = -1, then a workspace query is assumed; the routine
   *          only calculates the optimal size of the WORK array, returns
   *          this value as the first entry of the WORK array, and no error
   *          message related to LWORK is issued by XERBLA.
   *
   *  INFO    (output) INTEGER
   *          = 0: successful exit
   *          < 0: If INFO = -i, the i-th argument had an illegal value.
   *
   *  Further Details
   *  ===============
   *
   *  Based on contributions by
   *    A. Petitet, Computer Science Dept., Univ. of Tenn., Knoxville, USA
   *    E. Quintana-Orti, Depto. de Informatica, Universidad Jaime I, Spain
   *    G. Quintana-Orti, Depto. de Informatica, Universidad Jaime I, Spain
   *
   *  =====================================================================
   */

  dgelsy_(&M, &N, &NRHS, JT.array(), &LDA, X.array(), &LDA, JPVT.array(), &RCOND, &RANK,
          WORK.array(), &LWORK, &INFO);

  if (INFO < 0)
    {
      return M;
    }

  LWORK = (C_INT) WORK[0];
  WORK.resize(LWORK);

  dgelsy_(&M, &N, &NRHS, JT.array(), &LDA, X.array(), &LDA, JPVT.array(), &RCOND, &RANK,
          WORK.array(), &LWORK, &INFO);

  if (INFO < 0)
    {
      return M;
    }

  C_FLOAT64 Error = 0;

  if (RANK != M)
    {
      // We need to check whether the || Ax - b || is sufficiently small.
      // Calculate Ax
      char T = 'N';
      C_INT One = 1;
      C_FLOAT64 Alpha = 1.0;
      C_FLOAT64 Beta = 0.0;

      CVector< C_FLOAT64 > Ax = * reinterpret_cast<const CVectorCore< C_FLOAT64 > * >(&B);

      dgemm_(&T, &T, &One, &N, &N, &Alpha, X.array(), &One,
             const_cast< C_FLOAT64 * >(jacobian.array()), &N, &Beta, Ax.array(), &One);

      // Calculate absolute and relative error
      C_FLOAT64 *pAx = Ax.array();
      C_FLOAT64 *pAxEnd = pAx + Ax.size();
      const C_FLOAT64 *pB = B.array();
      C_FLOAT64 * pCurrentState = mpX;
      const C_FLOAT64 * pAtol = mAtol.array();

      // Assure that all values are updated.
      mpContainer->updateSimulatedValues(true);
      mpContainer->applyUpdateSequence(mUpdateConcentrations);

      const CMathObject * pMathObject = mpContainer->getMathObject(mpContainerStateTime + 1);
      C_FLOAT64 * const * ppCompartmentVolume = mCompartmentVolumes.array();
      C_FLOAT64 Number2Quantity = mpContainer->getModel().getNumber2QuantityFactor();

      C_FLOAT64 AbsoluteDistance = 0.0; // Largest relative distance
      C_FLOAT64 RelativeDistance = 0.0; // Total relative distance

      C_FLOAT64 tmp;

      for (; pAx != pAxEnd; ++pAx, ++pB, ++pCurrentState, ++pAtol, ++pMathObject, ++ppCompartmentVolume)
        {
          // Prevent division by 0
          tmp = fabs(*pAx - *pB) / std::max(fabs(*pCurrentState), *pAtol);
          RelativeDistance += tmp * tmp;

          tmp = fabs(*pAx - *pB);

          if (pMathObject->getEntityType() == CMath::EntityType::Species)
            {
              tmp /= mpContainer->getQuantity2NumberFactor() ***ppCompartmentVolume;
            }

          AbsoluteDistance += tmp * tmp;
        }

      RelativeDistance =
        isnan(RelativeDistance) ? std::numeric_limits< C_FLOAT64 >::infinity() : sqrt(RelativeDistance);
      AbsoluteDistance =
        isnan(AbsoluteDistance) ? std::numeric_limits< C_FLOAT64 >::infinity() : sqrt(AbsoluteDistance);

      Error = std::max(RelativeDistance, AbsoluteDistance);

      if (Error > *mpSSResolution)
        {
          RANK = 0;
        }
    }

  return M - RANK;
}
