// Copyright (C) 2011 - 2015 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#include "CMathContainer.h"
#include "CMathExpression.h"
#include "CMathEventQueue.h"

#include "model/CModel.h"
#include "model/CCompartment.h"
#include "model/CMetab.h"
#include "model/CModelValue.h"
#include "model/CObjectLists.h"
#include "CopasiDataModel/CCopasiDataModel.h"
#include "utilities/CNodeIterator.h"
#include "randomGenerator/CRandom.h"
#include "lapack/blaswrap.h"

// #define DEBUG_OUTPUT
// static
void CMathContainer::createRelocation(const size_t & n, const size_t & o,
                                      CMath::sRelocate & relocate,
                                      std::vector< CMath::sRelocate > & relocations,
                                      const bool & end)
{
  if (n != o)
    {
      if (end)
        {
          relocate.pValueEnd = relocate.pValueEnd + std::min(n, o);
          relocate.pObjectEnd = relocate.pObjectEnd + std::min(n, o);

          if (relocate.pValueStart != relocate.pValueEnd)
            {
              relocations.push_back(relocate);
            }

          relocate.pValueStart = relocate.pValueEnd - std::min(n, o) + o;
          relocate.pObjectStart = relocate.pObjectEnd - std::min(n, o) + o;
          relocate.pValueEnd = relocate.pValueStart;
          relocate.pObjectEnd = relocate.pObjectStart;
          relocate.offset += (n - o);
        }
      else
        {
          if (relocate.pValueStart != relocate.pValueEnd)
            {
              relocations.push_back(relocate);
            }

          relocate.pValueEnd += o;
          relocate.pObjectEnd += o;
          relocate.pValueStart = relocate.pValueEnd - std::min(n, o);
          relocate.pObjectStart = relocate.pObjectEnd - std::min(n, o);
          relocate.offset += (n - o);
        }
    }
  else if (n > 0)
    {
      relocate.pValueEnd += n;
      relocate.pObjectEnd += n;
    }
}

// static
void CMathContainer::relocateUpdateSequence(CObjectInterface::UpdateSequence & sequence,
    const std::vector< CMath::sRelocate > & relocations)
{
  CObjectInterface::UpdateSequence::iterator it = sequence.begin();
  CObjectInterface::UpdateSequence::iterator end = sequence.end();

  for (; it != end; ++it)
    {
      CMathContainer::relocateObject(*it, relocations);
    }
}

// static
void CMathContainer::relocateObjectSet(CObjectInterface::ObjectSet & objectSet,
                                       const std::vector< CMath::sRelocate > & relocations)
{
  CObjectInterface::ObjectSet ObjectSet;
  CObjectInterface::ObjectSet::iterator it = objectSet.begin();
  CObjectInterface::ObjectSet::iterator end = objectSet.end();

  for (; it != end; ++it)
    {
      CObjectInterface * pObject = const_cast< CObjectInterface * >(*it);
      CMathContainer::relocateObject(pObject, relocations);
      ObjectSet.insert(pObject);
    }

  objectSet = ObjectSet;
}

// static
void CMathContainer::relocateValue(C_FLOAT64 *& pValue,
                                   const std::vector< CMath::sRelocate > & relocations)
{
  std::vector< CMath::sRelocate >::const_iterator it = relocations.begin();
  std::vector< CMath::sRelocate >::const_iterator end = relocations.end();

  for (; it != end; ++it)
    if (it->pValueStart <= pValue && pValue < it->pValueEnd)
      {
        pValue = it->pNewValue + (pValue - it->pOldValue) + it->offset;
        break;
      }
}

// static
void CMathContainer::relocateValue(const C_FLOAT64 *& pValue, const std::vector< CMath::sRelocate > & relocations)
{
  relocateValue(const_cast< C_FLOAT64 *& >(pValue), relocations);
}

// static
void CMathContainer::relocateObject(CObjectInterface *& pObject,
                                    const std::vector< CMath::sRelocate > & relocations)
{
  if (pObject == NULL ||
      pObject == pObject->getDataObject())
    {
      return;
    }

  CMathObject * pMathObject = static_cast< CMathObject * >(pObject);
  relocateObject(pMathObject, relocations);

  pObject = pMathObject;
}

// static
void CMathContainer::relocateObject(const CObjectInterface *& pObject, const std::vector< CMath::sRelocate > & relocations)
{
  relocateObject(const_cast< CObjectInterface *& >(pObject), relocations);
}

// static
void CMathContainer::relocateObject(CMathObject *& pObject, const std::vector< CMath::sRelocate > & relocations)
{
  if (pObject == NULL) return;

  std::vector< CMath::sRelocate >::const_iterator it = relocations.begin();
  std::vector< CMath::sRelocate >::const_iterator end = relocations.end();

  for (; it != end; ++it)
    if (it->pObjectStart <= pObject && pObject < it->pObjectEnd)
      {
        pObject = it->pNewObject + (pObject - it->pOldObject) + it->offset;
        break;
      }
}

// static
void CMathContainer::relocateObject(const CMathObject *& pObject, const std::vector< CMath::sRelocate > & relocations)
{
  relocateObject(const_cast< CMathObject *& >(pObject), relocations);
}

CMathContainer::CMathContainer():
  CCopasiContainer("Math Container", NULL, "CMathContainer"),
  mpModel(NULL),
  mpAvogadro(NULL),
  mpQuantity2NumberFactor(NULL),
  mpProcessQueue(new CMathEventQueue(*this)),
  mpRandomGenerator(CRandom::createGenerator()),
  mValues(),
  mpValuesBuffer(NULL),
  mInitialExtensiveValues(),
  mInitialIntensiveValues(),
  mInitialExtensiveRates(),
  mInitialIntensiveRates(),
  mInitialParticleFluxes(),
  mInitialFluxes(),
  mInitialTotalMasses(),
  mInitialEventTriggers(),
  mExtensiveValues(),
  mIntensiveValues(),
  mExtensiveRates(),
  mIntensiveRates(),
  mParticleFluxes(),
  mFluxes(),
  mTotalMasses(),
  mEventTriggers(),
  mEventDelays(),
  mEventPriorities(),
  mEventAssignments(),
  mEventRoots(),
  mEventRootStates(),
  mPropensities(),
  mDependentMasses(),
  mDiscontinuous(),
  mDelayValues(),
  mDelayLags(),
  mTransitionTimes(),
  mInitialState(),
  mState(),
  mStateReduced(),
  mHistory(),
  mHistoryReduced(),
  mRate(),
  mRateReduced(),
  mInitialDependencies(),
  mTransientDependencies(),
  mSynchronizeInitialValuesSequenceExtensive(),
  mSynchronizeInitialValuesSequenceIntensive(),
  mApplyInitialValuesSequence(),
  mSimulationValuesSequence(),
  mSimulationValuesSequenceReduced(),
  mPrioritySequence(),
  mTransientDataObjectSequence(),
  mInitialStateValueExtensive(),
  mInitialStateValueIntensive(),
  mInitialStateValueAll(),
  mStateValues(),
  mReducedStateValues(),
  mSimulationRequiredValues(),
  mObjects(),
  mpObjectsBuffer(NULL),
  mEvents(),
  mpEventsBuffer(NULL),
  mReactions(),
  mpReactionsBuffer(NULL),
  mRootIsDiscrete(),
  mRootIsTimeDependent(),
  mRootProcessors(),
  mCreateDiscontinuousPointer(),
  mDataObject2MathObject(),
  mDataValue2MathObject(),
  mDataValue2DataObject(),
  mDiscontinuityEvents("Discontinuities", this),
  mDiscontinuityInfix2Object(),
  mTriggerInfix2Event(),
  mDelays(),
  mpDelaysBuffer(NULL),
  mIsAutonomous(true),
  mSize()
{
  memset(&mSize, 0, sizeof(mSize));
}

CMathContainer::CMathContainer(CModel & model):
  CCopasiContainer("Math Container", NULL, "CMathContainer"),
  mpModel(&model),
  mpAvogadro(NULL),
  mpQuantity2NumberFactor(NULL),
  mpProcessQueue(new CMathEventQueue(*this)),
  mpRandomGenerator(CRandom::createGenerator()),
  mValues(),
  mpValuesBuffer(NULL),
  mInitialExtensiveValues(),
  mInitialIntensiveValues(),
  mInitialExtensiveRates(),
  mInitialIntensiveRates(),
  mInitialParticleFluxes(),
  mInitialFluxes(),
  mInitialTotalMasses(),
  mInitialEventTriggers(),
  mExtensiveValues(),
  mIntensiveValues(),
  mExtensiveRates(),
  mIntensiveRates(),
  mParticleFluxes(),
  mFluxes(),
  mTotalMasses(),
  mEventTriggers(),
  mEventDelays(),
  mEventPriorities(),
  mEventAssignments(),
  mEventRoots(),
  mEventRootStates(),
  mPropensities(),
  mDependentMasses(),
  mDiscontinuous(),
  mDelayValues(),
  mDelayLags(),
  mTransitionTimes(),
  mInitialState(),
  mState(),
  mStateReduced(),
  mHistory(),
  mHistoryReduced(),
  mRate(),
  mRateReduced(),
  mInitialDependencies(),
  mTransientDependencies(),
  mSynchronizeInitialValuesSequenceExtensive(),
  mSynchronizeInitialValuesSequenceIntensive(),
  mApplyInitialValuesSequence(),
  mSimulationValuesSequence(),
  mSimulationValuesSequenceReduced(),
  mPrioritySequence(),
  mTransientDataObjectSequence(),
  mInitialStateValueExtensive(),
  mInitialStateValueIntensive(),
  mInitialStateValueAll(),
  mStateValues(),
  mReducedStateValues(),
  mSimulationRequiredValues(),
  mObjects(),
  mpObjectsBuffer(NULL),
  mEvents(),
  mpEventsBuffer(NULL),
  mReactions(),
  mpReactionsBuffer(NULL),
  mRootIsDiscrete(),
  mRootIsTimeDependent(),
  mRootProcessors(),
  mDataObject2MathObject(),
  mDataValue2MathObject(),
  mDataValue2DataObject(),
  mDiscontinuityEvents("Discontinuities", this),
  mDiscontinuityInfix2Object(),
  mTriggerInfix2Event(),
  mDelays(),
  mpDelaysBuffer(NULL),
  mIsAutonomous(true),
  mSize()
{
  memset(&mSize, 0, sizeof(mSize));

  // We do not want the model to know about the math container therefore we
  // do not use &model in the constructor of CCopasiContainer
  setObjectParent(mpModel);

  mpAvogadro = mpModel->getObject(CCopasiObjectName("Reference=Avogadro Constant"));
  mpQuantity2NumberFactor = mpModel->getObject(CCopasiObjectName("Reference=Quantity Conversion Factor"));
}

CMathContainer::CMathContainer(const CMathContainer & src):
  CCopasiContainer(src, NULL),
  mpModel(src.mpModel),
  mpAvogadro(src.mpAvogadro),
  mpQuantity2NumberFactor(src.mpQuantity2NumberFactor),
  mpProcessQueue(new CMathEventQueue(*this)),
  mpRandomGenerator(CRandom::createGenerator()),
  mValues(),
  mpValuesBuffer(NULL),
  mInitialExtensiveValues(),
  mInitialIntensiveValues(),
  mInitialExtensiveRates(),
  mInitialIntensiveRates(),
  mInitialParticleFluxes(),
  mInitialFluxes(),
  mInitialTotalMasses(),
  mInitialEventTriggers(),
  mExtensiveValues(),
  mIntensiveValues(),
  mExtensiveRates(),
  mIntensiveRates(),
  mParticleFluxes(),
  mFluxes(),
  mTotalMasses(),
  mEventTriggers(),
  mEventDelays(),
  mEventPriorities(),
  mEventAssignments(),
  mEventRoots(),
  mEventRootStates(),
  mPropensities(),
  mDependentMasses(),
  mDiscontinuous(),
  mDelayValues(),
  mDelayLags(),
  mInitialState(),
  mState(),
  mStateReduced(),
  mHistory(src.mHistory),
  mHistoryReduced(),
  mRate(),
  mRateReduced(),
  mInitialDependencies(src.mInitialDependencies),
  mTransientDependencies(src.mTransientDependencies),
  mSynchronizeInitialValuesSequenceExtensive(src.mSynchronizeInitialValuesSequenceExtensive),
  mSynchronizeInitialValuesSequenceIntensive(src.mSynchronizeInitialValuesSequenceIntensive),
  mApplyInitialValuesSequence(src.mApplyInitialValuesSequence),
  mSimulationValuesSequence(src.mSimulationValuesSequence),
  mSimulationValuesSequenceReduced(src.mSimulationValuesSequenceReduced),
  mPrioritySequence(src.mPrioritySequence),
  mTransientDataObjectSequence(src.mTransientDataObjectSequence),
  mInitialStateValueExtensive(src.mInitialStateValueExtensive),
  mInitialStateValueIntensive(src.mInitialStateValueIntensive),
  mInitialStateValueAll(src.mInitialStateValueAll),
  mStateValues(src.mStateValues),
  mReducedStateValues(src.mReducedStateValues),
  mSimulationRequiredValues(src.mSimulationRequiredValues),
  mObjects(),
  mpObjectsBuffer(NULL),
  mEvents(),
  mpEventsBuffer(NULL),
  mReactions(),
  mpReactionsBuffer(NULL),
  mRootIsDiscrete(src.mRootIsDiscrete),
  mRootIsTimeDependent(src.mRootIsTimeDependent),
  mRootProcessors(src.mRootProcessors),
  mDataObject2MathObject(src.mDataObject2MathObject),
  mDataValue2MathObject(src.mDataValue2MathObject),
  mDataValue2DataObject(src.mDataValue2DataObject),
  mDiscontinuityEvents("Discontinuities", this),
  mDiscontinuityInfix2Object(),
  mTriggerInfix2Event(),
  mDelays(),
  mpDelaysBuffer(NULL),
  mIsAutonomous(src.mIsAutonomous),
  mSize()
{
  // We do not want the model to know about the math container therefore we
  // do not use &model in the constructor of CCopasiContainer
  setObjectParent(mpModel);

  memset(&mSize, 0, sizeof(mSize));
  sSize size = src.mSize;
  std::vector< CMath::sRelocate > Relocations = resize(size);

  mValues = src.mValues;

  // Copy the objects
  CMathObject * pObject = mObjects.array();
  CMathObject * pObjectEnd = pObject + mObjects.size();
  const CMathObject * pObjectSrc = src.mObjects.array();

  for (; pObject != pObjectEnd; ++pObject, ++pObjectSrc)
    {
      pObject->copy(*pObjectSrc, *this);
      pObject->relocate(Relocations);
    }

  CMathEvent * pEvent = mEvents.array();
  CMathEvent * pEventEnd = pEvent + mEvents.size();
  const CMathEvent * pEventSrc = src.mEvents.array();

  for (; pEvent != pEventEnd; ++pEvent, ++pEventSrc)
    {
      pEvent->copy(*pEventSrc, *this);
      pEvent->relocate(Relocations);
    }

  CMathReaction * pReaction = mReactions.array();
  CMathReaction * pReactionEnd = pReaction + mReactions.size();
  const CMathReaction * pReactionSrc = src.mReactions.array();

  for (; pReaction != pReactionEnd; ++pReaction, ++pReactionSrc)
    {
      pReaction->copy(*pReactionSrc, *this);
      pReaction->relocate(Relocations);
    }

  CMathDelay * pDelay = mDelays.array();
  CMathDelay * pDelayEnd = pDelay + mDelays.size();
  const CMathDelay * pDelaySrc = src.mDelays.array();

  for (; pDelay != pDelayEnd; ++pDelay, ++pDelaySrc)
    {
      pDelay->copy(*pDelaySrc, *this);
      pDelay->relocate(Relocations);
    }
}

CMathContainer::~CMathContainer()
{
  pdelete(mpProcessQueue);
  pdelete(mpRandomGenerator);
  pdeletev(mpValuesBuffer)
  pdeletev(mpObjectsBuffer)
}

const CVectorCore< C_FLOAT64 > & CMathContainer::getValues() const
{
  return mValues;
}

CVectorCore< C_FLOAT64 > & CMathContainer::getValues()
{
  return mValues;
}

void CMathContainer::setValues(const CVectorCore< C_FLOAT64 > & values)
{
  assert(mValues.size() == values.size());

  mValues = values;
}

const CVectorCore< C_FLOAT64 > & CMathContainer::getInitialState() const
{
  return mInitialState;
}

CVectorCore< C_FLOAT64 > & CMathContainer::getInitialState()
{
  return mInitialState;
}

void CMathContainer::setInitialState(const CVectorCore< C_FLOAT64 > & initialState)
{
  assert(mInitialState.size() == initialState.size() ||
         mState.size()  == initialState.size());

  if (mInitialState.size() == initialState.size())
    {
      memcpy(mInitialState.array(), initialState.array(), initialState.size() * sizeof(C_FLOAT64));
    }
  else
    {
      memcpy(mInitialState.array() + mSize.nFixed, initialState.array(), initialState.size() * sizeof(C_FLOAT64));
    }
}

const CVectorCore< C_FLOAT64 > & CMathContainer::getState(const bool & reduced) const
{
  if (reduced)
    return mStateReduced;

  return mState;
}

void CMathContainer::setState(const CVectorCore< C_FLOAT64 > & state)
{
  assert(mState.size() >= state.size());

  // We must only copy if the states are different.
  if (mState.array() != state.array())
    {
      memcpy(mState.array(), state.array(), state.size() * sizeof(C_FLOAT64));
    }
}

bool CMathContainer::isStateValid() const
{
  const C_FLOAT64 * pIt = mState.array();
  const C_FLOAT64 * pEnd = pIt + mState.size();

  for (; pIt != pEnd; ++pIt)
    {
      if (isnan(*pIt))
        {
          return false;
        }
    }

  return true;
}

const bool & CMathContainer::isAutonomous() const
{
  return mIsAutonomous;
}

const CMathHistoryCore & CMathContainer::getHistory(const bool & reduced) const
{
  if (reduced)
    {
      return mHistoryReduced;
    }

  return mHistory;
}

void CMathContainer::setHistory(const CMathHistoryCore & history)
{
  assert(history.size() == mHistory.size() &&
         history.rows() == mHistory.rows() &&
         (history.cols() == mHistory.cols() ||
          history.cols() == mHistoryReduced.cols()));

  if (history.cols() < mHistory.cols())
    {
      mHistoryReduced = history;
    }
  else
    {
      mHistory = history;
    }
}

const CVectorCore< C_FLOAT64 > & CMathContainer::getDelayLags() const
{
  return mDelayLags;
}

CVector< C_FLOAT64 > CMathContainer::initializeAtolVector(const C_FLOAT64 & atol, const bool & reduced) const
{
  CVector< C_FLOAT64 > Atol;

  Atol.resize(getState(reduced).size());

  C_FLOAT64 * pAtol = Atol.array();
  C_FLOAT64 * pAtolEnd = pAtol + Atol.size();
  const C_FLOAT64 * pInitialValue = mInitialState.array() + mSize.nFixed;
  const CMathObject * pObject = getMathObject(getState(reduced).array());

  for (; pAtol != pAtolEnd; ++pAtol, ++pObject, ++pInitialValue)
    {
      *pAtol = atol;

      C_FLOAT64 InitialValue = fabs(*pInitialValue);

      switch (pObject->getEntityType())
        {
          case CMath::Species:
          {
            const CMetab * pMetab = static_cast< const CMetab * >(pObject->getDataObject()->getObjectParent());
            std::map< CCopasiObject *, CMathObject * >::const_iterator itFound
              = mDataObject2MathObject.find(pMetab->getCompartment()->getInitialValueReference());

            C_FLOAT64 Limit = fabs(* (C_FLOAT64 *) itFound->second->getValuePointer())
                              ** (C_FLOAT64 *) mpQuantity2NumberFactor->getValuePointer();

            if (InitialValue != 0.0)
              *pAtol *= std::min(Limit, InitialValue);
            else
              *pAtol *= std::max(1.0, Limit);
          }
          break;

          case CMath::GlobalQuantity:
          case CMath::Compartment:

            if (InitialValue != 0.0)
              *pAtol *= std::min(1.0, InitialValue);

            break;

            // These are fixed event targets the absolute tolerance can be large since they do not change
          default:
            *pAtol = std::max(1.0, *pAtol);
        }
    }

  return Atol;
}

