// Copyright (C) 2010 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

#include <QtCore/QString>
#include <QtCore/QList>

#include "CopasiDataModel/CCopasiDataModel.h"
#include "report/CCopasiRootContainer.h"
#include "function/CFunctionDB.h"

#include "CQMessageBox.h"
#include "CQFunctionDM.h"
#include "qtUtilities.h"

CQFunctionDM::CQFunctionDM(QObject *parent)
  : CQBaseDataModel(parent)
  , mNewName("function")
{
}

int CQFunctionDM::rowCount(const QModelIndex& C_UNUSED(parent)) const
{
  return (int) CCopasiRootContainer::getFunctionList()->loadedFunctions().size() + 1;
}
int CQFunctionDM::columnCount(const QModelIndex& C_UNUSED(parent)) const
{
  return TOTAL_COLS_FUNCTIONS;
}

Qt::ItemFlags CQFunctionDM::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return Qt::ItemIsEnabled;

  if (!isDefaultRow(index) && isFunctionReadOnly(index))
    return QAbstractItemModel::flags(index);

  if (index.column() == COL_NAME_FUNCTIONS)
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
  else
    return QAbstractItemModel::flags(index);
}

bool CQFunctionDM::isFunctionReadOnly(const QModelIndex &index) const
{
  const CFunction *pFunc = &CCopasiRootContainer::getFunctionList()->loadedFunctions()[index.row()];
  return pFunc->isReadOnly();
}

QVariant CQFunctionDM::data(const QModelIndex &index, int role) const
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

              case COL_NAME_FUNCTIONS:
                return QVariant(QString("New Function"));

              case COL_TYPE_FUNCTIONS:
                return QVariant(QString(FROM_UTF8(CEvaluationTree::TypeName[4])));

              default:
                return QVariant(QString(""));
            }
        }
      else
        {
          const CEvaluationTree *pFunc = &CCopasiRootContainer::getFunctionList()->loadedFunctions()[index.row()];

          if (pFunc == NULL)
            return QVariant();

          switch (index.column())
            {
              case COL_ROW_NUMBER:
                return QVariant(index.row() + 1);

              case COL_NAME_FUNCTIONS:
                return QVariant(QString(FROM_UTF8(pFunc->getObjectName())));

              case COL_TYPE_FUNCTIONS:
                return QVariant(QString(FROM_UTF8(CEvaluationTree::TypeName[pFunc->getType()])));

              case COL_MATH_DESC_FUNCTIONS:
                return QVariant(QString(FROM_UTF8(pFunc->getInfix())));

              case COL_SBML_ID_FUNCTIONS:
                return QVariant();
            }
        }
    }

  return QVariant();
}

QVariant CQFunctionDM::headerData(int section, Qt::Orientation orientation,
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

          case COL_NAME_FUNCTIONS:
            return QVariant(QString("Name"));

          case COL_TYPE_FUNCTIONS:
            return QVariant(QString("Type"));

          case COL_MATH_DESC_FUNCTIONS:
            return QVariant(QString("Mathematical Description"));

          case COL_SBML_ID_FUNCTIONS:
            return QVariant(QString("SBML ID"));

          default:
            return QVariant();
        }
    }
  else
    return QString("%1").arg(section + 1);
}

bool CQFunctionDM::setData(const QModelIndex &index, const QVariant &value,
                           int role)
{
  if (index.isValid() && role == Qt::EditRole)
    {
      bool defaultRow = isDefaultRow(index);

      if (defaultRow)
        {
          if (index.data() != value)
            {
              mNewName = (index.column() == COL_NAME_FUNCTIONS) ? value.toString() : "function";
              insertRow(rowCount(), index);
            }
          else
            return false;
        }

      CEvaluationTree *pFunc = &CCopasiRootContainer::getFunctionList()->loadedFunctions()[index.row()];

      if (pFunc == NULL)
        return false;

      if (index.column() == COL_NAME_FUNCTIONS)
        pFunc->setObjectName(TO_UTF8(value.toString()));
      else if (index.column() == COL_TYPE_FUNCTIONS)
        {
          if (index.data() != value)
            {
              QString msg;
              msg = "Type must not be changed for '" + FROM_UTF8(pFunc->getObjectName()) + "'.\n";

              CQMessageBox::information(NULL,
                                        "Unable to change Function Type",
                                        msg,
                                        QMessageBox::Ok, QMessageBox::Ok);
            }
        }
      else if (index.column() == COL_MATH_DESC_FUNCTIONS)
        {
          if (index.data() != value)
            {
              if (!pFunc->setInfix(TO_UTF8(value.toString())))
                {
                  QString msg;
                  msg = "Incorrect  mathematical description'" + FROM_UTF8(pFunc->getObjectName()) + "'.\n";

                  CQMessageBox::information(NULL,
                                            "Unable to change mathematical description",
                                            msg,
                                            QMessageBox::Ok, QMessageBox::Ok);
                }
            }
        }

      emit dataChanged(index, index);
      emit notifyGUI(ListViews::FUNCTION, ListViews::CHANGE, pFunc->getKey());
    }

  return true;
}

