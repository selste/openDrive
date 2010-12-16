/*******************************************************************************
  Project: Mini-TMCL

  Module:  TMC428.h
           Defintions for TMC428 register addresses and TMC428 support functions.

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

#define IDX_LOW_WORD 0x60
#define IDX_HIGH_WORD 0x62
#define IDX_COVER_POS_LEN 0x64
#define IDX_COVER_DATA 0x66

#define IDX_REF_SWITCHES 0x7c
#define IDX_SMGP 0x7e

#define TMC428_READ 0x01
#define READ_REG 0x01

#define RM_RAMP 0
#define RM_SOFT 1
#define RM_VELOCITY 2
#define RM_HOLD 3

#define NO_REF 0x0f

void ReadWriteSPI4(uint8_t *incoming, uint8_t *outgoing);
void ReadWrite428(uint8_t *Read, uint8_t *Write);
void Write428Bytes(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
void Write428Long(uint8_t Addr, int32_t Value);
void Write428Int(uint8_t Addr, int16_t Value);
void Write428Zero(uint8_t Addr);
uint16_t Read428Int(uint8_t Addr);
uint32_t Read428Long(uint8_t Addr);
uint8_t Read428Byte(uint8_t Addr, uint8_t Index);
uint8_t Read428Status(void);
void Init428(void);

int8_t SetAMax(uint8_t Axis, uint16_t AMax);
void Set428RampMode(uint8_t Axis, uint8_t RampMode);
void Set428SwitchMode(uint8_t Axis, uint8_t SwitchMode);
