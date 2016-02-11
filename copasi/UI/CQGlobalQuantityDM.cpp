// Copyright (C) 2010 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

#include <QtCore/QString>

#include "CopasiDataModel/CCopasiDataModel.h"
#include "report/CCopasiRootContainer.h"
#include "model/CModelValue.h"
#include "function/CExpression.h"

#include "CQMessageBox.h"
#include "CQGlobalQuantityDM.h"
#include "qtUtilities.h"

#include "model/CReaction.h"
#include "model/CReactionInterface.h"
#include "undoFramework/InsertGlobalQuantityRowsCommand.h"
#include "undoFramework/RemoveGlobalQuantityRowsCommand.h"
#include "undoFramework/RemoveAllGlobalQuantityRowsCommand.h"
#include "undoFramework/GlobalQuantityDataChangeCommand.h"
#include "undoFramework/UndoGlobalQuantityData.h"
#include "undoFramework/UndoReactionData.h"
#include "undoFramework/UndoSpeciesData.h"
#include "undoFramework/UndoEventData.h"
#include "undoFramework/UndoEventAssignmentData.h"
#include <copasi/UI/CQCopasiApplication.h>

CQGlobalQuantityDM::CQGlobalQuantityDM(QObject *parent)
  : CQBaseDataModel(parent)

{
  mTypes.push_back(FROM_UTF8(CModelEntity::StatusName[CModelEntity::FIXED]));
  mTypes.push_back(FROM_UTF8(CModelEntity::StatusName[CModelEntity::ASSIGNMENT]));
  mTypes.push_back(FROM_UTF8(CModelEntity::StatusName[CModelEntity::ODE]));

  mItemToType.push_back(CModelEntity::FIXED);
  mItemToType.push_back(CModelEntity::ASSIGNMENT);
  mItemToType.push_back(CModelEntity::ODE);
}

const QString & CQGlobalQuantityDM::indexToStatus(int index) const
{
  return mTypes[index];
}

int CQGlobalQuantityDM::statusToIndex(const QString& status) const
{
  return mTypes.indexOf(status);
}

const QStringList& CQGlobalQuantityDM::getTypes()
{
  return mTypes;
}

const std::vector< unsigned C_INT32 >& CQGlobalQuantityDM::getItemToType()
{
  return mItemToType;
}

int CQGlobalQuantityDM::rowCount(const QModelIndex& C_UNUSED(parent)) const
{
  return CCopasiRootContainer::getDatamodelList()->operator[](0).getModel()->getModelValues().size() + 1;
}
int CQGlobalQuantityDM::columnCount(const QModelIndex& C_UNUSED(parent)) const
{
  return TOTAL_COLS_GQ;
}

Qt::ItemFlags CQGlobalQuantityDM::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return Qt::ItemIsEnabled;

  if (index.column() == COL_NAME_GQ || index.column() == COL_TYPE_GQ)
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
  else if (index.column() == COL_INITIAL_GQ)
    {
      if (this->index(index.row(), COL_TYPE_GQ).data() == QString(FROM_UTF8(CModelEntity::StatusName[CModelEntity::ASSIGNMENT])))
        return QAbstractItemModel::flags(index) & ~Qt::ItemIsEnabled;
      else
        return QAbstractItemModel::flags(index)  | Qt::ItemIsEditable | Qt::ItemIsEnabled;
    }
  else
    return QAbstractItemModel::flags(index);
}

