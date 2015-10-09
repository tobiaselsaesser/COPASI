// Copyright (C) 2010 - 2015 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 - 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2005 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include <vector>
#include <limits>
#include <cmath>
#include <stdlib.h>

#include "copasi.h"

#include "CExperimentObjectMap.h"

#include "math/CMathContainer.h"
#include "utilities/utility.h"
#include "CopasiDataModel/CCopasiDataModel.h"

CExperimentObjectMap::CExperimentObjectMap(const std::string & name,
    const CCopasiContainer * pParent):
  CCopasiParameterGroup(name, pParent),
  mObjects(0),
  mLastColumn(0)
{initializeParameter();}

CExperimentObjectMap::CExperimentObjectMap(const CExperimentObjectMap & src,
    const CCopasiContainer * pParent):
  CCopasiParameterGroup(src, pParent),
  mObjects(src.mObjects),
  mLastColumn(src.mLastColumn)
{initializeParameter();}

CExperimentObjectMap::CExperimentObjectMap(const CCopasiParameterGroup & group,
    const CCopasiContainer * pParent):
  CCopasiParameterGroup(group, pParent),
  mObjects(0),
  mLastColumn(0)
{initializeParameter();}

CExperimentObjectMap::~CExperimentObjectMap() {}

void CExperimentObjectMap::initializeParameter()
{
  elevateChildren();
}

bool CExperimentObjectMap::elevateChildren()
{
  bool success = true;

  elements::iterator itColumn = beginIndex();
  elements::iterator endColumn = endIndex();

  if (itColumn != endColumn &&
      dynamic_cast< CCopasiParameterGroup * >(*itColumn) == NULL) // We have an old data format.
    {
      CCopasiParameterGroup New(getObjectName());

      for (; itColumn != endColumn; ++itColumn)
        {
          CCopasiParameterGroup * pGroup = New.assertGroup((*itColumn)->getObjectName());
          pGroup->assertParameter("Object CN", CCopasiParameter::CN, (*itColumn)->getValue< CRegisteredObjectName >());
        }

      clear();
      *this = New;
    }

  for (itColumn = beginIndex(); itColumn != endColumn; ++itColumn)
    if (((*itColumn) = elevate<CDataColumn, CCopasiParameterGroup>(*itColumn)) == NULL)
      success = false;

  return success;
}

bool CExperimentObjectMap::setNumCols(const size_t & numCols)
{
  if (numCols == size())
    return true;

  // We only clear the vector of parameter. We do not destroy the parameter they are still
  // accessible through CCopasiContainer::mObjects and thus will be automatically destroyed.
  clear();

  bool success = true;

  for (size_t col = 0; col < numCols; col++)
    {
      CCopasiParameterGroup * pGrp = assertGroup(StringPrint("%d", col));

      // assertGroup() adds only newly created groups to mValue.pGROUP. We need to add the existing
      // ones.
      if (size() < col + 1)
        static_cast< elements * >(mpValue)->push_back(pGrp);

      success &= (elevate<CDataColumn, CCopasiParameterGroup>(pGrp) != NULL);
    }

  return success;
}

size_t CExperimentObjectMap::getLastNotIgnoredColumn() const
{
  elements::iterator itColumn = beginIndex();
  elements::iterator endColumn = endIndex();

  C_INT32 LastNotIgnored = -1;

  for (; itColumn != endColumn; ++itColumn)
    if (static_cast< CDataColumn * >(*itColumn)->getRole() != CExperiment::ignore)
      {
        C_INT32 index = strtol(static_cast< CDataColumn * >(*itColumn)->getObjectName().c_str(), NULL, 10);

        if (index > LastNotIgnored) LastNotIgnored = index;
      }

  return LastNotIgnored;
}

// virtual
const std::string & CExperimentObjectMap::getName(const size_t & index) const
{
  static const std::string NoName("");

  const CDataColumn * pColumn =
    dynamic_cast< const CDataColumn * >(getGroup(StringPrint("%d", index)));

  if (pColumn)
    return pColumn->getObjectName();
  else
    return NoName;
}

bool CExperimentObjectMap::setRole(const size_t & index,
                                   const CExperiment::Type & role)
{
  CDataColumn * pColumn =
    dynamic_cast< CDataColumn * >(getGroup(StringPrint("%d", index)));

  if (pColumn)
    return pColumn->setRole(role);
  else
    return false;
}

