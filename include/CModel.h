// cmodel.h : interface of the CModel class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef COPASI_CModel
#define COPASI_CModel

#include <vector>
#include <string>


#include "CStep.h"
#include "CMoiety.h"
#include "tnt/tnt.h"
#include "tnt/cmat.h"
template < class CType > class CCopasiVector;
class CCompartment;

class CModel
{
//Attributes
private:
    /**
     *  title of the model
     */
    string mTitle;

    /**
     *  Comments
     */
    string mComments;

    // metabolites

    /**
     *  for array of compartments
     *  @supplierCardinality 0..*
     *  @associates <{CCompartment}>
     */
    CCopasiVector < CCompartment > * mCompartments;

    /**
     *  for array of metabolites
     */
    vector < CMetab * > mMetabolites;
    vector < CMetab * > mMetabolitesX;
    vector < CMetab * > mMetabolitesInd;
    vector < CMetab * > mMetabolitesDep;
    
    /**
     *  for array of steps
     *  @supplierCardinality 0..*
     *  @associates <{CStep}>
     */
    CCopasiVector < CStep > * mSteps;
    vector < CStep * > mStepsX;
    vector < CStep * > mStepsInd;
    
    /**
     *  for array of conserved moieties
     *  @supplierCardinality 0..*
     *  @associates <{CMoiety}>
     */
    CCopasiVector < CMoiety > * mMoieties;

    /**
     *   Stoichiometry Matrix
     */
    TNT::Matrix < C_FLOAT64 > mStoi;
    
    /**
     *   Reduced Stoichiometry Matrix
     */
    TNT::Matrix < C_FLOAT64 > mRedStoi;
    
    /**
     *   Conservation Relationship
     */
    TNT::Matrix < C_FLOAT64 > mConsRel;
    
#ifdef XXXX
    /**
     *  number of internal metabolites
     */
    C_INT32 mIntMetab;

    /**
     *  number of independent metabolites
     */
    C_INT32 mIndMetab;

    /**
     *  number of dependent metabolites
     */
    C_INT32 mDepMetab;

    /**
     *  number of external metabolites
     */
    C_INT32 mExtMetab;

    /**
     * 
     */
    // the overall transition time of the pathway
    C_FLOAT64 mTransTime;


    // reaction network

    /**
     * 
     */
    // for the stoichiometry matrix (ptrs for the rows)
    vector < vector < C_FLOAT32 > > mStoichiometry;

    /**
     * 
     */
    // for the reduced stoichiometry matrix (ptrs for the rows)
    vector < vector < C_FLOAT32 > > mRedStoi;

    /**
     * 
     */
    // to index rows from primary to secondary matrix notation - Stoi notation to RedStoi
    vector < C_INT32 > mRow;

    /**
     * 
     */
    // to index cols from primary to secondary matrix notation
    vector < C_INT32 > mCol;

    /**
     * 
     */
    // to index rows from secondary to primary matrix notation - RedStoi notation to Stoi
    vector < C_INT32 > mIRow;

    /**
     * 
     */
    // to index cols from secondary to primary matrix notation
    vector < C_INT32 > mICol;

    /**
     * 
     */
    // for the reaction structure description vectors
    vector < vector < C_INT32 > > mReactStruct;

    /**
     * 
     */
    // for the array of conservation relations (ptrs to the rows)
    vector < vector < C_FLOAT32 > > mConsRel;

    /**
     * 
     */
    // multipliers of gauss reduction (IntMet x IntMet)
    vector < vector < C_FLOAT32 > > mML;

    /**
     * 
     */
    // for the kernel of the stoichiometry matrix (ptrs for the rows)
    vector < vector < C_FLOAT32 > > mKernel;

    /**
     * 
     */
    // permutation matrix (index of Kernel's rows
    vector < C_INT32 > mKernelP;

    /**
     * 
     */
    // dimensions of the Kernel matrix
    C_INT32 mKernelI, mKernelJ;

