// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/layout/SBMLDocumentLoader.cpp,v $
//   $Revision: 1.1 $
//   $Name:  $
//   $Author: ssahle $
//   $Date: 2007/02/13 17:13:01 $
// End CVS Header

// Copyright (C) 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include "SBMLDocumentLoader.h"

#include "sbml/xml/ParseMessage.h"

#include "sbml/layout/SpeciesGlyph.h"
#include "sbml/layout/ReactionGlyph.h"
#include "sbml/layout/SpeciesReferenceGlyph.h"
#include "sbml/layout/TextGlyph.h"
#include "sbml/layout/BoundingBox.h"
#include "sbml/layout/Point.h"
#include "sbml/layout/Dimensions.h"
//#include "myTypes.h"
#include <iostream>

#include "CListOfLayouts.h"
#include "CLayout.h"
#include "CLReactionGlyph.h"
#include "CLGlyphs.h"

#include "sbml/SBMLUtils.h" //from the copasi sbml dir

CLayout* SBMLDocumentLoader::loadDocument(const char *filename)
{

  SBMLReader *reader = SBMLReader_create();
  this->sbmlDocP = reader->readSBML(filename);
  //std::vector<node*> nodeVector;
  CLayout* pLayout = new CLayout();
  //cout << this->sbmlDocP << endl;
  if ((this->sbmlDocP->getNumFatals()) == 0)
    {
      // check whether layout is included in SBML file
      modelP = this->sbmlDocP->getModel();
      if (modelP != NULL)
        {
          //layoutList = &modelP->getListOfLayouts();
          int numberOfLayouts = modelP->getListOfLayouts().getNumItems();
          std::cout << "number of layouts: " << numberOfLayouts << "  " << std::endl;
          if (numberOfLayouts > 0)
            {
              Layout *layout = modelP->getLayout(0); // take first layout stored
              //        pLayout = mapLayoutToGraph(modelP,layout);
            }
        }
    }
  else
    {
      int numberOfErrors = this->sbmlDocP->getNumFatals();
      std::cout << numberOfErrors << "  error(s)" << std::endl;
      ParseMessage *message;
      for (int i = 0;i < numberOfErrors;i++)
        {
          message = this->sbmlDocP->getFatal(i);
          std::cout << message->getMessage();
          std::cout << std::endl;
        }
    }
  return pLayout;
}

//static
void SBMLDocumentLoader::readListOfLayouts(CListOfLayouts & lol,
    const ListOf & sbmlList,
    const std::map<CCopasiObject*, SBase*> & copasimodelmap)
{
  //convert the map as used by the CLxxx constructors
  std::map<std::string, std::string> modelmap;

  std::string s1, s2;
  std::map<CCopasiObject*, SBase*>::const_iterator it;
  std::map<CCopasiObject*, SBase*>::const_iterator itEnd = copasimodelmap.end();
  for (it = copasimodelmap.begin(); it != itEnd; ++it)
    {
      s1 = SBMLUtils::getIdFromSBase(it->second);

      if (it->first)
        s2 = it->first->getKey();
      else
        s2 = "";

      if ((s1 != "") && (s2 != ""))
        modelmap[s1] = s2;
    }

  //iterate through list of layouts
  C_INT32 i, imax = sbmlList.getNumItems();
  for (i = 0; i < imax; ++i)
    {
      std::map<std::string, std::string> layoutmap;
      const Layout* tmp
      = dynamic_cast<const Layout*>(sbmlList.get(i));
      if (tmp)
        {
          CLayout * pLayout = createLayout(*tmp, modelmap, layoutmap);
          lol.addLayout(pLayout, layoutmap);
        }
    }
}

