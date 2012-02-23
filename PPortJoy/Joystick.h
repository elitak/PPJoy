/***************************************************************************
 *   PPJoy Virtual Joystick for Microsoft Windows                          *
 *   Copyright (C) 2011 Deon van der Westhuysen                            *
 *                                                                         *
 *   This program is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 ***************************************************************************/


#ifndef	__JOYSTICK_H__
#define	__JOYSTICK_H__

#include "PPortJoy.h"


#define	BUTTON1(RAWINPUT,NEGBITTEST)			{ (RAWINPUT)->Digital[4]= (!(NEGBITTEST)); }
#define	BUTTON2(RAWINPUT,NEGBITTEST)			{ (RAWINPUT)->Digital[5]= (!(NEGBITTEST)); }
#define	BUTTON3(RAWINPUT,NEGBITTEST)			{ (RAWINPUT)->Digital[6]= (!(NEGBITTEST)); }
#define	BUTTON4(RAWINPUT,NEGBITTEST)			{ (RAWINPUT)->Digital[7]= (!(NEGBITTEST)); }
#define	BUTTON5(RAWINPUT,NEGBITTEST)			{ (RAWINPUT)->Digital[8]= (!(NEGBITTEST)); }
#define	BUTTON6(RAWINPUT,NEGBITTEST)			{ (RAWINPUT)->Digital[9]= (!(NEGBITTEST)); }
#define	BUTTON7(RAWINPUT,NEGBITTEST)			{ (RAWINPUT)->Digital[10]= (!(NEGBITTEST)); }
#define	BUTTON8(RAWINPUT,NEGBITTEST)			{ (RAWINPUT)->Digital[11]= (!(NEGBITTEST)); }
#define	BUTTON9(RAWINPUT,NEGBITTEST)			{ (RAWINPUT)->Digital[12]= (!(NEGBITTEST)); }
#define	BUTTON10(RAWINPUT,NEGBITTEST)			{ (RAWINPUT)->Digital[13]= (!(NEGBITTEST)); }
#define	BUTTON11(RAWINPUT,NEGBITTEST)			{ (RAWINPUT)->Digital[14]= (!(NEGBITTEST)); }
#define	BUTTON12(RAWINPUT,NEGBITTEST)			{ (RAWINPUT)->Digital[15]= (!(NEGBITTEST)); }
#define	BUTTON13(RAWINPUT,NEGBITTEST)			{ (RAWINPUT)->Digital[16]= (!(NEGBITTEST)); }
#define	BUTTON14(RAWINPUT,NEGBITTEST)			{ (RAWINPUT)->Digital[17]= (!(NEGBITTEST)); }
#define	BUTTON15(RAWINPUT,NEGBITTEST)			{ (RAWINPUT)->Digital[18]= (!(NEGBITTEST)); }
#define	BUTTON16(RAWINPUT,NEGBITTEST)			{ (RAWINPUT)->Digital[19]= (!(NEGBITTEST)); }

//#define	AXIS_UDIGITAL(RAWINPUT,IDX,NMIN,NMAX)	{ (RAWINPUT)->Axis[IDX]= (PPJOY_AXIS_MAX+PPJOY_AXIS_MIN)/2+((!(NMAX))-(!(NMIN)))*(PPJOY_AXIS_MAX-PPJOY_AXIS_MIN)/2; }
//#define	AXISLR(RAWINPUT,NEGLEFT,NEGRIGHT)		AXIS_UDIGITAL(RAWINPUT,0,NEGLEFT,NEGRIGHT)
//#define	AXISUD(RAWINPUT,NEGUP,NEGDOWN)			AXIS_UDIGITAL(RAWINPUT,1,NEGUP,NEGDOWN)

#define	AXISLR(RAWINPUT,NEGLEFT,NEGRIGHT)		{ (RAWINPUT)->Digital[2]= (!(NEGLEFT)); (RAWINPUT)->Digital[3]= (!(NEGRIGHT)); }
#define	AXISUD(RAWINPUT,NEGUP,NEGDOWN)			{ (RAWINPUT)->Digital[0]= (!(NEGUP)); (RAWINPUT)->Digital[1]= (!(NEGDOWN)); }


#define	AXIS_UANALOG(RAWINPUT,IDX,DATA,MIN,MAX)	{ (RAWINPUT)->Analog[IDX]= ((DATA-MIN)*(PPJOY_AXIS_MAX-PPJOY_AXIS_MIN))/(MAX-MIN) + PPJOY_AXIS_MIN; }
#define	AXIS_UANALOG_64(RAWINPUT,IDX,DATA,MIN,MAX)	{ (RAWINPUT)->Analog[IDX]= (ULONG) (((DATA-MIN)*(PPJOY_AXIS_MAX-PPJOY_AXIS_MIN))/(MAX-MIN) + PPJOY_AXIS_MIN); }

