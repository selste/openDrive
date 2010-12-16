/***********************************************************
   Sample code for using the TMC428 with the TMC236.

  Header file containing the register names of the TMC428.

   Copyright (C) 2005 TRINAMIC Motion Control GmbH & Co. KG
                      Sternstrasse 67
                      D - 20357 Hamburg, Germany
                      http://www.trinamic.com/
   
   This program is free software; you can redistribute it and/or modify it 
   under the terms of the GNU General Public License as published by the 
   Free Software Foundation; either version 2 of the License, or (at your 
   option) any later version.
   
   This program is distributed in the hope that it will be useful, but 
   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
   or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
   for more details.
   
   You should have received a copy of the GNU General Public License along 
   with this program; if not, write to the Free Software Foundation, Inc., 
   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
************************************************************/


#define MOTOR0 0
#define MOTOR1 0x20
#define MOTOR2 0x40

#define IDX_XTARGET 0x00
#define IDX_XACTUAL 0x02
#define IDX_VMIN 0x04
#define IDX_VMAX 0x06
#define IDX_VTARGET 0x08
#define IDX_VACTUAL 0x0A
#define IDX_AMAX 0x0C
#define IDX_AACTUAL 0x0E
#define IDX_AGTAT_ALEAT 0x10
#define IDX_PMUL_PDIV 0x12
#define IDX_REFCONF_RM 0x14
#define IDX_IMASK_IFLAGS 0x16
#define IDX_PULSEDIV_RAMPDIV 0x18
#define IDX_DX_REFTOLERANCE 0x1A
#define IDX_XLATCHED 0x1C

#define IDX_SMGP 0x7e

#define READ_REG 0x01

#define RM_RAMP 0
#define RM_SOFT 1
#define RM_VELOCITY 2
#define RM_HOLD 3

#define NO_REF 0x0f

