;
;  /***************************************************************************
;   *   PPJoy Virtual Joystick for Microsoft Windows                          *
;   *   Copyright (C) 2011 Deon van der Westhuysen                            *
;   *                                                                         *
;   *   This program is free software: you can redistribute it and/or modify  *
;   *   it under the terms of the GNU General Public License as published by  *
;   *   the Free Software Foundation, either version 3 of the License, or     *
;   *   (at your option) any later version.                                   *
;   *                                                                         *
;   *   This program is distributed in the hope that it will be useful,       *
;   *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
;   *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
;   *   GNU General Public License for more details.                          *
;   *                                                                         *
;   *   You should have received a copy of the GNU General Public License     *
;   *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
;   *                                                                         *
;   ***************************************************************************/
;  


;#ifndef _PPJOYEVENTLOG_H_
;#define _PPJOYEVENTLOG_H_
;
MessageIdTypedef=NTSTATUS
;

MessageId=2048
Severity=Informational
Facility=Application
SymbolicName=PPJ_MSG_GENERICINFO
Language=English
%2
.

MessageId=+1
Severity=Warning
Facility=Application
SymbolicName=PPJ_MSG_GENERICWARNING
Language=English
%2
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_GENERICERROR
Language=English
%2
.

MessageId=+1
Severity=Informational
Facility=Application
SymbolicName=PPJ_MSG_DRIVERSTARTED
Language=English
Driver successfully loaded.
.

MessageId=+1
Severity=Informational
Facility=Application
SymbolicName=PPJ_MSG_DEVICESTARTED
Language=English
Joystick %2 successfully started.
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_STARTFAILED
Language=English
Unable to start joystick because %2
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_ERRORSTARTINGPDO
Language=English
Cannot start Physical Device Object for the joystick
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_ERRORREADINGPARAMS
Language=English
Error reading joystick parameters from the bus driver
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_ERRORJOYSTICKTYPE
Language=English
Unsupported joystick type
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_ERRORNOMAPPING
Language=English
Cannot create a mapping for the joystick
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_ERRORCONTROLDEV
Language=English
Cannot create the joystick control device
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_ERRORCONTROLLINK
Language=English
Cannot create a symlink for the joystick control device
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_ERRORIOCTLLINK
Language=English
Cannot create the symlink for the IOCTL joystick 
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_ERRORCHIPMODE
Language=English
Cannot set parallel port ChipMode
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_ERRORINITFUNC
Language=English
Joystick initialisation function failed
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_ERRORINTERRUPT
Language=English
Bus driver failed to allocate an interrupt for the parallel port
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_ERRORALLOCMEM
Language=English
Error allocating memory
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_ERRORTHREAD
Language=English
Cannot create async scanning thread
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_ERRORHIDREG
Language=English
Cannot registry with the HID minidriver
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_ERRORNOTIMING
Language=English
Cannot create timing parameters for the joystick
.

MessageId=+1
Severity=Informational
Facility=Application
SymbolicName=PPJ_MSG_DRIVERSTARTEDVER
Language=English
Driver version %2 successfully loaded.
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_ERRORREGUPDATES
Language=English
Error registering with bus driver for option values
.

;
;#endif
