// Copyright (C) 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#include "copasi.h"

#include "COPASIHandler.h"

#include "CXMLParser.h"
#include "utilities/CVersion.h"
#include "utilities/CCopasiParameter.h"
#include "report/CCopasiRootContainer.h"
#include "function/CFunction.h"

COPASIHandler::COPASIHandler(CXMLParser & parser, CXMLParserData & data):
  CXMLHandler(parser, data, CXMLHandler::COPASI)
{
  init();
}

// virtual
COPASIHandler::~COPASIHandler()
{}

// virtual
CXMLHandler * COPASIHandler::processStart(const XML_Char * pszName,
    const XML_Char ** papszAttrs)
{
  CXMLHandler * pHandlerToCall = NULL;

  const char * versionMajor;
  C_INT32 VersionMajor;
  const char * versionMinor;
  C_INT32 VersionMinor;
  const char * versionDevel;
  C_INT32 VersionDevel;
  bool CopasiSourcesModified = true;

  switch (mCurrentElement)
    {
      case COPASI:
        versionMajor = mpParser->getAttributeValue("versionMajor", papszAttrs, "0");
        VersionMajor = strToInt(versionMajor);
        versionMinor = mpParser->getAttributeValue("versionMinor", papszAttrs, "0");
        VersionMinor = strToInt(versionMinor);
        versionDevel = mpParser->getAttributeValue("versionDevel", papszAttrs, "0");
        VersionDevel = strToInt(versionDevel);
        CopasiSourcesModified = mpParser->toBool(mpParser->getAttributeValue("copasiSourcesModified", papszAttrs, "true"));

        mpData->pVersion->setVersion(VersionMajor, VersionMinor, VersionDevel, CopasiSourcesModified);
        break;

      case ParameterGroup:
      case ListOfFunctions:
      case Model:
      case ListOfTasks:
      case ListOfReports:
      case ListOfPlots:
      case ListOfLayouts:
      case SBMLReference:
      case ListOfUnitDefinitions:
        pHandlerToCall = mpParser->getHandler(mCurrentElement);

        break;

      case GUI:
        if (!mpData->pGUI)
          mCurrentElement = UNKNOWN;

        pHandlerToCall = mpParser->getHandler(mCurrentElement);

        break;

      default:
        fatalError();
        break;
    }

  return pHandlerToCall;
}

// virtual
CXMLHandler * COPASIHandler::processEnd(const XML_Char * pszName)
{
  CXMLHandler * pHandlerToCall = NULL;

  switch (mCurrentElement)
    {
      case COPASI:
      {
        // We need to handle the unmapped parameters of type key.
        std::vector< std::string >::iterator it = mpData->UnmappedKeyParameters.begin();
        std::vector< std::string >::iterator end = mpData->UnmappedKeyParameters.end();

        for (; it != end; ++it)
          {
            CCopasiParameter * pParameter =
              dynamic_cast< CCopasiParameter * >(CCopasiRootContainer::getKeyFactory()->get(*it));

            if (pParameter != NULL &&
                pParameter->getType() == CCopasiParameter::KEY)
              {
                CCopasiObject * pObject =
                  mpData->mpKeyMap->get(pParameter->getValue< std::string >());

                if (pObject != NULL)
                  pParameter->setValue(pObject->getKey());
                else
                  pParameter->setValue(std::string(""));
              }
          }

        // We need to remove the no longer needed expression "Objective Function" from the function list.
        if (mpData->pFunctionList != NULL &&
            mpData->pFunctionList->getIndex("Objective Function") != C_INVALID_INDEX)
          {
            mpData->pFunctionList->remove("Objective Function");
          }
      }
      break;

      case GUI:
        if (mpData->pGUI == NULL)
          {
            CCopasiMessage::getLastMessage();
          }

        break;

      default:
        break;
    }

  return pHandlerToCall;
}

// virtual
CXMLHandler::sProcessLogic * COPASIHandler::getProcessLogic() const
{
  static sProcessLogic Elements[] =
  {
    {"COPASI", COPASI, {ListOfFunctions, Model, ListOfTasks, ListOfReports, ListOfPlots, GUI, ListOfLayouts, SBMLReference, ListOfUnitDefinitions, BEFORE}},
    {"ListOfFunctions", ListOfFunctions, {Model, ListOfTasks, ListOfReports, ListOfPlots, GUI, ListOfLayouts, SBMLReference, ListOfUnitDefinitions, BEFORE}},
    {"Model", Model, {ListOfTasks, ListOfReports, ListOfPlots, GUI, ListOfLayouts, SBMLReference, ListOfUnitDefinitions, BEFORE}},
    {"ListOfTasks", ListOfTasks, {ListOfReports, ListOfPlots, GUI, ListOfLayouts, SBMLReference, ListOfUnitDefinitions, BEFORE}},
    {"ListOfReports", ListOfReports, {ListOfPlots, GUI, ListOfLayouts, SBMLReference, ListOfUnitDefinitions, BEFORE}},
    {"ListOfPlots", ListOfPlots, {GUI, ListOfLayouts, SBMLReference, ListOfUnitDefinitions, BEFORE}},
    {"GUI", GUI, {ListOfLayouts, SBMLReference, ListOfUnitDefinitions, BEFORE}},
    {"ListOfLayouts", ListOfLayouts, {SBMLReference, ListOfUnitDefinitions, BEFORE}},
    {"SBMLReference", SBMLReference, {ListOfUnitDefinitions, BEFORE}},
    {"ListOfUnitDefinitions", ListOfUnitDefinitions, {BEFORE}},
    {"ParameterGroup", ParameterGroup, {BEFORE}},
    {"BEFORE", BEFORE, {COPASI, ParameterGroup, BEFORE}}
  };

  return Elements;
}