const CVectorCore< C_FLOAT64 > & CMathContainer::getRate(const bool & reduced) const
{
  if (reduced)
    return mRateReduced;

  return mRate;
}

const CVectorCore< C_FLOAT64 > & CMathContainer::getTotalMasses() const
{
  return mTotalMasses;
}

const CVectorCore< C_FLOAT64 > & CMathContainer::getParticleFluxes() const
{
  return mParticleFluxes;
}

const CVectorCore< C_FLOAT64 > & CMathContainer::getFluxes() const
{
  return mFluxes;
}

const CVectorCore< C_FLOAT64 > & CMathContainer::getPropensities() const
{
  return mPropensities;
}

const CVectorCore< C_FLOAT64 > & CMathContainer::getRoots() const
{
  return mEventRoots;
}

const CVectorCore< bool > & CMathContainer::getRootIsDiscrete() const
{
  return mRootIsDiscrete;
}

const CVectorCore< bool > & CMathContainer::getRootIsTimeDependent() const
{
  return mRootIsTimeDependent;
}

CVector< CMathEvent::CTrigger::CRootProcessor * > & CMathContainer::getRootProcessors()
{
  return mRootProcessors;
}

void CMathContainer::updateInitialValues(const CModelParameter::Framework & framework)
{
  switch (framework)
    {
      case CModelParameter::Concentration:
        applyUpdateSequence(mSynchronizeInitialValuesSequenceIntensive);
        break;

      case CModelParameter::ParticleNumbers:
        applyUpdateSequence(mSynchronizeInitialValuesSequenceExtensive);
        break;
    }
}

void CMathContainer::applyInitialValues()
{
#ifdef DEBUG_OUTPUT
  std::cout << "Container Values: " << mValues << std::endl;
#endif // DEBUG_OUTPUT

  C_FLOAT64 * pInitial = mInitialExtensiveValues.array();
  C_FLOAT64 * pTransient = mExtensiveValues.array();

  memcpy(pTransient, pInitial, (pTransient - pInitial) * sizeof(C_FLOAT64));

#ifdef DEBUG_OUTPUT
  std::cout << "Container Values: " << mValues << std::endl;
#endif // DEBUG_OUTPUT

  applyUpdateSequence(mApplyInitialValuesSequence);

#ifdef DEBUG_OUTPUT
  std::cout << "Container Values: " << mValues << std::endl;
#endif // DEBUG_OUTPUT

  // Start the process queue
  mpProcessQueue->start();

  // Calculate the current root derivatives
  CVector< C_FLOAT64 > RootDerivatives;
  calculateRootDerivatives(RootDerivatives);

  // Determine the initial root states.
  CMathEvent::CTrigger::CRootProcessor ** pRoot = mRootProcessors.array();
  CMathEvent::CTrigger::CRootProcessor ** pRootEnd = pRoot + mRootProcessors.size();

  for (; pRoot != pRootEnd; ++pRoot)
    {
      (*pRoot)->calculateTrueValue();
    }

  // Determine the trigger values
  CMathObject * pTriggerObject = getMathObject(mEventTriggers.array());
  CMathObject * pTriggerObjectEnd = pTriggerObject + mEventTriggers.size();

  for (; pTriggerObject != pTriggerObjectEnd; ++pTriggerObject)
    {
      pTriggerObject->calculateValue();
    }

  // Fire events which triggers are true and which may fire at the initial time
  C_FLOAT64 * pTrigger = mEventTriggers.array();
  C_FLOAT64 * pTriggerEnd = pTrigger + mEventTriggers.size();
  CMathEvent * pEvent = mEvents.array();

  for (; pTrigger != pTriggerEnd; ++pTrigger, ++pEvent)
    {
      if (*pTrigger > 0.5 &&
          pEvent->fireAtInitialTime())
        {
          pEvent->fire(true);
        }
    }

#ifdef DEBUG_OUTPUT
  std::cout << "Container Values: " << mValues << std::endl;
#endif // DEBUG_OUTPUT

  // Determine roots which change state at the initial time point, i.e., roots which may have
  // a value of zero and a non zero derivative and check
  CVector< C_INT > FoundRoots(mEventRoots.size());
  C_INT * pFoundRoot = FoundRoots.array();
  C_FLOAT64 * pRootValue = mEventRoots.array();
  C_FLOAT64 * pRootDerivative = RootDerivatives.array();
  pRoot = mRootProcessors.array();

  for (; pRoot != pRootEnd; ++pRoot, ++pFoundRoot, ++pRootValue, ++pRootDerivative)
    {
      // Assume the root is not found.
      *pFoundRoot = 0;

      if (*pRootValue != 0.0)
        {
          continue;
        }

      if ((*pRoot)->isEquality())
        {
          if (*pRootDerivative < 0.0)
            {
              *pFoundRoot = 1;
            }
        }
      else
        {
          if (*pRootDerivative > 0.0)
            {
              *pFoundRoot = 1;
            }
        }
    }

  processRoots(false, FoundRoots);

#ifdef DEBUG_OUTPUT
  std::cout << "Container Values: " << mValues << std::endl;
#endif // DEBUG_OUTPUT
  return;
}

void CMathContainer::updateSimulatedValues(const bool & useMoieties)
{
  if (useMoieties)
    {
      applyUpdateSequence(mSimulationValuesSequenceReduced);
    }
  else
    {
      applyUpdateSequence(mSimulationValuesSequence);
    }
}

void CMathContainer::updateTransientDataValues()
{
  applyUpdateSequence(mTransientDataObjectSequence);
}

const CObjectInterface::UpdateSequence & CMathContainer::getSynchronizeInitialValuesSequence(const CModelParameter::Framework & framework) const
{
  switch (framework)
    {
      case CModelParameter::Concentration:
        return mSynchronizeInitialValuesSequenceIntensive;
        break;

      case CModelParameter::ParticleNumbers:
        return mSynchronizeInitialValuesSequenceExtensive;
        break;
    }

  return mSynchronizeInitialValuesSequenceExtensive;
}

const CObjectInterface::UpdateSequence & CMathContainer::getApplyInitialValuesSequence() const
{
  return mApplyInitialValuesSequence;
}

const CObjectInterface::UpdateSequence & CMathContainer::getSimulationValuesSequence(const bool & useMoieties) const
{
  if (useMoieties)
    {
      return mSimulationValuesSequenceReduced;
    }
  else
    {
      return mSimulationValuesSequence;
    }
}

const CObjectInterface::UpdateSequence & CMathContainer::getTransientDataValueSequence() const
{
  return mTransientDataObjectSequence;
}

void CMathContainer::updateHistoryValues(const bool & useMoieties)
{
  CMathHistoryCore * pHistory = (useMoieties) ? &mHistoryReduced : &mHistory;
  CMathDelay * pDelay = mDelays.array();
  CMathDelay * pDelayEnd = pDelay + mDelays.size();
  size_t lag = 0;

  for (; pDelay != pDelayEnd; ++lag, ++pDelay)
    {
      setState(pHistory->getRow(lag));
      pDelay->calculateDelayValues(useMoieties);
    }
}

void CMathContainer::updatePriorityValues()
{
  applyUpdateSequence(mPrioritySequence);
}

void CMathContainer::applyUpdateSequence(const CObjectInterface::UpdateSequence & updateSequence)
{
  UpdateSequence::const_iterator it = updateSequence.begin();
  UpdateSequence::const_iterator end = updateSequence.end();

  for (; it != end; ++it)
    {
      (*it)->calculateValue();
    }
}

void CMathContainer::fetchInitialState()
{
  C_FLOAT64 * pValue = mInitialState.array();
  C_FLOAT64 * pValueEnd = pValue + mInitialState.size();
  CMathObject * pObject = mObjects.array();

  for (; pValue != pValueEnd; ++pValue, ++pObject)
    {
      const CCopasiObject * pDataObject = pObject->getDataObject();

      if (pDataObject != NULL)
        {
          *pValue = *(C_FLOAT64 *)pDataObject->getValuePointer();
        }
      else
        {
          *pValue = std::numeric_limits< C_FLOAT64 >::quiet_NaN();
        }
    }

  return;
}

void CMathContainer::pushInitialState()
{
  C_FLOAT64 * pValue = mInitialState.array();
  C_FLOAT64 * pValueEnd = pValue + mInitialState.size();
  CMathObject * pObject = mObjects.array();

  for (; pValue != pValueEnd; ++pValue, ++pObject)
    {
      const CCopasiObject * pDataObject = pObject->getDataObject();

      if (pDataObject != NULL)
        {
          *(C_FLOAT64 *)pDataObject->getValuePointer() = *pValue;
        }
    }

  return;
}

void CMathContainer::fetchState()
{
  C_FLOAT64 * pValue = mState.array();
  C_FLOAT64 * pValueEnd = pValue + mState.size();
  CMathObject * pObject = mObjects.array();

  for (; pValue != pValueEnd; ++pValue, ++pObject)
    {
      const CCopasiObject * pDataObject = pObject->getDataObject();

      if (pDataObject != NULL)
        {
          *pValue = *(C_FLOAT64 *)pDataObject->getValuePointer();
        }
      else
        {
          *pValue = std::numeric_limits< C_FLOAT64 >::quiet_NaN();
        }
    }

  return;
}

void CMathContainer::pushState()
{
  C_FLOAT64 * pValue = mState.array();
  C_FLOAT64 * pValueEnd = pValue + mState.size();
  CMathObject * pObject = getMathObject(pValue);

  for (; pValue != pValueEnd; ++pValue, ++pObject)
    {
      const CCopasiObject * pDataObject = pObject->getDataObject();

      if (pDataObject != NULL)
        {
          *(C_FLOAT64 *)pDataObject->getValuePointer() = *pValue;
        }
    }

  return;
}

void CMathContainer::pushAllTransientValues()
{
  C_FLOAT64 * pValue = mExtensiveValues.array();
  C_FLOAT64 * pValueEnd = mValues.array() + mValues.size();
  CMathObject * pObject = getMathObject(pValue);

  for (; pValue != pValueEnd; ++pValue, ++pObject)
    {
      const CCopasiObject * pDataObject = pObject->getDataObject();

      if (pDataObject != NULL)
        {
          *(C_FLOAT64 *)pDataObject->getValuePointer() = *pValue;
        }
    }

  return;
}

// virtual
CCopasiObjectName CMathContainer::getCN() const
{
  return mpModel->getCN();
}

// virtual
const CObjectInterface * CMathContainer::getObject(const CCopasiObjectName & cn) const
{
  // Since the CN should be relative we check in the model first
  const CObjectInterface * pObject = mpModel->getObject(cn);

  if (pObject == NULL)
    {
#ifdef DEBUG_OUTPUT
      std::cout << "Data Object " << cn << " not found in model." << std::endl;
#endif // DEBUG_OUTPUT

      CObjectInterface::ContainerList ListOfContainer;
      ListOfContainer.push_back(mpModel);
      ListOfContainer.push_back(mpModel->getObjectDataModel());

      CCopasiObjectName ModelCN = mpModel->getCN();

      if (cn.getPrimary() != ModelCN.getPrimary())
        {
          pObject = CObjectInterface::GetObjectFromCN(ListOfContainer, ModelCN + "," + cn);
        }
      else
        {
          pObject = CObjectInterface::GetObjectFromCN(ListOfContainer, cn);
        }
    }

  const CMathObject * pMathObject = getMathObject(pObject);

  if (pMathObject != NULL)
    {
      return pMathObject;
    }

#ifdef DEBUG_OUTPUT
  std::cout << "Data Object " << cn << " (0x" << pObject << ") has no corresponding Math Object." << std::endl;
#endif // DEBUG_OUTPUT

  return pObject;
}

const CObjectInterface * CMathContainer::getObjectFromCN(const CCopasiObjectName & cn) const
{
  CObjectInterface::ContainerList ListOfContainer;
  ListOfContainer.push_back(this);
  ListOfContainer.push_back(mpModel);
  ListOfContainer.push_back(mpModel->getObjectDataModel());

  return CObjectInterface::GetObjectFromCN(ListOfContainer, cn);
}

CMathObject * CMathContainer::getMathObject(const CObjectInterface * pObject) const
{
  if (pObject == NULL)
    return NULL;

  std::map< CCopasiObject *, CMathObject * >::const_iterator found =
    mDataObject2MathObject.find(const_cast<CCopasiObject*>(static_cast< const CCopasiObject * >(pObject)));

  if (found != mDataObject2MathObject.end())
    {
      return found->second;
    }

  return NULL;
}

CMathObject * CMathContainer::getMathObject(const C_FLOAT64 * pDataValue) const
{
  if (pDataValue == NULL)
    return NULL;

  // Check whether we point to a math value.
  const C_FLOAT64 * pValues = mValues.array();

  if (pValues <= pDataValue && pDataValue < pValues + mValues.size())
    {
      return const_cast< CMathObject * >(mObjects.array() + (pDataValue - pValues));
    }

  std::map< C_FLOAT64 *, CMathObject * >::const_iterator found =
    mDataValue2MathObject.find(const_cast< C_FLOAT64 * >(pDataValue));

  if (found != mDataValue2MathObject.end())
    {
      return found->second;
    }

  return NULL;
}

CMathObject * CMathContainer::getMathObject(const CCopasiObjectName & cn) const
{
  return getMathObject(mpModel->getObject(cn));
}

CCopasiObject * CMathContainer::getDataObject(const C_FLOAT64 * pDataValue) const
{
  std::map< C_FLOAT64 *, CCopasiObject * >::const_iterator found =
    mDataValue2DataObject.find(const_cast< C_FLOAT64 * >(pDataValue));

  if (found != mDataValue2DataObject.end())
    {
      return found->second;
    }

  return NULL;
}

CMathObject * CMathContainer::getCompartment(const CMathObject * pObject) const
{
  if (pObject == NULL ||
      pObject->getEntityType() != CMath::Species)
    {
      return NULL;
    }

  CMetab * pMetab = static_cast< CMetab * >(pObject->getDataObject()->getObjectParent());

  return getMathObject(pMetab->getCompartment()->getValueReference());
}

CMathObject * CMathContainer::getLargestReactionCompartment(const CMathReaction * pReaction) const
{
  if (pReaction == NULL)
    {
      return NULL;
    }

  CMathObject * pLargestCompartment = NULL;
  CMathReaction::ObjectBalance::const_iterator it = pReaction->getObjectBalance().begin();
  CMathReaction::ObjectBalance::const_iterator end = pReaction->getObjectBalance().end();

  for (; it != end; ++it)
    {
      CMathObject * pCompartment = getCompartment(it->first);

      if (pLargestCompartment == NULL ||
          (pCompartment != NULL &&
           *(C_FLOAT64*)pLargestCompartment->getValuePointer() < * (C_FLOAT64*)pCompartment->getValuePointer()))
        {
          pLargestCompartment = pCompartment;
        }
    }

  return pLargestCompartment;
}

void CMathContainer::compile()
{
  // Clear old maps before allocation
  mDataObject2MathObject.clear();
  mDataValue2MathObject.clear();
  mDataValue2DataObject.clear();

  allocate();

  CMath::sPointers Pointers;
  initializePointers(Pointers);

#ifdef DEBUG_OUPUT
  printPointers(Pointers);
#endif // DEBUG_OUPUT

  initializeDiscontinuousCreationPointer();

  initializeObjects(Pointers);
  initializeEvents(Pointers);

  compileObjects();
  compileEvents();

  // These are only used during initialization for setting up the tracking of
  // discontinuities and are cleared afterwards.
  mDiscontinuityEvents.clear();
  mDiscontinuityInfix2Object.clear();
  mTriggerInfix2Event.clear();

  // Create eventual delays
  createDelays();

  createDependencyGraphs();
  createUpdateSequences();

  updateInitialValues(CModelParameter::ParticleNumbers);

  CMathReaction * pReaction = mReactions.array();
  CCopasiVector< CReaction >::const_iterator itReaction = mpModel->getReactions().begin();
  CCopasiVector< CReaction >::const_iterator endReaction = mpModel->getReactions().end();

  for (; itReaction != endReaction; ++itReaction)
    {
      // We ignore reactions which do not have any effect.
      if ((*itReaction)->getChemEq().getBalances().size() > 0)
        {
          pReaction->initialize(*itReaction, *this);
          ++pReaction;
        }
    }

  // TODO We may have unused event triggers and roots due to optimization
  // in the discontinuities.
  analyzeRoots();

  CMathDelay * pDelay = mDelays.array();
  CMathDelay * pDelayEnd = pDelay + mDelays.size();

  for (; pDelay != pDelayEnd; ++pDelay)
    {
      pDelay->createUpdateSequences();
    }

  // Check whether we have recursive delay value definitions
  {
    CObjectInterface::ObjectSet Changed;

    CMathObject *pObject = getMathObject(mDelayValues.array());
    CMathObject *pObjectEnd = pObject + mDelayValues.size();

    for (; pObject != pObjectEnd; ++pObject)
      {
        Changed.insert(pObject);
      }

    CObjectInterface::UpdateSequence Sequence;
    mTransientDependencies.getUpdateSequence(Sequence, CMath::DelayValues, Changed, Changed);

    if (!Sequence.empty())
      {
        // TODO CRITICAL Create a meaningful error message.
        fatalError();
      }
  }

#ifdef DEBUG_OUTPUT
  CMathObject *pObject = mObjects.array();
  CMathObject *pObjectEnd = pObject + mObjects.size();

  for (; pObject != pObjectEnd; ++pObject)
    {
      std::cout << *pObject;
    }

  std::cout << std::endl;
#endif // DEBUG_OUTPUT
}

const CModel & CMathContainer::getModel() const
{
  return *mpModel;
}

const size_t & CMathContainer::getCountFixedEventTargets() const
{
  return mSize.nEventTargets;
}

const size_t & CMathContainer::getCountODEs() const
{
  return mSize.nODE;
}

const size_t CMathContainer::getCountIndependentSpecies() const
{
  return mSize.nReactionSpecies - mSize.nMoieties;
}

const size_t & CMathContainer::getCountDependentSpecies() const
{
  return mSize.nMoieties;
}

const size_t & CMathContainer::getCountAssignments() const
{
  return mSize.nAssignment;
}

const size_t & CMathContainer::getCountFixed() const
{
  return mSize.nFixed;
}

const size_t & CMathContainer::getTimeIndex() const
{
  return mSize.nEventTargets;
}

CVectorCore< CMathReaction > & CMathContainer::getReactions()
{
  return mReactions;
}

const CVectorCore< CMathReaction > & CMathContainer::getReactions() const
{
  return mReactions;
}

const CMatrix< C_FLOAT64 > & CMathContainer::getStoichiometry(const bool & reduced) const
{
  if (reduced)
    {
      return mpModel->getRedStoi();
    }

  return mpModel->getStoi();
}

const CVectorCore< CMathEvent > & CMathContainer::getEvents() const
{
  return mEvents;
}

const CMathDependencyGraph & CMathContainer::getInitialDependencies() const
{
  return mInitialDependencies;
}

const CMathDependencyGraph & CMathContainer::getTransientDependencies() const
{
  return mTransientDependencies;
}

/**
 * Retrieve the objects which represent the initial state.
 * @return CObjectInterface::ObjectSet & stateObjects
 */
const CObjectInterface::ObjectSet & CMathContainer::getInitialStateObjects() const
{
  return mInitialStateValueAll;
}

