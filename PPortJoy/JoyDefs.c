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


/***************************************************************************/
/**                                                                       **/
/**  Parallel port joystick driver, (C) Deon van der Westhuysen 2002      **/
/**                                                                       **/
/**  JoyDefs.c  Structures to define support for the joystick types       **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/***************************************************************************/

#include "PPortJoy.h"
#include "hidusage.h"
#include "JoyDefs.h"

/* Included for the ECP chip mode parameters */
#include <Parallel.h>

UCHAR	DigitalJoy_OneButton_Mapping[]=	{
			2,1,0,1,	/*Axes, Buttons, Hats, Configs */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_X,	/* Axis 1 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_Y,	/* Axis 2 */
			HID_USAGE_PAGE_BUTTON,1,					/* Button 1 */
			/* Config 1 */
			DIGITAL_VALUE(2),DIGITAL_VALUE(3),			/* Left-Right axis */
			DIGITAL_VALUE(0),DIGITAL_VALUE(1),			/* Up-Down axis */
			DIGITAL_VALUE(4)							/* Button 1 */
		};

UCHAR	DigitalJoy_TwoButtons_Mapping[]=	{
			2,2,0,1,	/*Axes, Buttons, Hats, Configs */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_X,	/* Axis 1 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_Y,	/* Axis 2 */
			HID_USAGE_PAGE_BUTTON,1,					/* Button 1 */
			HID_USAGE_PAGE_BUTTON,2,					/* Button 2 */
			/* Config 1 */
			DIGITAL_VALUE(2),DIGITAL_VALUE(3),			/* Left-Right axis */
			DIGITAL_VALUE(0),DIGITAL_VALUE(1),			/* Up-Down axis */
			DIGITAL_VALUE(4),							/* Button 1 */
			DIGITAL_VALUE(5)							/* Button 2 */
		};

UCHAR	DigitalJoy_ThreeButtons_Mapping[]=	{
			2,3,0,1,	/*Axes, Buttons, Hats, Configs */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_X,	/* Axis 1 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_Y,	/* Axis 2 */
			HID_USAGE_PAGE_BUTTON,1,					/* Button 1 */
			HID_USAGE_PAGE_BUTTON,2,					/* Button 2 */
			HID_USAGE_PAGE_BUTTON,3,					/* Button 3 */
			/* Config 1 */
			DIGITAL_VALUE(2),DIGITAL_VALUE(3),			/* Left-Right axis */
			DIGITAL_VALUE(0),DIGITAL_VALUE(1),			/* Up-Down axis */
			DIGITAL_VALUE(4),							/* Button 1 */
			DIGITAL_VALUE(5),							/* Button 2 */
			DIGITAL_VALUE(6)							/* Button 3 */
		};

UCHAR	DigitalJoy_FourButtons_Mapping[]=	{
			2,4,0,1,	/*Axes, Buttons, Hats, Configs */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_X,	/* Axis 1 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_Y,	/* Axis 2 */
			HID_USAGE_PAGE_BUTTON,1,					/* Button 1 */
			HID_USAGE_PAGE_BUTTON,2,					/* Button 2 */
			HID_USAGE_PAGE_BUTTON,3,					/* Button 3 */
			HID_USAGE_PAGE_BUTTON,4,					/* Button 4 */
			/* Config 1 */
			DIGITAL_VALUE(2),DIGITAL_VALUE(3),			/* Left-Right axis */
			DIGITAL_VALUE(0),DIGITAL_VALUE(1),			/* Up-Down axis */
			DIGITAL_VALUE(4),							/* Button 1 */
			DIGITAL_VALUE(5),							/* Button 2 */
			DIGITAL_VALUE(6),							/* Button 3 */
			DIGITAL_VALUE(7)							/* Button 4 */
		};

UCHAR	DigitalJoy_FiveButtons_Mapping[]=	{
			2,5,0,1,	/*Axes, Buttons, Hats, Configs */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_X,	/* Axis 1 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_Y,	/* Axis 2 */
			HID_USAGE_PAGE_BUTTON,1,					/* Button 1 */
			HID_USAGE_PAGE_BUTTON,2,					/* Button 2 */
			HID_USAGE_PAGE_BUTTON,3,					/* Button 3 */
			HID_USAGE_PAGE_BUTTON,4,					/* Button 4 */
			HID_USAGE_PAGE_BUTTON,5,					/* Button 5 */
			/* Config 1 */
			DIGITAL_VALUE(2),DIGITAL_VALUE(3),			/* Left-Right axis */
			DIGITAL_VALUE(0),DIGITAL_VALUE(1),			/* Up-Down axis */
			DIGITAL_VALUE(4),							/* Button 1 */
			DIGITAL_VALUE(5),							/* Button 2 */
			DIGITAL_VALUE(6),							/* Button 3 */
			DIGITAL_VALUE(7),							/* Button 4 */
			DIGITAL_VALUE(8)							/* Button 5 */
		};

