//
//  $Id: args.c 56 2008-10-05 03:27:02Z jcw $
//  $Revision: 56 $
//  $Author: jcw $
//  $Date: 2008-10-04 23:27:02 -0400 (Sat, 04 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/monitor/args.c $
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

//
// Scheduler includes
//
#include "FreeRTOS.h"
#include "args.h"

//
//
//
static char *strtrim (char *s)
{
  char *t = s + strlen (s) - 1;

  while (t >= s && *t && isspace (*t))
    *t-- = '\0';

  while (*s && isspace (*s))
    s++;

  return s;
}

//
//  bufferLength includes the space reserved for the \0
//
int argsGetLine (int fd __attribute__ ((unused)), U8 *buffer, int bufferLength, const char *prompt)
{
  U8 *p;

  p = buffer;
  *p = '\0';

  printf ("%s", prompt);

  while (1)
  {
    U8 c;
    
    fflush (stdout);

    if (read (fd, &c, sizeof (c)) == sizeof (c))
    {
      switch (c)
      {
        case '\n' :
        case '\r' :
          printf ("\n");
          return strlen ((char *) buffer);

        case '\b' :
          if (p > buffer)
            *--p = '\0';
          printf ("\b \b");
          break;

        case 0x15 : // CTRL-U
          while (p != buffer)
          {
            printf ("\b \b");
            --p;
          }
          *p = '\0';
          break;

        case 0xfe :
        case 0xff :
          *buffer++ = c;
          *buffer = '\0';
          return 1;

        default : 
          if (p < buffer + bufferLength - 1 && c >= ' ' && c < 0x7f)
          { 
            *p++ = c;
            *p = '\0';
            printf ("%c", c); 
          }
          else
            printf ("%c", c); 

          break;
      }
    }
  }

  return 0;
}

//
//
//
typedef enum
{
  P_EATWHITESPACE = 0,
  P_GETCHARFIRST,
  P_GETCHAR,
  P_QUOTEDGETCHAR
}
PSTATE;

int argsParse (char *cmd, char **argv, int sizeofArgv, int *argc)
{
  int maxArgs = (sizeofArgv / sizeof (argv [0])) - 1;
  char *s = strtrim (cmd);
  PSTATE pstate = P_EATWHITESPACE;

  *argc = 0;
  memset (argv, 0, sizeofArgv);

  while (*s)
  {
    switch (pstate)
    {
      case P_EATWHITESPACE :
        {
          if (!isspace (*s))
            pstate = P_GETCHARFIRST;
          else
            s++;
        }
        break;

      case P_GETCHARFIRST :
        {
          *argv++ = s;

          if (++*argc == maxArgs)
            return 1;
          if (*s == '"')
            pstate = P_QUOTEDGETCHAR;
          else
            pstate = P_GETCHAR;

          s++;
        }
        break;

      case P_GETCHAR :
        {
          if (isspace (*s)) {
            pstate = P_EATWHITESPACE;
            *s = '\0';
          } 
          else if (*s == '"')
            pstate = P_QUOTEDGETCHAR;

          s++;
        }
        break;

      case P_QUOTEDGETCHAR :
        {
          if (*s == '"')
            pstate = P_GETCHAR;

          s++;
        }
        break;
    }
  }

  return 0;
}