const CObjectInterface::ObjectSet & CMathContainer::getStateObjects(const bool & reduced) const
{
  if (reduced)
    {
      return mReducedStateValues;
    }

  return mStateValues;
}

const CObjectInterface::ObjectSet & CMathContainer::getSimulationUpToDateObjects() const
{
  return mSimulationRequiredValues;
}

CEvaluationNode * CMathContainer::copyBranch(const CEvaluationNode * pSrc,
    const bool & replaceDiscontinuousNodes)
{
  CMath::Variables< CEvaluationNode * > Variables;

  return copyBranch(pSrc, Variables, replaceDiscontinuousNodes);
}

CEvaluationNode * CMathContainer::copyBranch(const CEvaluationNode * pNode,
    const CMath::Variables< CEvaluationNode * > & variables,
    const bool & replaceDiscontinuousNodes)
{
  CNodeContextIterator< const CEvaluationNode, std::vector< CEvaluationNode * > > itNode(pNode);
  CEvaluationNode * pCopy = NULL;

  while (itNode.next() != itNode.end())
    {
      if (*itNode == NULL)
        {
          continue;
        }

      // We need to replace variables, expand called trees, and handle discrete nodes.
      switch ((int) itNode->getType())
        {
            // Handle object nodes which are of type CN
          case (CEvaluationNode::OBJECT | CEvaluationNodeObject::CN):
          {
            // We need to map the object to a math object if possible.
            const CObjectInterface * pObject =
              getObject(static_cast< const CEvaluationNodeObject *>(*itNode)->getObjectCN());

            // Create a converted node
            pCopy = createNodeFromObject(pObject);
          }
          break;

          // Handle object nodes which are of type POINTER
          case (CEvaluationNode::OBJECT | CEvaluationNodeObject::POINTER):
          {
            const CObjectInterface * pObject =
              getMathObject(static_cast< const CEvaluationNodeObject *>(*itNode)->getObjectValuePtr());

            // Create a converted node
            pCopy = createNodeFromObject(pObject);
          }
          break;

          // Handle variables
          case (CEvaluationNode::VARIABLE | CEvaluationNodeVariable::ANY):
          {
            size_t Index =
              static_cast< const CEvaluationNodeVariable * >(*itNode)->getIndex();

            if (Index != C_INVALID_INDEX &&
                Index < variables.size())
              {
                pCopy = variables[Index]->copyBranch();
              }
            else
              {
                pCopy = new CEvaluationNodeConstant(CEvaluationNodeConstant::_NaN, itNode->getData());
              }
          }
          break;

          // Handle call nodes
          case (CEvaluationNode::CALL | CEvaluationNodeCall::FUNCTION):
          case (CEvaluationNode::CALL | CEvaluationNodeCall::EXPRESSION):
          {
            const CEvaluationNode * pCalledNode =
              static_cast< const CEvaluationNodeCall * >(*itNode)->getCalledTree()->getRoot();

            pCopy = copyBranch(pCalledNode, itNode.context(), replaceDiscontinuousNodes);

            // The variables have been copied into place we need to delete them.
            std::vector< CEvaluationNode * >::iterator it = itNode.context().begin();
            std::vector< CEvaluationNode * >::iterator end = itNode.context().end();

            for (; it != end; ++it)
              {
                delete *it;
              }
          }
          break;

          // Handle discrete nodes
          case (CEvaluationNode::CHOICE | CEvaluationNodeChoice::IF):
          case (CEvaluationNode::FUNCTION | CEvaluationNodeFunction::FLOOR):
          case (CEvaluationNode::FUNCTION | CEvaluationNodeFunction::CEIL):
          case (CEvaluationNode::OPERATOR | CEvaluationNodeOperator::MODULUS):

            if (replaceDiscontinuousNodes)
              {
                // The node is replaced with a pointer to a math object value.
                // The math object is calculated by an assignment with the target being the
                // math object
                pCopy = replaceDiscontinuousNode(*itNode, itNode.context());
              }
            else
              {
                pCopy = itNode->copyNode(itNode.context());
              }

            break;

          default:
            pCopy = itNode->copyNode(itNode.context());
            break;
        }

      if (itNode.parentContextPtr() != NULL)
        {
          itNode.parentContextPtr()->push_back(pCopy);
        }
    }

  // assert(pCopy != NULL);

  return pCopy;
}

CEvaluationNode *
CMathContainer::replaceDiscontinuousNode(const CEvaluationNode * pSrc,
    const std::vector< CEvaluationNode * > & children)
{
  bool success = true;

  CEvaluationNode * pNode = pSrc->copyNode(children);
  std::string DiscontinuityInfix = pNode->buildInfix();

  // Check whether we have the discontinuous node already created. This can happen if the
  // discontinuity was part of an expression for a variable in a function call.
  std::map< std::string, CMathObject * >::iterator itObject = mDiscontinuityInfix2Object.find(DiscontinuityInfix);

  if (itObject != mDiscontinuityInfix2Object.end())
    {
      // No need to copy we have already on object
      CMathObject * pDiscontinuity = itObject->second;

      // We need to advance both creation pointer to assure that we have the correct allocation
      // Mark the discontinuity objects as unused
      mCreateDiscontinuousPointer.pDiscontinuous->setValueType(CMath::ValueTypeUndefined);
      mCreateDiscontinuousPointer.pDiscontinuous += 1;

      // Mark the event as unused by setting the trigger to ''
      mCreateDiscontinuousPointer.pEvent->setTriggerExpression("", *this);
      mCreateDiscontinuousPointer.pEvent += 1;

      pdelete(pNode);

      // Return a pointer to a node pointing to the value of discontinuous object.
      return new CEvaluationNodeObject((C_FLOAT64 *) pDiscontinuity->getValuePointer());
    }

  // We have a new discontinuity
  CMathObject * pDiscontinuity = mCreateDiscontinuousPointer.pDiscontinuous;
  mCreateDiscontinuousPointer.pDiscontinuous += 1;

  // Map the discontinuity infix to the discontinuous object.
  mDiscontinuityInfix2Object[DiscontinuityInfix] = pDiscontinuity;

  // Create the expression to calculate the discontinuous object
  CMathExpression * pExpression = new CMathExpression("DiscontinuousExpression", *this);
  success &= static_cast< CEvaluationTree * >(pExpression)->setRoot(pNode);
  success &= pDiscontinuity->setExpressionPtr(pExpression);

  CMathEvent * pEvent = NULL;

  // Check whether we have already an event with the current trigger
  std::string TriggerInfix = createDiscontinuityTriggerInfix(pNode);
  std::map< std::string, CMathEvent * >::iterator itEvent = mTriggerInfix2Event.find(TriggerInfix);

  // We need to create an event.
  if (itEvent == mTriggerInfix2Event.end())
    {
      pEvent = mCreateDiscontinuousPointer.pEvent;

      // Set the trigger
      pEvent->setTriggerExpression(TriggerInfix, *this);

      // Map the trigger infix to the event.
      mTriggerInfix2Event[TriggerInfix] = pEvent;
    }
  else
    {
      pEvent = itEvent->second;

      // Mark the event as unused by setting the trigger to ''
      mCreateDiscontinuousPointer.pEvent->setTriggerExpression("", *this);
    }

  mCreateDiscontinuousPointer.pEvent += 1;

  // Add the current discontinuity as an assignment.
  pEvent->addAssignment(pDiscontinuity, pDiscontinuity);

  // Return a pointer to a node pointing to the value of discontinuous object.
  return new CEvaluationNodeObject((C_FLOAT64 *) pDiscontinuity->getValuePointer());
}

void CMathContainer::allocate()
{
  sSize Size;

  Size.nFixed = CObjectLists::getListOfConstObjects(CObjectLists::ALL_LOCAL_PARAMETER_VALUES, mpModel).size();
  Size.nFixed += mpModel->getStateTemplate().getNumFixed();
  Size.nEventTargets = 0;
  Size.nODE = mpModel->getStateTemplate().getNumIndependent() - mpModel->getNumIndependentReactionMetabs();
  Size.nReactionSpecies = mpModel->getNumIndependentReactionMetabs() + mpModel->getNumDependentReactionMetabs();
  Size.nAssignment = mpModel->getStateTemplate().getNumDependent() - mpModel->getNumDependentReactionMetabs();

  Size.nIntensiveValues = mpModel->getNumMetabs();

  Size.nReactions = 0;
  CCopasiVector< CReaction >::const_iterator itReaction = mpModel->getReactions().begin();
  CCopasiVector< CReaction >::const_iterator endReaction = mpModel->getReactions().end();

  for (; itReaction != endReaction; ++itReaction)
    {
      // We ignore reactions which do not have any effect.
      if ((*itReaction)->getChemEq().getBalances().size() > 0)
        {
          Size.nReactions++;
        }
    }

  Size.nMoieties = mpModel->getMoieties().size();

  Size.nDiscontinuities = 0;
  Size.nEvents = 0;
  Size.nEventAssignments = 0;
  Size.nEventRoots = 0;

  // Determine the space requirements for events.
  // We need to create events for nodes which are capable of introducing
  // discontinuous changes.
  createDiscontinuityEvents();
  Size.nDiscontinuities += mDiscontinuityEvents.size();
  Size.nEvents += Size.nDiscontinuities;

  // User defined events
  const CCopasiVector< CEvent > & Events = mpModel->getEvents();
  CCopasiVector< CEvent >::const_iterator itEvent = Events.begin();
  CCopasiVector< CEvent >::const_iterator endEvent = Events.end();

  Size.nEvents += Events.size();

  for (; itEvent != endEvent; ++itEvent)
    {
      CMathEvent Event;
      CMathEvent::allocate(Event, *itEvent, *this);

      Size.nEventRoots += Event.getTrigger().getRoots().size();
      Size.nEventAssignments += Event.getAssignments().size();
    }

  itEvent = mDiscontinuityEvents.begin();
  endEvent = mDiscontinuityEvents.end();

  for (; itEvent != endEvent; ++itEvent)
    {
      CMathEvent Event;
      CMathEvent::allocate(Event, *itEvent, *this);
      Size.nEventRoots += Event.getTrigger().getRoots().size();

      // We do not have to allocate an assignment as discontinuity object suffices
      // as target and assignment expression.
    }

  // The number of delays is only determined after the math container objects have been
  // constructed. At that point values and objects are reallocated.
  Size.nDelayLags = 0;
  Size.nDelayValues = 0;

  Size.pValue = NULL;
  Size.pObject = NULL;

  resize(Size);

  mValues = std::numeric_limits< C_FLOAT64 >::quiet_NaN();
}

void CMathContainer::initializeObjects(CMath::sPointers & p)
{
  std::set< const CModelEntity * > EventTargets = CObjectLists::getEventTargets(mpModel);

  std::vector< const CModelEntity * > FixedEntities;
  std::vector< const CModelEntity * > FixedEventTargetEntities;

  const CStateTemplate & StateTemplate = mpModel->getStateTemplate();

  // Determine which fixed entities are modified by events and which not.
  const CModelEntity *const* ppEntities = StateTemplate.beginFixed();
  const CModelEntity *const* ppEntitiesEnd = StateTemplate.endFixed();

  for (; ppEntities != ppEntitiesEnd; ++ppEntities)
    {
      if ((*ppEntities)->getStatus() == CModelEntity::ASSIGNMENT)
        continue;

      if (EventTargets.find(*ppEntities) == EventTargets.end())
        {
          FixedEntities.push_back(*ppEntities);
        }
      else
        {
          FixedEventTargetEntities.push_back(*ppEntities);
        }
    }

  // Process fixed entities which are not event targets.
  initializeMathObjects(FixedEntities, CMath::Fixed, p);

  // Process local reaction parameters
  std::vector<const CCopasiObject*> LocalReactionParameter =
    CObjectLists::getListOfConstObjects(CObjectLists::ALL_LOCAL_PARAMETER_VALUES, mpModel);
  initializeMathObjects(LocalReactionParameter, p);
  assert(mSize.nFixed == FixedEntities.size() + LocalReactionParameter.size());

  // Process fixed entities which are event targets.
  initializeMathObjects(FixedEventTargetEntities, CMath::EventTarget, p);
  assert(mSize.nEventTargets == FixedEventTargetEntities.size());

  // The simulation time
  // Extensive Initial Value
  map(mpModel->getInitialValueReference(), p.pInitialExtensiveValuesObject);
  CMathObject::initialize(p.pInitialExtensiveValuesObject, p.pInitialExtensiveValues,
                          CMath::Value, CMath::Model, CMath::Time, false, true,
                          mpModel->getInitialValueReference());

  // Extensive Value
  map(mpModel->getValueReference(), p.pExtensiveValuesObject);
  CMathObject::initialize(p.pExtensiveValuesObject, p.pExtensiveValues,
                          CMath::Value, CMath::Model, CMath::Time, false, false,
                          mpModel->getValueReference());

  // Initial Extensive Rate
  CMathObject::initialize(p.pInitialExtensiveRatesObject, p.pInitialExtensiveRates,
                          CMath::Rate, CMath::Model, CMath::Time, false, true,
                          mpModel->getRateReference());
  // Extensive Rate
  map(mpModel->getRateReference(), p.pExtensiveRatesObject);
  CMathObject::initialize(p.pExtensiveRatesObject, p.pExtensiveRates,
                          CMath::Rate, CMath::Model, CMath::Time, false, false,
                          mpModel->getRateReference());

  // Process entities which are determined by ODEs
  std::vector< const CModelEntity * > ODEEntities;

  ppEntities = StateTemplate.beginIndependent();
  ppEntitiesEnd = StateTemplate.endIndependent();

  for (; ppEntities != ppEntitiesEnd && (*ppEntities)->getStatus() == CModelEntity::ODE; ++ppEntities)
    {
      ODEEntities.push_back(*ppEntities);
    }

  initializeMathObjects(ODEEntities, CMath::ODE, p);
  assert(mSize.nODE == ODEEntities.size());

  // Process independent species
  std::vector< const CModelEntity * > IndependentSpecies;

  ppEntities = StateTemplate.beginIndependent();
  ppEntitiesEnd = StateTemplate.endIndependent();

  for (; ppEntities != ppEntitiesEnd; ++ppEntities)
    {
      if ((*ppEntities)->getStatus() != CModelEntity::REACTIONS)
        continue;

      IndependentSpecies.push_back(*ppEntities);
    }

  initializeMathObjects(IndependentSpecies, CMath::Independent, p);
  assert(mSize.nReactionSpecies - mSize.nMoieties == IndependentSpecies.size());

  // Process dependent species
  std::vector< const CModelEntity * > DependentSpecies;

  ppEntities = StateTemplate.beginDependent();
  ppEntitiesEnd = StateTemplate.endDependent();

  for (; ppEntities != ppEntitiesEnd && (*ppEntities)->getStatus() == CModelEntity::REACTIONS; ++ppEntities)
    {
      DependentSpecies.push_back(*ppEntities);
    }

  initializeMathObjects(DependentSpecies, CMath::Dependent, p);
  assert(mSize.nMoieties == DependentSpecies.size());

  // Process entities which are determined by assignments
  std::vector< const CModelEntity * > AssignmentEntities;

  // We continue with the pointer ppEntities
  ppEntitiesEnd = StateTemplate.endFixed();

  for (; ppEntities != ppEntitiesEnd && (*ppEntities)->getStatus() == CModelEntity::ASSIGNMENT; ++ppEntities)
    {
      AssignmentEntities.push_back(*ppEntities);
    }

  initializeMathObjects(AssignmentEntities, CMath::Assignment, p);
  assert(mSize.nAssignment == AssignmentEntities.size());

  // Process Reactions
  initializeMathObjects(mpModel->getReactions(), p);

  // Process Moieties
  initializeMathObjects(mpModel->getMoieties(), p);

  // Process Discontinuous Objects
  size_t n;
  assert(mSize.nDiscontinuities == mDiscontinuous.size());

  for (n = 0; n != mSize.nDiscontinuities; ++n)
    {
      CMathObject::initialize(p.pDiscontinuousObject, p.pDiscontinuous,
                              CMath::Discontinuous, CMath::Event, CMath::SimulationTypeUndefined,
                              false, false, NULL);
    }

  // Delay objects are allocated after all other objects are compiled.
}

void CMathContainer::initializeEvents(CMath::sPointers & p)
{
  CMathEvent * pEvent = mEvents.array();

  // User defined events
  const CCopasiVector< CEvent > & Events = mpModel->getEvents();
  CCopasiVector< CEvent >::const_iterator itEvent = Events.begin();
  CCopasiVector< CEvent >::const_iterator endEvent = Events.end();

  for (; itEvent != endEvent; ++itEvent, ++pEvent)
    {
      CMathEvent::allocate(*pEvent, *itEvent, *this);
      pEvent->initialize(p);
    }

  itEvent = mDiscontinuityEvents.begin();
  endEvent = mDiscontinuityEvents.end();

  for (; itEvent != endEvent; ++itEvent)
    {
      CMathEvent::allocate(*pEvent, *itEvent, *this);
      pEvent->initialize(p);
    }

  return;
}

bool CMathContainer::compileObjects()
{
  bool success = true;

  CMathObject *pObject = mObjects.array();
  CMathObject *pObjectEnd = pObject + mObjects.size();

  for (; pObject != pObjectEnd; ++pObject)
    {
      success &= pObject->compile(*this);
    }

  return success;
}

bool CMathContainer::compileEvents()
{
  bool success = true;

  CMathEvent * pItEvent = mEvents.array();

  CCopasiVector< CEvent >::const_iterator itEvent = mpModel->getEvents().begin();
  CCopasiVector< CEvent >::const_iterator endEvent = mpModel->getEvents().end();

  for (; itEvent != endEvent; ++pItEvent, ++itEvent)
    {
      success &= pItEvent->compile(*itEvent, *this);
    }

  // Events representing discontinuities.
  itEvent = mDiscontinuityEvents.begin();
  endEvent = mDiscontinuityEvents.end();

  for (; itEvent != endEvent; ++pItEvent, ++itEvent)
    {
      success &= pItEvent->compile(*this);
    }

  return success;
}

CEvaluationNode * CMathContainer::createNodeFromObject(const CObjectInterface * pObject)
{
  CEvaluationNode * pNode = NULL;

  if (pObject == NULL)
    {
      // We have an invalid value, i.e. NaN
      pNode = new CEvaluationNodeConstant(CEvaluationNodeConstant::_NaN, "NAN");
    }
  else if (pObject == mpAvogadro ||
           pObject == mpQuantity2NumberFactor)
    {
      pNode = new CEvaluationNodeNumber(*(C_FLOAT64 *) pObject->getValuePointer());
    }
  else
    {
      pNode = new CEvaluationNodeObject((C_FLOAT64 *) pObject->getValuePointer());

      // Check whether we have a data object
      if (pObject == pObject->getDataObject())
        {
          mDataValue2DataObject[(C_FLOAT64 *) pObject->getValuePointer()]
            = static_cast< CCopasiObject * >(const_cast< CObjectInterface * >(pObject));
        }
    }

  return pNode;
}

CEvaluationNode * CMathContainer::createNodeFromValue(const C_FLOAT64 * pDataValue)
{
  CEvaluationNode * pNode = NULL;
  CMathObject * pMathObject = NULL;

  if (pDataValue != NULL)
    {
      pMathObject = getMathObject(pDataValue);

      if (pMathObject != NULL)
        {
          pNode = new CEvaluationNodeObject((C_FLOAT64 *) pMathObject->getValuePointer());
        }
      else
        {
          // We must have a constant value like the conversion factor from the model.
          pNode = new CEvaluationNodeNumber(*pDataValue);
        }
    }
  else
    {
      // We have an invalid value, i.e. NaN
      pNode = new CEvaluationNodeConstant(CEvaluationNodeConstant::_NaN, "NAN");
    }

  return pNode;
}

