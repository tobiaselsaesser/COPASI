// Copyright (C) 2017 - 2018 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and University of
// of Connecticut School of Medicine.
// All rights reserved.

// Copyright (C) 2010 - 2016 by Pedro Mendes, Virginia Tech Intellectual
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





#include <QtCore/QString>
#include <QColor>   //might need to go to the header file
#include <QCursor>
#include <QtCore/QMutexLocker>

#include <qwt_symbol.h>
#include <qwt_legend.h>
#if QWT_VERSION > 0x060000
#include <qwt_legend_data.h>
#include <qwt_legend_label.h>
#include <qwt_plot_canvas.h>
#else
#include <qwt_legend_item.h>
#endif

#include <qwt_scale_engine.h>
#include <qwt_color_map.h>
#include <qwt_scale_widget.h>

#include <limits>
#include <algorithm>
#include <cmath>

#include "copasi/copasi.h"

#include "scrollzoomer.h"
#include "CopasiPlot.h"
#include "CQPlotColors.h"

#include "copasi/plot/CPlotSpecification.h"
#include "copasi/UI/qtUtilities.h"
#include "copasi/core/CRootContainer.h"
#include "copasi/model/CModel.h"
#include "copasi/commandline/CLocaleString.h"
#include "copasi/CopasiDataModel/CDataModel.h"

#include <copasi/plotUI/C2DCurveData.h>
#include <copasi/plotUI/CBandedGraphData.h>
#include <copasi/plotUI/CHistoCurveData.h>
#include <copasi/plotUI/CSpectorgramData.h>

#include <copasi/plotUI/C2DPlotCurve.h>
#include <copasi/plotUI/CPlotSpectogram.h>

#include <copasi/plotUI/CLinearColorMap.h>

#include <QApplication>

#define ActivitySize 8
C_FLOAT64 CopasiPlot::MissingValue = std::numeric_limits<C_FLOAT64>::quiet_NaN();

CopasiPlot::CopasiPlot(QWidget* parent):
  QwtPlot(parent),
  mCurves(0),
  mCurveMap(),
  mSpectograms(0),
  mSpectogramMap(),
  mDataBefore(0),
  mDataDuring(0),
  mDataAfter(0),
  mHaveBefore(false),
  mHaveDuring(false),
  mHaveAfter(false),
  mpPlotSpecification(NULL),
  mNextPlotTime(),
  mIgnoreUpdate(false),
  mpZoomer(NULL),
  mReplotFinished(false)
{}

CopasiPlot::CopasiPlot(const CPlotSpecification* plotspec, QWidget* parent):
  QwtPlot(parent),
  mCurves(0),
  mCurveMap(),
  mSpectograms(0),
  mSpectogramMap(),
  mDataBefore(0),
  mDataDuring(0),
  mDataAfter(0),
  mHaveBefore(false),
  mHaveDuring(false),
  mHaveAfter(false),
  mpPlotSpecification(NULL),
  mNextPlotTime(),
  mIgnoreUpdate(false),
  mpZoomer(NULL),
  mReplotFinished(false)
{
  QwtLegend *legend = new QwtLegend(this);

#if QWT_VERSION > 0x060000
  legend->setDefaultItemMode(QwtLegendData::Checkable);
  ((QwtPlotCanvas*)canvas())->setPaintAttribute(QwtPlotCanvas::Opaque, true);

  connect(legend, SIGNAL(checked(const QVariant &, bool, int)),
          SLOT(legendChecked(const QVariant &, bool)));

#else
  legend->setItemMode(QwtLegend::CheckableItem);
  setCanvasLineWidth(0);
  canvas()->setPaintAttribute(QwtPlotCanvas::PaintPacked, true);
#endif

  // whole legend can not be displayed at bottom on DARWIN
  // maybe a Qwt bug ?!?
#ifdef Darwin
  insertLegend(legend, QwtPlot::TopLegend);
#else
  insertLegend(legend, QwtPlot::BottomLegend);
#endif

  // Set up the zoom facility
  mpZoomer = new ScrollZoomer(canvas());
  mpZoomer->setRubberBandPen(QColor(Qt::black));
  mpZoomer->setTrackerPen(QColor(Qt::black));
  mpZoomer->setTrackerMode(QwtPicker::AlwaysOn);
  mpZoomer->setTrackerFont(this->font());

  // white background better for printing...
  setCanvasBackground(Qt::white);

  //  setTitle(FROM_UTF8(plotspec->getTitle()));

#if QWT_VERSION < 0x060000
  connect(this, SIGNAL(legendChecked(QwtPlotItem *, bool)),
          SLOT(showCurve(QwtPlotItem *, bool)));
#endif

  // Size the vectors to be able to store information for all activities.
  mData.resize(ActivitySize);
  mObjectValues.resize(ActivitySize);
  mObjectInteger.resize(ActivitySize);
  mDataSize.resize(ActivitySize);
  mDataIndex.clear();

  // Initialize from the plot specification
  initFromSpec(plotspec);
  connect(this, SIGNAL(replotSignal()), this, SLOT(replot()));
}

#if QWT_VERSION > 0x060000
void CopasiPlot::legendChecked(const QVariant &itemInfo, bool on)
{
  QwtPlotItem *plotItem = infoToItem(itemInfo);

  if (plotItem)
    showCurve(plotItem, on);
}
#endif