CExperiment::Type CExperimentObjectMap::getRole(const size_t & index) const
{
  const CDataColumn * pColumn =
    dynamic_cast< const CDataColumn * >(getGroup(StringPrint("%d", index)));

  if (pColumn)
    return pColumn->getRole();
  else
    return CExperiment::ignore;
}

bool CExperimentObjectMap::setObjectCN(const size_t & index,
                                       const std::string & CN)
{
  CDataColumn * pColumn =
    dynamic_cast< CDataColumn * >(getGroup(StringPrint("%d", index)));

  if (pColumn)
    return pColumn->setObjectCN((CCopasiObjectName) CN);
  else
    return false;
}

std::string CExperimentObjectMap::getObjectCN(const size_t & index) const
{
  const CDataColumn * pColumn =
    dynamic_cast< const CDataColumn * >(getGroup(StringPrint("%d", index)));

  if (pColumn)
    return pColumn->getObjectCN();
  else
    return "";
}

bool CExperimentObjectMap::setScale(const size_t & index,
                                    const C_FLOAT64 & weight)
{
  CDataColumn * pColumn =
    dynamic_cast< CDataColumn * >(getGroup(StringPrint("%d", index)));

  if (pColumn)
    return pColumn->setScale(weight);
  else
    return false;
}

C_FLOAT64 CExperimentObjectMap::getScale(const size_t & index) const
{
  const CDataColumn * pColumn =
    dynamic_cast< const CDataColumn * >(getGroup(StringPrint("%d", index)));

  if (pColumn)
    return pColumn->getScale();
  else
    return std::numeric_limits<C_FLOAT64>::quiet_NaN();
}

C_FLOAT64 CExperimentObjectMap::getDefaultScale(const size_t & index) const
{
  const CDataColumn * pColumn =
    dynamic_cast< const CDataColumn * >(getGroup(StringPrint("%d", index)));

  if (pColumn)
    return pColumn->getDefaultScale();
  else
    return std::numeric_limits<C_FLOAT64>::quiet_NaN();
}

bool CExperimentObjectMap::compile(const CMathContainer * pMathContainer)
{
  size_t i, imax = size();
  size_t Column;

  // We need to find out the size of the object map
  if (imax == 0)
    mLastColumn = C_INVALID_INDEX;
  else
    {
      mLastColumn = strtoul(getName(0).c_str(), NULL, 0);

      for (i = 1; i < imax; i++)
        {
          Column = strtoul(getName(i).c_str(), NULL, 0);

          if (mLastColumn < Column)
            mLastColumn = Column;
        }
    }

  mObjects.resize(mLastColumn + 1);
  mObjects = NULL;

  const CCopasiObject * pObject = NULL;
  std::string CN;

  for (i = 0; i < imax; i++)
    {
      if ((CN = getObjectCN(i)) == "") continue;

      if ((pObject = CObjectInterface::DataObject(pMathContainer->getObjectFromCN(CN))) != NULL &&
          pObject->isValueDbl())
        {
          Column = strtoul(getName(i).c_str(), NULL, 0);
          mObjects[Column] = pObject;
        }
      else
        return false;
    }

  return true;
}

const CVector< const CCopasiObject * > & CExperimentObjectMap::getDataObjects() const
{return mObjects;}

const size_t & CExperimentObjectMap::getLastColumn() const
{return mLastColumn;}

void CExperimentObjectMap::fixBuild55()
{
  CCopasiParameterGroup::index_iterator it = beginIndex();
  CCopasiParameterGroup::index_iterator end = endIndex();

  for (; it != end; ++it)
    {
      CDataColumn * pColumn = dynamic_cast< CDataColumn * >(*it);

      if (pColumn != NULL)
        {
          pColumn->fixBuild55();
        }
    }
}

CExperimentObjectMap::CDataColumn::CDataColumn(const std::string & name,
    const CCopasiContainer * pParent) :
  CCopasiParameterGroup(name, pParent),
  mpRole(NULL),
  mpObjectCN(NULL),
  mpScale(NULL)
{
  initializeParameter();
}

CExperimentObjectMap::CDataColumn::CDataColumn(const CDataColumn & src,
    const CCopasiContainer * pParent) :
  CCopasiParameterGroup(src, pParent),
  mpRole(NULL),
  mpObjectCN(NULL),
  mpScale(NULL)
{
  initializeParameter();
}