void CMathContainer::createDependencyGraphs()
{
  CMathObject *pObject = mObjects.array();
  CMathObject *pObjectEnd = pObject + (mExtensiveValues.array() - mInitialExtensiveValues.array());

  for (; pObject != pObjectEnd; ++pObject)
    {
      mInitialDependencies.addObject(pObject);
    }

  pObjectEnd = mObjects.array() + mObjects.size();

  for (; pObject != pObjectEnd; ++pObject)
    {
      mTransientDependencies.addObject(pObject);
    }

#ifdef COPASI_DEBUG_TRACE
  std::ofstream InitialDependencies("InitialDependencies.dot");
  mInitialDependencies.exportDOTFormat(InitialDependencies, "InitialDependencies");

  std::ofstream TransientDependencies("TransientDependencies.dot");
  mTransientDependencies.exportDOTFormat(TransientDependencies, "TransientDependencies");
#endif // COPASI_DEBUG_TRACE

  return;
}

void CMathContainer::createUpdateSequences()
{
  createSynchronizeInitialValuesSequence();
  createApplyInitialValuesSequence();
  createUpdateSimulationValuesSequence();
  createUpdateAllTransientDataValuesSequence();

  CMathEvent * pEvent = mEvents.array();
  CMathEvent * pEventEnd = pEvent + mEvents.size();

  for (; pEvent != pEventEnd; ++pEvent)
    {
      pEvent->createUpdateSequences();
    }
}

void CMathContainer::createSynchronizeInitialValuesSequence()
{
  // Collect all the changed objects, which are all initial state values
  mInitialStateValueAll.clear();
  mInitialStateValueExtensive.clear();
  mInitialStateValueIntensive.clear();

  // Collect all the requested objects
  CObjectInterface::ObjectSet RequestedExtensive;
  CObjectInterface::ObjectSet RequestedIntensive;

  const CMathObject * pObject = mObjects.array();
  const CMathObject * pObjectEnd = getMathObject(mExtensiveValues.array());

  for (; pObject != pObjectEnd; ++pObject)
    {
      mInitialStateValueAll.insert(pObject);

      switch (pObject->getValueType())
        {
          case CMath::Value:

            switch (pObject->getSimulationType())
              {
                case CMath::Fixed:
                case CMath::Time:
                  mInitialStateValueExtensive.insert(pObject);
                  mInitialStateValueIntensive.insert(pObject);
                  break;

                case CMath::EventTarget:
                case CMath::ODE:
                case CMath::Independent:
                case CMath::Dependent:
                case CMath::Conversion:

                  if (pObject->getEntityType() != CMath::Species)
                    {
                      mInitialStateValueExtensive.insert(pObject);
                      mInitialStateValueIntensive.insert(pObject);
                    }
                  else if (pObject->isIntensiveProperty())
                    {
                      mInitialStateValueIntensive.insert(pObject);
                      RequestedExtensive.insert(pObject);
                    }
                  else
                    {
                      mInitialStateValueExtensive.insert(pObject);
                      RequestedIntensive.insert(pObject);
                    }

                  break;

                case CMath::Assignment:
                  RequestedExtensive.insert(pObject);
                  RequestedIntensive.insert(pObject);
                  break;
              }

            break;

            // Everything which is not a value must be calculated.
          default:
            RequestedExtensive.insert(pObject);
            RequestedIntensive.insert(pObject);
            break;
        }
    }

  // Issue 1170: We need to add elements of the stoichiometry, reduced stoichiometry,
  // and link matrices.
  std::map< C_FLOAT64 *, CCopasiObject * >::const_iterator itDataObject = mDataValue2DataObject.begin();
  std::map< C_FLOAT64 *, CCopasiObject * >::const_iterator endDataObject = mDataValue2DataObject.end();

  for (; itDataObject != endDataObject; ++itDataObject)
    {
      mInitialStateValueExtensive.insert(itDataObject->second);
      mInitialStateValueIntensive.insert(itDataObject->second);
    }

  // Build the update sequence
  mInitialDependencies.getUpdateSequence(mSynchronizeInitialValuesSequenceExtensive,
                                         CMath::UpdateMoieties,
                                         mInitialStateValueExtensive,
                                         RequestedExtensive);
  mInitialDependencies.getUpdateSequence(mSynchronizeInitialValuesSequenceIntensive,
                                         CMath::UpdateMoieties,
                                         mInitialStateValueIntensive,
                                         RequestedIntensive);
}

void CMathContainer::createApplyInitialValuesSequence()
{
  // At this point all initial values as well as their transient counterparts are calculated
  CObjectInterface::ObjectSet Calculated;
  const CMathObject * pObject = mObjects.array();
  const CMathObject * pObjectEnd = getMathObject(mExtensiveValues.array());
  size_t TransientOffset = pObjectEnd - pObject;

  for (; pObject != pObjectEnd; ++pObject)
    {
      Calculated.insert(pObject);
      Calculated.insert(pObject + TransientOffset);
    }

  // Collect all the changed objects, which are all initial and transient state values
  CObjectInterface::ObjectSet Changed = mInitialStateValueExtensive;
  // Initial state values

  // Collect all the requested objects
  CObjectInterface::ObjectSet Requested;

  pObject = getMathObject(mExtensiveValues.array());
  pObjectEnd = mObjects.array() + mObjects.size();

  for (; pObject != pObjectEnd; ++pObject)
    {
      switch (pObject->getValueType())
        {
          case CMath::Value:

            switch (pObject->getSimulationType())
              {
                case CMath::Fixed:
                case CMath::Time:
                  Changed.insert(pObject);
                  break;

                case CMath::EventTarget:
                case CMath::ODE:
                case CMath::Independent:
                case CMath::Dependent:
                case CMath::Conversion:

                  if (pObject->getEntityType() != CMath::Species)
                    {
                      Changed.insert(pObject);
                    }
                  else if (pObject->isIntensiveProperty())
                    {
                      Requested.insert(pObject);
                    }
                  else
                    {
                      Changed.insert(pObject);
                    }

                  break;

                case CMath::Assignment:
                  Requested.insert(pObject);
                  break;
              }

            break;

            // Delay values are always calculate in a separate step
          case CMath::DelayValue:
            break;

            // Everything else must be calculated.
          default:
            Requested.insert(pObject);
            break;
        }
    }

  // Build the update sequence
  mTransientDependencies.getUpdateSequence(mApplyInitialValuesSequence, CMath::Default, Changed, Requested, Calculated);

  // It is possible that discontinuities only depend on constant values. Since discontinuities do not exist in the initial values
  // these are never calculate. It is save to prepend all discontinuities which are not already in the sequence
  if (mDiscontinuous.size() > 0)
    {
      // Find all discontinuities which are updated
      UpdateSequence::const_iterator it = mApplyInitialValuesSequence.begin();
      UpdateSequence::const_iterator end = mApplyInitialValuesSequence.end();

      CObjectInterface::ObjectSet UpdatedDiscontinuities;

      for (; it != end; ++it)
        {
          if (static_cast< CMathObject * >(*it)->getValueType() == CMath::Discontinuous)
            {
              UpdatedDiscontinuities.insert(*it);
            }
        }

      CObjectInterface * pDiscontinuity = getMathObject(mDiscontinuous.array());
      CObjectInterface * pDiscontinuityEnd = pDiscontinuity + mDiscontinuous.size();
      std::set< CObjectInterface * > OutofDateDiscontinuities;

      for (; pDiscontinuity != pDiscontinuityEnd; ++pDiscontinuity)
        {
          if (static_cast< CMathObject * >(pDiscontinuity)->getValueType() == CMath::Discontinuous &&
              UpdatedDiscontinuities.find(pDiscontinuity) == UpdatedDiscontinuities.end())
            {
              OutofDateDiscontinuities.insert(pDiscontinuity);
            }
        }

      if (OutofDateDiscontinuities.size() > 0)
        {
          mApplyInitialValuesSequence.insert(mApplyInitialValuesSequence.begin(), OutofDateDiscontinuities.begin(), OutofDateDiscontinuities.end());
        }
    }
}

void CMathContainer::createUpdateSimulationValuesSequence()
{
  mStateValues.clear();
  mReducedStateValues.clear();
  mSimulationRequiredValues.clear();

  // For the reduced model we force the values of the dependent variables to be calculated.
  CObjectInterface::ObjectSet ReducedSimulationRequiredValues;

  // Collect all the state objects, which are transient values of simulation type:
  //   Time, ODE, Independent, and Dependent (not for reduced model)

  const CMathObject * pObject = mObjects.array() + (mExtensiveValues.array() - mValues.array());
  const CMathObject * pObjectEnd = mObjects.array() + (mExtensiveRates.array() - mValues.array());

  for (; pObject != pObjectEnd; ++pObject)
    {
      switch (pObject->getSimulationType())
        {
          case CMath::EventTarget:
          case CMath::Time:
          case CMath::ODE:
          case CMath::Independent:
            mStateValues.insert(pObject);
            mReducedStateValues.insert(pObject);
            break;

          case CMath::Dependent:
            mStateValues.insert(pObject);
            ReducedSimulationRequiredValues.insert(pObject);
            break;

          case CMath::SimulationTypeUndefined:

            if (pObject->getValueType() == CMath::DelayValue)
              {
                mStateValues.insert(pObject);
                mReducedStateValues.insert(pObject);
              }

            break;

          default:
            break;
        }
    }

  // Collect all objects required for simulation, which are transient rates values of simulation type:
  //   ODE, Independent, and Dependent (not needed for reduced model) and EventRoots
  // The additional cost for calculating the rates for dependent species is neglected.
  pObject = mObjects.array() + (mExtensiveRates.array() - mValues.array()) + mSize.nFixed + mSize.nEventTargets + 1 /* Time */;
  pObjectEnd = mObjects.array() + (mIntensiveRates.array() - mValues.array());

  for (; pObject != pObjectEnd; ++pObject)
    {
      mSimulationRequiredValues.insert(pObject);
      ReducedSimulationRequiredValues.insert(pObject);
    }

  pObject = mObjects.array() + (mEventRoots.array() - mValues.array());
  pObjectEnd = mObjects.array() + (mEventRootStates.array() - mValues.array());

  for (; pObject != pObjectEnd; ++pObject)
    {
      mSimulationRequiredValues.insert(pObject);
      ReducedSimulationRequiredValues.insert(pObject);
    }

  pObject = mObjects.array() + (mDelayLags.array() - mValues.array());
  pObjectEnd = pObject + mDelayLags.size();

  for (; pObject != pObjectEnd; ++pObject)
    {
      mSimulationRequiredValues.insert(pObject);
      ReducedSimulationRequiredValues.insert(pObject);
    }

  // Build the update sequence
  mTransientDependencies.getUpdateSequence(mSimulationValuesSequence, CMath::Default, mStateValues, mSimulationRequiredValues);
  mTransientDependencies.getUpdateSequence(mSimulationValuesSequenceReduced, CMath::UseMoieties, mReducedStateValues, ReducedSimulationRequiredValues);

  // Determine whether the model is autonomous, i.e., no simulation required value depends on time.
  CObjectInterface::ObjectSet TimeObject;
  TimeObject.insert(getMathObject(mState.array() + getTimeIndex()));
  CObjectInterface::UpdateSequence TimeChange;
  mTransientDependencies.getUpdateSequence(TimeChange, CMath::Default, TimeObject, mSimulationRequiredValues);
  mIsAutonomous = (TimeChange.size() == 0);

  // Build the update sequence used to calculate the priorities in the event process queue.
  CObjectInterface::ObjectSet PriorityRequiredValues;
  pObject = getMathObject(mEventPriorities.array());
  pObjectEnd = pObject + mEventPriorities.size();

  for (; pObject != pObjectEnd; ++pObject)
    {
      PriorityRequiredValues.insert(pObject);
    }

  mTransientDependencies.getUpdateSequence(mPrioritySequence, CMath::Default, mStateValues, PriorityRequiredValues);
}

void CMathContainer::createUpdateAllTransientDataValuesSequence()
{
  // Collect all transient objects that have a data object associated
  CObjectInterface::ObjectSet TransientDataObjects;

  const CMathObject * pObject = mObjects.array() + (mExtensiveValues.array() - mValues.array());
  const CMathObject * pObjectEnd = mObjects.array() + mObjects.size();

  for (; pObject != pObjectEnd; ++pObject)
    {
      if (pObject->getDataObject() != NULL)
        {
          TransientDataObjects.insert(pObject);
        }
    }

  mTransientDependencies.getUpdateSequence(mTransientDataObjectSequence, CMath::Default, mStateValues, TransientDataObjects, mSimulationRequiredValues);
}

void CMathContainer::analyzeRoots()
{
  CObjectInterface::ObjectSet TimeValue;
  TimeValue.insert(getMathObject(mState.array() + getTimeIndex()));

  CObjectInterface::ObjectSet ContinousStateValues;
  const CMathObject * pStateObject = getMathObject(mState.array() + mSize.nEventTargets);
  const CMathObject * pStateObjectEnd = getMathObject(mState.array() + mState.size());

  for (; pStateObject != pStateObjectEnd; ++pStateObject)
    {
      ContinousStateValues.insert(pStateObject);
    }

  size_t RootCount = 0;
  C_FLOAT64 * pRootValue = mEventRoots.array();
  CMathObject * pRoot = getMathObject(pRootValue);
  CMathObject * pRootEnd = pRoot + mEventRoots.size();
  bool * pIsDiscrete = mRootIsDiscrete.array();
  bool * pIsTimeDependent = mRootIsTimeDependent.array();

  for (; pRoot != pRootEnd; ++pRoot, ++RootCount, ++pIsDiscrete, ++pRootValue)
    {
      if (pRoot->getExpressionPtr() == NULL)
        {
          *pRootValue = 1;
        }

      CObjectInterface::ObjectSet Requested;
      Requested.insert(pRoot);
      CObjectInterface::UpdateSequence UpdateSequence;

      mTransientDependencies.getUpdateSequence(UpdateSequence, CMath::Default, ContinousStateValues, Requested);
      *pIsDiscrete = (UpdateSequence.size() == 0);

      mTransientDependencies.getUpdateSequence(UpdateSequence, CMath::Default, TimeValue, Requested);
      *pIsTimeDependent = (UpdateSequence.size() != 0);
    }

  mEventRoots.initialize(RootCount, mEventRoots.array());
  mEventRootStates.initialize(RootCount, mEventRootStates.array());
  mRootIsDiscrete.resize(RootCount, true);
  mRootIsTimeDependent.resize(RootCount, true);

  mRootProcessors.resize(RootCount);

  CMathEvent * pEvent = mEvents.array();
  CMathEvent * pEventEnd = pEvent + mEvents.size();
  pRoot = getMathObject(mEventRoots.array());
  CMathEvent::CTrigger::CRootProcessor ** pRootProcessorPtr = mRootProcessors.array();

  for (; pEvent != pEventEnd; ++pEvent)
    {
      CMathEvent::CTrigger::CRootProcessor * pRootProcessor = const_cast< CMathEvent::CTrigger::CRootProcessor * >(pEvent->getTrigger().getRoots().array());
      CMathEvent::CTrigger::CRootProcessor * pRootProcessorEnd = pRootProcessor + pEvent->getTrigger().getRoots().size();

      for (; pRootProcessor != pRootProcessorEnd; ++pRootProcessor, ++pRoot, ++pRootProcessorPtr)
        {
          assert(pRootProcessor->mpRoot == pRoot);
          *pRootProcessorPtr = pRootProcessor;
        }
    }
}

void CMathContainer::calculateRootDerivatives(CVector< C_FLOAT64 > & rootDerivatives)
{
  updateSimulatedValues(false);

  CMatrix< C_FLOAT64 > Jacobian;
  calculateRootJacobian(Jacobian);

  rootDerivatives.resize(Jacobian.numRows());
  C_FLOAT64 * pDerivative = rootDerivatives.array();

  //We only consider the continuous state variables
  C_FLOAT64 * pRate = mRate.array() + mSize.nEventTargets;

  // Now multiply the the Jacobian with the rates
  char T = 'N';
  C_INT M = 1;
  C_INT N = (C_INT) Jacobian.numRows();
  C_INT K = (C_INT) Jacobian.numCols();
  C_FLOAT64 Alpha = 1.0;
  C_FLOAT64 Beta = 0.0;

  dgemm_(&T, &T, &M, &N, &K, &Alpha, pRate, &M,
         Jacobian.array(), &K, &Beta, pDerivative, &M);
}

void CMathContainer::calculateRootJacobian(CMatrix< C_FLOAT64 > & jacobian)
{
  size_t NumRows = mEventRoots.size();

  // Partial derivatives with respect to time and all variables determined by ODEs and reactions.
  size_t NumCols = 1 + mSize.nODE + mSize.nReactionSpecies;

  // The rates of all state variables in the current state.
  CVector< C_FLOAT64 > Rate = mRate;

  jacobian.resize(NumRows, NumCols);

  size_t Col = 0;

  C_FLOAT64 X1 = 0.0;
  C_FLOAT64 X2 = 0.0;
  C_FLOAT64 InvDelta = 0.0;

  CVector< C_FLOAT64 > Y1(NumRows);
  CVector< C_FLOAT64 > Y2(NumRows);

  C_FLOAT64 * pX = mState.array() + mSize.nEventTargets;
  C_FLOAT64 * pXEnd = mState.array() + mState.size();

  C_FLOAT64 * pJacobian = jacobian.array();
  C_FLOAT64 * pJacobianEnd = pJacobian + jacobian.size();

  const C_FLOAT64 * pRate = Rate.array() + mSize.nEventTargets;

  for (; pX != pXEnd; ++pX, ++Col, ++pRate)
    {
      C_FLOAT64 Store = *pX;

      if (fabs(*pRate) < 1e4 * std::numeric_limits< C_FLOAT64 >::epsilon() * fabs(Store) ||
          fabs(*pRate) < 1e4 * std::numeric_limits< C_FLOAT64 >::min())
        {
          if (fabs(Store) < 100.0 * std::numeric_limits< C_FLOAT64 >::min())
            {
              X1 = 0.0;

              if (Store < 0.0)
                X2 = -200.0 * std::numeric_limits< C_FLOAT64 >::min();
              else
                X2 = 200.0 * std::numeric_limits< C_FLOAT64 >::min();

              InvDelta = X2;
            }
          else
            {
              X1 = 0.999 * Store;
              X2 = 1.001 * Store;
              InvDelta = 500.0 / Store;
            }
        }
      else
        {
          X1 = Store - 0.001 * *pRate;
          X2 = Store + 0.001 * *pRate;
          InvDelta = 500.0 / *pRate;
        }

      *pX = X1;
      updateSimulatedValues(false);
      Y1 = mEventRoots;

      *pX = X2;
      updateSimulatedValues(false);
      Y2 = mEventRoots;

      *pX = Store;

      pJacobian = jacobian.array() + Col;
      C_FLOAT64 * pY1 = Y1.array();
      C_FLOAT64 * pY2 = Y2.array();

      for (; pJacobian < pJacobianEnd; pJacobian += NumCols, ++pY1, ++pY2)
        * pJacobian = (*pY2 - *pY1) * InvDelta;
    }

  // Undo the changes.
  updateSimulatedValues(false);
}

