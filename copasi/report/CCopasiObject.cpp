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

/**
 * Class CCopasiObject
 *
 * This class is the base class for all global accessible objects in COPASI.
 *
 * Copyright Stefan Hoops 2002
 */

#include <sstream>
#include <algorithm>

#include "copasi/copasi.h"
#include "copasi/report/CCopasiObjectName.h"
#include "copasi/report/CCopasiObject.h"
#include "copasi/report/CCopasiContainer.h"
#include "copasi/report/CCopasiStaticString.h"
#include "copasi/report/CRenameHandler.h"
#include "copasi/utilities/CCopasiVector.h"
#include "copasi/model/CModelValue.h"
#include "copasi/model/CModel.h"
#include "copasi/CopasiDataModel/CCopasiDataModel.h"
#include "copasi/report/CCopasiRootContainer.h"
#include "copasi/function/CFunctionDB.h"

//static
C_FLOAT64 CCopasiObject::DummyValue = 0.0;

//static
CRenameHandler * CCopasiObject::smpRenameHandler = NULL;

// static
const CCopasiObject * CObjectInterface::DataObject(const CObjectInterface * pInterface)
{
  if (pInterface != NULL)
    {
      return pInterface->getDataObject();
    }

  return NULL;
}

// static
CObjectInterface * CObjectInterface::GetObjectFromCN(const CObjectInterface::ContainerList & listOfContainer,
    const CCopasiObjectName & objName)
{
  CCopasiObjectName Primary = objName.getPrimary();
  std::string Type = Primary.getObjectType();

  // Check that we have a fully qualified CN
  if (objName.getPrimary() != "CN=Root" &&
      Type != "Separator" &&
      Type != "String")
    {
      return NULL;
    }

  const CObjectInterface * pObject = NULL;

  const CCopasiDataModel * pDataModel = NULL;

  CObjectInterface::ContainerList::const_iterator it = listOfContainer.begin();

  CObjectInterface::ContainerList::const_iterator end = listOfContainer.end();

  CCopasiObjectName ContainerName;

  std::string::size_type pos;

  bool CheckDataModel = true;

  //favor to search the list of container first
  for (; it != end && pObject == NULL; ++it)
    {
      if (*it == NULL)
        {
          continue;
        }

      if (pDataModel == NULL)
        {
          pDataModel = (*it)->getObjectDataModel();
        }

      CheckDataModel &= (pDataModel != *it);

      ContainerName = (*it)->getCN();

      while (ContainerName.getRemainder() != "")
        {
          ContainerName = ContainerName.getRemainder();
        }

      if ((pos = objName.find(ContainerName)) == std::string::npos)
        continue;

      if (pos + ContainerName.length() == objName.length())
        pObject = *it;
      else
        pObject = (*it)->getObject(objName.substr(pos + ContainerName.length() + 1));
    }

  // if still not found search the function database in the root container
  if (pObject == NULL)
    pObject = CCopasiRootContainer::getFunctionList()->getObject(objName);

  // last resort check the whole data model if we know it.
  // We need make sure that we do not  have infinite recursion
  if (pObject == NULL && pDataModel != NULL && CheckDataModel)
    {
      pObject = pDataModel->getObjectFromCN(objName);
    }

  return const_cast< CObjectInterface * >(pObject);
}

CValidity & CObjectInterface::getValidity()
{
  return mValidity;
}

const CValidity & CObjectInterface::getValidity() const
{
  return mValidity;
}

CObjectInterface::CObjectInterface()
{}

// virtual
CObjectInterface::~CObjectInterface()
{};

CCopasiObject::CCopasiObject():
  CObjectInterface(),
  mObjectName("No Name"),
  mObjectType("Unknown Type"),
  mpObjectParent(NULL),
  mpObjectDisplayName(NULL),
  mObjectFlag(0)
{}

CCopasiObject::CCopasiObject(const std::string & name,
                             const CCopasiContainer * pParent,
                             const std::string & type,
                             const unsigned C_INT32 & flag):
  CObjectInterface(),
  mObjectName((name == "") ? "No Name" : name),
  mObjectType(type),
  mpObjectParent(const_cast<CCopasiContainer * >(pParent)),
  mpObjectDisplayName(NULL),
  mObjectFlag(flag)
{
  if (mpObjectParent != NULL &&
      mpObjectParent->isContainer())
    {
      mpObjectParent->add(this, true);
    }
}

