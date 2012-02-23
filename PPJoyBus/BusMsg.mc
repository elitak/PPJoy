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
Bus enumerator successfully loaded.
.

MessageId=+1
Severity=Informational
Facility=Application
SymbolicName=PPJ_MSG_DEVICESTARTED
Language=English
Bus enumerator started.
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
SymbolicName=PPJ_MSG_ERRORALLOCMEM
Language=English
Error allocating memory
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_ERRORCREATINGBUS
Language=English
Cannot create parallel port bus device
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_ERRORBUSIF
Language=English
Cannot create bus driver Device Interface 
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_ERRORBUSATTACH
Language=English
Cannot attach bus device to lower level drivers
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_ERRORJOYREGISTRY
Language=English
Invalid joystick registry entry "%2" found - entry ignored
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_ERRORJOYCREATE
Language=English
Cannot create joystick PDO
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_ERRORPNPNOTIFY
Language=English
Cannot register for Plug 'n Play notification for parallel port device events
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_ERRORBADPORT
Language=English
Cannot allocate parallel port - invalid port number
.

MessageId=+1
Severity=Error
Facility=Application
SymbolicName=PPJ_MSG_ERRORALLOCPORT
Language=English
Cannot allocate the parallel port required for the joystick
.

MessageId=+1
Severity=Informational
Facility=Application
SymbolicName=PPJ_MSG_DRIVERSTARTEDVER
Language=English
Driver version %2 successfully loaded.
.

;
;#endif