CExperimentObjectMap::CDataColumn::CDataColumn(const CCopasiParameterGroup & group,
    const CCopasiContainer * pParent) :
  CCopasiParameterGroup(group, pParent),
  mpRole(NULL),
  mpObjectCN(NULL),
  mpScale(NULL)
{
  initializeParameter();
}

CExperimentObjectMap::CDataColumn::~CDataColumn()
{}

void CExperimentObjectMap::CDataColumn::initializeParameter()
{
  mpRole = (CExperiment::Type *) assertParameter("Role", CCopasiParameter::UINT, (unsigned C_INT32) CExperiment::ignore);

  CCopasiParameter * pParm = getParameter("Object CN");

  if (pParm != NULL)
    mpObjectCN = &pParm->getValue< CRegisteredObjectName >();

  pParm = getParameter("Weight");

  if (pParm != NULL)
    mpScale = &pParm->getValue< C_FLOAT64 >();

  elevateChildren();
}

CExperimentObjectMap::CDataColumn & CExperimentObjectMap::CDataColumn::operator = (const CExperimentObjectMap::CDataColumn & rhs)
{
  *mpRole = *rhs.mpRole;
  setObjectCN(rhs.getObjectCN());
  setScale(rhs.getScale());

  return *this;
}

bool CExperimentObjectMap::CDataColumn::elevateChildren()
{return true;}

bool CExperimentObjectMap::CDataColumn::setRole(const CExperiment::Type & role)
{
  *mpRole = role;
  return true;
}

const CExperiment::Type & CExperimentObjectMap::CDataColumn::getRole() const
{return *mpRole;}

bool CExperimentObjectMap::CDataColumn::setObjectCN(const std::string & objectCN)
{
  if (objectCN == "")
    {
      if (mpObjectCN != NULL)
        {
          removeParameter("Object CN");
          mpObjectCN = NULL;
        }
    }
  else
    {
      if (mpObjectCN != NULL)
        *mpObjectCN = objectCN;
      else
        mpObjectCN =
          assertParameter("Object CN", CCopasiParameter::CN, (CCopasiObjectName) objectCN);
    }

  return true;
}

std::string CExperimentObjectMap::CDataColumn::getObjectCN() const
{
  if (mpObjectCN != NULL)
    return *mpObjectCN;
  else
    return "";
}

bool CExperimentObjectMap::CDataColumn::setScale(const C_FLOAT64 & weight)
{
  if (isnan(weight))
    {
      if (mpScale != NULL)
        {
          removeParameter("Weight");
          mpScale = NULL;
        }

      return true;
    }

  C_FLOAT64 DefaultWeight = getDefaultScale();

  if (weight != DefaultWeight || isnan(DefaultWeight))
    {
      if (mpScale != NULL)
        *mpScale = weight;
      else
        mpScale = assertParameter("Weight", CCopasiParameter::UDOUBLE, weight);

      return true;
    }

  if (mpScale != NULL)
    {
      removeParameter("Weight");
      mpScale = NULL;
    }

  return true;
}

C_FLOAT64 CExperimentObjectMap::CDataColumn::getScale() const
{
  if (mpScale == NULL)
    return getDefaultScale();
  else
    return *mpScale;
}

C_FLOAT64 CExperimentObjectMap::CDataColumn::getDefaultScale() const
{
  if (mpObjectCN == NULL)
    return std::numeric_limits<C_FLOAT64>::quiet_NaN();

  CCopasiParameterGroup *pGroup =
    dynamic_cast< CCopasiParameterGroup * >(getObjectParent());

  if (pGroup == NULL)
    return std::numeric_limits<C_FLOAT64>::quiet_NaN();

  const CExperiment *pExperiment =
    dynamic_cast<const CExperiment * >(pGroup->getObjectParent());

  if (pExperiment == NULL)
    return std::numeric_limits<C_FLOAT64>::quiet_NaN();

  CObjectInterface::ContainerList ListOfContainer;
  ListOfContainer.push_back(getObjectDataModel());

  const CCopasiObject * pObject = CObjectInterface::DataObject(CObjectInterface::GetObjectFromCN(ListOfContainer, *mpObjectCN));

  if (pObject == NULL)
    return std::numeric_limits<C_FLOAT64>::quiet_NaN();

  return pExperiment->getDefaultScale(pObject);
}

void CExperimentObjectMap::CDataColumn::fixBuild55()
{
  if (mpScale != NULL)
    {
      *mpScale *= *mpScale;
    }
}