#define	AXISLR_UANALOG(RAWINPUT,DATA,MIN,MAX)	AXIS_UANALOG(RAWINPUT,0,DATA,MIN,MAX)
#define	AXISUD_UANALOG(RAWINPUT,DATA,MIN,MAX)	AXIS_UANALOG(RAWINPUT,1,DATA,MIN,MAX)
#define	AXISX2_UANALOG(RAWINPUT,DATA,MIN,MAX)	AXIS_UANALOG(RAWINPUT,2,DATA,MIN,MAX)
#define	AXISY2_UANALOG(RAWINPUT,DATA,MIN,MAX)	AXIS_UANALOG(RAWINPUT,3,DATA,MIN,MAX)

#if 0

// #define	MAX_ANALOG_RAW				64
// #define	MAX_DIGITAL_RAW				128

#if PPJOY_MAX_AXES<1
#undef AXISLR
#define	AXISLR(RAWINPUT,NEGLEFT,NEGRIGHT)
#define	AXISLR_UANALOG(RAWINPUT,DATA,MIN,MAX)
#endif

#if PPJOY_MAX_AXES<2
#undef AXISUD
#define	AXISUD(RAWINPUT,NEGUP,NEGDOWN)
#define	AXISUD_UANALOG(RAWINPUT,DATA,MIN,MAX)
#endif

#if PPJOY_MAX_AXES<3
#undef AXISUD
#define	AXISX2_UANALOG(RAWINPUT,DATA,MIN,MAX)
#endif

#if PPJOY_MAX_AXES<4
#undef AXISUD
#define	AXISY2_UANALOG(RAWINPUT,DATA,MIN,MAX)
#endif

#if PPJOY_MAX_BUTTONS<1
#undef	BUTTON1
#define	BUTTON1(RAWINPUT,NEGBITTEST)					
#endif

#if PPJOY_MAX_BUTTONS<2
#undef	BUTTON2
#define	BUTTON2(RAWINPUT,NEGBITTEST)					
#endif

#if PPJOY_MAX_BUTTONS<3
#undef	BUTTON3
#define	BUTTON3(RAWINPUT,NEGBITTEST)					
#endif

#if PPJOY_MAX_BUTTONS<4
#undef	BUTTON4
#define	BUTTON4(RAWINPUT,NEGBITTEST)					
#endif

#if PPJOY_MAX_BUTTONS<5
#undef	BUTTON5
#define	BUTTON5(RAWINPUT,NEGBITTEST)					
#endif

#if PPJOY_MAX_BUTTONS<6
#undef	BUTTON6
#define	BUTTON6(RAWINPUT,NEGBITTEST)					
#endif

#if PPJOY_MAX_BUTTONS<7
#undef	BUTTON7
#define	BUTTON7(RAWINPUT,NEGBITTEST)					
#endif

#if PPJOY_MAX_BUTTONS<8
#undef	BUTTON8
#define	BUTTON8(RAWINPUT,NEGBITTEST)					
#endif

#if PPJOY_MAX_BUTTONS<9
#undef	BUTTON9
#define	BUTTON9(RAWINPUT,NEGBITTEST)					
#endif

#if PPJOY_MAX_BUTTONS<10
#undef	BUTTON10
#define	BUTTON10(RAWINPUT,NEGBITTEST)					
#endif

#if PPJOY_MAX_BUTTONS<11
#undef	BUTTON11
#define	BUTTON11(RAWINPUT,NEGBITTEST)					
#endif

#if PPJOY_MAX_BUTTONS<12
#undef	BUTTON12
#define	BUTTON12(RAWINPUT,NEGBITTEST)					
#endif

#if PPJOY_MAX_BUTTONS<13
#undef	BUTTON13
#define	BUTTON13(RAWINPUT,NEGBITTEST)					
#endif

#if PPJOY_MAX_BUTTONS<14
#undef	BUTTON14
#define	BUTTON14(RAWINPUT,NEGBITTEST)					
#endif

#if PPJOY_MAX_BUTTONS<15
#undef	BUTTON15
#define	BUTTON15(RAWINPUT,NEGBITTEST)					
#endif

#if PPJOY_MAX_BUTTONS<16
#undef	BUTTON16
#define	BUTTON16(RAWINPUT,NEGBITTEST)					
#endif

#endif


