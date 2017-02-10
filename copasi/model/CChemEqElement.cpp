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

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

// CChemEqElement
//
// A class describing an element of a chemical equation
// (C) Stefan Hoops 2001
//

#include "copasi.h"
#include "utilities/CCopasiVector.h"
#include "utilities/CCopasiException.h"
#include "utilities/utility.h"
#include "CChemEqElement.h"
#include "CCompartment.h"
#include "report/CKeyFactory.h"
#include "report/CCopasiObjectReference.h"
#include "CMetabNameInterface.h"
#include "copasi/report/CCopasiRootContainer.h"

// static
CChemEqElement * CChemEqElement::create(const CData & data)
{
  return new CChemEqElement(data.getProperty(CData::OBJECT_NAME).toString(),
                            NO_PARENT);
}

CChemEqElement::CChemEqElement(const std::string & name,
                               const CCopasiContainer * pParent):
  CCopasiContainer(name, pParent, "Chemical Equation Element"),
  mMetaboliteKey(),
  mMultiplicity(0)
  //mpMetabolite(NULL)
{
  initObjects();
  CONSTRUCTOR_TRACE;
}

CChemEqElement::CChemEqElement(const CChemEqElement & src,
                               const CCopasiContainer * pParent):
  CCopasiContainer(src, pParent),
  mMetaboliteKey(src.mMetaboliteKey),
  mMultiplicity(src.mMultiplicity)
  //mpMetabolite(src.mpMetabolite)
{
  initObjects();
  CONSTRUCTOR_TRACE;
}

CChemEqElement::~CChemEqElement() {DESTRUCTOR_TRACE;}

void CChemEqElement::initObjects()
{
  addObjectReference("Multiplicity", mMultiplicity, CCopasiObject::ValueDbl);
  addObjectReference("Metab Key", mMetaboliteKey);
}

void CChemEqElement::cleanup() {}

void CChemEqElement::setMetabolite(const std::string & key)
{
  mMetaboliteKey = key;
  CMetab* tmp = dynamic_cast< CMetab * >(CCopasiRootContainer::getKeyFactory()->get(mMetaboliteKey));

  if (tmp)
    this->setObjectName("ChEqEl_" + tmp->getObjectName());
  else
    this->setObjectName("ChemEqElement");
}

const std::string & CChemEqElement::getMetaboliteKey() const
{return mMetaboliteKey;}

const CMetab * CChemEqElement::getMetabolite() const
{return dynamic_cast< CMetab * >(CCopasiRootContainer::getKeyFactory()->get(mMetaboliteKey));}

void CChemEqElement::setMultiplicity(const C_FLOAT64 multiplicity)
{mMultiplicity = multiplicity;}

void CChemEqElement::addToMultiplicity(const C_FLOAT64 multiplicity)
{mMultiplicity += multiplicity;}

const C_FLOAT64 & CChemEqElement::getMultiplicity() const
{
  return mMultiplicity;
}

std::ostream & operator<<(std::ostream &os, const CChemEqElement & d)
{
  os << "CChemEqElement: " << d.mMultiplicity
     << " * " << d.mMetaboliteKey << std::endl;

  return os;
}