    /**
     * 
     */
    // rank of the stoichiometry matrix
    C_INT32 mRank;

    /**
     * 
     */
    // number of reactions in equil at steady state
    C_INT32 mDetBalance;

    /**
     * 
     */
    // inverse of ml (IntMet x IntMet)
    vector < vector < C_FLOAT32 > > mLM;

    /**
     * 
     */
    // temporary file for debugging
    string   mDebugFile;   // temporary file for debugging
#endif // XXXX

public:
    /**
     * 
     */
    CModel();        // constructor

    /**
     * 
     */
    void Init();
    
    /**
     * 
     */
    ~CModel();        // destructor (deallocation code here)

    /**
     * 
     */
    void Delete();
    
    /**
     *  Loads an object with data coming from a CReadConfig object.
     *  (CReadConfig object reads an input stream)
     *  @param pconfigbuffer reference to a CReadConfig object.
     *  @return Fail
     */
    C_INT32 Load(CReadConfig &configBuffer);

    /**
     *  Saves the contents of the object to a CWriteConfig object.
     *  (Which usually has a file attached but may also have socket)
     *  @param pconfigbuffer reference to a CWriteConfig object.
     *  @return Fail
     */
    C_INT32 Save(CWriteConfig &configBuffer);

    /**
     *  Build the Stoichiometry Matrix from the chemical equations of the steps
     */
    void BuildStoi();
    
    /**
     *  LU-Decomposition of the stoichiometry matrix
     */
    void LUDecomposition();
    
    /**
     *  Set the status of the metabolites
     */
    void SetMetabolitesStatus();

    /**
     *  Build the Reduced Stoichiometry Matrix from the LU decomposition
     */
    void BuildRedStoi();

    /**
     *  Build the Conservation Relationssips based on the LU decomposition
     */
    void BuildConsRel();
    
    /**
     *  Build the Moities based on the LU decomposition
     */
    void BuildMoieties();
    
    /**
     *  This calculate the right hand side (ydot) of the ODE for LSODA
     */
    void LSODAEval(C_INT32 n, C_FLOAT64 t, C_FLOAT64 * y, C_FLOAT64 * ydot);

    vector < CMetab * > & GetMetabolitesInd();
    
    /**
     *  Set the vector of particle numbers for independent metabolites
     *  @param C_FLOAT64 & y
     */
    void SetParticleNoInd(const C_FLOAT64 & y) const;

    /**
     *  Get the dimension of the reduced problem
     *  @return C_INT32 dimension
     */
    C_INT32 GetDimension() const;

    /**
     *  Set the concentration of all metabolites as a result of the particle
     *  number of the independent metabolites
     *  param C_FLOAT64 & y
     */
    void SetConcentrations(const C_FLOAT64 *y);
    
#ifdef XXXX
    /**
     *  Assignement operator. 
     */
    CModel &operator=(CModel &);    // overloaded assignment operator

    /**
     * 
     */
    void Reset(C_INT32 nstep, C_INT32 nmetab,
               C_INT32 intmet, C_INT32 nmoiety,
               C_INT32 ncompart,
               const string &Tit);   // resets the model

    /**
     * 
     */
    void ResetStepMetab(C_INT32 nstep, C_INT32 nmetab, C_INT32 intmet, C_INT32 nmoiety); //resets all nut compartments and title

    /**
     * 
     */
    C_INT32 ResetCompartments(C_INT32 ncompart); // clears the compartments array and resizes it

    /**
     * 
     */
    void ClearStoi(void);    // zeroes all entries in stoichimetry matxs

    /**
     * 
     */
    void Clear(void);     // clears the model (incl resetting to 0)

    /**
     * 
     */
    void CountMetabolites(void);   // counts the metabolites in each class

    /**
     * 
     */
    void SortMetabolites(void);   // sorts the metabolite for secondary matrices