void CMathContainer::calculateJacobian(CMatrix< C_FLOAT64 > & jacobian,
                                       const C_FLOAT64 & derivationFactor,
                                       const bool & reduced)
{
  size_t Dim = getState(reduced).size() - getTimeIndex() - 1;
  jacobian.resize(Dim, Dim);

  C_FLOAT64 DerivationFactor = std::max(derivationFactor, 100.0 * std::numeric_limits< C_FLOAT64 >::epsilon());

  C_FLOAT64 * pState = mState.array() + getTimeIndex() + 1;
  const C_FLOAT64 * pRate = mRate.array() + getTimeIndex() + 1;

  size_t Col;

  C_FLOAT64 Store;
  C_FLOAT64 X1;
  C_FLOAT64 X2;
  C_FLOAT64 InvDelta;

  CVector< C_FLOAT64 > Y1(Dim);
  CVector< C_FLOAT64 > Y2(Dim);

  C_FLOAT64 * pY1;
  C_FLOAT64 * pY2;

  C_FLOAT64 * pX = pState;
  C_FLOAT64 * pXEnd = pX + Dim;

  C_FLOAT64 * pJacobian;
  C_FLOAT64 * pJacobianEnd = jacobian.array() + Dim * Dim;

  for (Col = 0; pX != pXEnd; ++pX, ++Col)
    {
      Store = *pX;

      // We only need to make sure that we do not have an underflow problem
      if (fabs(Store) < DerivationFactor)
        {
          X1 = 0.0;

          if (Store < 0.0)
            X2 = -2.0 * DerivationFactor;
          else
            X2 = 2.0 * DerivationFactor;;
        }
      else
        {
          X1 = Store * (1.0 + DerivationFactor);
          X2 = Store * (1.0 - DerivationFactor);
        }

      InvDelta = 1.0 / (X2 - X1);

      *pX = X1;
      updateSimulatedValues(reduced);
      memcpy(Y1.array(), pRate, Dim * sizeof(C_FLOAT64));

      *pX = X2;
      updateSimulatedValues(reduced);
      memcpy(Y2.array(), pRate, Dim * sizeof(C_FLOAT64));

      *pX = Store;

      pJacobian = jacobian.array() + Col;
      pY1 = Y1.array();
      pY2 = Y2.array();

      for (; pJacobian < pJacobianEnd; pJacobian += Dim, ++pY1, ++pY2)
        * pJacobian = (*pY2 - *pY1) * InvDelta;
    }

  updateSimulatedValues(reduced);
}

CMath::StateChange CMathContainer::processQueue(const bool & equality)
{
  return mpProcessQueue->process(equality);
}

void CMathContainer::processRoots(const bool & equality,
                                  const CVector< C_INT > & rootsFound)
{
#ifdef DEBUG_OUTPUT
  std::cout << rootsFound << std::endl;
#endif // DEBUG_OUTPUT

  // Reevaluate all non found roots.
  CMathEvent::CTrigger::CRootProcessor ** pRoot = mRootProcessors.array();
  CMathEvent::CTrigger::CRootProcessor ** pRootEnd = pRoot + mRootProcessors.size();
  const C_INT * pRootFound = rootsFound.array();

  for (; pRoot != pRootEnd; ++pRoot, ++pRootFound)
    {
      if (*pRootFound == CMath::NoToggle)
        {
          (*pRoot)->calculateTrueValue();
        }
    }

  // Calculate the trigger values and store them before the root processors
  // are changing the state
  CMathObject * pTrigger = getMathObject(mEventTriggers.array());
  CMathObject * pTriggerEnd = pTrigger + mEventTriggers.size();

  for (; pTrigger != pTriggerEnd; ++pTrigger)
    {
      pTrigger->calculateValue();
    }

  CVector< C_FLOAT64 > Before = mEventTriggers;

  // Toggle all found roots.
  pRoot = mRootProcessors.array();
  pRootFound = rootsFound.array();
  C_FLOAT64 & Time = mState[mSize.nEventTargets];

  for (; pRoot != pRootEnd; ++pRoot, ++pRootFound)
    {
      if (*pRootFound == CMath::ToggleBoth ||
          (*pRootFound == CMath::ToggleEquality && equality) ||
          (*pRootFound == CMath::ToggleInequality && !equality))
        {
          (*pRoot)->toggle(Time, equality);
        }
    }

  // Calculate the new trigger values
  pTrigger = getMathObject(mEventTriggers.array());

  for (; pTrigger != pTriggerEnd; ++pTrigger)
    {
      pTrigger->calculateValue();
    }

  // Find out which events fire and add them to the process queue
  C_FLOAT64 * pBefore = Before.array();
  C_FLOAT64 * pAfter = mEventTriggers.array();
  CMathEvent * pEvent = mEvents.array();
  CMathEvent * pEventEnd = pEvent + mEvents.size();

  // Compare Before and the current mEventTriggers
  for (; pEvent != pEventEnd; ++pEvent, ++pBefore, ++pAfter)
    {
      if (*pBefore != *pAfter)
        {
          // We fire on any change. It is the responsibility of the event to add or remove
          // actions to the process queue.
          pEvent->fire(equality);
        }
    }

  return;
}

void CMathContainer::processRoots(const CVector< C_INT > & rootsFound)
{
  // Calculate the trigger values and store them before the root processors
  // are changing the state
  CMathObject * pTrigger = getMathObject(mEventTriggers.array());
  CMathObject * pTriggerEnd = pTrigger + mEventTriggers.size();

  for (; pTrigger != pTriggerEnd; ++pTrigger)
    {
      pTrigger->calculateValue();
    }

  CVector< C_FLOAT64 > Before = mEventTriggers;

  // Toggle all found roots.
  CMathEvent::CTrigger::CRootProcessor ** pRoot = mRootProcessors.array();
  CMathEvent::CTrigger::CRootProcessor ** pRootEnd = pRoot + mRootProcessors.size();
  const C_INT * pRootFound = rootsFound.array();
  C_FLOAT64 & Time = mState[mSize.nEventTargets];

  for (; pRoot != pRootEnd; ++pRoot, ++pRootFound)
    {
      if (*pRootFound)
        {
          (*pRoot)->toggle(Time);
        }

      // We must not reevaluate.
    }

  // Calculate the new trigger values
  pTrigger = getMathObject(mEventTriggers.array());

  for (; pTrigger != pTriggerEnd; ++pTrigger)
    {
      pTrigger->calculateValue();
    }

  // Find out which events fire and add them to the process queue
  C_FLOAT64 * pBefore = Before.array();
  C_FLOAT64 * pAfter = mEventTriggers.array();
  CMathEvent * pEvent = mEvents.array();
  CMathEvent * pEventEnd = pEvent + mEvents.size();

  // Compare Before and the current mEventTriggers
  for (; pEvent != pEventEnd; ++pEvent, ++pBefore, ++pAfter)
    {
      if (*pBefore != *pAfter)
        {
          // We fire on any change. It is the responsibility of the event to add or remove
          // actions to the process queue.

          pEvent->fire(true);
        }
    }

  return;
}

CMathEventQueue & CMathContainer::getProcessQueue()
{
  return *mpProcessQueue;
}

C_FLOAT64 CMathContainer::getProcessQueueExecutionTime() const
{
  return mpProcessQueue->getProcessQueueExecutionTime();
}

void CMathContainer::initializePointers(CMath::sPointers & p)
{
  p.pInitialExtensiveValues = mInitialExtensiveValues.array();
  p.pInitialIntensiveValues = mInitialIntensiveValues.array();
  p.pInitialExtensiveRates = mInitialExtensiveRates.array();
  p.pInitialIntensiveRates = mInitialIntensiveRates.array();
  p.pInitialParticleFluxes = mInitialParticleFluxes.array();
  p.pInitialFluxes = mInitialFluxes.array();
  p.pInitialTotalMasses = mInitialTotalMasses.array();
  p.pInitialEventTriggers = mInitialEventTriggers.array();

  p.pExtensiveValues = mExtensiveValues.array();
  p.pIntensiveValues = mIntensiveValues.array();
  p.pExtensiveRates = mExtensiveRates.array();
  p.pIntensiveRates = mIntensiveRates.array();
  p.pParticleFluxes = mParticleFluxes.array();
  p.pFluxes = mFluxes.array();
  p.pTotalMasses = mTotalMasses.array();
  p.pEventTriggers = mEventTriggers.array();

  p.pEventDelays = mEventDelays.array();
  p.pEventPriorities = mEventPriorities.array();
  p.pEventAssignments = mEventAssignments.array();
  p.pEventRoots = mEventRoots.array();
  p.pEventRootStates = mEventRootStates.array();
  p.pPropensities = mPropensities.array();
  p.pDependentMasses = mDependentMasses.array();
  p.pDiscontinuous = mDiscontinuous.array();
  p.pDelayValue = mDelayValues.array();
  p.pDelayLag = mDelayLags.array();
  p.pTransitionTime = mTransitionTimes.array();

  C_FLOAT64 * pValues = mValues.array();
  CMathObject * pObjects = mObjects.array();

  p.pInitialExtensiveValuesObject = pObjects + (p.pInitialExtensiveValues - pValues);
  p.pInitialIntensiveValuesObject = pObjects + (p.pInitialIntensiveValues - pValues);
  p.pInitialExtensiveRatesObject = pObjects + (p.pInitialExtensiveRates - pValues);
  p.pInitialIntensiveRatesObject = pObjects + (p.pInitialIntensiveRates - pValues);
  p.pInitialParticleFluxesObject = pObjects + (p.pInitialParticleFluxes - pValues);
  p.pInitialFluxesObject = pObjects + (p.pInitialFluxes - pValues);
  p.pInitialTotalMassesObject = pObjects + (p.pInitialTotalMasses - pValues);
  p.pInitialEventTriggersObject = pObjects + (p.pInitialEventTriggers - pValues);

  p.pExtensiveValuesObject = pObjects + (p.pExtensiveValues - pValues);
  p.pIntensiveValuesObject = pObjects + (p.pIntensiveValues - pValues);
  p.pExtensiveRatesObject = pObjects + (p.pExtensiveRates - pValues);
  p.pIntensiveRatesObject = pObjects + (p.pIntensiveRates - pValues);
  p.pParticleFluxesObject = pObjects + (p.pParticleFluxes - pValues);
  p.pFluxesObject = pObjects + (p.pFluxes - pValues);
  p.pTotalMassesObject = pObjects + (p.pTotalMasses - pValues);
  p.pEventTriggersObject = pObjects + (p.pEventTriggers - pValues);

  p.pEventDelaysObject = pObjects + (p.pEventDelays - pValues);
  p.pEventPrioritiesObject = pObjects + (p.pEventPriorities - pValues);
  p.pEventAssignmentsObject = pObjects + (p.pEventAssignments - pValues);
  p.pEventRootsObject = pObjects + (p.pEventRoots - pValues);
  p.pEventRootStatesObject = pObjects + (p.pEventRootStates - pValues);
  p.pPropensitiesObject = pObjects + (p.pPropensities - pValues);
  p.pDependentMassesObject = pObjects + (p.pDependentMasses - pValues);
  p.pDiscontinuousObject = pObjects + (p.pDiscontinuous - pValues);
  p.pDelayValueObject = pObjects + (p.pDelayValue - pValues);
  p.pDelayLagObject = pObjects + (p.pDelayLag - pValues);
  p.pTransitionTimeObject = pObjects + (p.pTransitionTime - pValues);
}

#ifdef COPASI_DEBUG
void CMathContainer::printPointers(CMath::sPointers & p)
{
  size_t Index;
  std::cout << "Values:" << std::endl;
  Index = p.pInitialExtensiveValues - mInitialExtensiveValues.array();
  std::cout << "  mInitialExtensiveValues:[" << Index << "]" << ((mInitialExtensiveValues.size() <= Index) ? " Error" : "") << std::endl;
  Index = p.pInitialIntensiveValues - mInitialIntensiveValues.array();
  std::cout << "  mInitialIntensiveValues:[" << Index << "]" << ((mInitialIntensiveValues.size() <= Index) ? " Error" : "") << std::endl;
  Index = p.pInitialExtensiveRates - mInitialExtensiveRates.array();
  std::cout << "  mInitialExtensiveRates:[" << Index << "]" << ((mInitialExtensiveRates.size() <= Index) ? " Error" : "") << std::endl;
  Index = p.pInitialIntensiveRates - mInitialIntensiveRates.array();
  std::cout << "  mInitialIntensiveRates:[" << Index << "]" << ((mInitialIntensiveRates.size() <= Index) ? " Error" : "") << std::endl;
  Index = p.pInitialParticleFluxes - mInitialParticleFluxes.array();
  std::cout << "  mInitialParticleFluxes:[" << Index << "]" << ((mInitialParticleFluxes.size() <= Index) ? " Error" : "") << std::endl;
  Index = p.pInitialFluxes - mInitialFluxes.array();
  std::cout << "  mInitialFluxes:[" << Index << "]" << ((mInitialFluxes.size() <= Index) ? " Error" : "") << std::endl;
  Index = p.pInitialTotalMasses - mInitialTotalMasses.array();
  std::cout << "  mInitialTotalMasses:[" << Index << "]" << ((mInitialTotalMasses.size() <= Index) ? " Error" : "") << std::endl;
  Index = p.pInitialEventTriggers - mInitialEventTriggers.array();
  std::cout << "  mInitialEventTriggers:[" << Index << "]" << ((mInitialEventTriggers.size() <= Index) ? " Error" : "") << std::endl;

  Index = p.pExtensiveValues - mExtensiveValues.array();
  std::cout << "  mExtensiveValues:[" << Index << "]" << ((mExtensiveValues.size() <= Index) ? " Error" : "") << std::endl;
  Index = p.pIntensiveValues - mIntensiveValues.array();
  std::cout << "  mIntensiveValues:[" << Index << "]" << ((mIntensiveValues.size() <= Index) ? " Error" : "") << std::endl;
  Index = p.pExtensiveRates - mExtensiveRates.array();
  std::cout << "  mExtensiveRates:[" << Index << "]" << ((mExtensiveRates.size() <= Index) ? " Error" : "") << std::endl;
  Index = p.pIntensiveRates - mIntensiveRates.array();
  std::cout << "  mIntensiveRates:[" << Index << "]" << ((mIntensiveRates.size() <= Index) ? " Error" : "") << std::endl;
  Index = p.pParticleFluxes - mParticleFluxes.array();
  std::cout << "  mParticleFluxes:[" << Index << "]" << ((mParticleFluxes.size() <= Index) ? " Error" : "") << std::endl;
  Index = p.pFluxes - mFluxes.array();
  std::cout << "  mFluxes:[" << Index << "]" << ((mFluxes.size() <= Index) ? " Error" : "") << std::endl;
  Index = p.pTotalMasses - mTotalMasses.array();
  std::cout << "  mTotalMasses:[" << Index << "]" << ((mTotalMasses.size() <= Index) ? " Error" : "") << std::endl;
  Index = p.pEventTriggers - mEventTriggers.array();
  std::cout << "  mEventTriggers:[" << Index << "]" << ((mEventTriggers.size() <= Index) ? " Error" : "") << std::endl;

  Index = p.pEventDelays - mEventDelays.array();
  std::cout << "  mEventDelays:[" << Index << "]" << ((mEventDelays.size() <= Index) ? " Error" : "") << std::endl;
  Index = p.pEventPriorities - mEventPriorities.array();
  std::cout << "  mEventPriorities:[" << Index << "]" << ((mEventPriorities.size() <= Index) ? " Error" : "") << std::endl;
  Index = p.pEventAssignments - mEventAssignments.array();
  std::cout << "  mEventAssignments:[" << Index << "]" << ((mEventAssignments.size() <= Index) ? " Error" : "") << std::endl;
  Index = p.pEventRoots - mEventRoots.array();
  std::cout << "  mEventRoots:[" << Index << "]" << ((mEventRoots.size() <= Index) ? " Error" : "") << std::endl;
  Index = p.pEventRootStates - mEventRootStates.array();
  std::cout << "  mEventRootStates:[" << Index << "]" << ((mEventRootStates.size() <= Index) ? " Error" : "") << std::endl;
  Index = p.pPropensities - mPropensities.array();
  std::cout << "  mPropensities:[" << Index << "]" << ((mPropensities.size() <= Index) ? " Error" : "") << std::endl;
  Index = p.pDependentMasses - mDependentMasses.array();
  std::cout << "  mDependentMasses:[" << Index << "]" << ((mDependentMasses.size() <= Index) ? " Error" : "") << std::endl;
  Index = p.pDiscontinuous - mDiscontinuous.array();
  std::cout << "  mDiscontinuous:[" << Index << "]" << ((mDiscontinuous.size() <= Index) ? " Error" : "") << std::endl;
  Index = p.pDelayValue - mDelayValues.array();
  std::cout << "  mDelayValue:[" << Index << "]" << ((mDelayValues.size() <= Index) ? " Error" : "") << std::endl;
  Index = p.pDelayLag - mDelayLags.array();
  std::cout << "  mDelayLag:[" << Index << "]" << ((mDelayLags.size() <= Index) ? " Error" : "") << std::endl;
  Index = p.pTransitionTime - mTransitionTimes.array();
  std::cout << "  mTransitionTime:[" << Index << "]" << ((mTransitionTimes.size() <= Index) ? " Error" : "") << std::endl;
  std::cout << std::endl;
}
#endif // COPASI_DEBUG

void CMathContainer::initializeDiscontinuousCreationPointer()
{
  C_FLOAT64 * pValues = mValues.array();
  CMathObject * pObjects = mObjects.array();

  size_t nDiscontinuous = mDiscontinuous.size();
  size_t nEvents = mEvents.size() - nDiscontinuous;

  mCreateDiscontinuousPointer.pEvent = mEvents.array() + nEvents;
  mCreateDiscontinuousPointer.pDiscontinuous = pObjects + (mDiscontinuous.array() - pValues);

  /*
  mCreateDiscontinuousPointer.pEventDelay = pObjects + (mEventDelays.array() - pValues) + nEvents;
  mCreateDiscontinuousPointer.pEventPriority = pObjects + (mEventPriorities.array() - pValues) + nEvents;
  mCreateDiscontinuousPointer.pEventAssignment = pObjects + (mEventAssignments.array() - pValues) + nEventAssignments;
  mCreateDiscontinuousPointer.pEventTrigger = pObjects + (mEventTriggers.array() - pValues) + nEvents;
  mCreateDiscontinuousPointer.pEventRoot = pObjects + (mEventRoots.array() - pValues) + nEventRoots;
  */
}

// static
CMath::EntityType CMathContainer::getEntityType(const CModelEntity * pEntity)
{
  const CMetab * pSpecies = dynamic_cast< const CMetab * >(pEntity);

  if (pSpecies != NULL)
    {
      return CMath::Species;
    }
  else if (dynamic_cast< const CCompartment * >(pEntity) != NULL)
    {
      return CMath::Compartment;
    }
  else if (dynamic_cast< const CModelValue * >(pEntity) != NULL)
    {
      return CMath::GlobalQuantity;
    }

  return CMath::EntityTypeUndefined;
}