CPlotSpectogram *
CopasiPlot::createSpectogram(const CPlotItem *plotItem)
{
  QString strLimitZ = FROM_UTF8(plotItem->getValue<std::string>("maxZ"));
  bool flag;
  double limitZ = strLimitZ.toDouble(&flag);

  if (!flag)
    limitZ = std::numeric_limits<double>::quiet_NaN();

  bool logZ = plotItem->getValue<bool>("logZ");

  CPlotSpectogram *pSpectogram = new CPlotSpectogram(
    &mMutex,
    plotItem->getType(),
    plotItem->getActivity(),
    FROM_UTF8(plotItem->getTitle()),
    logZ,
    limitZ,
    plotItem->getValue<bool>("bilinear"));

  pSpectogram->attach(this);
  pSpectogram->setRenderHint(QwtPlotItem::RenderAntialiased);
  QPen pen; pen.setWidthF(0.5);
  pSpectogram->setDefaultContourPen(pen);

  std::string colorMap = *const_cast< CPlotItem * >(plotItem)->assertParameter("colorMap", CCopasiParameter::Type::STRING, std::string("Default"));

#if QWT_VERSION > 0x060000
  pSpectogram->setRenderThreadCount(0);

  if (colorMap == "Grayscale")
    {
      QwtLinearColorMap *colorMap = new CLinearColorMap(Qt::white, Qt::black);
      pSpectogram->setColorMap(colorMap);
    }
  else if (colorMap == "Yellow-Red")
    {
      QwtLinearColorMap *colorMap = new CLinearColorMap(Qt::yellow, Qt::red);
      pSpectogram->setColorMap(colorMap);
    }
  else if (colorMap == "Blue-White-Red")
    {
      CLinearColorMap *colorMap = new CLinearColorMap(Qt::blue, Qt::red);
      colorMap->setAbsoluteStop(0.0, Qt::white);
      pSpectogram->setColorMap(colorMap);
    }
  else
    {
      QwtLinearColorMap *colorMap = new CLinearColorMap(Qt::darkCyan, Qt::red);
      colorMap->addColorStop(0.1, Qt::cyan);
      colorMap->addColorStop(0.6, Qt::green);
      colorMap->addColorStop(0.95, Qt::yellow);

      pSpectogram->setColorMap(colorMap);
    }

#else

  if (colorMap == "Grayscale")
    {
      CLinearColorMap colorMap(Qt::white, Qt::black);
      pSpectogram->setColorMap(colorMap);
    }
  else if (colorMap == "Yellow-Red")
    {
      CLinearColorMap colorMap(Qt::yellow, Qt::red);
      pSpectogram->setColorMap(colorMap);
    }
  else if (colorMap == "Blue-White-Red")
    {
      CLinearColorMap colorMap(Qt::blue, Qt::red);
      colorMap.setAbsoluteStop(0.0, Qt::white);
      pSpectogram->setColorMap(colorMap);
    }
  else
    {
      CLinearColorMap colorMap(Qt::darkCyan, Qt::red);
      colorMap.addColorStop(0.1, Qt::cyan);
      colorMap.addColorStop(0.6, Qt::green);
      colorMap.addColorStop(0.95, Qt::yellow);

      pSpectogram->setColorMap(colorMap);
    }

#endif

  QString contours = FROM_UTF8(* const_cast< CPlotItem * >(plotItem)->assertParameter("contours", CCopasiParameter::Type::STRING, std::string("")));

  int levels = contours.toInt(&flag);

  if (flag)
    {
      // have only a certain number of levels, applying them here
      QwtValueList contourLevels;

      for (double level = 0.5; level < levels; level += 1.0)
        contourLevels += level;

      pSpectogram->setContourLevels(contourLevels);
      pSpectogram->setDisplayMode(QwtPlotSpectrogram::ContourMode, true);
    }
  else
    {
      // have explicit list of numbers to plot
      QStringList list = contours.split(QRegExp(",| |;"), QString::SkipEmptyParts);
      QwtValueList contourLevels;

      foreach(const QString & level, list)
      {
        contourLevels += level.toDouble();
      }

      pSpectogram->setContourLevels(contourLevels);
      pSpectogram->setDisplayMode(QwtPlotSpectrogram::ContourMode, true);
    }

  CDataModel* dataModel = mpPlotSpecification->getObjectDataModel();
  assert(dataModel != NULL);

  setAxisTitle(xBottom, FROM_UTF8(dataModel->getObject((plotItem->getChannels()[0]))->getObjectDisplayName()));
  enableAxis(xBottom);

  setAxisTitle(yLeft, FROM_UTF8(dataModel->getObject((plotItem->getChannels()[1]))->getObjectDisplayName()));
  enableAxis(yLeft);

#if QWT_VERSION > 0x060000
  setAxisScaleEngine(xTop,
                     logZ ? (QwtScaleEngine *)new QwtLogScaleEngine() : (QwtScaleEngine *)new QwtLinearScaleEngine());
#else
  setAxisScaleEngine(xTop,
                     logZ ? (QwtScaleEngine *)new QwtLog10ScaleEngine() : (QwtScaleEngine *)new QwtLinearScaleEngine());
#endif

  setAxisTitle(xTop, FROM_UTF8(dataModel->getObject((plotItem->getChannels()[2]))->getObjectDisplayName()));

  QwtScaleWidget *topAxis = axisWidget(QwtPlot::xTop);
  topAxis->setColorBarEnabled(true);

  enableAxis(xTop);

  return pSpectogram;
}

