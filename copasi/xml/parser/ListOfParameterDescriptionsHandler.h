// Copyright (C) 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#ifndef COPASI_ListOfParameterDescriptionsHandler
#define COPASI_ListOfParameterDescriptionsHandler

#include "copasi/xml/parser/CXMLHandler.h"

class ListOfParameterDescriptionsHandler : public CXMLHandler
{
private:
  ListOfParameterDescriptionsHandler();

public:
  /**
   * Constructor
   * @param CXMLParser & parser
   * @param CXMLParserData & data
   */
  ListOfParameterDescriptionsHandler(CXMLParser & parser, CXMLParserData & data);

  /**
   * Destructor
   */
  virtual ~ListOfParameterDescriptionsHandler();

protected:

  /**
   * Process the start of an element
   * @param const XML_Char *pszName
   * @param const XML_Char **papszAttrs
   * @return CXMLHandler * pHandlerToCall
   */
  virtual CXMLHandler * processStart(const XML_Char * pszName,
                                     const XML_Char ** papszAttrs);

  /**
   * Process the end of an element
   * @param const XML_Char *pszName
   * @return CXMLHandler * pHandlerToCall
   */
  virtual CXMLHandler * processEnd(const XML_Char * pszName);

  /**
   * Retrieve the structure containing the process logic for the handler.
   * @return sElementInfo *
   */
  virtual sProcessLogic * getProcessLogic() const;
};

#endif //COPASI_ListOfParameterDescriptionsHandler