void CMathContainer::initializeMathObjects(const std::vector<const CModelEntity*> & entities,
    const CMath::SimulationType & simulationType,
    CMath::sPointers & p)
{
  // Process entities.
  std::vector<const CModelEntity*>::const_iterator it = entities.begin();
  std::vector<const CModelEntity*>::const_iterator end = entities.end();

  CMath::EntityType EntityType;

  for (; it != end; ++it)
    {
      EntityType = getEntityType(*it);

      // Extensive Initial Value

      // The simulation type for initial values is either CMath::Assignment or CMath::Fixed
      // We must check whether the initial value must be calculated, i.e., whether it has
      // dependencies or not. In case of species it always possible that is must be calculated.

      CMath::SimulationType SimulationType = CMath::Fixed;
      CCopasiObject * pObject = (*it)->getInitialValueReference();

      if (EntityType == CMath::Species)
        {
          SimulationType = CMath::Conversion;
        }
      else if ((simulationType == CMath::Assignment && (*it)->getExpression() != "") ||
               (*it)->getInitialExpression() != "")
        {
          SimulationType = CMath::Assignment;
        }

      map(pObject, p.pInitialExtensiveValuesObject);
      CMathObject::initialize(p.pInitialExtensiveValuesObject, p.pInitialExtensiveValues,
                              CMath::Value, EntityType, SimulationType, false, true,
                              pObject);

      // Extensive Value
      SimulationType = simulationType;

      if (EntityType == CMath::Species &&
          simulationType == CMath::Assignment)
        {
          SimulationType = CMath::Conversion;
        }

      map((*it)->getValueReference(), p.pExtensiveValuesObject);
      CMathObject::initialize(p.pExtensiveValuesObject, p.pExtensiveValues,
                              CMath::Value, EntityType, SimulationType, false, false,
                              (*it)->getValueReference());

      // Initial Extensive Rate
      SimulationType = simulationType;

      if (simulationType == CMath::EventTarget)
        {
          SimulationType = CMath::Fixed;
        }

      CMathObject::initialize(p.pInitialExtensiveRatesObject, p.pInitialExtensiveRates,
                              CMath::Rate, EntityType, SimulationType, false, true,
                              (*it)->getRateReference());

      // Extensive Rate
      map((*it)->getRateReference(), p.pExtensiveRatesObject);
      CMathObject::initialize(p.pExtensiveRatesObject, p.pExtensiveRates,
                              CMath::Rate, EntityType, SimulationType, false, false,
                              (*it)->getRateReference());

      // Species have intensive values in addition to the extensive ones and transition time.
      if (EntityType == CMath::Species)
        {
          const CMetab *pSpecies = static_cast<const CMetab*>(*it);

          // Intensive Initial Value

          // The simulation type for initial values is either CMath::Assignment or CMath::Conversion
          // In case of species it always possible that is must be calculated.
          SimulationType = CMath::Conversion;

          if (simulationType == CMath::Assignment)
            {
              SimulationType = CMath::Assignment;
            }

          map(pSpecies->getInitialConcentrationReference(), p.pInitialIntensiveValuesObject);
          CMathObject::initialize(p.pInitialIntensiveValuesObject, p.pInitialIntensiveValues,
                                  CMath::Value, CMath::Species, SimulationType, true, true,
                                  pSpecies->getInitialConcentrationReference());

          // Intensive Value
          SimulationType = CMath::Conversion;

          if (simulationType == CMath::Assignment)
            {
              SimulationType = simulationType;
            }

          map(pSpecies->getConcentrationReference(), p.pIntensiveValuesObject);
          CMathObject::initialize(p.pIntensiveValuesObject, p.pIntensiveValues,
                                  CMath::Value, CMath::Species, SimulationType, true, false,
                                  pSpecies->getConcentrationReference());

          // Initial Intensive Rate
          CMathObject::initialize(p.pInitialIntensiveRatesObject, p.pInitialIntensiveRates,
                                  CMath::Rate, CMath::Species, CMath::Assignment, true, true,
                                  pSpecies->getConcentrationRateReference());

          // Intensive Rate
          map(pSpecies->getConcentrationRateReference(), p.pIntensiveRatesObject);
          CMathObject::initialize(p.pIntensiveRatesObject, p.pIntensiveRates,
                                  CMath::Rate, CMath::Species, CMath::Assignment, true, false,
                                  pSpecies->getConcentrationRateReference());

          // Transition Time
          map(pSpecies->getTransitionTimeReference(), p.pTransitionTimeObject);
          CMathObject::initialize(p.pTransitionTimeObject, p.pTransitionTime,
                                  CMath::TransitionTime, CMath::Species, CMath::Assignment, false, false,
                                  pSpecies->getTransitionTimeReference());
        }
    }
}

void CMathContainer::initializeMathObjects(const std::vector<const CCopasiObject *> & parameters,
    CMath::sPointers & p)
{
  // Process parameters.
  std::vector<const CCopasiObject *>::const_iterator it = parameters.begin();
  std::vector<const CCopasiObject *>::const_iterator end = parameters.end();

  for (; it != end; ++it)
    {
      // Extensive Initial Value
      map(const_cast< CCopasiObject * >(*it), p.pInitialExtensiveValuesObject);
      CMathObject::initialize(p.pInitialExtensiveValuesObject, p.pInitialExtensiveValues,
                              CMath::Value, CMath::LocalReactionParameter, CMath::Fixed, false, true,
                              *it);

      // Extensive Value
      CMathObject::initialize(p.pExtensiveValuesObject, p.pExtensiveValues,
                              CMath::Value, CMath::LocalReactionParameter, CMath::Fixed, false, false,
                              NULL);

      // Initial Extensive Rate
      CMathObject::initialize(p.pInitialExtensiveRatesObject, p.pInitialExtensiveRates,
                              CMath::Rate, CMath::LocalReactionParameter, CMath::Fixed, false, true,
                              NULL);

      // Extensive Rate
      CMathObject::initialize(p.pExtensiveRatesObject, p.pExtensiveRates,
                              CMath::Rate, CMath::LocalReactionParameter, CMath::Fixed, false, false,
                              NULL);
    }
}

void CMathContainer::initializeMathObjects(const CCopasiVector< CReaction > & reactions,
    CMath::sPointers & p)
{
  // Process reactions.
  CCopasiVector< CReaction >::const_iterator it = reactions.begin();
  CCopasiVector< CReaction >::const_iterator end = reactions.end();

  for (; it != end; ++it)
    {
      // We ignore reactions which do not have any effect.
      if ((*it)->getChemEq().getBalances().size() == 0)
        {
          continue;
        }

      // Initial Particle Flux
      CMathObject::initialize(p.pInitialParticleFluxesObject, p.pInitialParticleFluxes,
                              CMath::ParticleFlux, CMath::Reaction, CMath::SimulationTypeUndefined, false, true,
                              (*it)->getParticleFluxReference());

      // Particle Flux
      map((*it)->getParticleFluxReference(), p.pParticleFluxesObject);
      CMathObject::initialize(p.pParticleFluxesObject, p.pParticleFluxes,
                              CMath::ParticleFlux, CMath::Reaction, CMath::SimulationTypeUndefined, false, false,
                              (*it)->getParticleFluxReference());

      // Initial Flux
      CMathObject::initialize(p.pInitialFluxesObject, p.pInitialFluxes,
                              CMath::Flux, CMath::Reaction, CMath::SimulationTypeUndefined, false, true,
                              (*it)->getFluxReference());

      // Flux
      map((*it)->getFluxReference(), p.pFluxesObject);
      CMathObject::initialize(p.pFluxesObject, p.pFluxes,
                              CMath::Flux, CMath::Reaction, CMath::SimulationTypeUndefined, false, false,
                              (*it)->getFluxReference());

      // Propensity
      map((*it)->getPropensityReference(), p.pPropensitiesObject);
      CMathObject::initialize(p.pPropensitiesObject, p.pPropensities,
                              CMath::Propensity, CMath::Reaction, CMath::SimulationTypeUndefined, false, false,
                              (*it)->getPropensityReference());
    }
}

void CMathContainer::initializeMathObjects(const CCopasiVector< CMoiety > & moieties,
    CMath::sPointers & p)
{
  // Process reactions.
  CCopasiVector< CMoiety >::const_iterator it = moieties.begin();
  CCopasiVector< CMoiety >::const_iterator end = moieties.end();

  for (; it != end; ++it)
    {
      // Initial Total Mass
      CMathObject::initialize(p.pInitialTotalMassesObject, p.pInitialTotalMasses,
                              CMath::TotalMass, CMath::Moiety, CMath::SimulationTypeUndefined, false, true,
                              (*it)->getTotalNumberReference());

      // Total Mass
      map((*it)->getTotalNumberReference(), p.pTotalMassesObject);
      CMathObject::initialize(p.pTotalMassesObject, p.pTotalMasses,
                              CMath::TotalMass, CMath::Moiety, CMath::SimulationTypeUndefined, false, false,
                              (*it)->getTotalNumberReference());

      // Dependent
      map((*it)->getDependentNumberReference(), p.pDependentMassesObject);
      CMathObject::initialize(p.pDependentMassesObject, p.pDependentMasses,
                              CMath::DependentMass, CMath::Moiety, CMath::SimulationTypeUndefined, false, false,
                              (*it)->getDependentNumberReference());
    }
}

// static
bool CMathContainer::hasDependencies(const CCopasiObject * pObject)
{
  const CCopasiObject::DataObjectSet & Dependencies = pObject->getDirectDependencies();

  if (Dependencies.find(pObject->getObjectParent()) != Dependencies.end())
    {
      return Dependencies.size() > 1;
    }

  return Dependencies.size() > 0;
}

void CMathContainer::map(CCopasiObject * pDataObject, CMathObject * pMathObject)
{
  if (pDataObject != NULL)
    {
      mDataObject2MathObject[pDataObject] = pMathObject;
      mDataValue2MathObject[(C_FLOAT64 *) pDataObject->getValuePointer()] = pMathObject;
    }
}

C_FLOAT64 * CMathContainer::getInitialValuePointer(const C_FLOAT64 * pValue) const
{
  assert((mValues.array() <= pValue && pValue < mValues.array() + mValues.size()) || getDataObject(pValue) != NULL);

  const C_FLOAT64 * pInitialValue = pValue;

  if (mExtensiveValues.array() <= pValue && pValue < mEventDelays.array())
    {
      pInitialValue = mInitialExtensiveValues.array() + (pValue - mExtensiveValues.array());
    }

  return const_cast< C_FLOAT64 * >(pInitialValue);
}

CMath::Entity< CMathObject > CMathContainer::addAnalysisObject(const CMath::Entity< CCopasiObject > & entity,
    const CMath::SimulationType & simulationType,
    const std::string & infix)
{
  CMath::Entity< CMathObject > Entity;

  sSize Size = mSize;

  switch (simulationType)
    {
      case CMath::Fixed:
        Size.nFixed++;
        break;

      case CMath::ODE:
        Size.nODE++;
        break;

      case CMath::Assignment:
        Size.nAssignment++;
        break;

      case CMath::SimulationTypeUndefined:
      case CMath::EventTarget:
      case CMath::Time:
      case CMath::Dependent:
      case CMath::Independent:
      case CMath::Conversion:
        fatalError();
        break;
    }

  resize(Size);

  CExpression Expression("Source", this);

  if (!Expression.setInfix(infix)) return Entity;

  CMathObject * pObject = mObjects.array();
  CMathObject * pObjectEnd = pObject + mObjects.size();
  size_t kind = 0;

  for (; pObject != pObjectEnd; ++pObject)
    {
      if (pObject->getValueType() != CMath::ValueTypeUndefined ||
          pObject->getEntityType() != CMath::EntityTypeUndefined ||
          pObject->getSimulationType() != CMath::SimulationTypeUndefined) continue;

      C_FLOAT64 * pValue = (C_FLOAT64 *) pObject->getValuePointer();

      switch (kind)
        {
          case 0:
            CMathObject::initialize(pObject, pValue, CMath::Value, CMath::Analysis,
                                    (simulationType == CMath::Assignment) ? CMath::Assignment : CMath::Fixed,
                                    false, true, entity.InitialValue);

            if (simulationType == CMath::Assignment)
              {
                CMathExpression * pExpression = new CMathExpression("Assignment", *this);
                pExpression->CEvaluationTree::setRoot(copyBranch(Expression.getRoot(), false));
                pExpression->convertToInitialExpression();
                pObject->setExpressionPtr(pExpression);
              }

            if (entity.InitialValue != NULL)
              {
                map(entity.InitialValue, pObject);
              }

            Entity.InitialValue = pObject;
            break;

          case 1:
            CMathObject::initialize(pObject, pValue, CMath::Rate, CMath::Analysis,
                                    simulationType, false, true,
                                    entity.InitialRate);

            if (simulationType == CMath::ODE)
              {
                CMathExpression * pExpression = new CMathExpression("Rate", *this);
                pExpression->CEvaluationTree::setRoot(copyBranch(Expression.getRoot(), false));
                pExpression->convertToInitialExpression();
                pObject->setExpressionPtr(pExpression);
              }

            if (entity.InitialValue != NULL)
              {
                map(entity.InitialRate, pObject);
              }

            Entity.InitialRate = pObject;
            break;

          case 2:
            CMathObject::initialize(pObject, pValue, CMath::Value, CMath::Analysis,
                                    simulationType, false, false,
                                    entity.Value);

            if (simulationType == CMath::Assignment)
              {
                CMathExpression * pExpression = new CMathExpression("Assignment", *this);
                pExpression->CEvaluationTree::setRoot(copyBranch(Expression.getRoot(), false));
                pObject->setExpressionPtr(pExpression);
              }

            if (entity.InitialValue != NULL)
              {
                map(entity.Value, pObject);
              }

            Entity.Value = pObject;
            break;

          case 3:
            CMathObject::initialize(pObject, pValue, CMath::Rate, CMath::Analysis,
                                    simulationType, false, false,
                                    entity.Rate);

            if (simulationType == CMath::ODE)
              {
                CMathExpression * pExpression = new CMathExpression("Rate", *this);
                pExpression->CEvaluationTree::setRoot(copyBranch(Expression.getRoot(), false));
                pObject->setExpressionPtr(pExpression);
              }

            if (entity.InitialValue != NULL)
              {
                map(entity.Rate, pObject);
              }

            Entity.Rate = pObject;
            break;
        }

      pObject->compile(*this);
      mInitialDependencies.addObject(pObject);
    }

  createUpdateSequences();

  return Entity;
}

bool CMathContainer::removeAnalysisObject(CMath::Entity< CMathObject > & mathObjects)
{
  sSize Size = mSize;
  size_t Index = C_INVALID_INDEX;

  switch (mathObjects.Value->getSimulationType())
    {
      case CMath::Fixed:
        Size.nFixed--;
        Index += mSize.nFixed;
        break;

      case CMath::EventTarget:
        Size.nEventTargets--;
        Index += mSize.nFixed + mSize.nEventTargets;
        break;

      case CMath::ODE:
        Size.nODE--;
        Index += mSize.nFixed + mSize.nEventTargets + 2;
        break;

      case CMath::Assignment:
        Size.nAssignment--;
        Index += mSize.nFixed + mSize.nEventTargets + 1 + mSize.nODE + mSize.nReactionSpecies + mSize.nAssignment;
        break;

      case CMath::SimulationTypeUndefined:
      case CMath::Time:
      case CMath::Dependent:
      case CMath::Independent:
      case CMath::Conversion:
        fatalError();
        break;
    }

  // Check whether this is the last added entity.
  // Entities can only be removed in reverse order
  if (mathObjects.InitialValue != &mObjects[Index]) return false;

  // Update dependencies
  mInitialDependencies.removeObject(mathObjects.InitialValue);
  mInitialDependencies.removeObject(mathObjects.InitialRate);
  mTransientDependencies.removeObject(mathObjects.Value);
  mTransientDependencies.removeObject(mathObjects.Rate);

  // Remove references to removed objects
  mathObjects.InitialValue = NULL;
  mathObjects.InitialRate = NULL;
  mathObjects.Value = NULL;
  mathObjects.Rate = NULL;

  // Resize
  resize(Size);

  // Create Update sequences
  createUpdateSequences();
}