bool CopasiPlot::initFromSpec(const CPlotSpecification* plotspec)
{
  mIgnoreUpdate = true;
  mpPlotSpecification = plotspec;

  if (mpZoomer) mpZoomer->setEnabled(false);

  // size_t k, kmax = mpPlotSpecification->getItems().size();

  setTitle(FROM_UTF8(mpPlotSpecification->getTitle()));

  mCurves.resize(mpPlotSpecification->getItems().size());
  mCurves = NULL;

  mSpectograms.resize(mpPlotSpecification->getItems().size());
  mSpectograms = NULL;

  std::map< std::string, C2DPlotCurve * >::iterator found;

  CDataVector< CPlotItem >::const_iterator itPlotItem = mpPlotSpecification->getItems().begin();
  CDataVector< CPlotItem >::const_iterator endPlotItem = mpPlotSpecification->getItems().end();

  CVector< bool > Visible(mpPlotSpecification->getItems().size());
  Visible = true;
  bool * pVisible = Visible.array();

  for (; itPlotItem != endPlotItem; ++itPlotItem, ++pVisible)
    {
      // Qwt does not like it to reuse the curve as this may lead to access
      // violation. We therefore delete the curves but remember their visibility.
      if ((found = mCurveMap.find(itPlotItem->CCopasiParameter::getKey())) != mCurveMap.end())
        {
          *pVisible = found->second->isVisible();
        }
    }

  // Remove unused curves if definition has changed
  std::map< std::string, C2DPlotCurve * >::iterator it = mCurveMap.begin();
  std::map< std::string, C2DPlotCurve * >::iterator end = mCurveMap.end();

  for (; it != end; ++it)
    pdelete(it->second);

  mCurveMap.clear();

  std::map< std::string, CPlotSpectogram * >::iterator it2 = mSpectogramMap.begin();
  std::map< std::string, CPlotSpectogram * >::iterator end2 = mSpectogramMap.end();

  for (; it2 != end2; ++it2)
    pdelete(it2->second);

  mSpectogramMap.clear();

  itPlotItem = mpPlotSpecification->getItems().begin();
  pVisible = Visible.array();
  C2DPlotCurve ** ppCurve = mCurves.array();
  CPlotSpectogram** ppSpectogram = mSpectograms.array();
  unsigned long int k = 0;
  bool needLeft = false;
  bool needRight = false;

  for (; itPlotItem != endPlotItem; ++itPlotItem, ++pVisible, ++ppCurve, ++ppSpectogram, ++k)
    {
      if (itPlotItem->getType() == CPlotItem::spectogram)
        {
          CPlotSpectogram* pSpectogram = createSpectogram(itPlotItem);

          *ppSpectogram = pSpectogram;
          mSpectogramMap[itPlotItem->CCopasiParameter::getKey()] = pSpectogram;

          showCurve(pSpectogram, *pVisible);

          needLeft = true;

          continue;
        }

      // set up the curve
      C2DPlotCurve * pCurve = new C2DPlotCurve(&mMutex,
          itPlotItem->getType(),
          itPlotItem->getActivity(),
          FROM_UTF8(itPlotItem->getTitle()));
      *ppCurve = pCurve;

      mCurveMap[itPlotItem->CCopasiParameter::getKey()] = pCurve;

      //color handling should be similar for different curve types
      QColor color;

      if (pCurve->getType() == CPlotItem::curve2d
          || pCurve->getType() == CPlotItem::histoItem1d
          || pCurve->getType() == CPlotItem::bandedGraph)
        {
          std::string colorstr = itPlotItem->getValue< std::string >("Color");
          color = CQPlotColors::getColor(colorstr, k);
        }

      pCurve->setPen(color);
      pCurve->attach(this);

      showCurve(pCurve, *pVisible);

      if (pCurve->getType() == CPlotItem::curve2d
          || pCurve->getType() == CPlotItem::bandedGraph)
        {
          needLeft = true;
          pCurve->setRenderHint(QwtPlotItem::RenderAntialiased);

          unsigned C_INT32 linetype = itPlotItem->getValue< unsigned C_INT32 >("Line type");

          if (linetype == 0      //line
              || linetype == 3)  //line+symbols
            {
              pCurve->setStyle(QwtPlotCurve::Lines);
#if QWT_VERSION > 0x060000
              pCurve->setLegendAttribute(QwtPlotCurve::LegendShowLine);
#endif

              unsigned C_INT32 linesubtype = itPlotItem->getValue< unsigned C_INT32 >("Line subtype");
              C_FLOAT64 width = itPlotItem->getValue< C_FLOAT64 >("Line width");

              switch (linesubtype) //symbol type
                {
                  case 1:
                    pCurve->setPen(QPen(QBrush(color), width, Qt::DotLine, Qt::FlatCap));
                    break;

                  case 2:
                    pCurve->setPen(QPen(QBrush(color), width, Qt::DashLine));
                    break;

                  case 3:
                    pCurve->setPen(QPen(QBrush(color), width, Qt::DashDotLine));
                    break;

                  case 4:
                    pCurve->setPen(QPen(QBrush(color), width, Qt::DashDotDotLine));
                    break;

                  case 0:
                  default:
                    pCurve->setPen(QPen(QBrush(color), width, Qt::SolidLine));
                    break;
                }
            }

          if (linetype == 1) //points
            {
              C_FLOAT64 width = itPlotItem->getValue< C_FLOAT64 >("Line width");
              pCurve->setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap));
              pCurve->setStyle(QwtPlotCurve::Dots);
            }

          if (linetype == 2) //only symbols
            {
              pCurve->setStyle(QwtPlotCurve::NoCurve);
            }

          if (linetype == 2      //symbols
              || linetype == 3)  //line+symbols
            {
              unsigned C_INT32 symbolsubtype = itPlotItem->getValue< unsigned C_INT32 >("Symbol subtype");

              switch (symbolsubtype) //symbol type
                {
                  case 1:
#if QWT_VERSION > 0x060000
                    pCurve->setSymbol(new QwtSymbol(QwtSymbol::Cross, QBrush(), QPen(QBrush(color), 2), QSize(7, 7)));
                    pCurve->setLegendAttribute(QwtPlotCurve::LegendShowSymbol);
#else
                    pCurve->setSymbol(QwtSymbol(QwtSymbol::Cross, QBrush(), QPen(QBrush(color), 2), QSize(7, 7)));
#endif
                    break;

                  case 2:
#if QWT_VERSION > 0x060000
                    pCurve->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, QBrush(), QPen(QBrush(color), 1), QSize(8, 8)));
                    pCurve->setLegendAttribute(QwtPlotCurve::LegendShowSymbol);
#else
                    pCurve->setSymbol(QwtSymbol(QwtSymbol::Ellipse, QBrush(), QPen(QBrush(color), 1), QSize(8, 8)));
#endif
                    break;

                  case 0:
                  default:
#if QWT_VERSION > 0x060000
                    pCurve->setSymbol(new QwtSymbol(QwtSymbol::Cross, QBrush(color), QPen(QBrush(color), 1), QSize(5, 5)));
                    pCurve->setLegendAttribute(QwtPlotCurve::LegendShowSymbol);
#else
                    pCurve->setSymbol(QwtSymbol(QwtSymbol::Cross, QBrush(color), QPen(QBrush(color), 1), QSize(5, 5)));
#endif

                    break;
                }
            }
        } //2d curves and banded graphs

      if (pCurve->getType() == CPlotItem::bandedGraph)
        {
          //set fill color
          QColor c = color;
          c.setAlpha(64);
          pCurve->setBrush(c);
        }

      if (pCurve->getType() == CPlotItem::histoItem1d)
        {
          pCurve->setIncrement(itPlotItem->getValue< C_FLOAT64 >("increment"));

          pCurve->setStyle(QwtPlotCurve::Steps);
          pCurve->setYAxis(QwtPlot::yRight);
          pCurve->setCurveAttribute(QwtPlotCurve::Inverted);

          needRight = true;
        }
    }

  if (plotspec->isLogX())