UCHAR	DigitalJoy_EightButtons_Mapping[]=	{
			2,8,0,1,	/*Axes, Buttons, Hats, Configs */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_X,	/* Axis 1 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_Y,	/* Axis 2 */
			HID_USAGE_PAGE_BUTTON,1,					/* Button 1 */
			HID_USAGE_PAGE_BUTTON,2,					/* Button 2 */
			HID_USAGE_PAGE_BUTTON,3,					/* Button 3 */
			HID_USAGE_PAGE_BUTTON,4,					/* Button 4 */
			HID_USAGE_PAGE_BUTTON,5,					/* Button 5 */
			HID_USAGE_PAGE_BUTTON,6,					/* Button 6 */
			HID_USAGE_PAGE_BUTTON,7,					/* Button 7 */
			HID_USAGE_PAGE_BUTTON,8,					/* Button 8 */
			/* Config 1 */
			DIGITAL_VALUE(2),DIGITAL_VALUE(3),			/* Left-Right axis */
			DIGITAL_VALUE(0),DIGITAL_VALUE(1),			/* Up-Down axis */
			DIGITAL_VALUE(4),							/* Button 1 */
			DIGITAL_VALUE(5),							/* Button 2 */
			DIGITAL_VALUE(6),							/* Button 3 */
			DIGITAL_VALUE(7),							/* Button 4 */
			DIGITAL_VALUE(8),							/* Button 5 */
			DIGITAL_VALUE(9),							/* Button 6 */
			DIGITAL_VALUE(10),							/* Button 7 */
			DIGITAL_VALUE(11)							/* Button 8 */
		};

UCHAR	GenesisPad_Mapping[]=	{
			2,8,0,1,	/*Axes, Buttons, Hats, Configs */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_X,	/* Axis 1 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_Y,	/* Axis 2 */
			HID_USAGE_PAGE_BUTTON,1,					/* Button 1 */
			HID_USAGE_PAGE_BUTTON,2,					/* Button 2 */
			HID_USAGE_PAGE_BUTTON,3,					/* Button 3 */
			HID_USAGE_PAGE_BUTTON,4,					/* Button 4 */
			HID_USAGE_PAGE_BUTTON,5,					/* Button 5 */
			HID_USAGE_PAGE_BUTTON,6,					/* Button 6 */
			HID_USAGE_PAGE_BUTTON,7,					/* Button 7 */
			HID_USAGE_PAGE_BUTTON,8,					/* Button 8 */
			/* Config 1 */
			DIGITAL_VALUE(2),DIGITAL_VALUE(3),			/* Left-Right axis */
			DIGITAL_VALUE(0),DIGITAL_VALUE(1),			/* Up-Down axis */
			DIGITAL_VALUE(4),							/* Button 1 */
			DIGITAL_VALUE(5),							/* Button 2 */
			DIGITAL_VALUE(6),							/* Button 3 */
			DIGITAL_VALUE(7),							/* Button 4 */
			DIGITAL_VALUE(8),							/* Button 5 */
			DIGITAL_VALUE(9),							/* Button 6 */
			DIGITAL_VALUE(10),							/* Button 7 */
			DIGITAL_VALUE(11)							/* Button 8 */
		};

UCHAR	SNESPad_Mapping[]=	{
			2,12,0,1,	/*Axes, Buttons, Hats, Configs */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_X,	/* Axis 1 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_Y,	/* Axis 2 */
			HID_USAGE_PAGE_BUTTON,1,					/* Button 1 */
			HID_USAGE_PAGE_BUTTON,2,					/* Button 2 */
			HID_USAGE_PAGE_BUTTON,3,					/* Button 3 */
			HID_USAGE_PAGE_BUTTON,4,					/* Button 4 */
			HID_USAGE_PAGE_BUTTON,5,					/* Button 5 */
			HID_USAGE_PAGE_BUTTON,6,					/* Button 6 */
			HID_USAGE_PAGE_BUTTON,7,					/* Button 7 */
			HID_USAGE_PAGE_BUTTON,8,					/* Button 8 */
			HID_USAGE_PAGE_BUTTON,9,					/* Button 9 */
			HID_USAGE_PAGE_BUTTON,10,					/* Button 10 */
			HID_USAGE_PAGE_BUTTON,11,					/* Button 11 */
			HID_USAGE_PAGE_BUTTON,12,					/* Button 12 */
			/* Config 1 */
			DIGITAL_VALUE(2),DIGITAL_VALUE(3),			/* Left-Right axis */
			DIGITAL_VALUE(0),DIGITAL_VALUE(1),			/* Up-Down axis */
			DIGITAL_VALUE(4),							/* Button 1 */
			DIGITAL_VALUE(5),							/* Button 2 */
			DIGITAL_VALUE(6),							/* Button 3 */
			DIGITAL_VALUE(7),							/* Button 4 */
			DIGITAL_VALUE(8),							/* Button 5 */
			DIGITAL_VALUE(9),							/* Button 6 */
			DIGITAL_VALUE(10),							/* Button 7 */
			DIGITAL_VALUE(11),							/* Button 8 */
			DIGITAL_VALUE(12),							/* Button 9 */
			DIGITAL_VALUE(13),							/* Button 10 */
			DIGITAL_VALUE(14),							/* Button 11 */
			DIGITAL_VALUE(15)							/* Button 12 */
		};