CCopasiObject::CCopasiObject(const CCopasiObject & src,
                             const CCopasiContainer * pParent):
  CObjectInterface(),
  mObjectName(src.mObjectName),
  mObjectType(src.mObjectType),
  mpObjectParent(src.mpObjectParent),
  mpObjectDisplayName(NULL),
  mObjectFlag(src.mObjectFlag)
{
  if (pParent != INHERIT_PARENT)
    {
      mpObjectParent = const_cast<CCopasiContainer *>(pParent);
    }

  if (mpObjectParent != NULL)
    {
      mpObjectParent->add(this, true);
    }
}

CCopasiObject::~CCopasiObject()
{
  if (mpObjectParent)
    mpObjectParent->remove(this);

  pdelete(mpObjectDisplayName);
}

void CCopasiObject::print(std::ostream * ostream) const {(*ostream) << (*this);}

CCopasiObjectName CCopasiObject::getCN() const
{
  CCopasiObjectName CN;

  // if the object has a parent and if the object is not a datamodel,
  // we add the name of the parent to the common name
  if (isDataModel())
    CN = (std::string) "CN=Root";
  else if (mpObjectParent)
    {
      std::stringstream tmp;
      tmp << mpObjectParent->getCN();

      if (mpObjectParent->isNameVector())
        tmp << "[" << CCopasiObjectName::escape(mObjectName) << "]";
      else if (mpObjectParent->isVector())
        tmp << "[" << static_cast<const CCopasiVector< CCopasiObject > *>(mpObjectParent)->getIndex(this) << "]";
      else
        tmp << "," << CCopasiObjectName::escape(mObjectType)
            << "=" << CCopasiObjectName::escape(mObjectName);

      CN = tmp.str();
    }
  else
    {
      CN = CCopasiObjectName::escape(mObjectType)
           + "=" + CCopasiObjectName::escape(mObjectName);
    }

  return CN;
}

const CObjectInterface * CCopasiObject::getObject(const CCopasiObjectName & cn) const
{
  if (cn == "")
    {
      return this;
    }

  if (cn == "Property=DisplayName")
    {
      if (mpObjectDisplayName == NULL)
        {
          mpObjectDisplayName = new CCopasiStaticString();
        }

      *mpObjectDisplayName = getObjectDisplayName();

      return mpObjectDisplayName;
    }

  return NULL;
}

const CObjectInterface * CCopasiObject::getObjectFromCN(const CCopasiObjectName & cn) const
{
  CObjectInterface::ContainerList ListOfContainer;
  ListOfContainer.push_back(getObjectDataModel());

  return CObjectInterface::GetObjectFromCN(ListOfContainer, cn);
}

bool CCopasiObject::setObjectName(const std::string & name)
{
  std::string Name = (name == "") ? "No Name" : name;

  if (!isStaticString())
    {
      // We need to ensure that the name does not include any whitespace character except ' ' (space),
      // i.e., we convert '\t' (tab), '\n' (newline) and '\r' (return) to ' ' (space).
      std::string::iterator it = Name.begin();
      std::string::iterator end = Name.end();

      for (; it != end; ++it)
        {
          switch (*it)
            {
              case '\t':
              case '\n':
              case '\r':
                *it = ' ';
                break;

              default:
                break;
            }
        }
    }

  if (Name == mObjectName) return true;

  if (mpObjectParent &&
      mpObjectParent->isNameVector() &&
      mpObjectParent->getObject("[" + CCopasiObjectName::escape(Name) + "]"))
    return false;

  bool Add = (mpObjectParent != NULL && mpObjectParent->CCopasiContainer::remove(this));

  if (smpRenameHandler && mpObjectParent)
    {
      std::string oldCN = this->getCN();
      mObjectName = Name;
      std::string newCN = this->getCN();
      smpRenameHandler->handle(oldCN, newCN);

      //TODO performance considerations.
      //Right now after every rename the CNs are checked. In some cases
      //we may know that this is not necessary
    }
  else
    {
      mObjectName = Name;
    }

  if (Add)
    {
      mpObjectParent->CCopasiContainer::add(this, false);
    }

  return true;
}

/*virtual*/
std::string CCopasiObject::getObjectDisplayName() const
{
  std::string ret = "";

  if (mpObjectParent)
    {
      ret = mpObjectParent->getObjectDisplayName();

      if (ret == "(CN)Root" ||
          ret == "ModelList[]" ||
          ret.substr(0, 7) == "(Model)")
        {
          ret = "";
        }
    }

  if (ret.length() >= 2)
    if ((ret.substr(ret.length() - 2) == "[]") && (!isReference()))
      {
        ret.insert(ret.length() - 1, getObjectName());

        if (isNameVector() || isVector() || getObjectType() == "ParameterGroup")
          ret += "[]";

        return ret;
      }

  if ((ret.length() != 0) && (ret[ret.length() - 1] != '.'))
    ret += ".";

  if (isNameVector() || isVector() || getObjectType() == "ParameterGroup")
    ret += getObjectName() + "[]";
  else if (isReference()
           || getObjectType() == "Parameter"
           || getObjectType() == getObjectName())
    ret += getObjectName();
  else
    ret += "(" + getObjectType() + ")" + getObjectName();

  return ret;
}

