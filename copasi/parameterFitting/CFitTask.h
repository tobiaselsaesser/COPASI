/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/parameterFitting/CFitTask.h,v $
   $Revision: 1.1 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2005/09/16 19:08:34 $
   End CVS Header */

/**
 * CFitTask class.
 *
 * This class implements a parameter fitting task which is comprised of a
 * of a problem and a method.
 *  
 */

#ifndef COPASI_CFitTask
 #define COPASI_CFitTask

#include "optimization/COptTask.h"

class CFitTask : public COptTask
  {
    //Attributes

  public:
    /**
     * The methods which can be selected for performing this task.
     */
    static unsigned C_INT32 validMethods[];

    /**
     * default constructor
     * @param const CCopasiTask::Type & type (default: parameterFitting)
     * @param const CCopasiContainer * pParent (default: RootContainer)
     */
    CFitTask(const CCopasiTask::Type & type = CCopasiTask::parameterFitting,
             const CCopasiContainer * pParent = & RootContainer);

    /**
     * Copy constructor
     * @param const CFitTask & src
     * @param const CCopasiContainer * pParent (default: NULL)
     */
    CFitTask(const CFitTask & src, const CCopasiContainer * pParent = NULL);

    /**
     * Destructor
     */
    ~CFitTask();

    /**
     * cleanup()
     */
    void cleanup();

    /**
     * Set the call back of the task
     * @param CProcessReport * pCallBack
     * @result bool succes
     */
    virtual bool setCallBack(CProcessReport * pCallBack);

    /**
     * Initialize the task. If an ostream is given this ostream is used
     * instead of the target specified in the report. This allows nested 
     * tasks to share the same output device.
     * @param const OutputFlag & of
     * @param std::ostream * pOstream (default: NULL)
     * @return bool success
     */
    virtual bool initialize(const OutputFlag & of, std::ostream * pOstream);

    /**
     * Process the task with or without initializing to the initial state.
     * @param const bool & useInitialValues
     * @return bool success
     */
    virtual bool process(const bool & useInitialValues);

    /**
     * Set the method type applied to solve the task
     * @param const CCopasiMethod::SubType & type
     * @return bool success
     */
    virtual bool setMethodType(const int & type);
  };
#endif // COPASI_CFitTask
