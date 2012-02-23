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


#ifndef __PPJOYAPI_H__
#define __PPJOYAPI_H__

#ifdef __cplusplus
extern "C"
{
#endif

// JoyType for IOCTL/Virtual joysticks
#define	JOYTYPE_IOCTL		27
#define	JOYSUBTYPE_IOCTL	0

// Header for joystick mapping data
typedef struct
{
 unsigned char	NumAxes;
 unsigned char	NumButtons;
 unsigned char	NumHats;
 unsigned char	NumMaps;
 unsigned char	Data[1];
} JOYMAPPING;

// Different types of mapping that can be read
#define	MAP_SCOPE_DEFAULT	0
#define	MAP_SCOPE_DRIVER	1
#define	MAP_SCOPE_DEVICE	2

// Definition for a single joystick device
typedef struct
{
 char	LPTNumber;
 char	UnitNumber;
 char	JoyType;
 char	JoySubType;
} JOYENTRY;


//////////////////////////////////////////////////////////////////////////////
// These parameters are used for the functions below
// LPTNumber:  Number of LPT port, 0 is virtual joystick, 1=LPT1, etc.
// UnitNumber: Number of joystick on the port. 0 is the first joystick
// JoyType:    Type of joystick defined on the port, only one type per port
// JoySubType: Submodel of joystick. Can be mixed on the same port
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// PPJoy_Open should be called when your application starts up. Do not call
// it multiple times! If the return code is 0 the call failed and other
// PPJoyAPI function should not be called. Call PPJoy_Close (once) before
// the application exits.
//////////////////////////////////////////////////////////////////////////////

// Open the PPJoy device driver. Return code 0= failure, 1= success.
int PPJoy_OpenDriver (void);
// Close the PPJoy device driver after all the action is done.
void PPJoy_CloseDriver (void);

// Returns the list of joysticks defined in PPJoy. Caller should allocate
// memory for JoystickList array. ListSize is the number of entries that
// JoystickList can contain. Return code is the number of joysticks defined
// or -1 if some error occurs. If there are more joysticks than the buffer
// can hold the list is truncated.
int PPJoy_GetJoystickList (JOYENTRY *JoystickList, int ListSize);
// Deletes a defined joystick - return code 1 is success, 0 is failure
int PPJoy_DeleteJoystick (char LPTNumber, char UnitNumber, char JoyType);
// Adds a new joystick to PPJoy. DOES NOT CHECK the validity of the parameters
// - only checks that less than 16 joysticks are defined. Return code -1 means
// too many sticks defined, 0 if failure, 1 is success.
int PPJoy_AddJoystick (char LPTNumber, char UnitNumber, char JoyType, char JoySubType);


//////////////////////////////////////////////////////////////////////////////
// The functions below return printable names for joystick devices. These
// functions will always return a valid char pointer, even if the input
// parameters are not valid.
//////////////////////////////////////////////////////////////////////////////

// Return the name of a joystick type.
char* PPJoy_GetSubTypeName (char JoyType, char JoySubType);
// Return the name of the joystick sub type. Returns "" if only one subtype
char* PPJoy_GetJoyTypeName (char JoyType);

//////////////////////////////////////////////////////////////////////////////
// Functions to manipulate the axis and button mappings for a joystick.
// There are three types of mappings:
// - Default: builtin mapping for joystick, Cannot be deleted. 
// - Driver: used when no device mapping. Applies to all joysticks of same type
// - Device: maping for individual joystick (only). Most specific mapping
//////////////////////////////////////////////////////////////////////////////

// Reads the three types of mappings for a joystick. Allocates memory for the
// mapping buffers that the caller must free using PPJoy_FreeMapping. Returned
// mapping is NULL pointer if the mapping does not exist. Return code is 0 on
// failure and 1 on success.
int	PPJoy_ReadMappings (char LPTNumber, char UnitNumber, char JoyType, JOYMAPPING **DefaultMapping, JOYMAPPING **DriverMapping, JOYMAPPING **DeviceMapping);
// Frees a mapping returned by PPJoy_ReadMappings
void PPJoy_FreeMapping (JOYMAPPING *Mapping);
// Writes a new mapping for a joystick. Return code is a standard WIN32 return
// code. rc of 0 is no error. Scope determines which mapping is deleted.
// MapSize is the size of the mapping to write, in bytes.
int	PPJoy_WriteMapping (char LPTNumber, char UnitNumber, char JoyType, JOYMAPPING *Mapping, int MapSize, int Scope);
// Deletes a mapping for a joystick. Return code is a standard WIN32 return
// code. rc of 0 is no error. Scope determines which mapping is deleted.
int PPJoy_DeleteMapping (char LPTNumber, char UnitNumber, char JoyType, int Scope);

#ifdef __cplusplus
}
#endif

#endif
