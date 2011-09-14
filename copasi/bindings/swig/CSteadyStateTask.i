// Begin CVS Header 
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/bindings/swig/CSteadyStateTask.i,v $ 
//   $Revision: 1.6.2.1 $ 
//   $Name:  $ 
//   $Author: gauges $ 
//   $Date: 2011/09/14 15:11:48 $ 
// End CVS Header 

// Copyright (C) 2011 - 2010 by Pedro Mendes, Virginia Tech Intellectual 
// Properties, Inc., University of Heidelberg, and The University 
// of Manchester. 
// All rights reserved. 

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual 
// Properties, Inc., EML Research, gGmbH, University of Heidelberg, 
// and The University of Manchester. 
// All rights reserved. 

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual 
// Properties, Inc. and EML Research, gGmbH. 
// All rights reserved. 

// Copyright © 2005 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

%{

#include "steadystate/CSteadyStateMethod.h"
#include "steadystate/CSteadyStateTask.h"

%}

%ignore operator<<;

// process is ignored because I have written extension code in CCopasiTask
// that calls the task specific process, so this new process should be
// used for all tasks
%ignore CSteadyStateTask::process(const bool& useInitialValues);
%ignore CSteadyStateTask::initialize;
%ignore CSteadyStateTask::load;
%ignore CSteadyStateTask::print;

%rename (getResult) CSteadyStateTask::getResult() const; // unignore %getResult()

%include "steadystate/CSteadyStateTask.h"


