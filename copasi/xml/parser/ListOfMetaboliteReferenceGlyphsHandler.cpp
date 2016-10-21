// Copyright (C) 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#include "copasi.h"

#include "ListOfMetaboliteReferenceGlyphsHandler.h"
#include "CXMLParser.h"
#include "utilities/CCopasiMessage.h"

/**
 * Replace ListOfMetaboliteReferenceGlyphs with the name type of the handler and implement the
 * three methods below.
 */
ListOfMetaboliteReferenceGlyphsHandler::ListOfMetaboliteReferenceGlyphsHandler(CXMLParser & parser, CXMLParserData & data):
  CXMLHandler(parser, data, CXMLHandler::ListOfMetaboliteReferenceGlyphs)
{
  init();
}

// virtual
ListOfMetaboliteReferenceGlyphsHandler::~ListOfMetaboliteReferenceGlyphsHandler()
{}

// virtual
CXMLHandler * ListOfMetaboliteReferenceGlyphsHandler::processStart(const XML_Char * pszName,
    const XML_Char ** papszAttrs)
{
  CXMLHandler * pHandlerToCall = NULL;

  switch (mCurrentElement)
    {
      case ListOfMetaboliteReferenceGlyphs:
        // TODO CRITICAL Implement me!
        break;

        // TODO CRITICAL Implement me!

      default:
        CCopasiMessage(CCopasiMessage::EXCEPTION, MCXML + 2,
                       mpParser->getCurrentLineNumber(), mpParser->getCurrentColumnNumber(), pszName);
        break;
    }

  return pHandlerToCall;
}

// virtual
bool ListOfMetaboliteReferenceGlyphsHandler::processEnd(const XML_Char * pszName)
{
  bool finished = false;

  switch (mCurrentElement)
    {
      case ListOfMetaboliteReferenceGlyphs:
        finished = true;
        // TODO CRITICAL Implement me!
        break;

        // TODO CRITICAL Implement me!

      default:
        CCopasiMessage(CCopasiMessage::EXCEPTION, MCXML + 2,
                       mpParser->getCurrentLineNumber(), mpParser->getCurrentColumnNumber(), pszName);
        break;
    }

  return finished;
}

// virtual
CXMLHandler::sProcessLogic * ListOfMetaboliteReferenceGlyphsHandler::getProcessLogic() const
{
  // TODO CRITICAL Implement me!

  static sProcessLogic Elements[] =
  {
    {"BEFORE", BEFORE, {ListOfMetaboliteReferenceGlyphs, HANDLER_COUNT}},
    {"ListOfMetaboliteReferenceGlyphs", ListOfMetaboliteReferenceGlyphs, {AFTER, HANDLER_COUNT}},
    {"AFTER", AFTER, {HANDLER_COUNT}}
  };

  return Elements;
}
