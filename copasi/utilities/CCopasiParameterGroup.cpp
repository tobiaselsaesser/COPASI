// Copyright (C) 2010 - 2015 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 - 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2003 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/**
 *  CCopasiParameterGroup class.
 *  This class is used to describe parameters. This class is intended
 *  to be used with integration or optimization methods or reactions.
 *
 *  Created for COPASI by Stefan Hoops 2002
 */

#include <sstream>

#include "copasi.h"

#include "CCopasiParameterGroup.h"
#include "CCopasiMessage.h"

#include "utilities/utility.h"

CCopasiParameterGroup::CCopasiParameterGroup():
  CCopasiParameter("NoName", GROUP)
{}

CCopasiParameterGroup::CCopasiParameterGroup(const CCopasiParameterGroup & src,
    const CCopasiContainer * pParent):
  CCopasiParameter(src, pParent)
{
  *this = src;
}

CCopasiParameterGroup::CCopasiParameterGroup(const std::string & name,
    const CCopasiContainer * pParent,
    const std::string & objectType):
  CCopasiParameter(name, CCopasiParameter::GROUP, NULL, pParent, objectType)
{}

CCopasiParameterGroup::~CCopasiParameterGroup()
{
  clear();
}

// virtual
const CObjectInterface * CCopasiParameterGroup::getObject(const CCopasiObjectName & cn) const
{
  const CObjectInterface * pObjectInterface = CCopasiContainer::getObject(cn);

  if (pObjectInterface != NULL)
    {
      return pObjectInterface;
    }

  std::string UniqueName = cn.getObjectName();

  std::string::size_type pos = UniqueName.find_last_of('[');
  std::string Name = UniqueName.substr(0, pos);
  size_t Index = strToUnsignedInt(UniqueName.substr(pos + 1).c_str());
  size_t counter = C_INVALID_INDEX;

  index_iterator it = beginIndex();
  index_iterator end = endIndex();

  for (; it != end; ++it)
    {
      if ((*it)->getObjectName() == Name)
        {
          counter++;

          if (counter == Index)
            {
              return (*it)->getObject(cn.getRemainder());
            }
        }
    }

  return NULL;
}

bool CCopasiParameterGroup::elevateChildren() {return true;}

CCopasiParameterGroup & CCopasiParameterGroup::operator = (const CCopasiParameterGroup & rhs)
{
  if (getObjectName() != rhs.getObjectName())
    setObjectName(rhs.getObjectName());

  name_iterator itRHS = rhs.beginName();
  name_iterator endRHS = rhs.endName();

  name_iterator itLHS = beginName();
  name_iterator endLHS = endName();

  std::vector< std::string > ToBeRemoved;
  std::vector< CCopasiParameter * > ToBeAdded;

  CCopasiParameter * pLHS;
  CCopasiParameter * pRHS;

  while (itRHS != endRHS && itLHS != endLHS)
    {
      // We only assign parameters
      if ((pRHS = dynamic_cast< CCopasiParameter * >(itRHS->second)) == NULL)
        {
          ++itRHS;
          continue;
        }

      // We only assign parameters
      if ((pLHS = dynamic_cast< CCopasiParameter * >(itLHS->second)) == NULL)
        {
          ++itLHS;
          continue;
        }

      const std::string & NameLHS = pLHS->getObjectName();

      const std::string & NameRHS = pRHS->getObjectName();

      // The LHS parameter is missing on the RHS thus we need to remove it
      if (NameLHS < NameRHS)
        {
          ToBeRemoved.push_back(NameLHS);
          ++itLHS;
          continue;
        }

      // The RHS parameter is missing on the LHS thus we need to add it
      if (NameLHS > NameRHS)
        {
          ToBeAdded.push_back(pRHS);
          ++itRHS;
          continue;
        }

      // The names are equal it suffices to use the assignment operator of the parameter
      *pLHS = *pRHS;
      ++itLHS;
      ++itRHS;
    }

  // All remaining parameters of the LHS need to be removed
  while (itLHS != endLHS)
    {
      // We only assign parameters
      if ((pLHS = dynamic_cast< CCopasiParameter * >(itLHS->second)) != NULL)
        ToBeRemoved.push_back(pLHS->getObjectName());

      ++itLHS;
    }

  // All remaining parameter of the RHS need to be added
  while (itRHS != endRHS)
    {
      // We only assign parameters
      if ((pRHS = dynamic_cast< CCopasiParameter * >(itRHS->second)) != NULL)
        ToBeAdded.push_back(pRHS);

      ++itRHS;
    }

  // We remove the parameters
  std::vector< std::string >::const_iterator itToBeRemoved = ToBeRemoved.begin();
  std::vector< std::string >::const_iterator endToBeRemoved = ToBeRemoved.end();

  for (; itToBeRemoved != endToBeRemoved; ++itToBeRemoved)
    this->removeParameter(*itToBeRemoved);

  // We add the missing parameters
  CCopasiParameter * pParameter;
  std::vector< CCopasiParameter * >::const_iterator itToBeAdded = ToBeAdded.begin();
  std::vector< CCopasiParameter * >::const_iterator endToBeAdded = ToBeAdded.end();

  for (; itToBeAdded != endToBeAdded; ++itToBeAdded)
    {
      if ((*itToBeAdded)->getType() == GROUP)
        pParameter = new CCopasiParameterGroup(* static_cast< CCopasiParameterGroup * >(*itToBeAdded));
      else
        pParameter = new CCopasiParameter(**itToBeAdded);

      addParameter(pParameter);
    }

  return *this;
}