#define	GENESIS_SCAN_SKELETON(JOYSUBTYPE,SEL_PORT,SET_SEL_LOW,SET_SEL_HIGH,READ_LOW,READ_HIGH,READ_P3,BIT_DELAY,BIT6_DELAY)	\
{																			\
 ULONG	ScanDelay;															\
																			\
 ScanDelay= ((JOYSUBTYPE)==SUBTYPE_GENESYS6)?BIT6_DELAY:BIT_DELAY;			\
																			\
 /* Read "low" data */														\
 WRITE_PORT_UCHAR (SEL_PORT,SET_SEL_LOW);	/* 1 */							\
 KeStallExecutionProcessor (ScanDelay);										\
 { READ_LOW }																\
																			\
 WRITE_PORT_UCHAR (SEL_PORT,SET_SEL_HIGH);									\
 KeStallExecutionProcessor (ScanDelay);										\
 { READ_HIGH }																\
																			\
 if ((JOYSUBTYPE)==SUBTYPE_GENESYS6)										\
 {																			\
  WRITE_PORT_UCHAR (SEL_PORT,SET_SEL_LOW);	/* 2 */							\
  KeStallExecutionProcessor (ScanDelay);									\
  WRITE_PORT_UCHAR (SEL_PORT,SET_SEL_HIGH);									\
  KeStallExecutionProcessor (ScanDelay);									\
																			\
  WRITE_PORT_UCHAR (SEL_PORT,SET_SEL_LOW);	/* 3 */							\
  KeStallExecutionProcessor (ScanDelay);									\
  WRITE_PORT_UCHAR (SEL_PORT,SET_SEL_HIGH);									\
  KeStallExecutionProcessor (ScanDelay);									\
  { READ_P3 }																\
																			\
											/* 4 */							\
  /*																		\
  WRITE_PORT_UCHAR (SEL_PORT,SET_SEL_LOW);									\
  KeStallExecutionProcessor (ScanDelay);									\
  WRITE_PORT_UCHAR (SEL_PORT,SET_SEL_HIGH);									\
  */																		\
 }																			\
}

#define	GENESIS_REPORT_SKELETON(RAWINPUT,JOYSUBTYPE,LEFT_LOW,RIGHT_LOW,FIRE1_LOW,FIRE2_LOW,UP_HIGH,DOWN_HIGH,LEFT_HIGH,RIGHT_HIGH,FIRE1_HIGH,FIRE2_HIGH,UP_P3,DOWN_P3,LEFT_P3,RIGHT_P3)	\
{																			\
 BUTTON1(RAWINPUT,FIRE1_LOW)		/* A */									\
 BUTTON2(RAWINPUT,FIRE1_HIGH)		/* B */									\
 BUTTON3(RAWINPUT,FIRE2_HIGH)		/* C */									\
 BUTTON4(RAWINPUT,FIRE2_LOW)		/* Start */								\
 if ((JOYSUBTYPE)==SUBTYPE_GENESYS6)										\
 {																			\
  BUTTON5(RAWINPUT,LEFT_P3)			/* X */									\
  BUTTON6(RAWINPUT,DOWN_P3)			/* Y */									\
  BUTTON7(RAWINPUT,UP_P3)			/* Z */									\
  BUTTON8(RAWINPUT,RIGHT_P3)		/* Mode */								\
 }																			\
 else																		\
 {																			\
  BUTTON5(RAWINPUT,LEFT_LOW)		/* X */									\
  BUTTON6(RAWINPUT,RIGHT_LOW)		/* Y */									\
 }																			\
																			\
 AXISLR (RAWINPUT,LEFT_HIGH,RIGHT_HIGH)										\
 AXISUD (RAWINPUT,UP_HIGH,DOWN_HIGH)										\
}


#define	SNES_REPORT_SKELETON(RAWINPUT,JOYSUBTYPE,INPUTARRAY,BITMASK)		\
{																			\
 if ((JOYSUBTYPE)==SUBTYPE_SNES_NES)		/* NES pad */					\
 {																			\
  BUTTON1(RAWINPUT,((INPUTARRAY[0])&(BITMASK)))								\
  BUTTON2(RAWINPUT,((INPUTARRAY[1])&(BITMASK)))								\
  BUTTON3(RAWINPUT,((INPUTARRAY[2])&(BITMASK)))								\
  BUTTON4(RAWINPUT,((INPUTARRAY[3])&(BITMASK)))								\
 }																			\
 else										/* SNES or Virtual Gameboy */	\
 {																			\
  BUTTON1(RAWINPUT,((INPUTARRAY[8])&(BITMASK)))								\
  BUTTON2(RAWINPUT,((INPUTARRAY[0])&(BITMASK)))								\
  BUTTON3(RAWINPUT,((INPUTARRAY[2])&(BITMASK)))								\
  BUTTON4(RAWINPUT,((INPUTARRAY[3])&(BITMASK)))								\
  BUTTON5(RAWINPUT,((INPUTARRAY[9])&(BITMASK)))								\
  BUTTON6(RAWINPUT,((INPUTARRAY[1])&(BITMASK)))								\
  BUTTON7(RAWINPUT,((INPUTARRAY[10])&(BITMASK)))							\
  BUTTON8(RAWINPUT,((INPUTARRAY[11])&(BITMASK)))							\
  BUTTON9(RAWINPUT,((INPUTARRAY[12])&(BITMASK)))							\
  BUTTON10(RAWINPUT,((INPUTARRAY[13])&(BITMASK)))							\
  BUTTON11(RAWINPUT,((INPUTARRAY[14])&(BITMASK)))							\
  BUTTON12(RAWINPUT,((INPUTARRAY[15])&(BITMASK)))							\
 }																			\
																			\
 AXISLR (RAWINPUT,((INPUTARRAY[6])&(BITMASK)),((INPUTARRAY[7])&(BITMASK)))	\
 AXISUD (RAWINPUT,((INPUTARRAY[4])&(BITMASK)),((INPUTARRAY[5])&(BITMASK)))	\
}

#endif