UCHAR	Playstation_Mapping[]=	{
			4,16,1,2,	/*Axes, Buttons, Hats, Configs */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_X,	/* Axis 1 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_Y,	/* Axis 2 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_Z,	/* Axis 3 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_RZ,/* Axis 4 */
			HID_USAGE_PAGE_BUTTON,1,					/* Button 1 */
			HID_USAGE_PAGE_BUTTON,2,					/* Button 2 */
			HID_USAGE_PAGE_BUTTON,3,					/* Button 3 */
			HID_USAGE_PAGE_BUTTON,4,					/* Button 4 */
			HID_USAGE_PAGE_BUTTON,5,					/* Button 5 */
			HID_USAGE_PAGE_BUTTON,6,					/* Button 6 */
			HID_USAGE_PAGE_BUTTON,7,					/* Button 7 */
			HID_USAGE_PAGE_BUTTON,8,					/* Button 8 */
			HID_USAGE_PAGE_BUTTON,9,					/* Button 9 */
			HID_USAGE_PAGE_BUTTON,10,					/* Button 10 */
			HID_USAGE_PAGE_BUTTON,11,					/* Button 11 */
			HID_USAGE_PAGE_BUTTON,12,					/* Button 12 */
			HID_USAGE_PAGE_BUTTON,13,					/* Button 13 */
			HID_USAGE_PAGE_BUTTON,14,					/* Button 14 */
			HID_USAGE_PAGE_BUTTON,15,					/* Button 15 */
			HID_USAGE_PAGE_BUTTON,16,					/* Button 16 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_HATSWITCH,	/* POV Hat #1 */
			/* Config 1 */
			DIGITAL_VALUE(2),DIGITAL_VALUE(3),			/* Left-Right axis */
			DIGITAL_VALUE(0),DIGITAL_VALUE(1),			/* Up-Down axis */
			UNUSED_MAPPING,UNUSED_MAPPING,				/* X Rotation axis */
			UNUSED_MAPPING,UNUSED_MAPPING,				/* Y Rotation axis */
			DIGITAL_VALUE(4),							/* Button 1 */
			DIGITAL_VALUE(5),							/* Button 2 */
			DIGITAL_VALUE(6),							/* Button 3 */
			DIGITAL_VALUE(7),							/* Button 4 */
			DIGITAL_VALUE(8),							/* Button 5 */
			DIGITAL_VALUE(9),							/* Button 6 */
			UNUSED_MAPPING,								/* Button 7 */
			UNUSED_MAPPING,								/* Button 8 */
			UNUSED_MAPPING,								/* Button 9 */
			UNUSED_MAPPING,								/* Button 10 */
			UNUSED_MAPPING,								/* Button 11 */
			UNUSED_MAPPING,								/* Button 12 */
			UNUSED_MAPPING,								/* Button 13 */
			UNUSED_MAPPING,								/* Button 14 */
			UNUSED_MAPPING,								/* Button 15 */
			UNUSED_MAPPING,								/* Button 16 */
			DIGITAL_VALUE(13),							/* Hat1 Up */
			DIGITAL_VALUE(11),							/* Hat1 Down */
			DIGITAL_VALUE(10),							/* Hat1 Left */
			DIGITAL_VALUE(12),							/* Hat1 Right */
			/* Config 2 */
			ANALOG_VALUE(0),UNUSED_MAPPING,				/* Left-Right axis */
			ANALOG_VALUE(1),UNUSED_MAPPING,				/* Up-Down axis */
			ANALOG_VALUE(2),UNUSED_MAPPING,				/* X Rotation axis */
			ANALOG_VALUE(3),UNUSED_MAPPING,				/* Y Rotation axis */
			DIGITAL_VALUE(4),							/* Button 1 */
			DIGITAL_VALUE(5),							/* Button 2 */
			DIGITAL_VALUE(6),							/* Button 3 */
			DIGITAL_VALUE(7),							/* Button 4 */
			DIGITAL_VALUE(8),							/* Button 5 */
			DIGITAL_VALUE(9),							/* Button 6 */
			UNUSED_MAPPING,								/* Button 7 */
			UNUSED_MAPPING,								/* Button 8 */
			UNUSED_MAPPING,								/* Button 9 */
			UNUSED_MAPPING,								/* Button 10 */
			DIGITAL_VALUE(0),							/* Button 11 */
			DIGITAL_VALUE(1),							/* Button 12 */
			DIGITAL_VALUE(2),							/* Button 13 */
			DIGITAL_VALUE(3),							/* Button 14 */
			DIGITAL_VALUE(14),							/* Button 15 */
			DIGITAL_VALUE(15),							/* Button 16 */
			DIGITAL_VALUE(13),							/* Hat1 Up */
			DIGITAL_VALUE(11),							/* Hat1 Down */
			DIGITAL_VALUE(10),							/* Hat1 Left */
			DIGITAL_VALUE(12)							/* Hat1 Right */
		};

