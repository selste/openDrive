//
//  $Id: kbd.h 358 2009-01-12 11:29:57Z jcw $
//  $Revision: 358 $
//  $Author: jcw $
//  $Date: 2009-01-12 06:29:57 -0500 (Mon, 12 Jan 2009) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/kbd/kbd.h $
//

#ifndef _KBD_H_
#define _KBD_H_

//
//
//
int kbdGetKey (void);
int kbdGetKeyBlock (void);
int kbdGetKeyTimed (int ms);
void kbdInit (void);
signed portBASE_TYPE kbdTaskStart (void);
signed portBASE_TYPE kbdTaskStop (void);

#endif
