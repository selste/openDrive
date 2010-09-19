//
//  $Id: uiptask.h 345 2008-11-11 03:05:06Z jcw $
//  $Revision: 345 $
//  $Author: jcw $
//  $Date: 2008-11-10 22:05:06 -0500 (Mon, 10 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/uip/uiptask.h $
//

#ifndef UIP_TASK_H
#define UIP_TASK_H

#include "uip/uip.h"

//
//
//
typedef enum
{
  UIPSTATE_OK = 0,
  UIPSTATE_ALREADYRUNNING,
  UIPSTATE_NOTRUNNING,
  UIPSTATE_TASKDIDNTSTART,
  UIPSTATE_ENCJ28C60ERR,
}
uipState_e;

//
//
//
void uipFastPoll (struct uip_conn *conn);
portBASE_TYPE uipIsRunning (void);
uipState_e uipStart (int doDHCP);
uipState_e uipStop (void);

#endif
