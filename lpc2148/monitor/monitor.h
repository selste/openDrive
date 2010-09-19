//
//  $Id: monitor.h 285 2008-11-06 01:23:29Z jcw $
//  $Revision: 285 $
//  $Author: jcw $
//  $Date: 2008-11-05 20:23:29 -0500 (Wed, 05 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/monitor/monitor.h $
//

#ifndef _MONITOR_H_
#define _MONITOR_H_

#include "FreeRTOS.h"
#include "task.h"

//
//
//
typedef enum
{
  CMDTYPE_CMDLIST = 0,
  CMDTYPE_FUNCTION
}
cmdType_e;

//
//
//
typedef struct commandList_s
{
  const portCHAR *command;
  portCHAR minArgs;
  portCHAR maxArgs;
  cmdType_e cmdType;
  union
  {
    const void *trickGCC;
    int (*handler) (int argc, portCHAR **argv);
    struct commandList_s *commandList;
  };
  const portCHAR *description;
  const portCHAR *parameters;
}
commandList_t;

//
//
//
signed portBASE_TYPE monitorTaskStart (void);

#endif