CMathEvent * CMathContainer::addAnalysisEvent(const CEvent & dataEvent)
{
  sSize Size = mSize;
  sSize OldSize = mSize;

  CMathEvent Event;
  CMathEvent::allocate(Event, &dataEvent, *this);

  Size.nEvents++;
  Size.nEventRoots += Event.getTrigger().getRoots().size();
  Size.nEventAssignments += Event.getAssignments().size();

  resize(Size);

  // The new event is the last in the list
  CMathEvent * pEvent = mEvents.array() + OldSize.nEvents;
  CMathEvent::allocate(*pEvent, &dataEvent, *this);

  CMath::sPointers p;
  initializePointers(p);

  // Account for pre-existing objects
  p.pEventTriggers += OldSize.nEvents;
  p.pEventTriggersObject += OldSize.nEvents;
  p.pInitialEventTriggers += OldSize.nEvents;
  p.pInitialEventTriggersObject += OldSize.nEvents;
  p.pEventDelays += OldSize.nEvents;
  p.pEventDelaysObject += OldSize.nEvents;
  p.pEventPriorities += OldSize.nEvents;
  p.pEventPrioritiesObject += OldSize.nEvents;

  p.pEventRootStates += OldSize.nEventRoots;
  p.pEventRootStatesObject += OldSize.nEventRoots;
  p.pEventRoots += OldSize.nEventRoots;
  p.pEventRootsObject += OldSize.nEventRoots;

  p.pEventAssignments += OldSize.nAssignment;
  p.pEventAssignmentsObject += OldSize.nAssignment;

  pEvent->initialize(p);
  pEvent->compile(&dataEvent, *this);

  // Add the objects created for the event to the dependency graphs.
  initializePointers(p);

  mInitialDependencies.addObject(p.pInitialEventTriggersObject + OldSize.nEvents);
  mTransientDependencies.addObject(p.pEventTriggersObject + OldSize.nEvents);
  mTransientDependencies.addObject(p.pEventDelaysObject + OldSize.nEvents);
  mTransientDependencies.addObject(p.pEventPrioritiesObject + OldSize.nEvents);

  for (size_t i = OldSize.nEventRoots; i != mSize.nEventRoots; ++i)
    {
      mTransientDependencies.addObject(p.pEventRootStatesObject + i);
      mTransientDependencies.addObject(p.pEventRootsObject + i);
    }

  for (size_t i = OldSize.nAssignment; i != mSize.nEventRoots; ++i)
    {
      mTransientDependencies.addObject(p.pEventAssignmentsObject + i);
    }

  // Determine event targets and move objects accordingly.
  const CMathEvent::CAssignment * pAssignment = pEvent->getAssignments().array();
  const CMathEvent::CAssignment * pAssignmentEnd = pAssignment + pEvent->getAssignments().size();
  CMathObject * pFixedStart = getMathObject(mExtensiveValues.array());
  CMathObject * pFixedEnd = pFixedStart + mSize.nFixed;

  for (; pAssignment != pAssignmentEnd; ++pAssignment)
    {
      const CMathObject * pTarget = pAssignment->getTarget();

      if (pFixedStart <= pTarget && pTarget < pFixedEnd)
        {
          size_t CurrentIndex = (pTarget - pFixedStart);

          // Create a save haven
          CVector< C_FLOAT64 > EventTargetValues(4);
          CVector< CMathObject > EventTargetObjects(4);

          std::vector< CMath::sRelocate > Relocations;

          CMath::sRelocate RelocateTarget;
          RelocateTarget.pNewValue = EventTargetValues.array();
          RelocateTarget.pNewObject = EventTargetObjects.array();

          CMath::sRelocate RelocateOther;
          RelocateOther.pOldValue = mValues.array();
          RelocateOther.pNewValue = mValues.array();
          RelocateOther.pOldObject = mObjects.array();
          RelocateOther.pNewObject = mObjects.array();
          RelocateOther.offset = -1;

          // Initial value
          RelocateTarget.pValueStart = mInitialExtensiveValues.array() + CurrentIndex;
          RelocateTarget.pValueEnd = RelocateTarget.pValueStart + 1;
          RelocateTarget.pOldValue = RelocateTarget.pValueStart;
          RelocateTarget.pObjectStart = getMathObject(RelocateTarget.pValueStart);
          RelocateTarget.pObjectEnd = getMathObject(RelocateTarget.pValueEnd);
          RelocateTarget.pOldObject = getMathObject(RelocateTarget.pOldValue);
          RelocateTarget.offset = 0;
          Relocations.push_back(RelocateTarget);

          RelocateOther.pValueStart = RelocateTarget.pValueEnd;
          RelocateOther.pValueEnd = mInitialExtensiveValues.array() + mSize.nFixed  + mSize.nEventTargets;
          RelocateOther.pObjectStart = getMathObject(RelocateOther.pValueStart);
          RelocateOther.pObjectEnd = getMathObject(RelocateOther.pValueEnd);
          Relocations.push_back(RelocateOther);

          // Initial rate
          RelocateTarget.pValueStart = mInitialExtensiveRates.array() + CurrentIndex;
          RelocateTarget.pValueEnd = RelocateTarget.pValueStart + 1;
          RelocateTarget.pOldValue = RelocateTarget.pValueStart;
          RelocateTarget.pObjectStart = getMathObject(RelocateTarget.pValueStart);
          RelocateTarget.pObjectEnd = getMathObject(RelocateTarget.pValueEnd);
          RelocateTarget.pOldObject = getMathObject(RelocateTarget.pOldValue);
          RelocateTarget.offset++;
          Relocations.push_back(RelocateTarget);

          RelocateOther.pValueStart = RelocateTarget.pValueEnd;
          RelocateOther.pValueEnd = mInitialExtensiveRates.array() + mSize.nFixed  + mSize.nEventTargets;
          RelocateOther.pObjectStart = getMathObject(RelocateOther.pValueStart);
          RelocateOther.pObjectEnd = getMathObject(RelocateOther.pValueEnd);
          Relocations.push_back(RelocateOther);

          // Value
          RelocateTarget.pValueStart = mExtensiveValues.array() + CurrentIndex;
          RelocateTarget.pValueEnd = RelocateTarget.pValueStart + 1;
          RelocateTarget.pOldValue = RelocateTarget.pValueStart;
          RelocateTarget.pObjectStart = getMathObject(RelocateTarget.pValueStart);
          RelocateTarget.pObjectEnd = getMathObject(RelocateTarget.pValueEnd);
          RelocateTarget.pOldObject = getMathObject(RelocateTarget.pOldValue);
          RelocateTarget.offset++;
          Relocations.push_back(RelocateTarget);

          RelocateOther.pValueStart = RelocateTarget.pValueEnd;
          RelocateOther.pValueEnd = mExtensiveValues.array() + mSize.nFixed  + mSize.nEventTargets;
          RelocateOther.pObjectStart = getMathObject(RelocateOther.pValueStart);
          RelocateOther.pObjectEnd = getMathObject(RelocateOther.pValueEnd);
          Relocations.push_back(RelocateOther);

          // Rate
          RelocateTarget.pValueStart = mExtensiveRates.array() + CurrentIndex;
          RelocateTarget.pValueEnd = RelocateTarget.pValueStart + 1;
          RelocateTarget.pOldValue = RelocateTarget.pValueStart;
          RelocateTarget.pObjectStart = getMathObject(RelocateTarget.pValueStart);
          RelocateTarget.pObjectEnd = getMathObject(RelocateTarget.pValueEnd);
          RelocateTarget.pOldObject = getMathObject(RelocateTarget.pOldValue);
          RelocateTarget.offset++;
          Relocations.push_back(RelocateTarget);

          RelocateOther.pValueStart = RelocateTarget.pValueEnd;
          RelocateOther.pValueEnd = mExtensiveRates.array() + mSize.nFixed  + mSize.nEventTargets;
          RelocateOther.pObjectStart = getMathObject(RelocateOther.pValueStart);
          RelocateOther.pObjectEnd = getMathObject(RelocateOther.pValueEnd);
          Relocations.push_back(RelocateOther);

          Size = mSize;
          Size.nFixed--;
          Size.nEventTargets++;
          relocate(mValues, mObjects, Size, Relocations);

          // Move the event target out of the save haven.
          Relocations.clear();
          RelocateTarget.offset = 0;

          // Initial value
          RelocateTarget.pValueStart = EventTargetValues.array();
          RelocateTarget.pValueEnd = RelocateTarget.pValueStart + 1;
          RelocateTarget.pNewValue = mInitialExtensiveValues.array() + mSize.nFixed  + mSize.nEventTargets - 1;
          RelocateTarget.pOldValue = RelocateTarget.pValueStart;
          RelocateTarget.pObjectStart = EventTargetObjects.array();
          RelocateTarget.pObjectEnd = RelocateTarget.pObjectStart + 1;
          RelocateTarget.pNewObject = getMathObject(RelocateTarget.pNewValue);
          RelocateTarget.pOldObject = RelocateTarget.pObjectStart;
          Relocations.push_back(RelocateTarget);

          // Initial Rate
          RelocateTarget.pValueStart++;
          RelocateTarget.pValueEnd++;
          RelocateTarget.pNewValue = mInitialExtensiveRates.array() + mSize.nFixed  + mSize.nEventTargets - 1;
          RelocateTarget.pOldValue = RelocateTarget.pValueStart;
          RelocateTarget.pObjectStart++;
          RelocateTarget.pObjectEnd++;
          RelocateTarget.pNewObject = getMathObject(RelocateTarget.pNewValue);
          RelocateTarget.pOldObject = RelocateTarget.pObjectStart;
          Relocations.push_back(RelocateTarget);

          // Value
          RelocateTarget.pValueStart++;
          RelocateTarget.pValueEnd++;
          RelocateTarget.pNewValue = mExtensiveValues.array() + mSize.nFixed  + mSize.nEventTargets - 1;
          RelocateTarget.pOldValue = RelocateTarget.pValueStart;
          RelocateTarget.pObjectStart++;
          RelocateTarget.pObjectEnd++;
          RelocateTarget.pNewObject = getMathObject(RelocateTarget.pNewValue);
          RelocateTarget.pOldObject = RelocateTarget.pObjectStart;
          Relocations.push_back(RelocateTarget);

          // Rate
          RelocateTarget.pValueStart++;
          RelocateTarget.pValueEnd++;
          RelocateTarget.pNewValue = mExtensiveRates.array() + mSize.nFixed  + mSize.nEventTargets - 1;
          RelocateTarget.pOldValue = RelocateTarget.pValueStart;
          RelocateTarget.pObjectStart++;
          RelocateTarget.pObjectEnd++;
          RelocateTarget.pNewObject = getMathObject(RelocateTarget.pNewValue);
          RelocateTarget.pOldObject = RelocateTarget.pObjectStart;
          Relocations.push_back(RelocateTarget);

          Size = mSize;
          relocate(mValues, mObjects, Size, Relocations);

          pFixedEnd--;
        }
    }

  analyzeRoots();

  return pEvent;
}

bool CMathContainer::removeAnalysisEvent(CMathEvent *& pMathEvent)
{
  // We can only remove the last added event.
  CMathEvent * pEvent = &mEvents[mSize.nEvents - 1];

  if (pMathEvent != pEvent) return false;

  // Determine event targets and move objects accordingly.
  std::set< const CMathObject * > EventTargets;
  const CMathEvent::CAssignment * pAssignment = pEvent->getAssignments().array();
  const CMathEvent::CAssignment * pAssignmentEnd = pAssignment + pEvent->getAssignments().size();

  for (; pAssignment != pAssignmentEnd; ++pAssignment)
    {
      EventTargets.insert(pAssignment->getTarget());
    }

  sSize Size = mSize;

  Size.nEvents--;
  Size.nEventRoots -= pEvent->getTrigger().getRoots().size();
  Size.nEventAssignments -= pEvent->getAssignments().size();

  // Remove the objects created for the event from the dependency graphs.
  CMath::sPointers p;
  initializePointers(p);

  mInitialDependencies.removeObject(p.pInitialEventTriggersObject + Size.nEvents);
  mTransientDependencies.removeObject(p.pEventTriggersObject + Size.nEvents);
  mTransientDependencies.removeObject(p.pEventDelaysObject + Size.nEvents);
  mTransientDependencies.removeObject(p.pEventPrioritiesObject + Size.nEvents);

  for (size_t i = Size.nEventRoots; i != mSize.nEventRoots; ++i)
    {
      mTransientDependencies.removeObject(p.pEventRootStatesObject + i);
      mTransientDependencies.removeObject(p.pEventRootsObject + i);
    }

  for (size_t i = Size.nAssignment; i != mSize.nEventRoots; ++i)
    {
      mTransientDependencies.removeObject(p.pEventAssignmentsObject + i);
    }

  pEvent = NULL;
  pMathEvent = NULL;

  // Resize
  resize(Size);

  CMathObject * pEventTarget = getMathObject(mExtensiveValues.array()) + mSize.nFixed;
  CMathObject * pEventTargetEnd = pEventTarget + mSize.nEventTargets;

  // We have remove them in the reverse order than they were added;
  while (pEventTarget != pEventTargetEnd)
    {
      pEventTargetEnd--;

      // It is save to return since we only added to the end
      if (EventTargets.count(pEventTargetEnd) == 0) return true;

      // We have found a candidate we need to make sure that this is not a target of any other event
      // before we mark it as fixed.
      const CMathEvent * pEvent = mEvents.array();
      const CMathEvent * pEventEnd = pEvent + mEvents.size();

      for (; pEvent != pEventEnd; ++pEvent)
        {
          pAssignment = pEvent->getAssignments().array();
          pAssignmentEnd = pAssignment + pEvent->getAssignments().size();

          for (; pAssignment != pAssignmentEnd; ++pAssignment)
            {
              // It is save to return since we only added to the end
              if (EventTargets.count(pAssignment->getTarget()) == 0) return true;
            }
        }

      // We have found an event target which is now fixed.
      // Create a save haven
      CVector< C_FLOAT64 > EventTargetValues(mSize.nEventTargets * 4);
      CVector< CMathObject > EventTargetObjects(mSize.nEventTargets * 4);

      std::vector< CMath::sRelocate > Relocations;

      CMath::sRelocate RelocateTarget;
      RelocateTarget.pNewValue = EventTargetValues.array();
      RelocateTarget.pNewObject = EventTargetObjects.array();

      // Initial value
      RelocateTarget.pValueStart = mInitialExtensiveValues.array() + mSize.nFixed;
      RelocateTarget.pValueEnd = RelocateTarget.pValueStart + mSize.nEventTargets;
      RelocateTarget.pOldValue = RelocateTarget.pValueStart;
      RelocateTarget.pObjectStart = getMathObject(RelocateTarget.pValueStart);
      RelocateTarget.pObjectEnd = getMathObject(RelocateTarget.pValueEnd);
      RelocateTarget.pOldObject = getMathObject(RelocateTarget.pOldValue);
      RelocateTarget.offset = 0;
      Relocations.push_back(RelocateTarget);

      // Initial rate
      RelocateTarget.pValueStart = mInitialExtensiveRates.array() + mSize.nFixed;
      RelocateTarget.pValueEnd = RelocateTarget.pValueStart + mSize.nEventTargets;
      RelocateTarget.pOldValue = RelocateTarget.pValueStart;
      RelocateTarget.pObjectStart = getMathObject(RelocateTarget.pValueStart);
      RelocateTarget.pObjectEnd = getMathObject(RelocateTarget.pValueEnd);
      RelocateTarget.pOldObject = getMathObject(RelocateTarget.pOldValue);
      RelocateTarget.offset += mSize.nEventTargets;
      Relocations.push_back(RelocateTarget);

      // Value
      RelocateTarget.pValueStart = mExtensiveValues.array() + mSize.nFixed;
      RelocateTarget.pValueEnd = RelocateTarget.pValueStart + mSize.nEventTargets;
      RelocateTarget.pOldValue = RelocateTarget.pValueStart;
      RelocateTarget.pObjectStart = getMathObject(RelocateTarget.pValueStart);
      RelocateTarget.pObjectEnd = getMathObject(RelocateTarget.pValueEnd);
      RelocateTarget.pOldObject = getMathObject(RelocateTarget.pOldValue);
      RelocateTarget.offset += mSize.nEventTargets;
      Relocations.push_back(RelocateTarget);

      // Rate
      RelocateTarget.pValueStart = mExtensiveRates.array() + mSize.nFixed;
      RelocateTarget.pValueEnd = RelocateTarget.pValueStart + mSize.nEventTargets;
      RelocateTarget.pOldValue = RelocateTarget.pValueStart;
      RelocateTarget.pObjectStart = getMathObject(RelocateTarget.pValueStart);
      RelocateTarget.pObjectEnd = getMathObject(RelocateTarget.pValueEnd);
      RelocateTarget.pOldObject = getMathObject(RelocateTarget.pOldValue);
      RelocateTarget.offset += mSize.nEventTargets;
      Relocations.push_back(RelocateTarget);

      Size = mSize;
      relocate(mValues, mObjects, Size, Relocations);

      Relocations.clear();

      // Move the event target out of the save haven.
      Relocations.clear();
      RelocateTarget.offset = 0;

      CMath::sRelocate RelocateOther;
      RelocateOther.offset = 0;

      // Initial value
      RelocateTarget.pValueStart = EventTargetValues.array() + mSize.nEventTargets - 1;
      RelocateTarget.pValueEnd = RelocateTarget.pValueStart + 1;
      RelocateTarget.pOldValue = RelocateTarget.pValueStart;
      RelocateTarget.pNewValue = mInitialExtensiveValues.array() + mSize.nFixed;
      RelocateTarget.pObjectStart = EventTargetObjects.array() + mSize.nEventTargets - 1;
      RelocateTarget.pObjectEnd = RelocateTarget.pObjectStart + 1;
      RelocateTarget.pOldObject = RelocateTarget.pObjectStart;
      RelocateTarget.pNewObject = getMathObject(RelocateTarget.pNewValue);
      Relocations.push_back(RelocateTarget);

      RelocateOther.pValueStart = RelocateTarget.pValueEnd;
      RelocateOther.pValueEnd = RelocateOther.pValueStart + mSize.nEventTargets - 1;
      RelocateOther.pOldValue = RelocateOther.pValueStart;
      RelocateOther.pNewValue = RelocateTarget.pNewValue + 1;
      RelocateOther.pObjectStart = RelocateTarget.pObjectEnd;
      RelocateOther.pObjectEnd = RelocateOther.pObjectStart + mSize.nEventTargets - 1;
      RelocateOther.pOldObject = RelocateOther.pObjectStart;
      RelocateOther.pNewObject = getMathObject(RelocateOther.pNewValue);
      Relocations.push_back(RelocateOther);

      // Initial Rate
      RelocateTarget.pValueStart += mSize.nEventTargets;
      RelocateTarget.pValueEnd += mSize.nEventTargets;
      RelocateTarget.pOldValue += mSize.nEventTargets;
      RelocateTarget.pNewValue = mInitialExtensiveRates.array() + mSize.nFixed;
      RelocateTarget.pObjectStart += mSize.nEventTargets;
      RelocateTarget.pObjectEnd += mSize.nEventTargets;
      RelocateTarget.pOldObject += mSize.nEventTargets;
      RelocateTarget.pNewObject = getMathObject(RelocateTarget.pNewValue);
      Relocations.push_back(RelocateTarget);

      RelocateOther.pValueStart += mSize.nEventTargets;
      RelocateOther.pValueEnd += mSize.nEventTargets;
      RelocateOther.pOldValue += mSize.nEventTargets;
      RelocateOther.pNewValue = RelocateTarget.pNewValue + 1;
      RelocateOther.pObjectStart += mSize.nEventTargets;
      RelocateOther.pObjectEnd += mSize.nEventTargets;
      RelocateOther.pOldObject += mSize.nEventTargets;
      RelocateOther.pNewObject = getMathObject(RelocateOther.pNewValue);
      Relocations.push_back(RelocateOther);

      // Value
      RelocateTarget.pValueStart += mSize.nEventTargets;
      RelocateTarget.pValueEnd += mSize.nEventTargets;
      RelocateTarget.pOldValue += mSize.nEventTargets;
      RelocateTarget.pNewValue = mExtensiveValues.array() + mSize.nFixed;
      RelocateTarget.pObjectStart += mSize.nEventTargets;
      RelocateTarget.pObjectEnd += mSize.nEventTargets;
      RelocateTarget.pOldObject += mSize.nEventTargets;
      RelocateTarget.pNewObject = getMathObject(RelocateTarget.pNewValue);
      Relocations.push_back(RelocateTarget);

      RelocateOther.pValueStart += mSize.nEventTargets;
      RelocateOther.pValueEnd += mSize.nEventTargets;
      RelocateOther.pOldValue += mSize.nEventTargets;
      RelocateOther.pNewValue = RelocateTarget.pNewValue + 1;
      RelocateOther.pObjectStart += mSize.nEventTargets;
      RelocateOther.pObjectEnd += mSize.nEventTargets;
      RelocateOther.pOldObject += mSize.nEventTargets;
      RelocateOther.pNewObject = getMathObject(RelocateOther.pNewValue);
      Relocations.push_back(RelocateOther);

      // Rate
      RelocateTarget.pValueStart += mSize.nEventTargets;
      RelocateTarget.pValueEnd += mSize.nEventTargets;
      RelocateTarget.pOldValue += mSize.nEventTargets;
      RelocateTarget.pNewValue = mExtensiveRates.array() + mSize.nFixed;
      RelocateTarget.pObjectStart += mSize.nEventTargets;
      RelocateTarget.pObjectEnd += mSize.nEventTargets;
      RelocateTarget.pOldObject += mSize.nEventTargets;
      RelocateTarget.pNewObject = getMathObject(RelocateTarget.pNewValue);
      Relocations.push_back(RelocateTarget);

      RelocateOther.pValueStart += mSize.nEventTargets;
      RelocateOther.pValueEnd += mSize.nEventTargets;
      RelocateOther.pOldValue += mSize.nEventTargets;
      RelocateOther.pNewValue = RelocateTarget.pNewValue + 1;
      RelocateOther.pObjectStart += mSize.nEventTargets;
      RelocateOther.pObjectEnd += mSize.nEventTargets;
      RelocateOther.pOldObject += mSize.nEventTargets;
      RelocateOther.pNewObject = getMathObject(RelocateOther.pNewValue);
      Relocations.push_back(RelocateOther);

      Size = mSize;
      Size.nFixed++;
      Size.nEventTargets--;
      relocate(mValues, mObjects, Size, Relocations);
    }

  analyzeRoots();

  return true;
}

CRandom & CMathContainer::getRandomGenerator() const
{
  return * mpRandomGenerator;
}

void CMathContainer::createDiscontinuityEvents()
{
  CEvaluationNodeConstant VariableNode(CEvaluationNodeConstant::_NaN, "NAN");
  //size_t i, imax;

  // We need to create events for nodes which are capable of introducing
  // discontinuous changes.

  // Retrieve expression trees which contain discontinuities.
  std::vector< const CEvaluationTree * > TreesWithDiscontinuities =  mpModel->getTreesWithDiscontinuities();
  std::vector< const CEvaluationTree * >::const_iterator it = TreesWithDiscontinuities.begin();
  std::vector< const CEvaluationTree * >::const_iterator end = TreesWithDiscontinuities.end();

  for (; it != end; ++it)
    {
      createDiscontinuityEvents(*it);
    }
}

void CMathContainer::createDiscontinuityEvents(const CEvaluationTree * pTree)
{
  CEvaluationNodeConstant VariableNode(CEvaluationNodeConstant::_NaN, "NAN");
  CNodeIterator< const CEvaluationNode > itNode(pTree->getRoot());

  while (itNode.next() != itNode.end())
    {
      if (*itNode == NULL)
        {
          continue;
        }

      switch ((int) itNode->getType())
        {
          case (CEvaluationNode::CHOICE | CEvaluationNodeChoice::IF):
          case (CEvaluationNode::FUNCTION | CEvaluationNodeFunction::FLOOR):
          case (CEvaluationNode::FUNCTION | CEvaluationNodeFunction::CEIL):
          case (CEvaluationNode::OPERATOR | CEvaluationNodeOperator::MODULUS):
            createDiscontinuityDataEvent(*itNode);
            break;

            // Call nodes may include discontinuities but each called tree is handled
            // separately.
          case (CEvaluationNode::CALL | CEvaluationNodeCall::FUNCTION):
          case (CEvaluationNode::CALL | CEvaluationNodeCall::EXPRESSION):
            createDiscontinuityEvents(static_cast< const CEvaluationNodeCall * >(*itNode)->getCalledTree());
            break;

          default:
            break;
        }
    }

  return;
}

void CMathContainer::createDiscontinuityDataEvent(const CEvaluationNode * pNode)
{
  // We can create a data event without knowing the variables as the number
  // of roots is independent from the variable value.
  CEvent * pEvent = new CEvent();
  pEvent->setType(CEvent::Discontinuity);
  mDiscontinuityEvents.add(pEvent, true);

  pEvent->setTriggerExpression(createDiscontinuityTriggerInfix(pNode));
}