QVariant CQGlobalQuantityDM::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (index.row() >= rowCount())
    return QVariant();

  if (index.column() > 0 && role == Qt::ForegroundRole && !(flags(index) & Qt::ItemIsEditable))
    return QColor(Qt::darkGray);

  if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
      if (isDefaultRow(index))
        {
          switch (index.column())
            {
              case COL_ROW_NUMBER:
                return QVariant(QString(""));

              case COL_NAME_GQ:
                return QVariant(QString("New Quantity"));

              case COL_TYPE_GQ:
                return QVariant(QString(FROM_UTF8(CModelEntity::StatusName[mItemToType[0]])));

              case COL_INITIAL_GQ:
                return QVariant(QString::number(0.0, 'g', 10));

              default:
                return QVariant(QString(""));
            }
        }
      else
        {
          CModelValue *pGQ = &CCopasiRootContainer::getDatamodelList()->operator[](0).getModel()->getModelValues()[index.row()];
          const CExpression * pExpression = NULL;

          switch (index.column())
            {
              case COL_ROW_NUMBER:
                return QVariant(index.row() + 1);

              case COL_NAME_GQ:
                return QVariant(QString(FROM_UTF8(pGQ->getObjectName())));

              case COL_UNIT_GQ:
                return QVariant(QString(FROM_UTF8(pGQ->getUnitExpression())));

              case COL_TYPE_GQ:
                return QVariant(QString(FROM_UTF8(CModelEntity::StatusName[pGQ->getStatus()])));

              case COL_INITIAL_GQ:

                if (role == Qt::EditRole)
                  return QVariant(QString::number(pGQ->getInitialValue(), 'g', 10));
                else
                  return QVariant(pGQ->getInitialValue());

              case COL_TRANSIENT_GQ:
                return QVariant(pGQ->getValue());

              case COL_RATE_GQ:
                return QVariant(pGQ->getRate());

              case COL_IEXPRESSION_GQ:

                if (pGQ->getInitialExpression() != "")
                  {
                    pExpression = pGQ->getInitialExpressionPtr();

                    if (pExpression != NULL)
                      return QVariant(QString(FROM_UTF8(pExpression->getDisplayString())));
                    else
                      return QVariant();
                  }

                break;

              case COL_EXPRESSION_GQ:
              {
                pExpression = pGQ->getExpressionPtr();

                if (pExpression != NULL)
                  return QVariant(QString(FROM_UTF8(pExpression->getDisplayString())));
                else
                  return QVariant();
              }
            }
        }
    }

  return QVariant();
}

QVariant CQGlobalQuantityDM::headerData(int section, Qt::Orientation orientation,
                                        int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal)
    {
      switch (section)
        {
          case COL_ROW_NUMBER:
            return QVariant(QString("#"));

          case COL_NAME_GQ:
            return QVariant(QString("Name"));

          case COL_UNIT_GQ:
            return QVariant(QString("Unit"));

          case COL_TYPE_GQ:
            return QVariant(QString("     Type     "));

          case COL_INITIAL_GQ:
            return QVariant("Initial Value");

          case COL_TRANSIENT_GQ:
            return QVariant("Transient Value");

          case COL_RATE_GQ:
            return QVariant("Rate");

          case COL_IEXPRESSION_GQ:
            return QVariant("Initial Expression");

          case COL_EXPRESSION_GQ:
            return QVariant("Expression");

          default:
            return QVariant();
        }
    }
  else  //Vertical header
    return QString("%1").arg(section + 1);
}

bool CQGlobalQuantityDM::setData(const QModelIndex &index, const QVariant &value,
                                 int role)
{
  if (index.data() == value)
    return false;

  if (index.column() == COL_TYPE_GQ && index.data().toString() == QString(FROM_UTF8(CModelEntity::StatusName[mItemToType[value.toInt()]])))
    return false;

  bool defaultRow = isDefaultRow(index);

  if (defaultRow)
    {
      mpUndoStack->push(new InsertGlobalQuantityRowsCommand(rowCount(), 1, this, index, value));
    }
  else
    {
      mpUndoStack->push(new GlobalQuantityDataChangeCommand(index, value, role, this));
    }

  return true;
}

bool CQGlobalQuantityDM::insertRows(int position, int rows, const QModelIndex&)
{
  mpUndoStack->push(new InsertGlobalQuantityRowsCommand(position, rows, this));

  return true;
}

