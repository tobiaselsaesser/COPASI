// CStep 
// 
// Derived from Gepasi's cstep.cpp
// (C) Pedro Mendes 1995-2000
//
// Converted for Copasi by Stefan Hoops


#include "copasi.h"
#include "CGlobals.h"
#include "CStep.h"
#include "CCompartment.h"
#include "utilities.h"

CStep::CStep()
{
    mFlux = 0.0;
    mReversible = TRUE;
    mFunction = NULL;

    mSubstrates = NULL;
    mProducts = NULL;
    mModifiers = NULL;
    mParameters = NULL;
    mCallParameters = NULL;
}

CStep::CStep(const string & name)
{
    mName = name;
    mFlux = 0.0;
    mReversible = TRUE;
    mFunction = NULL;

    mSubstrates = NULL;
    mProducts = NULL;
    mModifiers = NULL;
    mParameters = NULL;
    mCallParameters = NULL;
}

void CStep::initialize()
{
    if (!mSubstrates) mSubstrates = new vector < CId2Metab >;
    if (!mProducts) mProducts = new vector < CId2Metab >;
    if (!mModifiers) mModifiers = new vector < CId2Metab >;
    if (!mParameters) mParameters = new vector < CId2Param >;
    if (!mCallParameters) mCallParameters = new vector < CCallParameter >;
}

CStep::~CStep() {}

void CStep::cleanup()
{
    if (mSubstrates) delete mSubstrates;
    mSubstrates = NULL;
    if (mProducts) delete mProducts;
    mProducts = NULL;
    if (mModifiers) delete mModifiers;
    mModifiers = NULL;
    if (mParameters) delete mParameters;
    mParameters = NULL;
    if (mCallParameters) delete mCallParameters;
    mCallParameters = NULL;
}

CStep & CStep::operator=(const CStep & rhs)
{
    mName        = rhs.mName;
    mChemEq      = rhs.mChemEq;
    mFunction    = rhs.mFunction;
    mFlux        = rhs.mFlux;
    mReversible  = rhs.mReversible;
    mSubstrates  = rhs.mSubstrates;
    mProducts    = rhs.mProducts;
    mModifiers   = rhs.mModifiers;
    mParameters  = rhs.mParameters;
    mCallParameters = rhs.mCallParameters;

    return *this;
}

C_INT32 CStep::load(CReadConfig & configbuffer)
{
    C_INT32 Fail = 0;
    C_INT32 Size = 0;
    C_INT32 i = 0;
    
    string KinType;
    
    initialize();
    
    if (Fail = configbuffer.getVariable("Step", "string", &mName,
                                        CReadConfig::SEARCH))
        return Fail;
    
    if (Fail = configbuffer.getVariable("Equation", "string", &mChemEq))
        return Fail;
    
    if (Fail = configbuffer.getVariable("KineticType", "string", &KinType))
        return Fail;

    setFunction(KinType);
    if (mFunction == NULL) return Fail = 1;

    initIdentifiers();
    
    if (Fail = configbuffer.getVariable("Flux", "C_FLOAT64", &mFlux))
        return Fail;
    
    if (Fail = configbuffer.getVariable("Reversible", "C_INT32", &mReversible))
        return Fail;
    
    if (configbuffer.getVersion() < "4")
        Fail = loadOld(configbuffer);
    else 
        Fail = loadNew(configbuffer);
        
    return Fail; 
}