#if QWT_VERSION > 0x060000
    setAxisScaleEngine(xBottom, new QwtLogScaleEngine());

#else
    setAxisScaleEngine(xBottom, new QwtLog10ScaleEngine());
#endif
  else
    setAxisScaleEngine(xBottom, new QwtLinearScaleEngine());

  setAxisAutoScale(xBottom);

  if (plotspec->isLogY())
#if QWT_VERSION > 0x060000
    setAxisScaleEngine(yLeft, new QwtLogScaleEngine());

#else
    setAxisScaleEngine(yLeft, new QwtLog10ScaleEngine());
#endif
  else
    setAxisScaleEngine(yLeft, new QwtLinearScaleEngine());

  setAxisAutoScale(yLeft);

  enableAxis(yLeft, needLeft);

  if (needRight)
    {
      setAxisScaleEngine(yRight, new QwtLinearScaleEngine());
      setAxisTitle(yRight, "Percent %");
      enableAxis(yRight);
    }

  mIgnoreUpdate = false;

  return true; //TODO really check!
}

const CPlotSpecification *
CopasiPlot::getPlotSpecification() const
{
  return mpPlotSpecification;
}

bool CopasiPlot::compile(CObjectInterface::ContainerList listOfContainer)
{
  clearBuffers();

  size_t i, imax;
  size_t j, jmax;

  std::pair< std::set< const CObjectInterface * >::iterator, bool > Inserted;
  std::pair< Activity, size_t > DataIndex;
  std::vector< std::set < const CObjectInterface * > > ActivityObjects;

  ActivityObjects.resize(ActivitySize);

  // Loop over all curves.
  imax = mpPlotSpecification->getItems().size();
  mDataIndex.resize(imax);

  std::vector< std::vector < const CObjectInterface * > >::iterator itX;

  for (i = 0; i < imax; ++i)
    {
      const CPlotItem * pItem = &mpPlotSpecification->getItems()[i];
      bool isSpectogram = pItem->getType() == CPlotItem::spectogram;
      Activity ItemActivity = pItem->getActivity();
      DataIndex.first = ItemActivity;

      // Loop over all channels
      jmax = pItem->getNumChannels();
      mDataIndex[i].resize(jmax);

      for (j = 0; j < jmax; ++j)
        {
          const CObjectInterface * pObj = CObjectInterface::GetObjectFromCN(listOfContainer, pItem->getChannels()[j]);

          if (pObj)
            mObjects.insert(pObj);
          else
            CCopasiMessage(CCopasiMessage::WARNING, MCCopasiTask + 6,
                           pItem->getChannels()[j].c_str());

          // Remember the actual order for saving the data.
          // Note, we are currently only dealing with 2D curves and histograms.
          // In addition the data is not normalized. The same data column may appear
          // multiple times, e.g. as X value and as Y value for another curve.
          if (j == 0)
            {
              // We have an X value
              for (itX = mSaveCurveObjects.begin(); itX != mSaveCurveObjects.end(); ++itX)
                if (*itX->begin() == pObj) break;

              if (itX == mSaveCurveObjects.end())
                {
                  std::vector < const CObjectInterface * > NewX;
                  NewX.push_back(pObj);

                  mSaveCurveObjects.push_back(NewX);
                  itX = mSaveCurveObjects.end() - 1;

                  if (!isSpectogram)
                    setAxisUnits(xBottom, pObj);
                }

              if (pItem->getType() == CPlotItem::histoItem1d)
                mSaveHistogramObjects.push_back(pObj);
            }
          else
            {
              itX->push_back(pObj);

              if (!isSpectogram)
                setAxisUnits(yLeft, pObj);
            }

          Inserted = ActivityObjects[ItemActivity].insert(pObj);

          if (Inserted.second)
            {
              if (ItemActivity & COutputInterface::BEFORE) mHaveBefore = true;

              if (ItemActivity & COutputInterface::DURING) mHaveDuring = true;

              if (ItemActivity & COutputInterface::AFTER) mHaveAfter = true;

              // The insert was successful
              DataIndex.second = ActivityObjects[ItemActivity].size() - 1;

              // Allocate the data buffer
              mData[ItemActivity].push_back(new CVector<double>(1000));

              // Store the pointer to the current object value. (Only if it has a double or integer value
              // and the value pointer actually exists. If not, use a dummy value.)
              void * tmp;

              const CDataObject * pDataObject = CObjectInterface::DataObject(pObj);

              if (pDataObject != NULL &&
                  (tmp = pObj->getValuePointer()) != NULL &&
                  (pDataObject->hasFlag(CDataObject::ValueInt) || pDataObject->hasFlag(CDataObject::ValueDbl)))
                {
                  mObjectValues[ItemActivity].push_back((C_FLOAT64 *) tmp); //pObj->getValuePointer());
                  mObjectInteger[ItemActivity].push_back(pDataObject->hasFlag(CDataObject::ValueInt));
                }
              else
                {
                  mObjectValues[ItemActivity].push_back(&MissingValue);
                  mObjectInteger[ItemActivity].push_back(false);
                }

              // Store [curve][channel] to data index
              mDataIndex[i][j] = DataIndex;

              // Store the [Activity][object] to data index.
              mObjectIndex[ItemActivity][pObj] = DataIndex.second;
            }
          else
            {
              // The object already existed we only need to
              // store [curve][channel] to data index.
              DataIndex.second = mObjectIndex[ItemActivity][pObj];
              mDataIndex[i][j] = DataIndex;
            }
        }
    }

  // We need to set the curve data here!
  size_t k = 0;
  C2DPlotCurve ** itCurves = mCurves.array();
  C2DPlotCurve ** endCurves = itCurves + mCurves.size();

  for (; itCurves != endCurves; ++itCurves, ++k)
    {
      if (*itCurves == NULL) continue;

      std::vector< CVector< double > * > & data = mData[(*itCurves)->getActivity()];

      switch ((*itCurves)->getType())
        {
          case CPlotItem::curve2d:
#if QWT_VERSION > 0x060000
            (*itCurves)->setData(new C2DCurveData(*data[mDataIndex[k][0].second],
                                                  *data[mDataIndex[k][1].second],
                                                  0));
#else
            (*itCurves)->setData(C2DCurveData(*data[mDataIndex[k][0].second],
                                              *data[mDataIndex[k][1].second],
                                              0));
#endif
            break;

          case CPlotItem::bandedGraph:
#if QWT_VERSION > 0x060000
            (*itCurves)->setData(new CBandedGraphData(*data[mDataIndex[k][0].second],
                                 *data[mDataIndex[k][1].second],
                                 *data[mDataIndex[k][2].second],
                                 0));
#else
            (*itCurves)->setData(CBandedGraphData(*data[mDataIndex[k][0].second],
                                                  *data[mDataIndex[k][1].second],
                                                  *data[mDataIndex[k][2].second],
                                                  0));
#endif
            break;

          case CPlotItem::histoItem1d:
#if QWT_VERSION > 0x060000
            (*itCurves)->setData(new CHistoCurveData(*data[mDataIndex[k][0].second],
                                 0,
                                 mCurves[k]->getIncrement()));
#else
            (*itCurves)->setData(CHistoCurveData(*data[mDataIndex[k][0].second],
                                                 0,
                                                 mCurves[k]->getIncrement()));
#endif
            break;

          default:
            fatalError();
            break;
        }
    }

  k = 0;
  CPlotSpectogram ** itSpectrograms = mSpectograms.array();
  CPlotSpectogram ** endSpectrograms = itSpectrograms + mSpectograms.size();

  for (; itSpectrograms != endSpectrograms; ++itSpectrograms, ++k)
    {
      if (*itSpectrograms == NULL) continue;

      std::vector< CVector< double > * > & data = mData[(*itSpectrograms)->getActivity()];

      switch ((*itSpectrograms)->getType())
        {
          case CPlotItem::spectogram:
#if QWT_VERSION > 0x060000
            (*itSpectrograms)->setData(
              new CSpectorgramData(
                *data[mDataIndex[k][0].second],
                *data[mDataIndex[k][1].second],
                *data[mDataIndex[k][2].second],
                0,
                (*itSpectrograms)->getLogZ(),
                (*itSpectrograms)->getLimitZ(),
                (*itSpectrograms)->getBilinear()
              )
            );
#else
            (*itSpectrograms)->setData(
              CSpectorgramData(
                *data[mDataIndex[k][0].second],
                *data[mDataIndex[k][1].second],
                *data[mDataIndex[k][2].second],
                0,
                (*itSpectrograms)->getLogZ(),
                (*itSpectrograms)->getLimitZ(),
                (*itSpectrograms)->getBilinear()
              ));
#endif
            break;

          default:
            fatalError();
            break;
        }
    }

  mNextPlotTime = CCopasiTimeVariable::getCurrentWallTime();
  mReplotFinished = true;

  return true;
}

