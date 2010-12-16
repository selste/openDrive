/*******************************************************************************
  Project: Mini-TMCL

  Module:  Globals.c
           Global variables and data structures

   Copyright (C) 2008 TRINAMIC Motion Control GmbH & Co KG
                      Sternstraﬂe 67
                      D - 20357 Hamburg, Germany
                      http://www.trinamic.com/

   This program is free software; you can redistribute it and/or modify it
   freely.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.
*******************************************************************************/

#include "Mini-TMCL.h"
// #include "Commands.h"


//Global parameters (here only used for the RS232 interface)
TModuleConfig ModuleConfig=
{
  0,      //RS232 bitrate (0=9600)
  1,      //RS232 address
  2       //RS232 reply address
};

//Motor configuration data
TMotorConfig MotorConfig[3]=
{{
  1678,   //VMax
  1,      //AMax
  2,      //Pulsediv
  3       //Rampdiv
 },
 {
  1000,   //VMax
  500,    //AMax
  2,      //Pulsediv
  3       //Rampdiv
 },
 {
  1000,   //VMax
  500,    //AMax
  2,      //Pulsediv
  3       //Rampdiv
 }
};