C_INT32 CStep::save(CWriteConfig & configbuffer)
{
    C_INT32 Fail = 0;
    C_INT32 Size = 0;
    C_INT32 i = 0;
    
    if (Fail = configbuffer.setVariable("Step", "string", &mName))
        return Fail;
    
    if (Fail = configbuffer.setVariable("Equation", "string", &mChemEq))
        return Fail;
    
    string KinType = mFunction->getName();
    if (Fail = configbuffer.setVariable("KineticType", "string", &KinType))
        return Fail;

    if (Fail = configbuffer.setVariable("Flux", "C_FLOAT64", &mFlux))
        return Fail;
    
    if (Fail = configbuffer.setVariable("Reversible", "C_INT32", &mReversible))
        return Fail;
    
    Size = mSubstrates->size();
    if (Fail = configbuffer.setVariable("Substrates", "C_INT32", &Size))
        return Fail;
    for (i = 0; i < Size; i++)
    {
        if (Fail = configbuffer.setVariable("Identifier", "string",
                                            &(*mSubstrates)[i].mIdentifierName))
            return Fail;
        if (Fail = configbuffer.setVariable("Compartment", "string",
                                            &(*mSubstrates)[i].mCompartmentName))
            return Fail;
        if (Fail = configbuffer.setVariable("Metabolite", "string",
                                            &(*mSubstrates)[i].mMetaboliteName))
            return Fail;
    }
    
    Size = mProducts->size();
    if (Fail = configbuffer.setVariable("Products", "C_INT32", &Size))
        return Fail;
    for (i = 0; i < Size; i++)
    {
        if (Fail = configbuffer.setVariable("Identifier", "string",
                                            &(*mProducts)[i].mIdentifierName))
            return Fail;
        if (Fail = configbuffer.setVariable("Compartment", "string",
                                            &(*mProducts)[i].mCompartmentName))
            return Fail;
        if (Fail = configbuffer.setVariable("Metabolite", "string",
                                            &(*mProducts)[i].mMetaboliteName))
            return Fail;
    }

    Size = mModifiers->size();
    if (Fail = configbuffer.setVariable("Modifiers", "C_INT32", &Size))
        return Fail;
    for (i = 0; i < Size; i++)
    {
        if (Fail = configbuffer.setVariable("Identifier", "string",
                                            &(*mModifiers)[i].mIdentifierName))
            return Fail;
        if (Fail = configbuffer.setVariable("Compartment", "string",
                                            &(*mModifiers)[i].mCompartmentName))
            return Fail;
        if (Fail = configbuffer.setVariable("Metabolite", "string",
                                            &(*mModifiers)[i].mMetaboliteName))
            return Fail;
    }

    Size = mParameters->size();
    if (Fail = configbuffer.setVariable("Constants", "C_INT32", &Size))
        return Fail;
    for (i = 0; i < Size; i++)
    {
        if (Fail = configbuffer.setVariable("Identifier", "string",
                                            &(*mParameters)[i].mIdentifierName))
            return Fail;
        if (Fail = configbuffer.setVariable("Value", "C_FLOAT64",
                                            &(*mParameters)[i].mValue))
            return Fail;
    }
    
    return Fail; 
}

vector < CStep::CId2Metab > &CStep::substrates() {return *mSubstrates;}

vector < CStep::CId2Metab > &CStep::products() {return *mProducts;}

vector < CStep::CId2Metab > &CStep::modifiers() {return *mModifiers;}

vector < CStep::CId2Param > &CStep::parameters() {return *mParameters;}

string CStep::getName() const {return mName;}

string CStep::getChemEq() const {return mChemEq;}

CBaseFunction & CStep::getFunction() {return *mFunction;}

C_FLOAT64 CStep::getFlux() const {return mFlux;}

C_INT16 CStep::isReversible() const {return (mReversible == TRUE);}

void CStep::setName(const string & name) {mName = name;}

void CStep::setChemEq(const string & chemEq) {mChemEq = chemEq;}

void CStep::setFlux(C_FLOAT64 flux) {mFlux = flux;}

void CStep::setReversible(C_INT16 reversible) {mReversible = reversible;}

void CStep::setFunction(const string & functionName)
{
    mFunction = &Copasi.FunctionDB.findFunction(functionName);
}

