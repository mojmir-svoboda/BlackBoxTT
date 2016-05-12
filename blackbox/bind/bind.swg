%module example
%{
/* Includes the header in the wrapper code */
#include "bind.h"
%}

/* Parse the header file to generate wrappers */
%include "bind.h"