//static
CLayout * SBMLDocumentLoader::createLayout(const Layout & sbmlLayout,
    const std::map<std::string, std::string> & modelmap,
    std::map<std::string, std::string> & layoutmap,
    const CCopasiContainer * pParent)
{
  CLayout* layout = new CLayout(sbmlLayout, layoutmap, pParent);

  C_INT32 i, imax;

  //compartments
  imax = sbmlLayout.getListOfCompartmentGlyphs().getNumItems();
  for (i = 0; i < imax; ++i)
    {
      const CompartmentGlyph* tmp
      = dynamic_cast<const CompartmentGlyph*>(sbmlLayout.getListOfCompartmentGlyphs().get(i));
      if (tmp)
        layout->addCompartmentGlyph(new CLCompartmentGlyph(*tmp, modelmap, layoutmap));
    }

  //species
  imax = sbmlLayout.getListOfSpeciesGlyphs().getNumItems();
  for (i = 0; i < imax; ++i)
    {
      const SpeciesGlyph* tmp
      = dynamic_cast<const SpeciesGlyph*>(sbmlLayout.getListOfSpeciesGlyphs().get(i));
      if (tmp)
        layout->addMetaboliteGlyph(new CLMetabGlyph(*tmp, modelmap, layoutmap));
    }

  //reactions
  //TODO

  //text
  //TODO

  //additional
  //TODO

  //second pass text
  //TODO

  return layout;
}

// for the moment just create a vector of pointers to the nodes that are represented by the SpeciesGlyph
/*  network* SBMLDocumentLoader::mapLayoutToGraph(Model *model,Layout *layout){
    // check size of graph
    Dimensions& dimGraph = layout->getDimensions();
    double width = dimGraph.getWidth();
    double height = dimGraph.getHeight();
    // get nodes
    ListOf listOfSpeciesGlyphs = layout->getListOfSpeciesGlyphs();
    int numberOfSpeciesGlyphs = listOfSpeciesGlyphs.getNumItems();
    std::vector<node*> nodePVec(numberOfSpeciesGlyphs);
    //nodePVec.resize(numberOfSpeciesGlyphs);
    for (int i=0;i<numberOfSpeciesGlyphs;i++){
      SpeciesGlyph *glyph = (SpeciesGlyph*)listOfSpeciesGlyphs.get(i);
      BoundingBox box = glyph->getBoundingBox();
      Point p = box.getPosition();
      Dimensions dim = box.getDimensions();
      node *n = new node(p.getXOffset() + (dim.getWidth() / 2.0),
                p.getYOffset() + (dim.getHeight() / 2.0));
        nodePVec[i] = n;
        //nodePVec.push_back(n);
    }

    // now get reactions
    ListOf listOfReactionGlyphs = layout->getListOfReactionGlyphs();
    int numberOfReactionGlyphs = listOfReactionGlyphs.getNumItems();
    std::vector<reaction*> reactionPVec(numberOfReactionGlyphs);
    //reactionPVec.resize(numberOfReactionGlyphs);
    cout << "number of reactions: " << numberOfReactionGlyphs << endl;
    int i;
    for (i=0;i<numberOfReactionGlyphs;i++){
      //cout << "reaction: " << i << endl;
      ReactionGlyph *rGlyphP= (ReactionGlyph*)listOfReactionGlyphs.get(i);
      // now extract id (from model, if present), reaction curce and speciesReferenceGlyph info
      string id = rGlyphP->getReactionId();

      Curve *curve = rGlyphP->getCurve(); // get curve defining reaction arrow
      vector <curveSegment*> reactionCurveSegments = this->getCurveSegments(curve);

      // get edges to reactants (stored in speciesReferences)
      ListOf& listOfSpeciesReferenceGlyphs = rGlyphP->getListOfSpeciesReferenceGlyphs ();
      int numberOfSpecRefGlyphs = listOfSpeciesReferenceGlyphs.getNumItems();
      SpeciesReferenceGlyph *specRefGl;
      string speciesGlId;
      vector <curveSegment*> reactantCurveSegments;
      vector <edge*>  edges;
      roleType role;
      edge *e;
      node *n;
      vector<node*>::iterator itPos;
      //cout << "number of speciesReferenceGlyphs: " << numberOfSpecRefGlyphs << endl;
      //edges.resize(numberOfSpecRefGlyphs); // allocate space for numberOfSpecRefGlyphs edges
      //edges = vector <edge*>(numberOfSpecRefGlyphs);
      int j;
      for (j=0;j<numberOfSpecRefGlyphs;j++){
        specRefGl = (SpeciesReferenceGlyph*)listOfSpeciesReferenceGlyphs.get(j);
        speciesGlId = specRefGl->getSpeciesGlyphId(); // string id of species glyph
        // now find corresponding node
        //itPos = find_if(nodePVec.begin(),nodePVec.end(),std::bind2nd(ptr_fun(node::hasID),speciesGlId));
        //find_if(nodePVec.begin(),nodePVec.end(),mem_fun(&node::hasID));
        itPos = find_if(nodePVec.begin(),nodePVec.end(),std::bind2nd(mem_fun(&node::hasID),speciesGlId));
        if (itPos != nodePVec.end()){
          n = *itPos;
          // cout << "id 1: " << (*itPos)->getID() << endl;
          // cout << "id 2: " << speciesGlId << endl;
        }
        else {// create "empty" node
          n = new node(); // will get id="empty"
        }
        curve = specRefGl->getCurve();
        reactantCurveSegments = getCurveSegments(curve);
        if (specRefGl->isSetRole()){
          string rStr = specRefGl->getRoleString();
          role = typeConverter::mapStringToRole(rStr);
          //cout << n->getID() << "  "  << rStr << endl;
        }
        else
          role = undefined;
        e = new edge(n,reactantCurveSegments,role);
        edges.push_back(e);
      }
      // now create reaction data structure
      reaction *r = new reaction(id,reactionCurveSegments,edges);
      reactionPVec[i] = r;
      edges.clear();
    }
    // now get text labels
    ListOf& listOfTextGlyphs = layout->getListOfTextGlyphs();
    int numberOfTextGlyphs = listOfTextGlyphs.getNumItems();
    vector<label*> labelPVec;
    label *labelP;
    TextGlyph  *tGlyphP;
    int j;
    string text;

    for (j=0;j<numberOfTextGlyphs;j++){
        tGlyphP = (TextGlyph*)listOfTextGlyphs.get(j);
        text = tGlyphP->getText();
        if (text == "") // if there is no text attribute (i.e.empty)
          text = tGlyphP->getOriginOfTextId(); // try other attribute
        BoundingBox &box = tGlyphP->getBoundingBox();
        Point &pos = box.getPosition();
        Dimensions &dim = box.getDimensions();
        labelP = new label(text,pos.getXOffset(),pos.getYOffset(),dim.getWidth(), dim.getHeight());
        //labelP->printLabel();
        labelPVec.push_back(labelP);
        //cout << "text: " << tGlyphP->getText() << endl;
        //cout << "origin: " << tGlyphP->getOriginOfTextId() << endl;
    }

    network* netP = new network(width, height, nodePVec,reactionPVec,labelPVec);
    return netP;
  }*/