void CStep::initIdentifiers()
{
    C_INT32 i;
    C_INT32 Count;
    
    if (!mFunction) fatalError();

    mCallParameters->clear();
    
    mCallParameters->resize(mFunction->callParameters().size());
    for (i = 0; i < mCallParameters->size(); i++)
    {
        (*mCallParameters)[i].
            setType(mFunction->callParameters()[i]->getType());

        Count = mFunction->callParameters()[i]->getCountLow();
        (*mCallParameters)[i].identifiers().resize(Count);
        for (C_INT32 j = 0; j < Count; j++)
            (*mCallParameters)[i].identifiers()[j] = NULL;
    }
}

void CStep::setIdentifiers()
{
    pair < C_INT32, C_INT32 > Tuple;
    
    C_INT32 i, j;
    C_INT32 OldSize;
    
    for (i = 0; i < mSubstrates->size(); i++)
    {
        if ((*mSubstrates)[i].mIdentifierName == "") continue;
        
        Tuple = mFunction->findIdentifier((*mSubstrates)[i].mIdentifierName);

        if (Tuple.first < 0 || Tuple.second < 0) fatalError();
        if ((*mCallParameters)[Tuple.first].getType() 
            != CCallParameter::VECTOR_DOUBLE) fatalError();

        if ((OldSize = (*mCallParameters)[Tuple.first].identifiers().size()) <
            Tuple.second + 1)
        {
            (*mCallParameters)[Tuple.first].identifiers().
                resize(Tuple.second + 1);
            for( j = OldSize; j < Tuple.second + 1; j++)
                (*mCallParameters)[Tuple.first].identifiers()[j] = NULL;
        }
        
        (*mCallParameters)[Tuple.first].identifiers()[Tuple.second] =
            (*mSubstrates)[i].mpMetabolite->getConcentration();
    }
    
    for (i = 0; i < mProducts->size(); i++)
    {
        if ((*mProducts)[i].mIdentifierName == "") continue;
        
        Tuple = mFunction->findIdentifier((*mProducts)[i].mIdentifierName);

        if (Tuple.first < 0 || Tuple.second < 0) fatalError();
        if ((*mCallParameters)[Tuple.first].getType()
            != CCallParameter::VECTOR_DOUBLE) fatalError();
                                                     
        if ((OldSize = (*mCallParameters)[Tuple.first].identifiers().size())
            < Tuple.second + 1)
        {
            (*mCallParameters)[Tuple.first].identifiers().
                resize(Tuple.second + 1);
            for( j = OldSize; j < Tuple.second + 1; j++)
                (*mCallParameters)[Tuple.first].identifiers()[j] = NULL;
        }
        
        (*mCallParameters)[Tuple.first].identifiers()[Tuple.second] =
            (*mProducts)[i].mpMetabolite->getConcentration();
    }

    for (i = 0; i < mModifiers->size(); i++)
    {
        if ((*mModifiers)[i].mIdentifierName == "") continue;
        
        Tuple = mFunction->findIdentifier((*mModifiers)[i].mIdentifierName);

        if (Tuple.first < 0 || Tuple.second < 0) fatalError();
        if ((*mCallParameters)[Tuple.first].getType()
            != CCallParameter::VECTOR_DOUBLE) fatalError();
                                                     
        if ((OldSize = (*mCallParameters)[Tuple.first].identifiers().size())
            < Tuple.second + 1)
        {
            (*mCallParameters)[Tuple.first].identifiers().
                resize(Tuple.second + 1);
            for( j = OldSize; j < Tuple.second + 1; j++)
                (*mCallParameters)[Tuple.first].identifiers()[j] = NULL;
        }
        
        (*mCallParameters)[Tuple.first].identifiers()[Tuple.second] =
            (*mModifiers)[i].mpMetabolite->getConcentration();
    }

    for (i = 0; i < mParameters->size(); i++)
    {
        if ((*mParameters)[i].mIdentifierName == "") continue;
        
        Tuple = mFunction->findIdentifier((*mParameters)[i].mIdentifierName);

        if (Tuple.first < 0 || Tuple.second < 0) fatalError();
        if ((*mCallParameters)[Tuple.first].getType()
            != CCallParameter::VECTOR_DOUBLE) fatalError();
                                                     
        if ((OldSize = (*mCallParameters)[Tuple.first].identifiers().size())
            < Tuple.second + 1)
        {
            (*mCallParameters)[Tuple.first].identifiers().
                resize(Tuple.second + 1);
            for( j = OldSize; j < Tuple.second + 1; j++)
                (*mCallParameters)[Tuple.first].identifiers()[j] = NULL;
        }
        
        (*mCallParameters)[Tuple.first].identifiers()[Tuple.second] =
            &(*mParameters)[i].mValue;
    }
}