bool CQGlobalQuantityDM::removeRows(int position, int rows)
{
  if (rows <= 0)
    return true;

  beginRemoveRows(QModelIndex(), position, position + rows - 1);

  CModel * pModel = CCopasiRootContainer::getDatamodelList()->operator[](0).getModel();

  std::vector< std::string > DeletedKeys;
  DeletedKeys.resize(rows);

  std::vector< std::string >::iterator itDeletedKey;
  std::vector< std::string >::iterator endDeletedKey = DeletedKeys.end();

  CCopasiVector< CModelValue >::const_iterator itRow = pModel->getModelValues().begin() + position;

  for (itDeletedKey = DeletedKeys.begin(); itDeletedKey != endDeletedKey; ++itDeletedKey, ++itRow)
    {
      *itDeletedKey = itRow->getKey();
    }

  for (itDeletedKey = DeletedKeys.begin(); itDeletedKey != endDeletedKey; ++itDeletedKey)
    {
      pModel->removeModelValue(*itDeletedKey);
      emit notifyGUI(ListViews::MODELVALUE, ListViews::DELETE, *itDeletedKey);
      emit notifyGUI(ListViews::MODELVALUE, ListViews::DELETE, ""); //Refresh all as there may be dependencies.
    }

  endRemoveRows();

  return true;
}

bool CQGlobalQuantityDM::removeRows(QModelIndexList rows, const QModelIndex&)
{
  mpUndoStack->push(new RemoveGlobalQuantityRowsCommand(rows, this, QModelIndex()));

  return true;
}

bool CQGlobalQuantityDM::globalQuantityDataChange(const QModelIndex &index, const QVariant &value, int role)
{
  if (!index.isValid() || role != Qt::EditRole)
    return false;

  bool defaultRow = isDefaultRow(index);

  if (defaultRow)
    {
      if (index.column() == COL_TYPE_GQ)
        {
          if (index.data().toString() != QString(FROM_UTF8(CModelEntity::StatusName[mItemToType[value.toInt()]])))
            insertRow(rowCount(), index);
          else
            return false;
        }
      else if (index.data() != value)
        insertRow(rowCount(), index);
      else
        return false;
    }

  GET_MODEL_OR(pModel, return false);

  switchToWidget(CCopasiUndoCommand::GLOBALQUANTITYIES);

  CModelValue *pGQ = &pModel->getModelValues()[index.row()];

  if (index.column() == COL_NAME_GQ)
    pGQ->setObjectName(TO_UTF8(value.toString()));
  else if (index.column() == COL_TYPE_GQ)
    pGQ->setStatus((CModelEntity::Status) mItemToType[value.toInt()]);
  else if (index.column() == COL_INITIAL_GQ)
    pGQ->setInitialValue(value.toDouble());

  if (defaultRow && this->index(index.row(), COL_NAME_GQ).data().toString() == "quantity")
    pGQ->setObjectName(TO_UTF8(createNewName("quantity", COL_NAME_GQ)));

  emit dataChanged(index, index);
  emit notifyGUI(ListViews::MODELVALUE, ListViews::CHANGE, pGQ->getKey());

  return true;
}

void CQGlobalQuantityDM::insertNewGlobalQuantityRow(int position, int rows, const QModelIndex& index, const QVariant& value)
{
  GET_MODEL_OR_RETURN(pModel);

  beginInsertRows(QModelIndex(), position, position + rows - 1);

  int column = index.column();

  for (int row = 0; row < rows; ++row)
    {
      QString name = createNewName(index.isValid() && column == COL_NAME_GQ ? value.toString() : "quantity", COL_NAME_GQ);

      double initial = index.isValid() && column == COL_INITIAL_GQ ? value.toDouble() : 0.0;

      CModelValue *pGQ = pModel->createModelValue(TO_UTF8(name), initial);

      if (pGQ == NULL) continue;

      if (index.isValid() && column == COL_TYPE_GQ)
        {
          pGQ->setStatus((CModelEntity::Status) mItemToType[value.toInt()]);
        }

      emit notifyGUI(ListViews::MODELVALUE, ListViews::ADD, pGQ->getKey());
    }

  endInsertRows();
}

void CQGlobalQuantityDM::deleteGlobalQuantityRow(UndoGlobalQuantityData *pGlobalQuantityData)
{
  GET_MODEL_OR_RETURN(pModel);

  switchToWidget(CCopasiUndoCommand::GLOBALQUANTITYIES);

  size_t index = pModel->getModelValues().getIndex(pGlobalQuantityData->getName());

  if (index == C_INVALID_INDEX)
    return;

  removeRow((int) index);
}

