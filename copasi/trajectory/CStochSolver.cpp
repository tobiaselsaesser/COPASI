#include <math.h>

#include "copasi.h"
#include "CStochSolver.h"
#include "CTrajectory.h"
#include "model/model.h"

CStochSolver::CStochSolver()
    : mMethodType(CTrajectory::STOCH_DIRECT),
      mMethod(0)
{
}

CStochSolver::CStochSolver(C_INT32 method_type)
    : mMethodType(method_type),
      mMethod(0)
{}

void CStochSolver::initialize(CModel *model)
{
    if (mMethodType == CTrajectory::STOCH_NEXTREACTION)
    {
      //        mMethod = new CStochNextReactionMethod(model);
    }
    else
    {
        mMethod = new CStochDirectMethod(model);
    }
    mMethod->initMethod();
}

CStochSolver::~CStochSolver() {cleanup();}

void CStochSolver::cleanup()
{
    if (mMethod)
    {
        mMethod->cleanup();
        delete mMethod;
    }
    mMethod = NULL;
}

CStochMethod *CStochSolver::getStochMethod()
{
    return mMethod;
}

CStochMethod::CStochMethod()
    : mModel(NULL),
      mFail(0)
{
    mRandomGenerator = new CRandom();
}


CStochMethod::CStochMethod(CModel *model)
    : mModel(model),
      mFail(0)
{
    mRandomGenerator = new CRandom();
}

CStochMethod::~CStochMethod() {cleanup();}
 
void CStochMethod::cleanup()
{
    delete mRandomGenerator;
    mRandomGenerator = 0;
}

C_INT32 CStochMethod::updatePropensities()
{
    mA0 = 0;
    for (unsigned C_INT32 i = 0; i < mModel->getReactions().size(); i++)
    {
        calculateAmu(i);
        mA0 += mAmu[i];
    }
    return 0;
}

C_INT32 CStochMethod::calculateAmu(C_INT32 index)
{
    // We need the product of the cmu and hmu for this step.
    // We calculate this in one go, as there are fewer steps to 
    // perform and we eliminate some possible rounding errors.
    C_FLOAT32 amu = 1; // initially
    C_INT32 total_substrates = 0;
    C_INT32 num_ident = 0;
    C_INT32 number = 0;
    C_INT32 lower_bound;
    // First, find the reaction associated with this index.
    // Keep a pointer to this.
    CChemEq *chemeq = &mModel->getReactions()[index]->getChemEq();
    // Iterate through each substrate in the reaction 
    CCopasiVector<CChemEqElement> &substrates = chemeq->getSubstrates();
    for (unsigned C_INT32 i = 0; i < substrates.size(); i++)
    {
      // :TODO: getMultiplicity is not necessarily integer!
        num_ident = substrates[i]->getMultiplicity();
        total_substrates += num_ident;
      // :TODO: getNumber is not necessarily integer!
        number = substrates[i]->getMetabolite().getNumber();
        lower_bound = number - num_ident;
        while (number > lower_bound)
        {
            amu *= number;
            number--;
        }
    }
    // We assume that all substrates are in the same compartment.
    // If there are no substrates, then volume is irrelevant. Otherwise,
    // we can use the volume of the compartment for the first substrate.
    if (substrates.size() > 0)
    {
        C_FLOAT64 volume = 
          substrates[0]->getMetabolite().getCompartment()->getVolume();
        amu /= pow(volume, total_substrates-1);
    }
    C_FLOAT64 rate;
    // amu *= rate_with_factor; // XXX :TODO: rate (including factor from dynamics)
    mAmu[index] = amu;

    // :TODO: Is this correct Carel?
    return index;
}
    
CStochDirectMethod::CStochDirectMethod()
    : CStochMethod()
{
}

CStochDirectMethod::CStochDirectMethod(CModel *model)
    : CStochMethod(model)
{
}

CStochDirectMethod::~CStochDirectMethod() {cleanup();}

C_INT32 CStochDirectMethod::initMethod()
{
    // Populate the vector of propensities
    for (unsigned C_INT32 i = 0; i < mModel->getReactions().size(); i++)
    {
        mAmu.push_back(0);
    }
    return 0;
}

C_FLOAT64 CStochDirectMethod::doStep(C_FLOAT64 initial_time)
{
    updatePropensities();
    C_INT32 rxn = getReaction();
    C_FLOAT64 step_time = getTime();
    updateSystemState(rxn);
    return initial_time + step_time;
}

C_INT32 CStochDirectMethod::getReaction()
{
    C_FLOAT64 rand1 = mRandomGenerator->getUniformRandom();
    C_FLOAT64 sum;
    unsigned C_INT32 index = 0;
    while (index < mModel->getReactions().size())
    {
        sum += mAmu[index] / mA0;
        if (rand1 <= sum)
        {
            return index;
        }
    }
    index++;
    // shouldn't get here
    return mFail;
}

C_FLOAT64 CStochDirectMethod::getTime()
{
    C_FLOAT32 rand2 = mRandomGenerator->getUniformRandom();
    return -1 * log(rand2 / mA0);
}

C_INT32 CStochDirectMethod::updateSystemState(C_INT32 rxn)
{
    // Change the particle numbers according to which step took place.
    // First, get the vector of balances in the reaction we've got.
    // (This vector expresses the number change of each metabolite 
    // in the reaction.) Then step through each balance, using its 
    // multiplicity to calculate a new value for the associated 
    // metabolite. Finally, update the metabolite.
    CCopasiVector <CChemEqElement> &balances = 
        mModel->getReactions()[rxn]->getChemEq().getBalances();
    CChemEqElement *bal = 0;
    C_FLOAT32 new_num;
    for (unsigned C_INT32 i = 0; i < balances.size(); i++)
    {
        bal = balances[i];
        new_num = bal->getMetabolite().getNumber() + bal->getMultiplicity();
        bal->getMetabolite().setNumber(new_num);
    }
    return 0;
}