void CStep::checkIdentifiers()
{
    for (C_INT32 i = 0; i < mCallParameters->size(); i++)
    {
        for (C_INT32 j = 0; j < (*mCallParameters)[i].identifiers().size(); j++)
            if (!(*mCallParameters)[i].identifiers()[j]) fatalError();
    }
}

void CStep::compile(const CCopasiVector < CCompartment > * compartments)
{
    C_INT32 i;
    
    for (i = 0; i < mSubstrates->size(); i++)
        (*mSubstrates)[i].mpMetabolite = 
            &(*compartments)[(*mSubstrates)[i].mCompartmentName].
            metabolites()[(*mSubstrates)[i].mMetaboliteName];
    
    for (i = 0; i < mProducts->size(); i++)
        (*mProducts)[i].mpMetabolite = 
            &(*compartments)[(*mProducts)[i].mCompartmentName].
            metabolites()[(*mProducts)[i].mMetaboliteName];
    
    for (i = 0; i < mModifiers->size(); i++)
        (*mModifiers)[i].mpMetabolite = 
            &(*compartments)[(*mModifiers)[i].mCompartmentName].
            metabolites()[(*mModifiers)[i].mMetaboliteName];
    
    initIdentifiers();
    setIdentifiers();
    checkIdentifiers();
}

C_INT32 CStep::loadNew(CReadConfig & configbuffer)
{
    C_INT32 Fail = 0;
    C_INT32 Size;
    C_INT32 i;
    
    if (Fail = configbuffer.getVariable("Substrates", "C_INT32", &Size))
        return Fail;
    mSubstrates->resize(Size);
    for (i=0; i < Size; i++)
    {
        if (Fail = configbuffer.getVariable("Identifier", "string",
                                            &(*mSubstrates)[i].mIdentifierName))
            return Fail;
        if (Fail = configbuffer.getVariable("Compartment", "string",
                                            &(*mSubstrates)[i].mCompartmentName))
            return Fail;
        if (Fail = configbuffer.getVariable("Metabolite", "string",
                                            &(*mSubstrates)[i].mMetaboliteName))
            return Fail;
    }
    
    if (Fail = configbuffer.getVariable("Products", "C_INT32", &Size))
        return Fail;
    mProducts->resize(Size);
    for (i=0; i < Size; i++)
    {
        if (Fail = configbuffer.getVariable("Identifier", "string",
                                            &(*mProducts)[i].mIdentifierName))
            return Fail;
        if (Fail = configbuffer.getVariable("Compartment", "string",
                                            &(*mProducts)[i].mCompartmentName))
            return Fail;
        if (Fail = configbuffer.getVariable("Metabolite", "string",
                                            &(*mProducts)[i].mMetaboliteName))
            return Fail;
    }

    if (Fail = configbuffer.getVariable("Modifiers", "C_INT32", &Size))
        return Fail;
    mModifiers->resize(Size);
    for (i = 0; i < Size; i++)
    {
        if (Fail = configbuffer.getVariable("Identifier", "string",
                                            &(*mModifiers)[i].mIdentifierName))
            return Fail;
        if (Fail = configbuffer.getVariable("Compartment", "string",
                                            &(*mModifiers)[i].mCompartmentName))
            return Fail;
        if (Fail = configbuffer.getVariable("Metabolite", "string",
                                            &(*mModifiers)[i].mMetaboliteName))
            return Fail;
    }

    if (Fail = configbuffer.getVariable("Constants", "C_INT32", &Size))
        return Fail;
    mParameters->resize(Size);
    for (i = 0; i < Size; i++)
    {
        if (Fail = configbuffer.getVariable("Identifier", "string",
                                            &(*mParameters)[i].mIdentifierName))
            return Fail;
        if (Fail = configbuffer.getVariable("Value", "C_FLOAT64",
                                            &(*mParameters)[i].mValue))
            return Fail;
    }
    
    return Fail;
}

