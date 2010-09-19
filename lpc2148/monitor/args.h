//
//  $Id: args.h 42 2008-10-04 18:40:36Z jcw $
//  $Revision: 42 $
//  $Author: jcw $
//  $Date: 2008-10-04 14:40:36 -0400 (Sat, 04 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/monitor/args.h $
//

#ifndef _ARGS_H_
#define _ARGS_H_

int argsGetLine (int fd, U8 *buffer, int bufferLength, const char *prompt);
int argsParse (char *cmd, char **argv, int sizeofArgv, int *argc);

#endif