    /**
     * 
     */
    void Structural(void);    // identifies structural properties

    /**
     * 
     */
    C_INT32 ElementaryModes(CWriteConfig &configbuffer); // finds the elementary modes

    /**
     * 
     */
    void AddToStoi(C_INT32 StepNo, 
                   C_INT32 MetabNo,
                   C_INT32 Sign, C_INT32 Value); // adds an entry in Stoi and in the relevant step

    /**
     * 
     */
    void SetupMoiety(void);    // sets the description and concentration of moieties

    /**
     * 
     */
    void UpdateChemEq(void);    // updates the ChemEq equations of all steps

    /**
     * 
     */
    void InitDebug(void);    // For debug output

    /**
     * 
     */
    void PrintSummary(void);    // For debug output

    /**
     * 
     */
    void PrintMatrices(void);   // For debug output

    /**
     * 
     */
    C_INT32 FindMetab(string &Target);  // returns the index of the metab

    /**
     * 
     */
    C_INT32 FindStep(string &Target);  // returns the index of the step

    /**
     * 
     */
    C_INT32 FindMoiety(string &Target);  // returns the index of the moiety

    /**
     * 
     */
    C_INT32 FindCompartment(string &Target);// returns the index of the compartment

    /**
     * 
     */
    void CalculateMoieties(void);  // calculates the total masses of the moieties

    /**
     * 
     */
    C_INT32 SubstrateMolecularity(C_INT32 st); // returns molecularity (substrates)

    /**
     * 
     */
    C_INT32 ProductMolecularity(C_INT32 st);  // returns molecularity (products)

    /**
     * 
     */
    void UpdateCompartments(void);  // updates references to compartments

    /**
     * 
     */
    void SaveUDKin(CWriteConfig &configbuffer); // saves the user-defined kinetic types in this model

    /**
     * 
     */
    C_INT32 AddMetabolite(string &NewName);// adds a new metabolite to the model

    /**
     * 
     */
    void UpdateMoieties(void);

    /**
     * 
     */
    // determine the kernel of the stoichiometry matrix
    C_INT32 GetKernel(void);

private:

    /**
     * 
     */
    // operations
    // allocate memory and set pointers for a model
    void AllocModel(C_INT32 nstep, C_INT32 nmetab, C_INT32 C_INT32met, C_INT32 nmoiety, C_INT32 ncompart);

    /**
     * 
     */
    // deallocate memory and free the pointers
    void DeAlloc(void);

    /**
     * 
     */
    // initilizes the indeces to RedStoi
    void InitIndex(void);  

    /**
     * 
     */
    // copies Stoi into RedStoi
    void InitRedStoi(void);

    /**
     * 
     */
    // finds metabolites that must be external
    void SpotExternal(void);

    /**
     * 
     */
    // checks if a row in RedStoi is empty
    C_INT32 EmptyRow(C_INT32 r);

    /**
     * 
     */
    // switches two rows in the secondary matrices
    void RedStoiRowSwitch(C_INT32 r1, C_INT32 r2);

    /**
     * 
     */
    // switches two cols in the secondary matrices
    void RedStoiColSwitch(C_INT32 c1, C_INT32 c2);

    /**
     * 
     */
    // allocates space for the ml and lm matrices
    C_INT32 CreateLM(void);

    /**
     * 
     */
    // deletes the space used by ml and lm matrices
    void DestroyLM(void);

    /**
     * 
     */
    // reduces RedStoi
    C_INT32 Gauss(void);

    /**
     * 
     */
    // identifies the conseravation relations
    void SpotDependent(void);

    /**
     * 
     */
    // Creates the mapping for secondary matrices
    void MapSecondary(void);

    /**
     * 
     */
    // mark the status of internal metabolites
    void MarkStatus(void);

    /**
     * 
     */
    // rebuild the stoichiometry matrix
    void RebuildStoi(C_INT32 flag);
#endif // XXXX
};

#endif // CModel
