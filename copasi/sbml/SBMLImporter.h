/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/sbml/SBMLImporter.h,v $
   $Revision: 1.22 $
   $Name:  $
   $Author: gauges $ 
   $Date: 2005/07/20 09:34:12 $
   End CVS Header */

#ifndef SBMLIMPORTER_H__
#define SBMLIMPORTER_H__

#include <string>
#include <map>
#include "sbml/math/ASTNode.h"

#include "function/CFunctionDB.h"
#include "sbml/StdException.h"
#include "sbml/UnitDefinition.h"
#include "model/CModel.h"

class SBMLDocument;
class CCompartment;
class CMetab;
class CReaction;
class Reaction;
class Species;
class Model;
class Compartment;
class SBMLDocument;
class ConverterASTNode;
class Parameter;
class FunctionDefinition;
class SBase;

class SBMLImporter
  {
  protected:
    std::map<std::string, CMetab*> speciesMap;
    CFunctionDB* functionDB;
    bool mIncompleteModel;
    unsigned int mLevel;
    std::map<CEvaluationTree*, std::string> sbmlIdMap;
    std::vector<std::string> newFunctionsKeys;

    /**
     * Creates and returns a Copasi CModel from the SBMLDocument given as argument.
     */
    CModel* createCModelFromSBMLDocument(SBMLDocument* doc, std::map<CCopasiObject*, SBase*>& copasi2sbmlmap);

    /**
     * Creates and returns a Copasi CFunction from the SBML FunctionDefinition
     * given as argument.
     */
    CFunction* createCFunctionFromFunctionDefinition(const FunctionDefinition* sbmlFunction, CFunctionDB* pTmpFunctionDB);

    CFunction* createCFunctionFromFunctionTree(const FunctionDefinition* pSBMLFunction);

    /**
     * Creates and returns a Copasi CCompartment from the SBML Compartment
     * given as argument.
     */
    CCompartment* createCCompartmentFromCompartment(const Compartment* sbmlComp, CModel* copasiModel, std::map<CCopasiObject*, SBase*>& copasi2sbmlmap);

    /**
     * Creates and returns a Copasi CMetab from the given SBML Species object.
     */
    CMetab* createCMetabFromSpecies(const Species* sbmlSpecies, CModel* copasiModel, CCompartment* copasiCompartment, std::map<CCopasiObject*, SBase*>& copasi2sbmlmap);

    /**
     * Creates and returns a Copasi CModelValue from the given SBML Parameter object.
     */
    CModelValue* createCModelValueFromParameter(const Parameter* sbmlParameter, CModel* copasiModel, std::map<CCopasiObject*, SBase*>& copasi2sbmlmap);

    /**
     * Creates and returns a Copasi CReaction object from the given SBML
     * Reaction object.
     */
    CReaction* createCReactionFromReaction(const Reaction* sbmlReaction, const Model* sbmlModel, CModel* cmodel, std::map<CCopasiObject*, SBase*>& copasi2sbmlmap, CFunctionDB* pTmpFunctionDB, const std::map<std::string, std::string>& nameMapping);

    /**
     * Replaces SBML user defined functions with the actual funtcion definition.
     */
    ConverterASTNode* replaceUserDefinedFunctions(ASTNode* node, const Model* model);

    /**
     * Creates a map of each parameter of the function definition and its
     * corresponding parameter in the function call.
     */
    std::map<std::string , ASTNode*> createBVarMap(const ASTNode* uDefFunction, const ASTNode* function);

    /**
     * Returns the user defined SBML function definition that belongs to the given
     * name, or NULL if none can be found.
     */
    FunctionDefinition* getFunctionDefinitionForName(const std::string name, const Model* model);

    /**
     * Replaces the variables in a function definition with the actual function
     * parameters that were used when the function was called. The function returns
     * a pointer to the ConverterAST node with the replaced variables.
     */
    ConverterASTNode* replaceBvars(const ASTNode* node, std::map<std::string, ASTNode*> bvarMap);

    /**
     * This function replaces the AST_FUNCTION_POWER ASTNodes in a ASTNode tree
     * with the AST_POWER node.
     */
    void replacePowerFunctionNodes(ASTNode* node);

    /**
     * Returns the copasi VolumeUnit corresponding to the given SBML Volume
     *  UnitDefinition.
     */
    CModel::VolumeUnit handleVolumeUnit(const UnitDefinition* uDef);

    /**
     * Returns the copasi QuantityUnit corresponding to the given SBML
     *  Substance UnitDefinition.
     */
    CModel::QuantityUnit handleSubstanceUnit(const UnitDefinition* uDef);

    /**
     * Returns the copasi TimeUnit corresponding to the given SBML Time
     *  UnitDefinition.
     */
    CModel::TimeUnit handleTimeUnit(const UnitDefinition* uDef);

    /**
     * Replaces all occurences of the log function with two arguments by
     * a division of two separate calls to log.
     */
    void replaceLog(ConverterASTNode* sourceNode);

    /**
     * Replaces all occurences of the root function with two arguments by
     * a call to the power function with the inverse of the first argument.
     */
    void replaceRoot(ConverterASTNode* sourceNode);

    /**
     * Replaces the ids of named nodes in an ASTNode tree with
     * the correspondingCopasi Common Names.
     */
    bool sbmlId2CopasiCN(ASTNode* pNode, std::map<CCopasiObject*, SBase*>& copasi2sbmlmap, CCopasiParameterGroup& pParamGroup);

    /**
     * Upon import a function object might change its name due to naming conflicts in the function
     * database. So after importing all function, all call node data has to be replaced by new data
     * which represents the new function name.
     * The same has to be done later on for function call nodes in reaction kinetics.
     */
    void replaceCallNodeNames(CEvaluationTree* tree, const std::map<std::string, std::string>& nameMapping);

    CFunction* findCorrespondingFunction(const CFunction* tree, const CReaction* reaction);

  public:
    SBMLImporter();
    ~SBMLImporter();
    CModel* readSBML(std::string filename, CFunctionDB* funDB, SBMLDocument* pSBMLDocument, std::map<CCopasiObject*, SBase*>& copasi2sbmlmap);
    static void printMap(const std::map<CCopasiObject*, SBase*>& map);
    void restoreFunctionDB();
  };

#endif
