// Copyright (C) 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#include "copasi.h"

#include "BoundingBoxHandler.h"

/**
 * Replace BoundingBox with the name type of the handler and implement the
 * three methods below.
 */
BoundingBoxHandler::BoundingBoxHandler(CXMLParser & parser, CXMLParserData & data):
  CXMLHandler(parser, data, CXMLHandler::BoundingBox)
{
  init();
}

// virtual
BoundingBoxHandler::~BoundingBoxHandler()
{}

// virtual
CXMLHandler * BoundingBoxHandler::processStart(const XML_Char * pszName,
    const XML_Char ** papszAttrs)
{
  CXMLHandler * pHandlerToCall = NULL;

  // TODO CRITICAL Implement me!

  return pHandlerToCall;
}

// virtual
CXMLHandler * BoundingBoxHandler::processEnd(const XML_Char * pszName)
{
  CXMLHandler * pHandlerToCall = NULL;

  // TODO CRITICAL Implement me!

  return pHandlerToCall;
}

// virtual
CXMLHandler::sProcessLogic * BoundingBoxHandler::getProcessLogic() const
{
  // TODO CRITICAL Implement me!

  static sProcessLogic Elements[] =
  {
    {"BoundingBox", BoundingBox, {BEFORE}},
    {"BEFORE", BEFORE, {BoundingBox, BEFORE}}
  };

  return Elements;
}