void CopasiPlot::output(const Activity & activity)
{
  size_t i, imax;
  C_INT32 ItemActivity;

  if (mHaveBefore && (activity == COutputInterface::BEFORE)) mDataBefore++;
  else if (mHaveDuring && (activity == COutputInterface::DURING)) mDataDuring++;
  else if (mHaveAfter && (activity == COutputInterface::AFTER)) mDataAfter++;

  for (ItemActivity = 0; ItemActivity < ActivitySize; ItemActivity++)
    if ((ItemActivity & activity) && mData[ItemActivity].size())
      {
        std::vector< CVector< double > * > & data = mData[ItemActivity];
        size_t & ndata = mDataSize[ItemActivity];

        if ((imax = data.size()) != 0)
          {
            if (ndata >= data[0]->size())
              {
                resizeCurveData(ItemActivity);
              }

            //the data that needs to be stored internally:
            for (i = 0; i < imax; ++i)
              if (mObjectInteger[ItemActivity][i])
                (*data[i])[ndata] = *(C_INT32 *)mObjectValues[ItemActivity][i];
              else
                (*data[i])[ndata] = *mObjectValues[ItemActivity][i];

            ++ndata;
          }
      }

  updatePlot();
}

void CopasiPlot::separate(const Activity & activity)
{
  size_t i, imax;
  C_INT32 ItemActivity;

  if (mHaveBefore && (activity == COutputInterface::BEFORE)) mDataBefore++;

  if (mHaveDuring && (activity == COutputInterface::DURING)) mDataDuring++;

  if (mHaveAfter && (activity == COutputInterface::AFTER)) mDataAfter++;

  for (ItemActivity = 0; ItemActivity < ActivitySize; ItemActivity++)
    if ((ItemActivity & activity) && mData[ItemActivity].size())
      {
        std::vector< CVector< double > * > & data = mData[ItemActivity];
        size_t & ndata = mDataSize[ItemActivity];

        if ((imax = data.size()) != 0)
          {
            if (ndata >= data[0]->size())
              {
                resizeCurveData(ItemActivity);
              }

            //the data that needs to be stored internally:
            for (i = 0; i < imax; ++i)
              (*data[i])[ndata] = MissingValue;

            ++ndata;
          }
      }

  updatePlot();

  return;
}

void CopasiPlot::finish()
{

  // We need to force a replot, i.e., the next mNextPlotTime should be in the past.
  mNextPlotTime = 0;

  replot();

  if (mpZoomer)
    {
      mpZoomer->setEnabled(true);
      mpZoomer->setZoomBase();
    }
}

