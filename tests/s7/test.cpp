#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "s7.h"

static s7_pointer add1(s7_scheme *sc, s7_pointer args)
{
  /* all added functions have this form, args is a list, 
   *    s7_car(args) is the first arg, etc 
   */
  if (s7_is_integer(s7_car(args)))
    return(s7_make_integer(sc, 1 + s7_integer(s7_car(args))));
  return(s7_wrong_type_arg_error(sc, "add1", 1, s7_car(args), "an integer"));
}

int main(int argc, char **argv)
{
  s7_scheme *s7;
  char buffer[512];
  char response[1024];

  s7 = s7_init();
  
  s7_define_function(s7, "add1", add1, 1, 0, false, "(add1 int) adds 1 to int");
                                      /* add the function "add1" to the interpreter.
                                       *   1, 0, false -> one required arg,
				       *                  no optional args,
				       *                  no "rest" arg
				       */
 s7_define_variable(s7, "my-pi", s7_make_real(s7, 3.14159265));

  while (1)                           /* fire up a "repl" */
    {
      fprintf(stdout, "\n> ");        /* prompt for input */
      fgets(buffer, 512, stdin);
      if ((buffer[0] != '\n') || 
	  (strlen(buffer) > 1))
	{                            
	  sprintf(response, "(write %s)", buffer);
	  s7_eval_c_string(s7, response); /* evaluate input and write the result */
	}
    }
}
