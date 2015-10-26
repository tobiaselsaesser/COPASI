// Copyright (C) 2014 - 2015 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

/*
 * UndoData.h
 *
 *  Created on: 15 Aug 2014
 *      Author: dada
 */

#ifndef UNDODATA_H_
#define UNDODATA_H_

#include <QObject>
#include <string>

class CModel;
class UndoGlobalQuantityData;
class UndoReactionData;
class UndoEventData;
class UndoSpeciesData;
class UndoCompartmentData;
/**
 * Base class for all COPASI undo data
 */
class UndoData : public QObject
{
  Q_OBJECT

public:
  UndoData(const std::string &key = "",
           const std::string &name = "",
           const std::string &type = "");

  virtual ~UndoData();

  /**
   * when overidden in subclasses this function
   * will restore dependent objects.
   */
  virtual void restoreDependentObjects(CModel* pModel);

  /**
   * @return the key of the object
   */
  const std::string &getKey() const;

  /**
   * sets the key for the object
   * @param key the key
   */
  void setKey(const std::string &key);

  /**
   * @return true if key is set, false otherwise.
   */
  bool hasKey() const;

  /**
   * @return the name of the object
   */
  const std::string &getName() const;

  /**
   * sets the name
   * @param name the name
   */
  void setName(const std::string & name);

  /**
   * @return trye if name is set, false otherwise
   */
  bool hasName() const;

  /**
   * @return the type of the object
   */
  const std::string &getType() const;

  /**
   * @param type the type
   */
  void setType(const std::string &type);

  static void restoreDependentObjects(CModel* pModel,
                                      QList <UndoGlobalQuantityData *> *pGlobalQuantityData);
  static void restoreDependentObjects(CModel* pModel,
                                      QList <UndoReactionData *> *pReactionData);
  static void restoreDependentObjects(CModel* pModel,
                                      QList <UndoEventData *> *pEventData);
  static void restoreDependentObjects(CModel* pModel,
                                      QList <UndoSpeciesData *> *pSpeciesData);
  static void restoreDependentObjects(CModel* pModel,
                                      QList <UndoCompartmentData *> *pCompartmentData);

private:
  std::string mKey;
  std::string mName;
  std::string mType;
};

#endif /* UNDODATA_H_ */