std::string CMathContainer::createDiscontinuityTriggerInfix(const CEvaluationNode * pNode)
{
  std::string TriggerInfix;

  // We need to define a data event for each discontinuity.
  switch ((int) pNode->getType())
    {
      case (CEvaluationNode::CHOICE | CEvaluationNodeChoice::IF):
        TriggerInfix = static_cast< const CEvaluationNode * >(pNode->getChild())->buildInfix();
        break;

      case (CEvaluationNode::FUNCTION | CEvaluationNodeFunction::FLOOR):
      case (CEvaluationNode::FUNCTION | CEvaluationNodeFunction::CEIL):
        TriggerInfix = "sin(PI*(" + static_cast< const CEvaluationNode * >(pNode->getChild())->buildInfix() + ")) > 0";
        break;

      case (CEvaluationNode::OPERATOR | CEvaluationNodeOperator::MODULUS):
        TriggerInfix = "sin(PI*(" + static_cast< const CEvaluationNode * >(pNode->getChild())->buildInfix();
        TriggerInfix += ")) > 0 || sin(PI*(" + static_cast< const CEvaluationNode * >(pNode->getChild()->getSibling())->buildInfix() + ")) > 0";
        break;

      default:
        fatalError();
        break;
    }

  return TriggerInfix;
}

void CMathContainer::createDelays()
{
  // We check all transient objects for the occurrences of delays
  CMathObject * pObject = getMathObject(mExtensiveValues.array());
  CMathObject * pObjectEnd = getMathObject(mDelayValues.array());

  // The above returns NULL if no delays are present.
  if (pObjectEnd == NULL)
    {
      pObjectEnd = const_cast< CMathObject * >(mObjects.array() + mObjects.size());
    }

  CMath::DelayData DelayData;

  for (; pObject != pObjectEnd; ++pObject)
    {
      pObject->appendDelays(DelayData);
    }

  if (DelayData.size() == 0)
    {
      mHistory.resize(0, 0, 0);

      return;
    }

  sSize Size = mSize;
  Size.nDelayLags = 0;
  Size.nDelayValues = 0;

  CMath::DelayData::iterator itDelayLag = DelayData.begin();
  CMath::DelayData::iterator endDelayLag = DelayData.end();
  std::string LagKey = "";
  std::vector< size_t > LagValueCounts;
  std::vector< size_t >::reverse_iterator itCurrentLagValueCount;

  for (; itDelayLag != endDelayLag; ++itDelayLag)
    {
      if (itDelayLag->first != LagKey)
        {
          LagKey = itDelayLag->first;
          Size.nDelayLags++;
          LagValueCounts.push_back((size_t) 0);
          itCurrentLagValueCount = LagValueCounts.rbegin();
        }

      CMath::DelayValueData::iterator itDelayValue = itDelayLag->second.begin();
      CMath::DelayValueData::iterator endDelayValue = itDelayLag->second.end();
      std::string ValueKey = "";

      for (; itDelayValue != endDelayValue; ++itDelayValue)
        {
          if (itDelayValue->first != ValueKey)
            {
              ValueKey = itDelayValue->first;
              Size.nDelayValues++;
              (*itCurrentLagValueCount)++;
            }
        }
    }

  std::vector< CMath::sRelocate > Relocations = resize(Size);

  // Update the mappings of the delays
  for (itDelayLag = DelayData.begin(); itDelayLag != endDelayLag; ++itDelayLag)
    {
      CMath::DelayValueData::iterator itDelayValue = itDelayLag->second.begin();
      CMath::DelayValueData::iterator endDelayValue = itDelayLag->second.end();

      for (; itDelayValue != endDelayValue; ++itDelayValue)
        {
          CMathContainer::relocateObject(itDelayValue->second.second, Relocations);
        }
    }

  pObject = getMathObject(mDelayValues.array());
  pObjectEnd = pObject + mDelayValues.size();
  C_FLOAT64 * pValue = mDelayValues.array();

  while (pObject != pObjectEnd)
    {
      CMathObject::initialize(pObject, pValue, CMath::DelayValue, CMath::Delay,
                              CMath::SimulationTypeUndefined, false, false, NULL);
    }

  pObjectEnd += mSize.nDelayLags;

  while (pObject != pObjectEnd)
    {
      CMathObject::initialize(pObject, pValue, CMath::DelayLag, CMath::Delay,
                              CMath::SimulationTypeUndefined, false, false, NULL);
    }

  CMathDelay * pDelay = mDelays.array();
  CMathObject * pDelayValueObject = getMathObject(mDelayValues.array());
  CMathObject * pDelayLagObject = getMathObject(mDelayLags.array());
  std::vector< size_t >::const_iterator itLagValueCount = LagValueCounts.begin();

  itDelayLag = DelayData.begin();
  LagKey = "";

  for (; itDelayLag != endDelayLag; ++itDelayLag)
    {
      // If we have a new key we create a new delay
      if (itDelayLag->first != LagKey)
        {
          // Advance the pointers except for the first time
          if (LagKey != "")
            {
              ++pDelay;
              ++pDelayLagObject;
              ++itLagValueCount;
              ++pDelayValueObject;
            }

          LagKey = itDelayLag->first;
          pDelay->create(itDelayLag, *itLagValueCount, *this, pDelayLagObject);
        }

      CMath::DelayValueData::iterator itDelayValue = itDelayLag->second.begin();
      CMath::DelayValueData::iterator endDelayValue = itDelayLag->second.end();
      std::string ValueKey = "";
      size_t index = 0;

      for (; itDelayValue != endDelayValue; ++itDelayValue)
        {
          if (itDelayValue->first != ValueKey)
            {
              ValueKey = itDelayValue->first;
              ++pDelayValueObject;
              ++index;

              pDelay->addValueObject(itDelayValue, index, pDelayValueObject);
            }

          // Update the expression in which the delay occurred.
          pDelay->modifyMathObject(itDelayValue, index);
        }
    }
}

void CMathContainer::createRelocations(const CMathContainer::sSize & size, std::vector< CMath::sRelocate > & relocations)
{
  CMath::sRelocate Relocate;
  Relocate.pValueStart = mSize.pValue;
  Relocate.pValueEnd = mSize.pValue;
  Relocate.pOldValue = mSize.pValue;
  Relocate.pNewValue = size.pValue;

  Relocate.pObjectStart = mSize.pObject;
  Relocate.pObjectEnd = mSize.pObject;
  Relocate.pOldObject = mSize.pObject;
  Relocate.pNewObject = size.pObject;

  Relocate.offset = 0;

  // Initial Values
  createRelocation(size.nFixed, mSize.nFixed, Relocate, relocations);
  createRelocation(size.nEventTargets, mSize.nEventTargets, Relocate, relocations);
  createRelocation(1, 1, Relocate, relocations);
  createRelocation(size.nODE, mSize.nODE, Relocate, relocations, false);
  createRelocation(size.nReactionSpecies, mSize.nReactionSpecies, Relocate, relocations);
  createRelocation(size.nAssignment, mSize.nAssignment, Relocate, relocations);
  createRelocation(size.nIntensiveValues, mSize.nIntensiveValues, Relocate, relocations);

  // Initial Rates
  createRelocation(size.nFixed, mSize.nFixed, Relocate, relocations);
  createRelocation(size.nEventTargets, mSize.nEventTargets, Relocate, relocations);
  createRelocation(1, 1, Relocate, relocations);
  createRelocation(size.nODE, mSize.nODE, Relocate, relocations, false);
  createRelocation(size.nReactionSpecies, mSize.nReactionSpecies, Relocate, relocations);
  createRelocation(size.nAssignment, mSize.nAssignment, Relocate, relocations);
  createRelocation(size.nIntensiveValues, mSize.nIntensiveValues, Relocate, relocations);

  // Internal Values
  createRelocation(size.nReactions, mSize.nReactions, Relocate, relocations);
  createRelocation(size.nReactions, mSize.nReactions, Relocate, relocations);
  createRelocation(size.nMoieties, mSize.nMoieties, Relocate, relocations);
  createRelocation(size.nEvents, mSize.nEvents, Relocate, relocations);

  // Transient Values
  createRelocation(size.nFixed, mSize.nFixed, Relocate, relocations);
  createRelocation(size.nEventTargets, mSize.nEventTargets, Relocate, relocations);
  createRelocation(1, 1, Relocate, relocations);
  createRelocation(size.nODE, mSize.nODE, Relocate, relocations, false);
  createRelocation(size.nReactionSpecies, mSize.nReactionSpecies, Relocate, relocations);
  createRelocation(size.nAssignment, mSize.nAssignment, Relocate, relocations);
  createRelocation(size.nIntensiveValues, mSize.nIntensiveValues, Relocate, relocations);

  // Transient Rates
  createRelocation(size.nFixed, mSize.nFixed, Relocate, relocations);
  createRelocation(size.nEventTargets, mSize.nEventTargets, Relocate, relocations);
  createRelocation(1, 1, Relocate, relocations);
  createRelocation(size.nODE, mSize.nODE, Relocate, relocations, false);
  createRelocation(size.nReactionSpecies, mSize.nReactionSpecies, Relocate, relocations);
  createRelocation(size.nAssignment, mSize.nAssignment, Relocate, relocations);
  createRelocation(size.nIntensiveValues, mSize.nIntensiveValues, Relocate, relocations);

  // Internal Values
  createRelocation(size.nReactions, mSize.nReactions, Relocate, relocations);
  createRelocation(size.nReactions, mSize.nReactions, Relocate, relocations);
  createRelocation(size.nMoieties, mSize.nMoieties, Relocate, relocations);
  createRelocation(size.nEvents, mSize.nEvents, Relocate, relocations);

  createRelocation(size.nEvents, mSize.nEvents, Relocate, relocations);
  createRelocation(size.nEvents, mSize.nEvents, Relocate, relocations);
  createRelocation(size.nEventAssignments, mSize.nEventAssignments, Relocate, relocations);
  createRelocation(size.nEventRoots, mSize.nEventRoots, Relocate, relocations);
  createRelocation(size.nEventRoots, mSize.nEventRoots, Relocate, relocations);
  createRelocation(size.nReactions, mSize.nReactions, Relocate, relocations);
  createRelocation(size.nMoieties, mSize.nMoieties, Relocate, relocations);
  createRelocation(size.nDiscontinuities, mSize.nDiscontinuities, Relocate, relocations);
  createRelocation(size.nDelayValues, mSize.nDelayValues, Relocate, relocations);
  createRelocation(size.nDelayLags, mSize.nDelayLags, Relocate, relocations);
  createRelocation(size.nIntensiveValues, mSize.nIntensiveValues, Relocate, relocations);

  if (Relocate.pValueStart != Relocate.pValueEnd)
    {
      relocations.push_back(Relocate);
    }
}

std::vector< CMath::sRelocate > CMathContainer::resize(CMathContainer::sSize & size)
{
  // Determine the offsets
  // We have to cast all pointers to size_t to avoid pointer overflow.
  size_t nExtensiveValues = size.nFixed + size.nEventTargets + 1 + size.nODE + size.nReactionSpecies + size.nAssignment;

  size_t ObjectCount = 4 * (nExtensiveValues + size.nIntensiveValues) +
                       5 * size.nReactions +
                       3 * size.nMoieties +
                       size.nDiscontinuities +
                       4 * size.nEvents + size.nEventAssignments + 2 * size.nEventRoots +
                       size.nDelayLags + size.nDelayValues +
                       size.nIntensiveValues;

  // We need to preserve the old values and objects until they are properly relocated;
  CVectorCore< C_FLOAT64 > OldValues;
  OldValues.initialize(mValues);
  mpValuesBuffer = (ObjectCount > 0) ? new C_FLOAT64[ObjectCount] : NULL;
  mValues.initialize(ObjectCount, mpValuesBuffer);
  size.pValue = mValues.array();
  mValues = std::numeric_limits< C_FLOAT64 >::quiet_NaN();

  CVectorCore< CMathObject > OldObjects;
  OldObjects.initialize(mObjects);
  mpObjectsBuffer = (ObjectCount > 0) ? new CMathObject[ObjectCount] : NULL;
  mObjects.initialize(ObjectCount, mpObjectsBuffer);
  size.pObject = mObjects.array();

  // Now we create the move information.
  std::vector< CMath::sRelocate > Relocations;
  createRelocations(size, Relocations);

  relocate(OldValues, OldObjects, size, Relocations);

  // Delete the old objects
  if (OldValues.array() != NULL) delete [] OldValues.array();

  if (OldObjects.array() != NULL) delete [] OldObjects.array();

  return Relocations;
}

void CMathContainer::relocate(CVectorCore< C_FLOAT64 > &oldValues,
                              CVectorCore< CMathObject > &oldObjects,
                              const sSize & size,
                              const std::vector< CMath::sRelocate > & Relocations)
{
  size_t nExtensiveValues = size.nFixed + size.nEventTargets + 1 + size.nODE + size.nReactionSpecies + size.nAssignment;

  // Move the objects to the new location
  C_FLOAT64 * pValue = oldValues.array();
  C_FLOAT64 * pValueEnd = pValue + oldValues.size();
  CMathObject * pObject = oldObjects.array();

  for (; pValue != pValueEnd; ++pValue, ++pObject)
    {
      // If we are here we are actually resizing
      C_FLOAT64 * pNewValue = pValue;
      relocateValue(pNewValue, Relocations);

      if (pValue != pNewValue)
        {
          *pNewValue = *pValue;
        }
      else
        {
          *pNewValue = *pValue;
        }

      CMathObject * pNewObject = pObject;
      relocateObject(pNewObject, Relocations);

      if (pObject != pNewObject)
        {
          *pNewObject = *pObject;
          pNewObject->relocate(Relocations);
          pObject->moved();
        }
      else
        {
          *pNewObject = *pObject;
        }
    }

  C_FLOAT64 * pArray = size.pValue;
  mInitialExtensiveValues.initialize(nExtensiveValues, pArray);
  pArray += nExtensiveValues;
  mInitialIntensiveValues.initialize(size.nIntensiveValues, pArray);
  pArray += size.nIntensiveValues;
  mInitialExtensiveRates.initialize(nExtensiveValues, pArray);
  pArray += nExtensiveValues;
  mInitialIntensiveRates.initialize(size.nIntensiveValues, pArray);
  pArray += size.nIntensiveValues;
  mInitialParticleFluxes.initialize(size.nReactions, pArray);
  pArray += size.nReactions;
  mInitialFluxes.initialize(size.nReactions, pArray);
  pArray += size.nReactions;
  mInitialTotalMasses.initialize(size.nMoieties, pArray);
  pArray += size.nMoieties;
  mInitialEventTriggers.initialize(size.nEvents, pArray);
  pArray += size.nEvents;

  mExtensiveValues.initialize(nExtensiveValues, pArray);
  pArray += nExtensiveValues;
  mIntensiveValues.initialize(size.nIntensiveValues, pArray);
  pArray += size.nIntensiveValues;
  mExtensiveRates.initialize(nExtensiveValues, pArray);
  pArray += nExtensiveValues;
  mIntensiveRates.initialize(size.nIntensiveValues, pArray);
  pArray += size.nIntensiveValues;
  mParticleFluxes.initialize(size.nReactions, pArray);
  pArray += size.nReactions;
  mFluxes.initialize(size.nReactions, pArray);
  pArray += size.nReactions;
  mTotalMasses.initialize(size.nMoieties, pArray);
  pArray += size.nMoieties;
  mEventTriggers.initialize(size.nEvents, pArray);
  pArray += size.nEvents;

  mEventDelays.initialize(size.nEvents, pArray);
  pArray += size.nEvents;
  mEventPriorities.initialize(size.nEvents, pArray);
  pArray += size.nEvents;
  mEventAssignments.initialize(size.nEventAssignments, pArray);
  pArray += size.nEventAssignments;
  mEventRoots.initialize(size.nEventRoots, pArray);
  pArray += size.nEventRoots;
  mEventRootStates.initialize(size.nEventRoots, pArray);
  pArray += size.nEventRoots;
  mPropensities.initialize(size.nReactions, pArray);
  pArray += size.nReactions;
  mDependentMasses.initialize(size.nMoieties, pArray);
  pArray += size.nMoieties;
  mDiscontinuous.initialize(size.nDiscontinuities, pArray);
  pArray += size.nDiscontinuities;
  mDelayValues.initialize(size.nDelayValues, pArray);
  pArray += size.nDelayValues;
  mDelayLags.initialize(size.nDelayLags, pArray);
  pArray += size.nDelayLags;
  mTransitionTimes.initialize(size.nIntensiveValues, pArray);
  pArray += size.nIntensiveValues;

  assert(pArray == mValues.array() + mValues.size());

  mInitialState.initialize(nExtensiveValues + size.nIntensiveValues,  mValues.array());
  mState.initialize(size.nEventTargets + 1 + size.nODE + size.nReactionSpecies, mExtensiveValues.array() + size.nFixed);
  mStateReduced.initialize(mState.size() - size.nMoieties, mExtensiveValues.array() + size.nFixed);
  mRate.initialize(mState.size(), mExtensiveRates.array() + size.nFixed);
  mRateReduced.initialize(mStateReduced.size(), mExtensiveRates.array() + size.nFixed);

  mHistory.resize(mDelayLags.size(), mState.size(), mState.size());
  mHistoryReduced.initialize(mDelayLags.size(), mStateReduced.size(), mState.size(), mHistory.array());

  mRootProcessors.resize(size.nEventRoots, true);
  mRootIsDiscrete.resize(size.nEventRoots, true);
  mRootIsTimeDependent.resize(size.nEventRoots, true);

  relocateUpdateSequence(mSynchronizeInitialValuesSequenceExtensive, Relocations);
  relocateUpdateSequence(mSynchronizeInitialValuesSequenceIntensive, Relocations);
  relocateUpdateSequence(mApplyInitialValuesSequence, Relocations);
  relocateUpdateSequence(mSimulationValuesSequence, Relocations);
  relocateUpdateSequence(mSimulationValuesSequenceReduced, Relocations);
  relocateUpdateSequence(mPrioritySequence, Relocations);
  relocateUpdateSequence(mTransientDataObjectSequence, Relocations);

  relocateObjectSet(mInitialStateValueExtensive, Relocations);
  relocateObjectSet(mInitialStateValueIntensive, Relocations);
  relocateObjectSet(mInitialStateValueAll, Relocations);
  relocateObjectSet(mStateValues, Relocations);
  relocateObjectSet(mReducedStateValues, Relocations);
  relocateObjectSet(mSimulationRequiredValues, Relocations);

  std::map< CCopasiObject *, CMathObject * >::iterator itDataObject2MathObject = mDataObject2MathObject.begin();
  std::map< CCopasiObject *, CMathObject * >::iterator endDataObject2MathObject = mDataObject2MathObject.end();

  for (; itDataObject2MathObject != endDataObject2MathObject; ++itDataObject2MathObject)
    {
      pObject = itDataObject2MathObject->second;
      relocateObject(pObject, Relocations);
      itDataObject2MathObject->second = pObject;
    }

  std::map< C_FLOAT64 *, CMathObject * >::iterator itDataValue2MathObject = mDataValue2MathObject.begin();
  std::map< C_FLOAT64 *, CMathObject * >::iterator endDataValue2MathObject = mDataValue2MathObject.end();

  for (; itDataValue2MathObject != endDataValue2MathObject; ++itDataValue2MathObject)
    {
      pObject = itDataValue2MathObject->second;
      relocateObject(pObject, Relocations);
      itDataValue2MathObject->second = pObject;
    }

  mInitialDependencies.relocate(Relocations);
  mTransientDependencies.relocate(Relocations);

  // Relocate the Events
  relocateVector(mEvents, mpEventsBuffer, size.nEvents, Relocations);

  // Relocate the Reactions
  relocateVector(mReactions, mpReactionsBuffer, size.nReactions, Relocations);

  // Relocate Delays
  relocateVector(mDelays, mpDelaysBuffer, size.nDelayLags, Relocations);

  mSize = size;
}