void CCopasiParameterGroup::print(std::ostream * ostream) const
{*ostream << *this;}

std::ostream &operator<<(std::ostream &os, const CCopasiParameterGroup & o)
{
  os << "<<< Parameter Group: " << o.getObjectName() << std::endl;

  CCopasiParameterGroup::elements::const_iterator it = o.beginIndex();
  CCopasiParameterGroup::elements::const_iterator end = o.endIndex();

  for (; it != end; ++it)
    {
      (*it)->print(&os);
      os << std::endl;
    }

  os << ">>> Parameter Group: " << o.getObjectName() << std::endl;
  return os;
}

bool operator==(const CCopasiParameterGroup & lhs,
                const CCopasiParameterGroup & rhs)
{
  if (lhs.getObjectName() != rhs.getObjectName()) return false;

  if (lhs.size() != rhs.size()) return false;

  CCopasiParameterGroup::elements::const_iterator itLhs = lhs.beginIndex();
  CCopasiParameterGroup::elements::const_iterator endLhs = lhs.endIndex();
  CCopasiParameterGroup::elements::const_iterator itRhs = rhs.beginIndex();

  for (; itLhs != endLhs; ++itLhs, ++itRhs)
    if (!(**itLhs == **itRhs)) return false;

  return true;
}

bool CCopasiParameterGroup::addParameter(const CCopasiParameter & parameter)
{
  if (parameter.getType() == CCopasiParameter::GROUP)
    {
      CCopasiParameterGroup * pGroup =
        new CCopasiParameterGroup(*dynamic_cast<const CCopasiParameterGroup *>(&parameter));
      addParameter(pGroup);
    }
  else
    {
      CCopasiParameter * pParameter = new CCopasiParameter(parameter);
      addParameter(pParameter);
    }

  return true;
}

void CCopasiParameterGroup::addParameter(CCopasiParameter * pParameter)
{
  if (pParameter == NULL) return;

  CCopasiContainer::add(pParameter, true);
  static_cast< elements * >(mpValue)->push_back(pParameter);
}

CCopasiParameterGroup::name_iterator CCopasiParameterGroup::beginName() const
{return const_cast< CCopasiContainer::objectMap * >(&getObjects())->begin();}

CCopasiParameterGroup::name_iterator CCopasiParameterGroup::endName() const
{return const_cast< CCopasiContainer::objectMap * >(&getObjects())->end();}

CCopasiParameterGroup::index_iterator CCopasiParameterGroup::beginIndex() const
{return static_cast< elements * >(mpValue)->begin();}

CCopasiParameterGroup::index_iterator CCopasiParameterGroup::endIndex() const
{return static_cast< elements * >(mpValue)->end();}

bool CCopasiParameterGroup::addParameter(const std::string & name,
    const CCopasiParameter::Type type)
{
  CCopasiParameter * pParameter;

  if (type == GROUP)
    pParameter = new CCopasiParameterGroup(name);
  else
    pParameter = new CCopasiParameter(name, type);

  addParameter(pParameter);

  return true;
}

bool CCopasiParameterGroup::addGroup(const std::string & name)
{
  addParameter(new CCopasiParameterGroup(name));
  return true;
}

CCopasiParameterGroup * CCopasiParameterGroup::assertGroup(const std::string & name)
{
  CCopasiParameterGroup * pGrp = getGroup(name);

  if (pGrp) return pGrp;

  removeParameter(name);

  addGroup(name);
  return getGroup(name);
}

bool CCopasiParameterGroup::removeParameter(const std::string & name)
{
  size_t index = getIndex(name);

  if (index != C_INVALID_INDEX)
    {
      index_iterator it = static_cast< elements * >(mpValue)->begin() + index;

      pdelete(*it);
      static_cast< elements * >(mpValue)->erase(it, it + 1);

      return true;
    }

  return false;
}

bool CCopasiParameterGroup::removeParameter(const size_t & index)
{
  if (index < size())
    {
      index_iterator it = static_cast< elements * >(mpValue)->begin() + index;

      pdelete(*it);
      static_cast< elements * >(mpValue)->erase(it, it + 1);

      return true;
    }

  return false;
}

CCopasiParameter * CCopasiParameterGroup::getParameter(const std::string & name)
{
  std::pair < CCopasiContainer::objectMap::const_iterator,
      CCopasiContainer::objectMap::const_iterator > range =
        getObjects().equal_range(name);

  if (range.first == range.second) return NULL;

  return
    dynamic_cast<CCopasiParameter *>(const_cast< CCopasiObject * >(range.first->second));
}

