/**
  CTrajectory.h
  copyright            : (C) 2001 by
 *
 */

#ifndef CTRAJECTORY_H
#define CTRAJECTORY_H

#include "CModel.h"
#include "CODESolver.h"
// #include "COutputEvent.h"

class CTrajectory
{
//Attributes
private:

  /**
   *  The pointer to the CModel with which we are working
   */
    CModel * mModel;

    /**
     *  The pointer to the CODESolver with which we are working
     */
    CODESolver * mODESolver;

    /**
     *  The time points
     */
    C_INT32 mPoints;

    /**
     *  The size of the array mY
     */
    C_INT32 mN;

    /**
     *  The end of time point
     */
    C_FLOAT64 mEndTime;

    /**
     *  Pointer to array of doubles "y", vector of mole numbers for each particle
     */
    C_FLOAT64 * mY;

    /**
     *  the method to use
     */
    C_INT32 mMethod;

//Operations
public:

    /**
     * default constructor
     */
    CTrajectory();
	
    /*
     * A CTrajectory constructor
     */	
    CTrajectory(CModel * aModel, C_INT32 aPoints,
                C_FLOAT64 aEndTime, C_INT32 aMethod);

    /**
     * Copy constructor
     * @param source a CTrajectory object for copy
     */
    CTrajectory(const CTrajectory& source);

    /**
     * Object assignment overloading
     * @param source a CTrajectory object for copy
     * @return an assigned CTrajectory object
     */
    CTrajectory& operator=(const CTrajectory& source);


    /**
     * destructor
     */	
    ~CTrajectory();

    /**
     * initialize()
     * @param CModel * aModel
     */
    void initialize(CModel * aModel);
        
    /**
     * cleanup()
     */
    void cleanup();
        
    /**
     * Set the CModel member variable
     * @param aModel a CModel pointer to be set as mCModel
     */	
    void setModel(CModel * aModel);

    /**
     *  Get the CModel member variable
     *  @return mCModel
     */	
    CModel * getModel() const;

    /**
     * Set the CODESolver member variable
     * @param aSolver a CODESolver pointer to be set as mModel member
     */	
    void setODESolver(CODESolver * aSolver);

    /**
     *  Get the CODESolver member variable
     *  @return mODESolver
     */	
    CODESolver * getSolver() const;

    /*
     * Set the time points
     * @param anInt an int passed to be set as mPoints
     */
    void setPoints(const C_INT32 anInt);

    /*
     * Get the time points
     * @return mPoints
     */
    C_INT32 getPoints() const;

    /*
     * Set the size of array mY, can be obtained from CModel (how??)
     * @param anInt an int passed to be set as mN
     */
    void setArrSize(const C_INT32 anInt);

    /*
     * Get the mY array size
     * @return mN
     */
    C_INT32 getArrSize() const;

    /*
     * Set the end of time points
     * @param aDouble a double type to be set as mEndTime
     */	
    void setEndTime(const C_FLOAT64 aDouble);

    /*
     * Get the end of time points
     * @return mEndTime
     */
    C_FLOAT64 getEndTime() const;

    /*
     * Set the point to the array with the left hand side value
     * @param arrDouble an array of double that will be set as mY
     */	
    void setMY(const C_FLOAT64 * arrDouble);

    /*
     * Get the point to the array with the left hand side value
     * @return mY
     */
    C_FLOAT64 * getMY() const;

    /*
     * Set the method to use
     * @param anInt an int type that will set as mTypeOfSolver
     */
    void setMethod(const C_INT32 anInt);

    /*
     * Get the type of solver
     * @return mTypeOfSolver
     */
    C_INT32 getMethod() const;


    /*
     * Process the CTrajectory primary function
     */	
    void process();
	
};


#endif //CTRAJECOTRY_H