UCHAR	IOCTLJoy_Mapping[]=	{
			8,			/* Axes */
#ifdef	IOCTLJOY_64BUTTON
			64,			/* Buttons */
#else
			16,			/* Buttons */
#endif
			2,1,			/* Hats, Configs */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_X,	/* Axis 1 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_Y,	/* Axis 2 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_Z,	/* Axis 3 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_RZ,/* Axis 4 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_SLIDER,	/* Axis 5 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_RX,/* Axis 6 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_RY,/* Axis 7 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_DIAL,		/* Axis 8 */
			HID_USAGE_PAGE_BUTTON,1,					/* Button 1 */
			HID_USAGE_PAGE_BUTTON,2,					/* Button 2 */
			HID_USAGE_PAGE_BUTTON,3,					/* Button 3 */
			HID_USAGE_PAGE_BUTTON,4,					/* Button 4 */
			HID_USAGE_PAGE_BUTTON,5,					/* Button 5 */
			HID_USAGE_PAGE_BUTTON,6,					/* Button 6 */
			HID_USAGE_PAGE_BUTTON,7,					/* Button 7 */
			HID_USAGE_PAGE_BUTTON,8,					/* Button 8 */
			HID_USAGE_PAGE_BUTTON,9,					/* Button 9 */
			HID_USAGE_PAGE_BUTTON,10,					/* Button 10 */
			HID_USAGE_PAGE_BUTTON,11,					/* Button 11 */
			HID_USAGE_PAGE_BUTTON,12,					/* Button 12 */
			HID_USAGE_PAGE_BUTTON,13,					/* Button 13 */
			HID_USAGE_PAGE_BUTTON,14,					/* Button 14 */
			HID_USAGE_PAGE_BUTTON,15,					/* Button 15 */
			HID_USAGE_PAGE_BUTTON,16,					/* Button 16 */
#ifdef	IOCTLJOY_64BUTTON
			HID_USAGE_PAGE_BUTTON,17,					/* Button 17 */
			HID_USAGE_PAGE_BUTTON,18,					/* Button 18 */
			HID_USAGE_PAGE_BUTTON,19,					/* Button 19 */
			HID_USAGE_PAGE_BUTTON,20,					/* Button 20 */
			HID_USAGE_PAGE_BUTTON,21,					/* Button 21 */
			HID_USAGE_PAGE_BUTTON,22,					/* Button 22 */
			HID_USAGE_PAGE_BUTTON,23,					/* Button 23 */
			HID_USAGE_PAGE_BUTTON,24,					/* Button 24 */
			HID_USAGE_PAGE_BUTTON,25,					/* Button 25 */
			HID_USAGE_PAGE_BUTTON,26,					/* Button 26 */
			HID_USAGE_PAGE_BUTTON,27,					/* Button 27 */
			HID_USAGE_PAGE_BUTTON,28,					/* Button 28 */
			HID_USAGE_PAGE_BUTTON,29,					/* Button 29 */
			HID_USAGE_PAGE_BUTTON,30,					/* Button 30 */
			HID_USAGE_PAGE_BUTTON,31,					/* Button 31 */
			HID_USAGE_PAGE_BUTTON,32,					/* Button 32 */
			HID_USAGE_PAGE_BUTTON,33,					/* Button 33 */
			HID_USAGE_PAGE_BUTTON,34,					/* Button 34 */
			HID_USAGE_PAGE_BUTTON,35,					/* Button 35 */
			HID_USAGE_PAGE_BUTTON,36,					/* Button 36 */
			HID_USAGE_PAGE_BUTTON,37,					/* Button 37 */
			HID_USAGE_PAGE_BUTTON,38,					/* Button 38 */
			HID_USAGE_PAGE_BUTTON,39,					/* Button 39 */
			HID_USAGE_PAGE_BUTTON,40,					/* Button 40 */
			HID_USAGE_PAGE_BUTTON,41,					/* Button 41 */
			HID_USAGE_PAGE_BUTTON,42,					/* Button 42 */
			HID_USAGE_PAGE_BUTTON,43,					/* Button 43 */
			HID_USAGE_PAGE_BUTTON,44,					/* Button 44 */
			HID_USAGE_PAGE_BUTTON,45,					/* Button 45 */
			HID_USAGE_PAGE_BUTTON,46,					/* Button 46 */
			HID_USAGE_PAGE_BUTTON,47,					/* Button 47 */
			HID_USAGE_PAGE_BUTTON,48,					/* Button 48 */
			HID_USAGE_PAGE_BUTTON,49,					/* Button 49 */
			HID_USAGE_PAGE_BUTTON,50,					/* Button 50 */
			HID_USAGE_PAGE_BUTTON,51,					/* Button 51 */
			HID_USAGE_PAGE_BUTTON,52,					/* Button 52 */
			HID_USAGE_PAGE_BUTTON,53,					/* Button 53 */
			HID_USAGE_PAGE_BUTTON,54,					/* Button 54 */
			HID_USAGE_PAGE_BUTTON,55,					/* Button 55 */
			HID_USAGE_PAGE_BUTTON,56,					/* Button 56 */
			HID_USAGE_PAGE_BUTTON,57,					/* Button 57 */
			HID_USAGE_PAGE_BUTTON,58,					/* Button 58 */
			HID_USAGE_PAGE_BUTTON,59,					/* Button 59 */
			HID_USAGE_PAGE_BUTTON,60,					/* Button 60 */
			HID_USAGE_PAGE_BUTTON,61,					/* Button 61 */
			HID_USAGE_PAGE_BUTTON,62,					/* Button 62 */
			HID_USAGE_PAGE_BUTTON,63,					/* Button 63 */
			HID_USAGE_PAGE_BUTTON,64,					/* Button 64 */
#endif
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_HATSWITCH,	/* POV Hat #1 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_HATSWITCH,	/* POV Hat #2 */
			/* Config 1 */
			ANALOG_VALUE(0),UNUSED_MAPPING,				/* Left-Right axis */
			ANALOG_VALUE(1),UNUSED_MAPPING,				/* Up-Down axis */
			ANALOG_VALUE(2),UNUSED_MAPPING,				/* X Rotation axis */
			ANALOG_VALUE(3),UNUSED_MAPPING,				/* Y Rotation axis */
			ANALOG_VALUE(4),UNUSED_MAPPING,				/* Z Position axis */
			ANALOG_VALUE(5),UNUSED_MAPPING,				/* Z Rotation axis */
			ANALOG_VALUE(6),UNUSED_MAPPING,				/* Throttle axis */
			ANALOG_VALUE(7),UNUSED_MAPPING,				/* Rudder axis */
			DIGITAL_VALUE(0),							/* Button 1 */
			DIGITAL_VALUE(1),							/* Button 2 */
			DIGITAL_VALUE(2),							/* Button 3 */
			DIGITAL_VALUE(3),							/* Button 4 */
			DIGITAL_VALUE(4),							/* Button 5 */
			DIGITAL_VALUE(5),							/* Button 6 */
			DIGITAL_VALUE(6),							/* Button 7 */
			DIGITAL_VALUE(7),							/* Button 8 */
			DIGITAL_VALUE(8),							/* Button 9 */
			DIGITAL_VALUE(9),							/* Button 10 */
			DIGITAL_VALUE(10),							/* Button 11 */
			DIGITAL_VALUE(11),							/* Button 12 */
			DIGITAL_VALUE(12),							/* Button 13 */
			DIGITAL_VALUE(13),							/* Button 14 */
			DIGITAL_VALUE(14),							/* Button 15 */
			DIGITAL_VALUE(15),							/* Button 16 */
#ifdef	IOCTLJOY_64BUTTON
			DIGITAL_VALUE(16),							/* Button 17 */
			DIGITAL_VALUE(17),							/* Button 18 */
			DIGITAL_VALUE(18),							/* Button 19 */
			DIGITAL_VALUE(19),							/* Button 20 */
			DIGITAL_VALUE(20),							/* Button 21 */
			DIGITAL_VALUE(21),							/* Button 22 */
			DIGITAL_VALUE(22),							/* Button 23 */
			DIGITAL_VALUE(23),							/* Button 24 */
			DIGITAL_VALUE(24),							/* Button 25 */
			DIGITAL_VALUE(25),							/* Button 26 */
			DIGITAL_VALUE(26),							/* Button 27 */
			DIGITAL_VALUE(27),							/* Button 28 */
			DIGITAL_VALUE(28),							/* Button 29 */
			DIGITAL_VALUE(29),							/* Button 30 */
			DIGITAL_VALUE(30),							/* Button 31 */
			DIGITAL_VALUE(31),							/* Button 32 */
			DIGITAL_VALUE(32),							/* Button 33 */
			DIGITAL_VALUE(33),							/* Button 34 */
			DIGITAL_VALUE(34),							/* Button 35 */
			DIGITAL_VALUE(35),							/* Button 36 */
			DIGITAL_VALUE(36),							/* Button 37 */
			DIGITAL_VALUE(37),							/* Button 38 */
			DIGITAL_VALUE(38),							/* Button 39 */
			DIGITAL_VALUE(39),							/* Button 40 */
			DIGITAL_VALUE(40),							/* Button 41 */
			DIGITAL_VALUE(41),							/* Button 42 */
			DIGITAL_VALUE(42),							/* Button 43 */
			DIGITAL_VALUE(43),							/* Button 44 */
			DIGITAL_VALUE(44),							/* Button 45 */
			DIGITAL_VALUE(45),							/* Button 46 */
			DIGITAL_VALUE(46),							/* Button 47 */
			DIGITAL_VALUE(47),							/* Button 48 */
			DIGITAL_VALUE(48),							/* Button 49 */
			DIGITAL_VALUE(49),							/* Button 50 */
			DIGITAL_VALUE(50),							/* Button 51 */
			DIGITAL_VALUE(51),							/* Button 52 */
			DIGITAL_VALUE(52),							/* Button 53 */
			DIGITAL_VALUE(53),							/* Button 54 */
			DIGITAL_VALUE(54),							/* Button 55 */
			DIGITAL_VALUE(55),							/* Button 56 */
			DIGITAL_VALUE(56),							/* Button 57 */
			DIGITAL_VALUE(57),							/* Button 58 */
			DIGITAL_VALUE(58),							/* Button 59 */
			DIGITAL_VALUE(59),							/* Button 60 */
			DIGITAL_VALUE(60),							/* Button 61 */
			DIGITAL_VALUE(61),							/* Button 62 */
			DIGITAL_VALUE(62),							/* Button 63 */
			DIGITAL_VALUE(63),							/* Button 64 */

			DIGITAL_VALUE(64),							/* Hat1 Up */
			DIGITAL_VALUE(65),							/* Hat1 Down */
			DIGITAL_VALUE(66),							/* Hat1 Left */
			DIGITAL_VALUE(67),							/* Hat1 Right */
#else
			DIGITAL_VALUE(16),							/* Hat1 Up */
			DIGITAL_VALUE(17),							/* Hat1 Down */
			DIGITAL_VALUE(18),							/* Hat1 Left */
			DIGITAL_VALUE(19),							/* Hat1 Right */
#endif
			ANALOG_VALUE(8),							/* Hat2 Up */
			UNUSED_MAPPING,								/* Hat2 Down */
			UNUSED_MAPPING,								/* Hat2 Left */
			UNUSED_MAPPING								/* Hat2 Right */
		};