const std::string & CCopasiObject::getObjectName() const {return mObjectName;}

const std::string & CCopasiObject::getObjectType() const {return mObjectType;}

bool CCopasiObject::setObjectParent(const CCopasiContainer * pParent)
{
  if (pParent == mpObjectParent)
    return true;

  if (mpObjectParent != NULL &&
      pParent != NULL)
    mpObjectParent->remove(this);

  mpObjectParent = const_cast<CCopasiContainer *>(pParent);

  return true;
}

CCopasiContainer * CCopasiObject::getObjectParent() const {return mpObjectParent;}

CCopasiContainer *
CCopasiObject::getObjectAncestor(const std::string & type) const
{
  CCopasiContainer * p = getObjectParent();

  while (p)
    {
      if (p->getObjectType() == type) return p;

      p = p->getObjectParent();
    }

  return NULL;
}

void CCopasiObject::clearDirectDependencies()
{
  mDependencies.clear();
}

void CCopasiObject::setDirectDependencies(const CCopasiObject::DataObjectSet & directDependencies)
{
  mDependencies = directDependencies;
}

const CCopasiObject::DataObjectSet &
CCopasiObject::getDirectDependencies(const CCopasiObject::DataObjectSet & /* context */) const
{
  return mDependencies;
}

void CCopasiObject::addDirectDependency(const CCopasiObject * pObject)
{
  mDependencies.insert(pObject);
  return;
}

void CCopasiObject::removeDirectDependency(const CCopasiObject * pObject)
{
  mDependencies.erase(pObject);
  return;
}

const CObjectInterface::ObjectSet & CCopasiObject::getPrerequisites() const
{
  return * reinterpret_cast< const CObjectInterface::ObjectSet * >(&mDependencies);
}

bool CCopasiObject::isPrerequisiteForContext(const CObjectInterface * pObject,
    const CMath::SimulationContextFlag & /* context */,
    const CObjectInterface::ObjectSet & changedObjects) const
{
  // If the object is among the changed objects it does not depend on anything else.
  if (changedObjects.find(this) != changedObjects.end())
    return false;

#ifdef COPASI_DEBUG
  const CObjectInterface::ObjectSet & Prerequisites = getPrerequisites();

  // This method should only be called for objects which are prerequisites.
  // We check for this only in debug mode.
  assert(Prerequisites.find(pObject) != Prerequisites.end());
#endif // COPASI_DEBUG

  return true;
}

void CCopasiObject::getAllDependencies(CCopasiObject::DataObjectSet & dependencies,
                                       const CCopasiObject::DataObjectSet & context) const
{
  const CCopasiObject::DataObjectSet & DirectDependencies = getDirectDependencies(context);
  CCopasiObject::DataObjectSet::const_iterator it = DirectDependencies.begin();
  CCopasiObject::DataObjectSet::const_iterator end = DirectDependencies.end();

  std::pair<CCopasiObject::DataObjectSet::iterator, bool> Inserted;

  for (; it != end; ++it)
    {
      // Dual purpose insert
      Inserted = dependencies.insert(*it);

      // The direct dependency *it was among the dependencies
      // we assume also its dependencies have been added already.
      if (!Inserted.second) continue;

      // Add all the dependencies of the direct dependency *it.
      (*it)->getAllDependencies(dependencies, context);
    }
}

// virtual
bool CCopasiObject::mustBeDeleted(const CCopasiObject::DataObjectSet & deletedObjects) const
{
  DataObjectSet::const_iterator it = mDependencies.begin();
  DataObjectSet::const_iterator end = mDependencies.end();

  for (; it != end; ++it)
    {
      if (deletedObjects.find(*it) != deletedObjects.end())
        {
          return true;
        }
    }

  return deletedObjects.find(this) != deletedObjects.end();
}

bool CCopasiObject::dependsOn(CCopasiObject::DataObjectSet candidates,
                              const CCopasiObject::DataObjectSet & context) const
{
  CCopasiObject::DataObjectSet verified;
  return hasCircularDependencies(candidates, verified, context);
}