void CopasiPlot::updateCurves(const size_t & activity)
{
  if (activity == C_INVALID_INDEX)
    {
      C_INT32 ItemActivity;

      for (ItemActivity = 0; ItemActivity < ActivitySize; ItemActivity++)
        updateCurves(ItemActivity);

      return;
    }

  size_t k = 0;
  C2DPlotCurve ** itCurves = mCurves.array();
  C2DPlotCurve ** endCurves = itCurves + mCurves.size();

  for (; itCurves != endCurves; ++itCurves, ++k)
    {
      if (*itCurves == NULL) continue;

      if ((size_t)(*itCurves)->getActivity() == activity)
        {
          (*itCurves)->setDataSize(mDataSize[activity]);
        }
    }

  k = 0;
  CPlotSpectogram ** itSpectograms = mSpectograms.array();
  CPlotSpectogram ** endSpectograms = itSpectograms + mSpectograms.size();

  for (; itSpectograms != endSpectograms; ++itSpectograms, ++k)
    {
      if (*itSpectograms == NULL) continue;

      if ((size_t)(*itSpectograms)->getActivity() == activity)
        {
          (*itSpectograms)->setDataSize(mDataSize[activity]);
#if QWT_VERSION > 0x060000
          QwtScaleWidget *topAxis = axisWidget(QwtPlot::xTop);
          const QwtInterval zInterval = (*itSpectograms)->data()->interval(Qt::ZAxis);
          topAxis->setColorBarEnabled(true);

          const QwtColorMap* currentMap = (*itSpectograms)->colorMap();
          const CLinearColorMap* linearColormap = dynamic_cast<const CLinearColorMap*>(currentMap);

          if (linearColormap != NULL)
            topAxis->setColorMap(zInterval, new CLinearColorMap(*linearColormap));
          else
            topAxis->setColorMap(zInterval, const_cast<QwtColorMap*>(currentMap));

          setAxisScale(QwtPlot::xTop, zInterval.minValue(), zInterval.maxValue());
#else

          QwtScaleWidget *topAxis = axisWidget(QwtPlot::xTop);
          topAxis->setColorBarEnabled(true);
          topAxis->setColorMap((*itSpectograms)->data().range(),
                               (*itSpectograms)->colorMap());
          setAxisScale(QwtPlot::xTop,
                       (*itSpectograms)->data().range().minValue(),
                       (*itSpectograms)->data().range().maxValue());

#endif
        }
    }
}

void CopasiPlot::resizeCurveData(const size_t & activity)
{
  std::vector< CVector< double > * > & data = mData[activity];
  std::vector< CVector< double > * >::iterator it = data.begin();

  std::vector< CVector< double > * > OldData = data;
  std::vector< CVector< double > * >::iterator itOld = OldData.begin();
  std::vector< CVector< double > * >::iterator endOld = OldData.end();

  size_t oldSize = (*it)->size();
  size_t newSize = 2 * (*it)->size();

  // We must not deallocate the old data since this will create a window of time
  // were the GUI thread may access the old location before it is notified.
  for (; itOld != endOld; ++itOld, ++it)
    {
      *it = new CVector< double >(newSize);
      memcpy((*it)->array(), (*itOld)->array(), oldSize * sizeof(double));
    }

  // Tell the curves that the location of the data has changed
  // otherwise repaint events could crash
  size_t k = 0;
  C2DPlotCurve ** itCurves = mCurves.array();
  C2DPlotCurve ** endCurves = itCurves + mCurves.size();

  for (; itCurves != endCurves; ++itCurves, ++k)
    {
      if (*itCurves == NULL) continue;

      if ((size_t)(*itCurves)->getActivity() == activity)
        {
          std::vector< CVector< double > * > & data = mData[activity];

          switch ((*itCurves)->getType())
            {
              case CPlotItem::curve2d:
                (*itCurves)->reallocatedData(data[mDataIndex[k][0].second],
                                             data[mDataIndex[k][1].second]);
                break;

              case CPlotItem::bandedGraph:
                (*itCurves)->reallocatedData(data[mDataIndex[k][0].second],
                                             data[mDataIndex[k][1].second],
                                             data[mDataIndex[k][2].second]);
                break;

              case CPlotItem::histoItem1d:
                (*itCurves)->reallocatedData(data[mDataIndex[k][0].second],
                                             NULL);
                break;

              default:
                fatalError();
                break;
            }
        }
    }

  k = 0;
  CPlotSpectogram ** itSpectograms = mSpectograms.array();
  CPlotSpectogram ** endSpectograms = itSpectograms + mSpectograms.size();

  for (; itSpectograms != endSpectograms; ++itSpectograms, ++k)
    {
      if (*itSpectograms == NULL) continue;

      if ((size_t)(*itSpectograms)->getActivity() == activity)
        {
          std::vector< CVector< double > * > & data = mData[activity];

          switch ((*itSpectograms)->getType())
            {
              case CPlotItem::spectogram:
                (*itSpectograms)->reallocatedData(data[mDataIndex[k][0].second],
                                                  data[mDataIndex[k][1].second],
                                                  data[mDataIndex[k][2].second]);
                break;

              default:
                fatalError();
                break;
            }
        }
    }

  // It is now save to delete the old data since the GUI thread has been notified.
  for (itOld = OldData.begin(); itOld != endOld; ++itOld)
    {
      // pdelete(*itOld);
    }
}

void CopasiPlot::updatePlot()
{
  if (mReplotFinished)
    {
      mReplotFinished = false;
      emit replotSignal();
    }
}

//-----------------------------------------------------------------------------

/*void CopasiPlot::enableZoom(bool enabled)
{
  zoomOn = enabled;
}*/

//-----------------------------------------------------------------------------

CopasiPlot::~CopasiPlot()
{
  clearBuffers();
}