C_INT32 CStep::loadOld(CReadConfig & configbuffer)
{
    string name;
    
    C_INT32 Fail = 0;
    C_INT32 Size;
    C_INT32 i;
    C_INT32 index;

    if (Fail = configbuffer.getVariable("Substrates", "C_INT32", &Size))
        return Fail;
    mSubstrates->resize(Size);
    
    if (Fail = configbuffer.getVariable("Products", "C_INT32", &Size))
        return Fail;
    mProducts->resize(Size);

    if (Fail = configbuffer.getVariable("Modifiers", "C_INT32", &Size))
        return Fail;
    mModifiers->resize(Size);

    if (Fail = configbuffer.getVariable("Constants", "C_INT32", &Size))
        return Fail;
    mParameters->resize(Size);

    for (i = 0; i < mSubstrates->size(); i++)
    {
        name = StringPrint("Subs%d", i);
        configbuffer.getVariable(name, "C_INT32", &index);

        if (mFunction->getName().substr(0,11) == "Mass action")
            (*mSubstrates)[i].mIdentifierName = StringPrint("substrate_%d", i);
        else if (mFunction->callParameters()[0]->identifiers(N_SUBSTRATE).size()
                < i + 1)
            (*mSubstrates)[i].mIdentifierName = "";
        else
            (*mSubstrates)[i].mIdentifierName = mFunction->callParameters()[0]->
                identifiers(N_SUBSTRATE)[i]->getName();
        
        (*mSubstrates)[i].mMetaboliteName = 
            Copasi.OldMetabolites[index].getName();
    }
    
    for (i = 0; i < mProducts->size(); i++)
    {
        name = StringPrint("Prod%d", i);
        configbuffer.getVariable(name, "C_INT32", &index);
        
        if (mFunction->getName().substr(0,11) == "Mass action")
            (*mProducts)[i].mIdentifierName = StringPrint("product_%d", i);
        else if (mFunction->callParameters()[0]->identifiers(N_PRODUCT).size()
                < i + 1)
            (*mProducts)[i].mIdentifierName = "";
        else
            (*mProducts)[i].mIdentifierName = mFunction->callParameters()[0]->
                identifiers(N_PRODUCT)[i]->getName();
        
        (*mProducts)[i].mMetaboliteName = 
            Copasi.OldMetabolites[index].getName();
    }
    
    for (i = 0; i < mModifiers->size(); i++)
    {
        name = StringPrint("Modf%d", i);
        configbuffer.getVariable(name, "C_INT32", &index);
        
        if (mFunction->callParameters()[0]->identifiers(N_MODIFIER).size()
                < i + 1)
            (*mModifiers)[i].mIdentifierName = "";
        else
            (*mModifiers)[i].mIdentifierName = mFunction->callParameters()[0]->
                identifiers(N_MODIFIER)[i]->getName();

        (*mModifiers)[i].mMetaboliteName = 
            Copasi.OldMetabolites[index].getName();
    }
    
    for (i = 0; i < mParameters->size(); i++)
    {
        name = StringPrint("Param%d", i);
        configbuffer.getVariable(name, "C_FLOAT64", 
                                 &(*mParameters)[i].mValue);
        if (mFunction->getName().substr(0,11) == "Mass action")
        {
            if (i)
                (*mParameters)[i].mIdentifierName = "kp";
            else
                (*mParameters)[i].mIdentifierName = "ks";
        }
        else if (mFunction->callParameters()[0]->identifiers(N_KCONSTANT).size()
                < i + 1)
            (*mParameters)[i].mIdentifierName = "";
        else
            (*mParameters)[i].mIdentifierName = mFunction->callParameters()[0]->
            identifiers(N_KCONSTANT)[i]->getName();
    }
    
        
    return Fail;
}

