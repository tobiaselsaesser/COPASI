// Copyright (C) 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#include "copasi.h"

#include "RenderCubicBezierHandler.h"

/**
 * Replace RenderCubicBezier with the name type of the handler and implement the
 * three methods below.
 */
RenderCubicBezierHandler::RenderCubicBezierHandler(CXMLParser & parser, CXMLParserData & data):
  CXMLHandler(parser, data, CXMLHandler::RenderCubicBezier)
{
  init();
}

// virtual
RenderCubicBezierHandler::~RenderCubicBezierHandler()
{}

// virtual
CXMLHandler * RenderCubicBezierHandler::processStart(const XML_Char * pszName,
    const XML_Char ** papszAttrs)
{
  CXMLHandler * pHandlerToCall = NULL;

  // TODO CRITICAL Implement me!

  return pHandlerToCall;
}

// virtual
CXMLHandler * RenderCubicBezierHandler::processEnd(const XML_Char * pszName)
{
  CXMLHandler * pHandlerToCall = NULL;

  // TODO CRITICAL Implement me!

  return pHandlerToCall;
}

// virtual
CXMLHandler::sProcessLogic * RenderCubicBezierHandler::getProcessLogic() const
{
  // TODO CRITICAL Implement me!

  static sProcessLogic Elements[] =
  {
    {"RenderCubicBezier", RenderCubicBezier, {BEFORE}},
    {"BEFORE", BEFORE, {RenderCubicBezier, BEFORE}}
  };

  return Elements;
}