bool CopasiPlot::saveData(const std::string & filename)
{
  // No objects.
  if (!mObjects.size()) return true;

  // Find out whether we have any data.
  C_INT32 ItemActivity;

  for (ItemActivity = 0; ItemActivity < ActivitySize; ItemActivity++)
    if (mDataSize[ItemActivity] != 0) break;

  // No data
  if (ItemActivity == ActivitySize) return true;

  std::ofstream fs(CLocaleString::fromUtf8(filename).c_str());

  if (!fs.good()) return false;

  // Write the table header
  fs << "# ";

  std::vector< std::vector < const CObjectInterface  * > >::const_iterator itX;
  std::vector< std::vector < const CObjectInterface * > >::const_iterator endX =
    mSaveCurveObjects.end();

  std::vector < const CObjectInterface * >::const_iterator it;
  std::vector < const CObjectInterface * >::const_iterator end;

  for (itX = mSaveCurveObjects.begin(); itX != endX; ++itX)
    for (it = itX->begin(), end = itX->end(); it != end; ++it)
      if (CObjectInterface::DataObject(*it) != NULL)
        fs << CObjectInterface::DataObject(*it)->getObjectDisplayName() << "\t";
      else
        fs << "Not found\t";

  fs << "\n";

  size_t i, imax = mObjects.size();
  std::vector< CVector< double > * > Data;
  Data.resize(imax);

  std::vector< CVector< double > * >::const_iterator itData;
  std::vector< CVector< double > * >::const_iterator endData = Data.end();

  std::vector< size_t > Offset;
  std::vector< size_t >::const_iterator itOffset;

  Offset.resize(imax);

  std::map< Activity, std::map< const CObjectInterface *, size_t > >::iterator itActivity;
  std::map< const CObjectInterface *, size_t >::iterator itObject;

  if (mDataBefore)
    {
      for (itX = mSaveCurveObjects.begin(), i = 0; itX != endX; ++itX)
        for (it = itX->begin(), end = itX->end(); it != end; ++it, ++i)
          {
            if ((itActivity = mObjectIndex.find(COutputInterface::BEFORE)) != mObjectIndex.end() &&
                (itObject = itActivity->second.find(*it)) != itActivity->second.end())
              {
                Data[i] = mData[COutputInterface::BEFORE][itObject->second];
                continue;
              }

            if ((itActivity = mObjectIndex.find((COutputInterface::Activity)(COutputInterface::BEFORE | COutputInterface::DURING))) != mObjectIndex.end() &&
                (itObject = itActivity->second.find(*it)) != itActivity->second.end())
              {
                Data[i] = mData[COutputInterface::BEFORE | COutputInterface::DURING][itObject->second];
                continue;
              }

            if ((itActivity = mObjectIndex.find((COutputInterface::Activity)(COutputInterface::BEFORE | COutputInterface::AFTER))) != mObjectIndex.end() &&
                (itObject = itActivity->second.find(*it)) != itActivity->second.end())
              {
                Data[i] = mData[COutputInterface::BEFORE | COutputInterface::AFTER][itObject->second];
                continue;
              }

            if ((itActivity = mObjectIndex.find((COutputInterface::Activity)(COutputInterface::BEFORE | COutputInterface::DURING | COutputInterface::AFTER))) != mObjectIndex.end() &&
                (itObject = itActivity->second.find(*it)) != itActivity->second.end())
              {
                Data[i] = mData[COutputInterface::BEFORE | COutputInterface::DURING | COutputInterface::AFTER][itObject->second];
                continue;
              }

            Data[i] = NULL;
          }

      for (i = 0; i < mDataBefore; i++)
        {
          for (itData = Data.begin(); itData != endData; ++itData)
            {
              if (*itData) fs << (**itData)[i];
              else fs << MissingValue;

              fs << "\t";
            }

          fs << std::endl;
        }
    }

  if (mDataDuring)
    {
      for (itX = mSaveCurveObjects.begin(), i = 0; itX != endX; ++itX)
        for (it = itX->begin(), end = itX->end(); it != end; ++it, ++i)
          {
            if ((itActivity = mObjectIndex.find(COutputInterface::DURING)) != mObjectIndex.end() &&
                (itObject = itActivity->second.find(*it)) != itActivity->second.end())
              {
                Data[i] = mData[COutputInterface::DURING][itObject->second];
                Offset[i] = 0;
                continue;
              }

            if ((itActivity = mObjectIndex.find((COutputInterface::Activity)(COutputInterface::BEFORE | COutputInterface::DURING))) != mObjectIndex.end() &&
                (itObject = itActivity->second.find(*it)) != itActivity->second.end())
              {
                Data[i] = mData[COutputInterface::BEFORE | COutputInterface::DURING][itObject->second];
                Offset[i] = mDataBefore;
                continue;
              }

            if ((itActivity = mObjectIndex.find((COutputInterface::Activity)(COutputInterface::DURING | COutputInterface::AFTER))) != mObjectIndex.end() &&
                (itObject = itActivity->second.find(*it)) != itActivity->second.end())
              {
                Data[i] = mData[COutputInterface::DURING | COutputInterface::AFTER][itObject->second];
                Offset[i] = 0;
                continue;
              }

            if ((itActivity = mObjectIndex.find((COutputInterface::Activity)(COutputInterface::BEFORE | COutputInterface::DURING | COutputInterface::AFTER))) != mObjectIndex.end() &&
                (itObject = itActivity->second.find(*it)) != itActivity->second.end())
              {
                Data[i] = mData[COutputInterface::BEFORE | COutputInterface::DURING | COutputInterface::AFTER][itObject->second];
                Offset[i] = mDataBefore;
                continue;
              }

            Data[i] = NULL;
          }

      for (i = 0; i < mDataDuring; i++)
        {
          for (itData = Data.begin(), itOffset = Offset.begin(); itData != endData; ++itData)
            {
              if (*itData) fs << (**itData)[i + *itOffset];
              else fs << MissingValue;

              fs << "\t";
            }

          fs << std::endl;
        }
    }

  if (mDataAfter)
    {
      for (itX = mSaveCurveObjects.begin(), i = 0; itX != endX; ++itX)
        for (it = itX->begin(), end = itX->end(); it != end; ++it, ++i)
          {
            if ((itActivity = mObjectIndex.find(COutputInterface::AFTER)) != mObjectIndex.end() &&
                (itObject = itActivity->second.find(*it)) != itActivity->second.end())
              {
                Data[i] = mData[COutputInterface::AFTER][itObject->second];
                Offset[i] = 0;
                continue;
              }

            if ((itActivity = mObjectIndex.find((COutputInterface::Activity)(COutputInterface::BEFORE | COutputInterface::AFTER))) != mObjectIndex.end() &&
                (itObject = itActivity->second.find(*it)) != itActivity->second.end())
              {
                Data[i] = mData[COutputInterface::BEFORE | COutputInterface::AFTER][itObject->second];
                Offset[i] = mDataBefore;
                continue;
              }

            if ((itActivity = mObjectIndex.find((COutputInterface::Activity)(COutputInterface::DURING | COutputInterface::AFTER))) != mObjectIndex.end() &&
                (itObject = itActivity->second.find(*it)) != itActivity->second.end())
              {
                Data[i] = mData[COutputInterface::DURING | COutputInterface::AFTER][itObject->second];
                Offset[i] = mDataDuring;
                continue;
              }

            if ((itActivity = mObjectIndex.find((COutputInterface::Activity)(COutputInterface::BEFORE | COutputInterface::DURING | COutputInterface::AFTER))) != mObjectIndex.end() &&
                (itObject = itActivity->second.find(*it)) != itActivity->second.end())
              {
                Data[i] = mData[COutputInterface::BEFORE | COutputInterface::DURING | COutputInterface::AFTER][itObject->second];
                Offset[i] = mDataBefore + mDataDuring;
                continue;
              }

            Data[i] = NULL;
          }

      for (i = 0; i < mDataAfter; i++)
        {
          for (itData = Data.begin(), itOffset = Offset.begin(); itData != endData; ++itData)
            {
              if (*itData) fs << (**itData)[i + *itOffset];
              else fs << MissingValue;

              fs << "\t";
            }

          fs << std::endl;
        }
    }

  bool FirstHistogram = true;
  size_t HistogramIndex = 0;

  C2DPlotCurve ** itCurves = mCurves.array();
  C2DPlotCurve ** endCurves = itCurves + mCurves.size();

  for (; itCurves != endCurves; ++itCurves)
    {
      if (*itCurves == NULL) continue;

      if ((*itCurves)->getType() == CPlotItem::histoItem1d)
        {
          if (FirstHistogram)
            {
              fs << "\n# The histograms: \n";
              FirstHistogram = false;
            }

          if (CObjectInterface::DataObject(mSaveHistogramObjects[HistogramIndex]) != NULL)
            fs << CObjectInterface::DataObject(mSaveHistogramObjects[HistogramIndex])->getObjectDisplayName();
          else
            fs << "Not found";

          fs << std::endl;

#if QWT_VERSION > 0x060000
          CHistoCurveData * pData = static_cast< CHistoCurveData * >((*itCurves)->data());
#else
          CHistoCurveData * pData = static_cast< CHistoCurveData * >(&(*itCurves)->data());
#endif
          size_t i, imax = pData->size();

          for (i = 0; i < imax; ++i)
            {
              fs << pData->x(i) << "\t" << pData->y(i) << "\n";
            }
        }
    }

  fs.close();

  if (!fs.good()) return false;

  return true;
}

