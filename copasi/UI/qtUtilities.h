// Copyright (C) 2010 - 2014 by Pedro Mendes, Virginia Tech Intellectual
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

#ifndef COPASI_qtUtilities
#define COPASI_qtUtilities

#include <QtCore/QString>
#include "utilities/CCopasiParameter.h"

class QStringList;
class CCopasiParameterGroup;
class CopasiWidget;
class QObject;

/**
 * Retrieve a parameter from a parameter group and convert it to a QString.
 * @param const CCopasiParameterGroup * group
 * @param const size_t & index
 * @param CCopasiParameter::Type * type (default: NULL)
 * @return QString value
 */
QString getParameterValue(const CCopasiParameterGroup * group,
                          const size_t & index,
                          CCopasiParameter::Type * type = NULL);
/**
 * Retrieve a parameter from a parameter group and convert it to a QString.
 * @param const CCopasiParameterGroup * group
 * @param const std::string & name
 * @param CCopasiParameter::Type * type (default: NULL)
 * @return QString value
 */
QString getParameterValue(const CCopasiParameterGroup * group,
                          const std::string & name,
                          CCopasiParameter::Type * type = NULL);
/**
 * Set a parameter of a parameter group from a value given as a QString.
 * Success = true is returned when the set operation was succesful, i.e.,
 * value == getParameterValue.
 * @param CCopasiParameterGroup * group
 * @param const size_t & index
 * @param const QString & value
 * @return bool success.
 */
bool setParameterValue(CCopasiParameterGroup * group,
                       const size_t & index,
                       const QString & value);

/**
 * Set a parameter of a parameter group from a value given as a QString.
 * Success = true is returned when the set operation was succesful, i.e.,
 * value == getParameterValue.
 * @param CCopasiParameterGroup * group
 * @param const std::string & name
 * @param const QString & value
 * @return bool success.
 */
bool setParameterValue(CCopasiParameterGroup * group,
                       const std::string & name,
                       const QString & value);

/**
 * Convert a utf8 encoded std::string to a QString
 */
#define FROM_UTF8(__x) QString::fromUtf8((__x).c_str())
#define TO_UTF8(__x) (__x).toUtf8().data()

/**
 * Retrieve the COPASI data model the displayed data is associated with
 * @param QObject * pObject
 * @return const CopasiWidget * pCopasiWidget
 */
const CopasiWidget * GetCopasiWidget(const QObject * pObject);

/**
 * Checks whether the file exists. If the file exist the user
 * is prompted whether to overwrite the file. The return value is
 * one of QMessageBox::Yes, QMessageBox::No, or QMessageBox::Cancel.
 * If the file does not exist QMessageBox::Yes is returned.
 * param const QString & file
 * @return C_INT32 overwrite
 */
C_INT32 checkSelection(const QString & file);

/**
 * This converts a std::vector<std::string> into a QStringList
 */
void vectorOfStrings2QStringList(std::vector<std::string> vs, QStringList & qsl);

#endif // COPASI_qtUtilities
