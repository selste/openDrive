/*******************************************************************************
  Project: Mini-TMCL

  Module:  Mini-TMCL.h
           Globaal definitions

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

#include <stdint.h>


#define FALSE 0
#define TRUE 1

#define BYTE3(var) *((uint8_t*)&var+3)
#define BYTE2(var) *((uint8_t*)&var+2)
#define BYTE1(var) *((uint8_t*)&var+1)
#define BYTE0(var) *((uint8_t*)&var)



//Global settings (in this example only RS232 settings)
typedef struct
{
  uint8_t SerialBitrate;
  uint8_t SerialModuleAddress;
  uint8_t SerialHostAddress;
} TModuleConfig;

//Motor configuration data
typedef struct
{
  uint16_t VMax;
  uint16_t AMax;
  uint8_t PulseDiv;
  uint8_t RampDiv;
} TMotorConfig;