UCHAR	ButtonArray_30Buttons_Mapping[]=	{
			2,30,0,1,	/*Axes, Buttons, Hats, Configs */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_X,	/* Axis 1 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_Y,	/* Axis 2 */
			HID_USAGE_PAGE_BUTTON,1,					/* Button 1 */
			HID_USAGE_PAGE_BUTTON,2,					/* Button 2 */
			HID_USAGE_PAGE_BUTTON,3,					/* Button 3 */
			HID_USAGE_PAGE_BUTTON,4,					/* Button 4 */
			HID_USAGE_PAGE_BUTTON,5,					/* Button 5 */
			HID_USAGE_PAGE_BUTTON,6,					/* Button 6 */
			HID_USAGE_PAGE_BUTTON,7,					/* Button 7 */
			HID_USAGE_PAGE_BUTTON,8,					/* Button 8 */
			HID_USAGE_PAGE_BUTTON,9,					/* Button 9 */
			HID_USAGE_PAGE_BUTTON,10,					/* Button 10 */
			HID_USAGE_PAGE_BUTTON,11,					/* Button 11 */
			HID_USAGE_PAGE_BUTTON,12,					/* Button 12 */
			HID_USAGE_PAGE_BUTTON,13,					/* Button 13 */
			HID_USAGE_PAGE_BUTTON,14,					/* Button 14 */
			HID_USAGE_PAGE_BUTTON,15,					/* Button 15 */
			HID_USAGE_PAGE_BUTTON,16,					/* Button 16 */
			HID_USAGE_PAGE_BUTTON,17,					/* Button 17 */
			HID_USAGE_PAGE_BUTTON,18,					/* Button 18 */
			HID_USAGE_PAGE_BUTTON,19,					/* Button 19 */
			HID_USAGE_PAGE_BUTTON,20,					/* Button 20 */
			HID_USAGE_PAGE_BUTTON,21,					/* Button 21 */
			HID_USAGE_PAGE_BUTTON,22,					/* Button 22 */
			HID_USAGE_PAGE_BUTTON,23,					/* Button 23 */
			HID_USAGE_PAGE_BUTTON,24,					/* Button 24 */
			HID_USAGE_PAGE_BUTTON,25,					/* Button 25 */
			HID_USAGE_PAGE_BUTTON,26,					/* Button 26 */
			HID_USAGE_PAGE_BUTTON,27,					/* Button 27 */
			HID_USAGE_PAGE_BUTTON,28,					/* Button 28 */
			HID_USAGE_PAGE_BUTTON,29,					/* Button 29 */
			HID_USAGE_PAGE_BUTTON,30,					/* Button 30 */
			/* Config 1 */
			UNUSED_MAPPING,UNUSED_MAPPING,				/* Left-Right axis */
			UNUSED_MAPPING,UNUSED_MAPPING,				/* Up-Down axis */
			DIGITAL_VALUE(0),							/* Button 1 */
			DIGITAL_VALUE(1),							/* Button 2 */
			DIGITAL_VALUE(2),							/* Button 3 */
			DIGITAL_VALUE(3),							/* Button 4 */
			DIGITAL_VALUE(4),							/* Button 5 */
			DIGITAL_VALUE(5),							/* Button 6 */
			DIGITAL_VALUE(6),							/* Button 7 */
			DIGITAL_VALUE(7),							/* Button 8 */
			DIGITAL_VALUE(8),							/* Button 9 */
			DIGITAL_VALUE(9),							/* Button 10 */
			DIGITAL_VALUE(10),							/* Button 11 */
			DIGITAL_VALUE(11),							/* Button 12 */
			DIGITAL_VALUE(12),							/* Button 13 */
			DIGITAL_VALUE(13),							/* Button 14 */
			DIGITAL_VALUE(14),							/* Button 15 */
			DIGITAL_VALUE(15),							/* Button 16 */
			DIGITAL_VALUE(16),							/* Button 17 */
			DIGITAL_VALUE(17),							/* Button 18 */
			DIGITAL_VALUE(18),							/* Button 19 */
			DIGITAL_VALUE(19),							/* Button 20 */
			DIGITAL_VALUE(20),							/* Button 21 */
			DIGITAL_VALUE(21),							/* Button 22 */
			DIGITAL_VALUE(22),							/* Button 23 */
			DIGITAL_VALUE(23),							/* Button 24 */
			DIGITAL_VALUE(24),							/* Button 25 */
			DIGITAL_VALUE(25),							/* Button 26 */
			DIGITAL_VALUE(26),							/* Button 27 */
			DIGITAL_VALUE(27),							/* Button 28 */
			DIGITAL_VALUE(28),							/* Button 29 */
			DIGITAL_VALUE(29)							/* Button 30 */
		};