const CCopasiParameter * CCopasiParameterGroup::getParameter(const std::string & name) const
{
  std::pair < CCopasiContainer::objectMap::const_iterator,
      CCopasiContainer::objectMap::const_iterator > range =
        getObjects().equal_range(name);

  if (range.first == range.second) return NULL;

  return
    dynamic_cast<CCopasiParameter *>(range.first->second);
}

CCopasiParameter * CCopasiParameterGroup::getParameter(const size_t & index)
{
  if (index < size())
    return *(static_cast< elements * >(mpValue)->begin() + index);

  return NULL;
}

const CCopasiParameter * CCopasiParameterGroup::getParameter(const size_t & index) const
{
  if (index < size())
    return *(static_cast< elements * >(mpValue)->begin() + index);

  return NULL;
}

CCopasiParameterGroup * CCopasiParameterGroup::getGroup(const std::string & name)
{return dynamic_cast<CCopasiParameterGroup *>(getParameter(name));}

const CCopasiParameterGroup * CCopasiParameterGroup::getGroup(const std::string & name) const
{return dynamic_cast<const CCopasiParameterGroup *>(getParameter(name));}

CCopasiParameterGroup * CCopasiParameterGroup::getGroup(const size_t & index)
{return dynamic_cast<CCopasiParameterGroup *>(getParameter(index));}

const CCopasiParameterGroup * CCopasiParameterGroup::getGroup(const size_t & index) const
{return dynamic_cast<const CCopasiParameterGroup *>(getParameter(index));}

CCopasiParameter::Type CCopasiParameterGroup::getType(const size_t & index) const
{
  CCopasiParameter * pParameter =
    const_cast< CCopasiParameterGroup * >(this)->getParameter(index);

  if (pParameter) return pParameter->getType();

  return CCopasiParameter::INVALID;
}

std::string CCopasiParameterGroup::getKey(const std::string & name) const
{
  CCopasiParameter * pParameter =
    const_cast< CCopasiParameterGroup * >(this)->getParameter(name);

  if (pParameter) return pParameter->getKey();

  return "Not Found";
}

std::string CCopasiParameterGroup::getKey(const size_t & index) const
{
  CCopasiParameter * pParameter =
    const_cast< CCopasiParameterGroup * >(this)->getParameter(index);

  if (pParameter) return pParameter->getKey();

  return "Not Found";
}

const std::string & CCopasiParameterGroup::getName(const size_t & index) const
{
  static std::string Invalid("Invalid Index");

  CCopasiParameter * pParameter =
    const_cast< CCopasiParameterGroup * >(this)->getParameter(index);

  if (pParameter) return pParameter->getObjectName();

  return Invalid;
}

bool CCopasiParameterGroup::swap(const size_t & iFrom,
                                 const size_t & iTo)
{
  index_iterator from = beginIndex() + iFrom;
  index_iterator to = beginIndex() + iTo;

  return swap(from, to);
}

bool CCopasiParameterGroup::swap(index_iterator & from,
                                 index_iterator & to)
{
  if (from < beginIndex() || endIndex() <= from ||
      to < beginIndex() || endIndex() <= to)
    return false;

  CCopasiParameter *tmp = *from;
  *from = *to;
  *to = tmp;

  return true;
}

size_t CCopasiParameterGroup::size() const
{return static_cast< elements * >(mpValue)->size();}

void CCopasiParameterGroup::clear()
{
  if (mpValue != NULL)
    {
      index_iterator it = static_cast< elements * >(mpValue)->begin();
      index_iterator end = static_cast< elements * >(mpValue)->end();

      for (; it != end; ++it) pdelete(*it);

      static_cast< elements * >(mpValue)->clear();
    }
}

size_t CCopasiParameterGroup::getIndex(const std::string & name) const
{
  index_iterator it = static_cast< elements * >(mpValue)->begin();
  index_iterator end = static_cast< elements * >(mpValue)->end();

  for (size_t i = 0; it != end; ++it, ++i)
    if (name == (*it)->getObjectName()) return i;;

  return C_INVALID_INDEX;
}

std::string CCopasiParameterGroup::getUniqueParameterName(const CCopasiParameter * pParameter) const
{
  size_t counter = C_INVALID_INDEX;
  size_t Index = C_INVALID_INDEX;

  std::string Name = pParameter->getObjectName();

  index_iterator it = static_cast< elements * >(mpValue)->begin();
  index_iterator end = static_cast< elements * >(mpValue)->end();

  for (; it != end; ++it)
    {
      if ((*it)->getObjectName() == Name)
        {
          counter++;

          if (*it == pParameter)
            {
              Index = counter;
            }
        }
    }

  if (counter == 0 || Index == C_INVALID_INDEX)
    {
      return Name;
    }

  std::stringstream UniqueName;
  UniqueName << Name << "[" << Index << "]";

  return UniqueName.str();
}
