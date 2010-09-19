//
//  $Id: argsdispatch.c 104 2008-10-10 19:44:23Z jcw $
//  $Revision: 104 $
//  $Author: jcw $
//  $Date: 2008-10-10 15:44:23 -0400 (Fri, 10 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/monitor/argsdispatch.c $
//

#include <stdio.h>
#include <string.h>

#include "monitor.h"
#include "argsdispatch.h"

extern const commandList_t *activeCommandList;

//
//
//
int argsDispatch (const commandList_t *cl, int argc, char **argv)
{
  activeCommandList = cl;

  while (cl->command)
  {
    if (!strcmp (cl->command, argv [0]))
    {
      int ret = 0;

      if ((argc == 2) && !strcmp (argv [1], "?"))
        printf ("%s\n", cl->parameters);
      else if ((argc - 1) < cl->minArgs)
        printf ("Too few arguments to command (%d expected)\n", cl->minArgs);
      else if (cl->cmdType == CMDTYPE_CMDLIST)
        argsDispatch (cl->commandList, argc - 1, &argv [1]);
      else if ((argc - 1) > cl->maxArgs)
        printf ("Too many arguments to command (%d maximum)\n", cl->maxArgs);
      else
        ret = (*cl->handler) (argc - 1, &argv [1]);

      return ret;
    }

    cl++;
  }

  if (!cl->command)
    printf ("Unknown command \"%s\"\n", argv [0]);

  return 0;
}