UCHAR	BuddyBox_Mapping[]=	{
			8,16,0,1,	/*Axes, Buttons, Hats, Configs */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_X,	/* Axis 1 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_Y,	/* Axis 2 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_Z,	/* Axis 3 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_RZ,/* Axis 4 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_SLIDER,	/* Axis 5 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_RX,/* Axis 6 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_RY,/* Axis 7 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_DIAL,		/* Axis 8 */
			HID_USAGE_PAGE_BUTTON,1,					/* Button 1 */
			HID_USAGE_PAGE_BUTTON,2,					/* Button 2 */
			HID_USAGE_PAGE_BUTTON,3,					/* Button 3 */
			HID_USAGE_PAGE_BUTTON,4,					/* Button 4 */
			HID_USAGE_PAGE_BUTTON,5,					/* Button 5 */
			HID_USAGE_PAGE_BUTTON,6,					/* Button 6 */
			HID_USAGE_PAGE_BUTTON,7,					/* Button 7 */
			HID_USAGE_PAGE_BUTTON,8,					/* Button 8 */
			HID_USAGE_PAGE_BUTTON,9,					/* Button 9 */
			HID_USAGE_PAGE_BUTTON,10,					/* Button 10 */
			HID_USAGE_PAGE_BUTTON,11,					/* Button 11 */
			HID_USAGE_PAGE_BUTTON,12,					/* Button 12 */
			HID_USAGE_PAGE_BUTTON,13,					/* Button 13 */
			HID_USAGE_PAGE_BUTTON,14,					/* Button 14 */
			HID_USAGE_PAGE_BUTTON,15,					/* Button 15 */
			HID_USAGE_PAGE_BUTTON,16,					/* Button 16 */
			/* Config 1 */
			ANALOG_VALUE(0),UNUSED_MAPPING,				/* Left-Right axis */
			ANALOG_VALUE(1),UNUSED_MAPPING,				/* Up-Down axis */
			ANALOG_VALUE(2),UNUSED_MAPPING,				/* X Rotation axis */
			ANALOG_VALUE(3),UNUSED_MAPPING,				/* Y Rotation axis */
			ANALOG_VALUE(4),UNUSED_MAPPING,				/* Z Position axis */
			ANALOG_VALUE(5),UNUSED_MAPPING,				/* Z Rotation axis */
			ANALOG_VALUE(6),UNUSED_MAPPING,				/* Throttle axis */
			ANALOG_VALUE(7),UNUSED_MAPPING,				/* Rudder axis */
			DIGITAL_VALUE(0),							/* Button 1 */
			DIGITAL_VALUE(1),							/* Button 2 */
			DIGITAL_VALUE(2),							/* Button 3 */
			DIGITAL_VALUE(3),							/* Button 4 */
			DIGITAL_VALUE(4),							/* Button 5 */
			DIGITAL_VALUE(5),							/* Button 6 */
			DIGITAL_VALUE(6),							/* Button 7 */
			DIGITAL_VALUE(7),							/* Button 8 */
			DIGITAL_VALUE(8),							/* Button 9 */
			DIGITAL_VALUE(9),							/* Button 10 */
			DIGITAL_VALUE(10),							/* Button 11 */
			DIGITAL_VALUE(11),							/* Button 12 */
			DIGITAL_VALUE(12),							/* Button 13 */
			DIGITAL_VALUE(13),							/* Button 14 */
			DIGITAL_VALUE(14),							/* Button 15 */
			DIGITAL_VALUE(15)							/* Button 16 */
		};

