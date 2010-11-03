// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/layout/CLGlobalRenderInformation.cpp,v $
//   $Revision: 1.3.2.1 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2010/11/03 17:08:15 $
// End CVS Header

// Copyright (C) 2010 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#include <sstream>
#include <assert.h>

#define USE_LAYOUT 1

#ifdef USE_CRENDER_EXTENSION
#define USE_RENDER 1
#endif // USE_CRENDER_EXTENSION

#include <sbml/layout/render/GlobalRenderInformation.h>

#include "CLGlobalRenderInformation.h"

#include <copasi/report/CCopasiRootContainer.h>
#include <copasi/report/CKeyFactory.h>

/**
 *  Constructor.
 */
CLGlobalRenderInformation::CLGlobalRenderInformation(CCopasiContainer* pParent):
    CLRenderInformationBase("GlobalRenderInformation", pParent)
{
  this->mKey = CCopasiRootContainer::getKeyFactory()->add("GlobalRenderInformation", this);
}

/**
 * Copy constructor
 */
CLGlobalRenderInformation::CLGlobalRenderInformation(const CLGlobalRenderInformation& source, CCopasiContainer* pParent):
    CLRenderInformationBase(source, pParent),
    mListOfStyles(source.mListOfStyles, this)
{
  this->mKey = CCopasiRootContainer::getKeyFactory()->add("GlobalRenderInformation", this);
}

/**
 * Constructor to generate object from the corresponding SBML object.
 */
CLGlobalRenderInformation::CLGlobalRenderInformation(const GlobalRenderInformation& source,
    /*
            std::map<std::string,std::string>& colorIdToKeyMap,
            std::map<std::string,std::string>& gradientIdToKeyMap,
            std::map<std::string,std::string>& lineEndingIdToKeyMap,
      */
    CCopasiContainer* pParent):
    //CLRenderInformationBase(source,"GlobalRenderInformation",colorIdToKeyMap,gradientIdToKeyMap,lineEndingIdToKeyMap,pParent)
    CLRenderInformationBase(source, "GlobalRenderInformation", pParent)
{
  this->mKey = CCopasiRootContainer::getKeyFactory()->add("GlobalRenderInformation", this);
  unsigned int i, iMax = source.getNumStyles();

  for (i = 0; i < iMax; ++i)
    {
      this->mListOfStyles.add(new CLGlobalStyle(*static_cast<const GlobalStyle*>(source.getStyle(i))), true);
    }
}

/**
 * Returns the number of styles.
 */
unsigned int CLGlobalRenderInformation::getNumStyles() const
{
  return this->mListOfStyles.size();
}

/**
 * Returns a pointer to the LitOfStyles object.
 */
CCopasiVector<CLGlobalStyle>* CLGlobalRenderInformation::getListOfStyles()
{
  return &(this->mListOfStyles);
}

/**
 * Returns a pointer to the ListOfStyles object.
 */
const CCopasiVector<CLGlobalStyle>* CLGlobalRenderInformation::getListOfStyles() const
{
  return &(this->mListOfStyles);
}

/**
 * Returns a pointer to the style with the given index.
 * If the index is invalid, NULL is returned.
 */
CLStyle* CLGlobalRenderInformation::getStyle(unsigned int i)
{
  return (i < this->mListOfStyles.size()) ? this->mListOfStyles[i] : NULL;
}

/**
 * Returns a pointer to the style with the given index.
 * If the index is invalid, NULL is returned.
 */
const CLStyle* CLGlobalRenderInformation::getStyle(unsigned int i) const
{
  return (i < this->mListOfStyles.size()) ? this->mListOfStyles[i] : NULL;
}

CLGlobalStyle* CLGlobalRenderInformation::createStyle()
{
  CLGlobalStyle* pStyle = new CLGlobalStyle();
  this->mListOfStyles.add(pStyle, true);
  return pStyle;
}

void CLGlobalRenderInformation::addStyle(const CLGlobalStyle* pStyle)
{
  this->mListOfStyles.add(new CLGlobalStyle(*pStyle), true);
}

/**
 * Converts this object to the corresponding SBML object.
 *
 */
GlobalRenderInformation* CLGlobalRenderInformation::toSBML(unsigned int level, unsigned int version) const
{
  GlobalRenderInformation* pLRI = new GlobalRenderInformation(level, version);
  //this->addSBMLAttributes(pLRI,colorKeyToIdMap,gradientKeyToIdMap,lineEndingKeyToIdMap);
  this->addSBMLAttributes(pLRI);
  unsigned int i, iMax = this->mListOfStyles.size();

  for (i = 0; i < iMax; ++i)
    {
      GlobalStyle* pStyle = static_cast<const CLGlobalStyle*>(this->getStyle(i))->toSBML(level, version);
      pLRI->addStyle(pStyle);
      delete pStyle;
    }

  return pLRI;
}