CStep::CId2Metab::CId2Metab() {}

CStep::CId2Metab::~CId2Metab() {}

CStep::CId2Param::CId2Param() {}

CStep::CId2Param::~CId2Param() {}

vector < CStep::ELEMENT > CStep::getChemStructure() const
{
    vector < ELEMENT > Structure;
    ELEMENT Element;
    
    string Left;
    string Right;
    
    string::size_type equal = 0;
    string::size_type pos = 0;
    
    equal = mChemEq.find("=");
    if (equal == string::npos) 
    {
        equal = mChemEq.find("->");
        Right = mChemEq.substr(equal+2);
    }
    else
        Right = mChemEq.substr(equal+1);
    Left = mChemEq.substr(0,equal);

    while (pos != string::npos)
    {
        Element = extractElement(Left, pos);
        Element.mValue *= -1.0; 
        addElement(Element, Structure);
    }
    
    pos = 0;
    while (pos != string::npos)
    {
        Element = extractElement(Right, pos);
        addElement(Element, Structure);
    }
    
    return Structure;
}

CStep::ELEMENT CStep::extractElement(const string & input, 
                              string::size_type & pos) const
{
    ELEMENT Element;
    string Value;
    
    string::size_type Start = input.find_first_not_of(" ", pos);
    string::size_type End = input.find("+", Start);
    string::size_type Multiplier = input.find("*", Start);
    string::size_type NameStart;
    string::size_type NameEnd;
    
    if (Multiplier == string::npos || Multiplier > End)
    {
        NameStart = Start;
        Element.mValue = 1.0;
    }
    else
    {
        NameStart = input.find_first_not_of(" ",Multiplier+1);
        Value = input.substr(Start, Multiplier - Start);
        Element.mValue = atof(Value.c_str());
    }
    
    NameEnd = input.find_first_of(" +", NameStart);
    Element.mName = input.substr(NameStart, NameEnd - NameStart);

    pos = (End == string::npos) ? End: End+1;
    return Element;
}

void CStep::addElement(const ELEMENT & element,
                       vector < ELEMENT > & structure) const
{
    C_INT32 i;

    for (i=0; i < structure.size(); i++)
        if (element.mName == structure[i].mName) break;
    
    if (i >= structure.size()) 
        structure.push_back(element);
    else
        structure[i].mValue += element.mValue;
}

void CStep::old2New(const vector < CMetab* > & metabolites)
{
    C_INT32 i, j;
    
    for (i = 0; i < mSubstrates->size(); i++)
    {
        for (j = 0; j < metabolites.size(); j++)
            if (metabolites[j]->getName() ==
                (*mSubstrates)[i].mMetaboliteName) break;
        (*mSubstrates)[i].mCompartmentName =
            metabolites[j]->getCompartment()->getName();
    }
    
    for (i = 0; i < mProducts->size(); i++)
    {
        for (j = 0; j < metabolites.size(); j++)
            if (metabolites[j]->getName() ==
                (*mProducts)[i].mMetaboliteName) break;
        (*mProducts)[i].mCompartmentName =
            metabolites[j]->getCompartment()->getName();
    }
    
    for (i = 0; i < mModifiers->size(); i++)
    {
        for (j = 0; j < metabolites.size(); j++)
            if (metabolites[j]->getName() ==
                (*mModifiers)[i].mMetaboliteName) break;
        (*mModifiers)[i].mCompartmentName =
            metabolites[j]->getCompartment()->getName();
    }
}

C_FLOAT64 CStep::calculate() 
{
    return mFunction->calcValue(*mCallParameters);
}