UCHAR	ButtonArray_12Buttons_Mapping[]=	{
			2,12,0,1,	/*Axes, Buttons, Hats, Configs */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_X,	/* Axis 1 */
			HID_USAGE_PAGE_GENERIC,HID_USAGE_GENERIC_Y,	/* Axis 2 */
			HID_USAGE_PAGE_BUTTON,1,					/* Button 1 */
			HID_USAGE_PAGE_BUTTON,2,					/* Button 2 */
			HID_USAGE_PAGE_BUTTON,3,					/* Button 3 */
			HID_USAGE_PAGE_BUTTON,4,					/* Button 4 */
			HID_USAGE_PAGE_BUTTON,5,					/* Button 5 */
			HID_USAGE_PAGE_BUTTON,6,					/* Button 6 */
			HID_USAGE_PAGE_BUTTON,7,					/* Button 7 */
			HID_USAGE_PAGE_BUTTON,8,					/* Button 8 */
			HID_USAGE_PAGE_BUTTON,9,					/* Button 9 */
			HID_USAGE_PAGE_BUTTON,10,					/* Button 10 */
			HID_USAGE_PAGE_BUTTON,11,					/* Button 11 */
			HID_USAGE_PAGE_BUTTON,12,					/* Button 12 */
			/* Config 1 */
			UNUSED_MAPPING,UNUSED_MAPPING,				/* Left-Right axis */
			UNUSED_MAPPING,UNUSED_MAPPING,				/* Up-Down axis */
			DIGITAL_VALUE(0),							/* Button 1 */
			DIGITAL_VALUE(1),							/* Button 2 */
			DIGITAL_VALUE(2),							/* Button 3 */
			DIGITAL_VALUE(3),							/* Button 4 */
			DIGITAL_VALUE(4),							/* Button 5 */
			DIGITAL_VALUE(5),							/* Button 6 */
			DIGITAL_VALUE(6),							/* Button 7 */
			DIGITAL_VALUE(7),							/* Button 8 */
			DIGITAL_VALUE(8),							/* Button 9 */
			DIGITAL_VALUE(9),							/* Button 10 */
			DIGITAL_VALUE(10),							/* Button 11 */
			DIGITAL_VALUE(11)							/* Button 12 */
		};

/**************************************************************************/
/* Declare global variables defined in PPortJoy.h                         */
/**************************************************************************/