//  bool SBMLDocumentLoader::hasID(node *n,const string& s){
//  string idstring = n->getID();
//  if (((idstring.compare(s)) == 0) ||
//    ((idstring.find(s)) == string::npos))
//    return true;
//  else
//  return false;
//}

//   vector <curveSegment*> SBMLDocumentLoader::getCurveSegments(Curve *curve){
//       ListOf& listOfCurveSegments = curve->getListOfCurveSegments ();
//       int numberOfCurveSegments = listOfCurveSegments.getNumItems();
//       vector <curveSegment*> mySegments;
//       //mySegments.resize(numberOfCurveSegments);
//       LineSegment *lineSeg;
//       curveSegment *mySegment;
//       Point p1,p2;
//       for (int j=0;j<numberOfCurveSegments;j++){
//         lineSeg = (LineSegment*)listOfCurveSegments.get(j);
//         p1 = lineSeg->getStart();
//         p2 = lineSeg->getEnd();
//         // now get base points on potential curve, if defined
//         // to be filled............
//
//         // create new data structure
//         coordList coords = vector<coord>();
//         coords.push_back(make_pair(p1.getXOffset(),p1.getYOffset()));
//         coords.push_back(make_pair(p2.getXOffset(),p2.getYOffset()));
//
//         mySegment = new curveSegment(coords); // list of start + end points
//         mySegments.push_back(mySegment);
//         //mySegment->printSegment();
//}
//       return mySegments;
//}
//