bool CQFunctionDM::insertRows(int position, int rows, const QModelIndex&)
{
  beginInsertRows(QModelIndex(), position, position + rows - 1);

  for (int row = 0; row < rows; ++row)
    {
      CFunction *pFunc;
      QString Name = createNewName(mNewName, COL_NAME_FUNCTIONS);

      CCopasiRootContainer::getFunctionList()->add(pFunc = new CKinFunction(TO_UTF8(Name)), true);
      emit notifyGUI(ListViews::FUNCTION, ListViews::ADD, pFunc->getKey());
    }

  endInsertRows();

  mNewName = "function";

  return true;
}

bool CQFunctionDM::removeRows(int position, int rows)
{
  if (rows <= 0)
    return true;

  std::vector< std::string > DeletedKeys;
  DeletedKeys.resize(rows);

  std::vector< std::string >::iterator itDeletedKey;
  std::vector< std::string >::iterator endDeletedKey = DeletedKeys.end();

  CCopasiVector< CFunction >::const_iterator itRow =
    CCopasiRootContainer::getFunctionList()->loadedFunctions().begin() + position;
  int row = 0;

  for (itDeletedKey = DeletedKeys.begin(), row = 0; itDeletedKey != endDeletedKey; ++itDeletedKey, ++itRow, ++row)
    {
      if (isFunctionReadOnly(this->index(position + row, 0)))
        {
          *itDeletedKey = "";
        }
      else
        {
          *itDeletedKey = itRow->getKey();
        }
    }

  beginRemoveRows(QModelIndex(), position, position + row - 1);

  for (itDeletedKey = DeletedKeys.begin(), row = 0; itDeletedKey != endDeletedKey; ++itDeletedKey, ++row)
    {
      if (*itDeletedKey != "")
        {
          CCopasiRootContainer::getFunctionList()->removeFunction(*itDeletedKey);
          emit notifyGUI(ListViews::FUNCTION, ListViews::DELETE, *itDeletedKey);
          emit notifyGUI(ListViews::FUNCTION, ListViews::DELETE, ""); //Refresh all as there may be dependencies.
        }
    }

  endRemoveRows();

  return true;
}

bool CQFunctionDM::removeRows(QModelIndexList rows, const QModelIndex&)
{
  if (rows.isEmpty())
    return false;

  assert(CCopasiRootContainer::getDatamodelList()->size() > 0);
  CCopasiDataModel* pDataModel = &CCopasiRootContainer::getDatamodelList()->operator[](0);
  assert(pDataModel != NULL);
  CModel * pModel = pDataModel->getModel();

  if (pModel == NULL)
    return false;

//Build the list of pointers to items to be deleted
//before actually deleting any item.
  QList <CEvaluationTree *> pFunctions;
  CFunction * pFunction;
  QModelIndexList::const_iterator i;

  for (i = rows.begin(); i != rows.end(); ++i)
    {
      if (!isDefaultRow(*i) &&
          (pFunction = &CCopasiRootContainer::getFunctionList()->loadedFunctions()[i->row()]) != NULL &&
          !pFunction->isReadOnly())
        pFunctions.append(&CCopasiRootContainer::getFunctionList()->loadedFunctions()[i->row()]);
    }

  QList <CEvaluationTree *>::const_iterator j;

  for (j = pFunctions.begin(); j != pFunctions.end(); ++j)
    {
      CEvaluationTree * pFunction = *j;

      size_t delRow =
        CCopasiRootContainer::getFunctionList()->loadedFunctions().CCopasiVector< CFunction >::getIndex(pFunction);

      if (delRow != C_INVALID_INDEX)
        {
          QMessageBox::StandardButton choice =
            CQMessageBox::confirmDelete(NULL, "function",
                                        FROM_UTF8(pFunction->getObjectName()),
                                        pFunction->getDeletedObjects());

          if (choice == QMessageBox::Ok)
            removeRow((int) delRow);
        }
    }

  return true;
}