/* Table used to old the chipmode for the port, the initialisation and */
/* read routines for each joystick tipe                                */
JOYSTICKDEF		JoyDefs[PPJOY_MAX_JOYTYPE+1]=
				{
				{0,					NULL,				NULL,				NULL},												/* 0 */
				{ECR_SPP_MODE,		InitTheMaze,		ReadTheMaze,		(PJOYSTICK_MAP)DigitalJoy_ThreeButtons_Mapping},	/* 1 */
				{ECR_SPP_MODE,		InitIanHarries,		ReadIanHarries,		(PJOYSTICK_MAP)DigitalJoy_OneButton_Mapping},		/* 2 */
				{ECR_SPP_MODE,		InitTurboGraFX,		ReadTurboGraFX,		(PJOYSTICK_MAP)DigitalJoy_FiveButtons_Mapping},		/* 3 */
				{ECR_BYTE_PIO_MODE,	InitLinux0802,		ReadLinux0802,		(PJOYSTICK_MAP)DigitalJoy_OneButton_Mapping},		/* 4 */
				{ECR_BYTE_PIO_MODE,	InitLinuxDB9c,		ReadLinuxDB9c,		(PJOYSTICK_MAP)DigitalJoy_EightButtons_Mapping},	/* 5 */
				{ECR_SPP_MODE,		InitTormod,			ReadTormod,			(PJOYSTICK_MAP)DigitalJoy_FourButtons_Mapping},		/* 6 */
				{ECR_SPP_MODE,		InitDirectPro,		ReadDirectPro,		(PJOYSTICK_MAP)DigitalJoy_TwoButtons_Mapping},		/* 7 */
				{ECR_SPP_MODE,		InitTurboGraFX,		ReadTurboGraFX,		(PJOYSTICK_MAP)DigitalJoy_FiveButtons_Mapping},		/* 8 */
				{ECR_SPP_MODE,		InitLPTJoystick,	ReadLPTJoystick,	(PJOYSTICK_MAP)DigitalJoy_OneButton_Mapping},		/* 9 */
				{ECR_SPP_MODE,		InitCHAMPGames,		ReadCHAMPGames,		(PJOYSTICK_MAP)DigitalJoy_OneButton_Mapping},		/* 10 */
				{ECR_BYTE_PIO_MODE,	InitSTFormat,		ReadSTFormat,		(PJOYSTICK_MAP)DigitalJoy_OneButton_Mapping},		/* 11 */
				{ECR_SPP_MODE,		InitSNESKey2600,	ReadSNESKey2600,	(PJOYSTICK_MAP)DigitalJoy_OneButton_Mapping},		/* 12 */
				{ECR_BYTE_PIO_MODE,	InitAmiga4Play,		ReadAmiga4Play,		(PJOYSTICK_MAP)DigitalJoy_TwoButtons_Mapping},		/* 13 */
				{ECR_BYTE_PIO_MODE,	InitPCAE,			ReadPCAE,			(PJOYSTICK_MAP)DigitalJoy_OneButton_Mapping},		/* 14 */
				{ECR_BYTE_PIO_MODE,	InitGenesisLin,		ReadGenesisLin,		(PJOYSTICK_MAP)GenesisPad_Mapping},					/* 15 */
				{ECR_SPP_MODE,		InitGenesisDPP,		ReadGenesisDPP,		(PJOYSTICK_MAP)GenesisPad_Mapping},					/* 16 */
				{ECR_BYTE_PIO_MODE,	InitGenesisNTP,		ReadGenesisNTP,		(PJOYSTICK_MAP)GenesisPad_Mapping},					/* 17 */
				{ECR_SPP_MODE,		InitSNESPad,		ReadSNESPad,		(PJOYSTICK_MAP)SNESPad_Mapping},					/* 18 */
				{ECR_SPP_MODE,		InitSNESPad,		ReadSNESPad,		(PJOYSTICK_MAP)SNESPad_Mapping},					/* 19 */
				{ECR_BYTE_PIO_MODE,	InitGenesisCC,		ReadGenesisCC,		(PJOYSTICK_MAP)GenesisPad_Mapping},					/* 20 */
				{ECR_SPP_MODE,		InitGenesisSNES,	ReadGenesisSNES,	(PJOYSTICK_MAP)GenesisPad_Mapping},					/* 21 */
				{ECR_SPP_MODE,		InitPSXStandard,	ReadPSXPBLIB,		(PJOYSTICK_MAP)Playstation_Mapping},				/* 22 */
				{ECR_SPP_MODE,		InitPSXStandard,	ReadPSXDirectPad,	(PJOYSTICK_MAP)Playstation_Mapping},				/* 23 */
				{ECR_SPP_MODE,		InitPSXStandard,	ReadPSXLinux,		(PJOYSTICK_MAP)Playstation_Mapping},				/* 24 */
				{ECR_SPP_MODE,		InitPSXStandard,	ReadPSXNTPadXP,		(PJOYSTICK_MAP)Playstation_Mapping},				/* 25 */
				{ECR_SPP_MODE,		InitPSXStandard,	ReadPSXMegaTap,		(PJOYSTICK_MAP)Playstation_Mapping},				/* 26 */
				{0,					InitNullFunc,		ReadNullFunc,		(PJOYSTICK_MAP)IOCTLJoy_Mapping},					/* 27 */
				{ECR_SPP_MODE,		InitLinuxGamecon,	ReadLinuxGamecon,	(PJOYSTICK_MAP)DigitalJoy_TwoButtons_Mapping},		/* 28 */
				{ECR_SPP_MODE,		InitLPTSwitch,		ReadLPTSwitch,		(PJOYSTICK_MAP)ButtonArray_30Buttons_Mapping},		/* 29 */
				{ECR_SPP_MODE,		InitFMSBuddyBox,	ReadNullFunc,		(PJOYSTICK_MAP)BuddyBox_Mapping},					/* 30 */
				{ECR_SPP_MODE,		InitPowerPadLin,	ReadPowerPadLin,	(PJOYSTICK_MAP)ButtonArray_12Buttons_Mapping},		/* 31 */
				{ECR_SPP_MODE,		InitGenesisDPP6,	ReadGenesisDPP6,	(PJOYSTICK_MAP)GenesisPad_Mapping}					/* 16 */
				};

#if (!(	(IF_THEMAZE==1) && (IF_IANHARRIES==2) &&					\
		(IF_TURBOGRAFX==3) && (IF_LINUX0802==4) &&					\
		(IF_LINUXDB9C==5) && (IF_TORMOD==6) &&						\
		(IF_DIRECTPRO==7) && (IF_TURBOGFX45==8) &&					\
		(IF_LPTJOYSTICK==9) && (IF_CHAMPGAMES==10) &&				\
		(IF_STFORMAT==11) && (IF_SNESKEY2600==12) &&				\
		(IF_AMIGA4PLAYER==13) && (IF_PCAE==14) &&					\
		(IF_GENESISLIN==15) && (IF_GENESISDPP==16) &&				\
		(IF_GENESISNTP==17) && (IF_SNESPADLIN==18) &&				\
		(IF_SNESPADDPP==19) && (IF_GENESISCC==20) &&				\
		(IF_GENESISSNES==21) && (IF_PSXPBLIB==22) &&				\
		(IF_PSXDPADPRO==23) && (IF_PSXLINUX==24) &&					\
		(IF_PSXNTPADXP==25) && (IF_PSXMEGATAP==26) &&				\
		(IF_IOCTL==27) && (IF_LINUXGAMECON==28) &&					\
		(IF_LPTSWITCH==29) && (IF_FMSBUDDYBOX==30) &&				\
		(IF_POWERPADLIN==31) && (IF_GENESISDPP6==32)				\
	))
#error Please update the order of the entries in the JoyDefs table to match #define values
#endif
