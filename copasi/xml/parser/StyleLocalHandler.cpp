// Copyright (C) 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#include "copasi.h"

#include "StyleLocalHandler.h"

/**
 * Replace StyleLocal with the name type of the handler and implement the
 * three methods below.
 */
StyleLocalHandler::StyleLocalHandler(CXMLParser & parser, CXMLParserData & data):
  CXMLHandler(parser, data, CXMLHandler::StyleLocal)
{
  init();
}

// virtual
StyleLocalHandler::~StyleLocalHandler()
{}

// virtual
CXMLHandler * StyleLocalHandler::processStart(const XML_Char * pszName,
    const XML_Char ** papszAttrs)
{
  CXMLHandler * pHandlerToCall = NULL;

  // TODO CRITICAL Implement me!

  return pHandlerToCall;
}

// virtual
CXMLHandler * StyleLocalHandler::processEnd(const XML_Char * pszName)
{
  CXMLHandler * pHandlerToCall = NULL;

  // TODO CRITICAL Implement me!

  return pHandlerToCall;
}

// virtual
CXMLHandler::sProcessLogic * StyleLocalHandler::getProcessLogic() const
{
  // TODO CRITICAL Implement me!

  static sProcessLogic Elements[] =
  {
    {"StyleLocal", StyleLocal, {BEFORE}},
    {"BEFORE", BEFORE, {StyleLocal, BEFORE}}
  };

  return Elements;
}