void CopasiPlot::showCurve(QwtPlotItem *item, bool on)
{
  item->setVisible(on);
  item->setItemAttribute(QwtPlotItem::AutoScale, on);
#if QWT_VERSION > 0x060000
  QwtLegend *lgd = qobject_cast<QwtLegend *>(legend());
  QList<QWidget *> legendWidgets =
    lgd->legendWidgets(itemToInfo(item));

  if (legendWidgets.size() == 1)
    {
      QwtLegendLabel *legendLabel =
        qobject_cast<QwtLegendLabel *>(legendWidgets[0]);

      if (legendLabel)
        legendLabel->setChecked(on);
    }

#else
  QWidget *w = legend()->find(item);

  if (w && w->inherits("QwtLegendItem"))
    static_cast< QwtLegendItem * >(w)->setChecked(on);

#endif

  if (!mIgnoreUpdate)
    replot();
}

void CopasiPlot::setCurvesVisibility(const bool & visibility)
{
  std::map< std::string, C2DPlotCurve * >::iterator it = mCurveMap.begin();
  std::map< std::string, C2DPlotCurve * >::iterator end = mCurveMap.end();

  for (; it != end; ++it)
    {
      it->second->setVisible(visibility);
      it->second->setItemAttribute(QwtPlotItem::AutoScale, visibility);
#if QWT_VERSION > 0x060000
      QwtLegend *lgd = qobject_cast<QwtLegend *>(legend());
      QList<QWidget *> legendWidgets =
        lgd->legendWidgets(itemToInfo(it->second));

      if (legendWidgets.size() == 1)
        {
          QwtLegendLabel *legendLabel =
            qobject_cast<QwtLegendLabel *>(legendWidgets[0]);

          if (legendLabel)
            legendLabel->setChecked(visibility);
        }

#else
      QWidget *w = legend()->find(it->second);

      if (w && w->inherits("QwtLegendItem"))
        static_cast< QwtLegendItem * >(w)->setChecked(visibility);

#endif
    }

  if (!mIgnoreUpdate)
    replot();
}

void CopasiPlot::clearBuffers()
{
  mObjects.clear();

  size_t Activity;
  size_t i, imax;

  for (Activity = 0; Activity < ActivitySize; Activity++)
    {
      std::vector< CVector< double > * > & data = mData[Activity];

      // Delete each QMemArray
      for (i = 0, imax = data.size(); i < imax; i++)
        if (data[i] != NULL) delete data[i];

      data.clear();

      mObjectValues[Activity].clear();
      mObjectInteger[Activity].clear();
      mDataSize[Activity] = 0;
    }

  mDataIndex.clear();
  mObjectIndex.clear();

  mSaveCurveObjects.clear();
  mSaveHistogramObjects.clear();

  mDataBefore = 0;
  mDataDuring = 0;
  mDataAfter = 0;

  mHaveBefore = false;
  mHaveDuring = false;
  mHaveAfter = false;
}

void CopasiPlot::setAxisUnits(const C_INT32 & index,
                              const CObjectInterface * pObjectInterface)
{
  const CDataObject * pObject = CObjectInterface::DataObject(pObjectInterface);

  if (pObject == NULL) return;

  std::string Units = CUnit::prettyPrint(pObject->getUnits());

  if (Units == "?")
    {
      Units.clear();
    }

  if (Units != "")
    setAxisTitle(index, FROM_UTF8(Units));

  return;
}

// virtual
void CopasiPlot::replot()
{
  if (mNextPlotTime < CCopasiTimeVariable::getCurrentWallTime())
    {
      // skip rendering when shift is pressed
      Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();

      if (((int)mods & (int)Qt::ShiftModifier) == (int)Qt::ShiftModifier &&
          !mNextPlotTime.isZero())
        {
          mReplotFinished = true;
          return;
        }

      CCopasiTimeVariable Delta = CCopasiTimeVariable::getCurrentWallTime();

      {
        QMutexLocker Locker(&mMutex);
        updateCurves(C_INVALID_INDEX);
      }

      QwtPlot::replot();

      Delta = CCopasiTimeVariable::getCurrentWallTime() - Delta;

      if (!mSpectogramMap.empty())
        mNextPlotTime = CCopasiTimeVariable::getCurrentWallTime() + 10 * Delta.getMicroSeconds();
      else
        mNextPlotTime = CCopasiTimeVariable::getCurrentWallTime() + 3 * Delta.getMicroSeconds();
    }

  mReplotFinished = true;
}