void CQGlobalQuantityDM::addGlobalQuantityRow(UndoGlobalQuantityData *pGlobalQuantityData)
{
  GET_MODEL_OR_RETURN(pModel);

  switchToWidget(CCopasiUndoCommand::GLOBALQUANTITYIES);

  beginInsertRows(QModelIndex(), 1, 1);
  CModelValue *pGlobalQuantity = pGlobalQuantityData->restoreObjectIn(pModel);

  if (pGlobalQuantity != NULL)
    emit notifyGUI(ListViews::MODELVALUE, ListViews::ADD, pGlobalQuantity->getKey());

  endInsertRows();
}

bool CQGlobalQuantityDM::removeGlobalQuantityRows(QModelIndexList rows, const QModelIndex&)
{
  if (rows.isEmpty())
    return false;

  GET_MODEL_OR(pModel, return false);

  switchToWidget(CCopasiUndoCommand::GLOBALQUANTITYIES);

  //Build the list of pointers to items to be deleted
  //before actually deleting any item.
  QList <CModelValue *> pGlobalQuantities;
  QModelIndexList::const_iterator i;

  for (i = rows.begin(); i != rows.end(); ++i)
    {
      if (!isDefaultRow(*i) && &pModel->getModelValues()[i->row()])
        pGlobalQuantities.append(&pModel->getModelValues()[i->row()]);
    }

  QList <CModelValue *>::const_iterator j;

  for (j = pGlobalQuantities.begin(); j != pGlobalQuantities.end(); ++j)
    {
      CModelValue * pGQ = *j;

      size_t delRow =
        pModel->getModelValues().CCopasiVector< CModelValue >::getIndex(pGQ);

      if (delRow == C_INVALID_INDEX)
        continue;

      QMessageBox::StandardButton choice =
        CQMessageBox::confirmDelete(NULL, "quantity",
                                    FROM_UTF8(pGQ->getObjectName()),
                                    pGQ->getDeletedObjects());

      if (choice == QMessageBox::Ok)
        removeRow((int) delRow);
    }

  return true;
}

bool CQGlobalQuantityDM::insertGlobalQuantityRows(QList <UndoGlobalQuantityData *>& pData)
{
  GET_MODEL_OR(pModel, return false);

  //reinsert all the GlobalQuantities
  QList <UndoGlobalQuantityData *>::const_iterator i;

  for (i = pData.begin(); i != pData.end(); ++i)
    {
      UndoGlobalQuantityData * data = *i;

      if (pModel->getModelValues().getIndex(data->getName()) != C_INVALID_INDEX)
        continue;

      beginInsertRows(QModelIndex(), 1, 1);
      CModelValue *pGlobalQuantity = data->restoreObjectIn(pModel);

      if (pGlobalQuantity != NULL)
        emit notifyGUI(ListViews::MODELVALUE, ListViews::ADD, pGlobalQuantity->getKey());

      endInsertRows();
    }

  switchToWidget(CCopasiUndoCommand::GLOBALQUANTITYIES);

  return true;
}

void CQGlobalQuantityDM::deleteGlobalQuantityRows(QList <UndoGlobalQuantityData *>& pData)
{
  GET_MODEL_OR_RETURN(pModel);

  switchToWidget(CCopasiUndoCommand::GLOBALQUANTITYIES);

  QList <UndoGlobalQuantityData *>::const_iterator j;

  for (j = pData.begin(); j != pData.end(); ++j)
    {
      UndoGlobalQuantityData * data = *j;
      size_t index = pModel->getModelValues().getIndex(data->getName());
      removeRow((int) index);
    }
}

bool CQGlobalQuantityDM::clear()
{
  mpUndoStack->push(new RemoveAllGlobalQuantityRowsCommand(this, QModelIndex()));
  return true;
}

bool CQGlobalQuantityDM::removeAllGlobalQuantityRows()
{
  return removeRows(0, rowCount() - 1);
}