bool CCopasiObject::hasCircularDependencies(CCopasiObject::DataObjectSet & candidates,
    CCopasiObject::DataObjectSet & verified,
    const CCopasiObject::DataObjectSet & context) const
{
  bool hasCircularDependencies = false;

  if (verified.count(this) != 0)
    return hasCircularDependencies;

  const CCopasiObject::DataObjectSet & DirectDependencies = getDirectDependencies(context);
  CCopasiObject::DataObjectSet::const_iterator it = DirectDependencies.begin();
  CCopasiObject::DataObjectSet::const_iterator end = DirectDependencies.end();

  std::pair<CCopasiObject::DataObjectSet::iterator, bool> Inserted;

  // Dual purpose insert
  Inserted = candidates.insert(this);

  // Check whether the insert was successful, if not
  // the object "this" was among the candidates. Thus we have a detected
  // a circular dependency
  if (Inserted.second)
    {
      for (; it != end && !hasCircularDependencies; ++it)
        {
          hasCircularDependencies = (*it)->hasCircularDependencies(candidates, verified, context);
        }

      // Remove the inserted object this from the candidates to avoid any
      // side effects.
      candidates.erase(this);
    }
  else
    hasCircularDependencies = true;

  // The element has been checked and does not need to be checked again.
  verified.insert(this);

  return hasCircularDependencies;
}

void * CCopasiObject::getValuePointer() const
{
  return NULL;
}

// virtual
const CCopasiObject * CCopasiObject::getDataObject() const
{
  return this;
}

const CCopasiObject * CCopasiObject::getValueObject() const
{
  return NULL;
}

bool CCopasiObject::isContainer() const
{return (0 < (mObjectFlag & Container));}

bool CCopasiObject::isVector() const
{return (0 < (mObjectFlag & Vector));}

bool CCopasiObject::isMatrix() const
{return (0 < (mObjectFlag & Matrix));}

bool CCopasiObject::isNameVector() const
{return (0 < (mObjectFlag & NameVector));}

bool CCopasiObject::isReference() const
{return (0 < (mObjectFlag & Reference));}

bool CCopasiObject::isValueBool() const
{return (0 < (mObjectFlag & ValueBool));}

bool CCopasiObject::isValueInt() const
{return (0 < (mObjectFlag & ValueInt));}

bool CCopasiObject::isValueInt64() const
{return (0 < (mObjectFlag & ValueInt64));}

bool CCopasiObject::isValueDbl() const
{return (0 < (mObjectFlag & ValueDbl));}

bool CCopasiObject::isNonUniqueName() const
{return (0 < (mObjectFlag & NonUniqueName));}

bool CCopasiObject::isStaticString() const
{return (0 < (mObjectFlag & StaticString));}

bool CCopasiObject::isValueString() const
{return (0 < (mObjectFlag & ValueString));}

bool CCopasiObject::isSeparator() const
{return (0 < (mObjectFlag & Separator));}

bool CCopasiObject::isArray() const
{return (0 < (mObjectFlag & Array));}

bool CCopasiObject::isDataModel() const
{return (0 < (mObjectFlag & DataModel));}

bool CCopasiObject::isRoot() const
{return (0 < (mObjectFlag & Root));}

const std::string & CCopasiObject::getKey() const
{
  static std::string DefaultKey("");

  return DefaultKey;
}

// virtual
const std::string CCopasiObject::getUnits() const
{
  if (mpObjectParent != NULL)
    return mpObjectParent->getChildObjectUnits(this);

  return "?";
}

std::ostream &operator<<(std::ostream &os, const CCopasiObject & o)
{
  os << "Name:      " << o.getObjectDisplayName() << std::endl;
  os << "Type:      " << o.getObjectType() << std::endl;
  os << "Container: " << o.isContainer() << std::endl;
  os << "Vector:    " << o.isVector() << std::endl;
  os << "VectorN:   " << o.isNameVector() << std::endl;
  os << "Matrix:    " << o.isMatrix() << std::endl;
  os << "Reference: " << o.isReference() << std::endl;
  os << "Bool:      " << o.isValueBool() << std::endl;
  os << "Int:       " << o.isValueInt() << std::endl;
  os << "Dbl:       " << o.isValueDbl() << std::endl;

  return os;
}

/**
 * Returns a pointer to the CCopasiDataModel the element belongs to.
 * If there is no instance of CCopasiDataModel in the ancestor tree, NULL
 * is returned.
 */
CCopasiDataModel* CCopasiObject::getObjectDataModel() const
{
  const CCopasiObject * pObject = this;

  while (pObject != NULL)
    {
      if (pObject->isDataModel())
        return const_cast< CCopasiDataModel * >(static_cast<const CCopasiDataModel * >(pObject));

      pObject = pObject->getObjectParent();
    }

  return NULL;
}
