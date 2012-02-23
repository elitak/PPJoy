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
/**  EventLog.c  Routine to send messages to the Windows Event Log.       **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/***************************************************************************/

#include "PPortJoy.h"
#include <wchar.h>

#define	DWORD	ULONG

/**************************************************************************/
/* Declare the code segments into which each routine will be placed       */
/* Used to declare pagable and initialisation code.                       */
/**************************************************************************/

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (PAGE,PPortJoy_WriteEventLog)
#endif

/**************************************************************************/
/* Routine synchronously send an IRP to the driver indicated by the       */
/* DeviceObject parameter. Once the routine returns the lower level       */
/* drivers will have processed the IRP. If CopyStack is set then this     */
/* routine will copy the current stack location for the next driver,      */
/* else the caller should set up the next stack themselves.               */
/**************************************************************************/
void PPortJoy_WriteEventLog (NTSTATUS ErrorMessageCode, PVOID DumpData, USHORT DumpSize, PCWSTR Message)
{
 PIO_ERROR_LOG_PACKET	LogPacket;
 PUCHAR					StrPtr;
 USHORT					MsgLen;

 PAGED_CODE();

 if (!Message)
  Message= L"";

 MsgLen= (wcslen(Message)+1)*sizeof(WCHAR);
 DumpSize= (DumpSize+1) & 0xFC;
 if (!DumpData)
  DumpSize= 0;

 LogPacket= IoAllocateErrorLogEntry (Globals.DriverObject,(UCHAR)(sizeof(IO_ERROR_LOG_PACKET)+DumpSize+MsgLen));
 if (!LogPacket)
 {
  PPJOY_DBGPRINT (FILE_EVENTLOG|PPJOY_ERROR, ("Cannot allocate LogEntry packet!!") );
  return;
 }

 StrPtr= ((PUCHAR)LogPacket->DumpData)+DumpSize;

 LogPacket->MajorFunctionCode= 0;
 LogPacket->RetryCount= 0;
 LogPacket->DumpDataSize= DumpSize;
 LogPacket->NumberOfStrings= (*Message)?1:0;
 LogPacket->StringOffset= (USHORT) (StrPtr-(PUCHAR)LogPacket);
// LogPacket->EventCategory= 0;
 LogPacket->ErrorCode= ErrorMessageCode;
 LogPacket->UniqueErrorValue= 0;
 LogPacket->FinalStatus= STATUS_SUCCESS;
 LogPacket->SequenceNumber= 0;

 if (DumpData)
  RtlCopyMemory(LogPacket->DumpData,DumpData,DumpSize);

 if (*Message)
 {
  RtlCopyMemory(StrPtr,Message,MsgLen);
  StrPtr+= MsgLen;
 }

 PPJOY_DBGPRINT (FILE_EVENTLOG|PPJOY_BABBLE, ("Writing EventLog entry 0x%p",LogPacket) );

 IoWriteErrorLogEntry (LogPacket);
}



