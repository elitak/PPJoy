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
/**  AsyncIO.c  Background thread that reads buttons for joysticks with   **/
/**             "complex" interface requirements.                         **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/***************************************************************************/

#include "PPortJoy.h"

#ifdef USE_SCAN_THREAD


/**************************************************************************/
/* Declare the code segments into which each routine will be placed       */
/* Used to declare pagable and initialisation code.                       */
/**************************************************************************/

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (PAGE, PPJoy_StartScanThread)
#endif

/**************************************************************************/
/* Thread to scan joysticks that need an elaborate read sequence.         */
/**************************************************************************/
VOID PPJoy_ScanThread (IN PVOID StartContext)
{
 int			Count;
 NTSTATUS		ntStatus;
 LARGE_INTEGER	delay;

 LARGE_INTEGER	stamp1;
 LARGE_INTEGER	stamp2;
 LARGE_INTEGER	freq;

 PPJOY_DBGPRINT (FILE_ASYNC|PPJOY_FENTRY, ("PPJoy_ScanThread") );

 delay.QuadPart= -200000;	// 20ms == 50Hz
 Count= 0;

 stamp2= KeQueryPerformanceCounter (&freq);
 while (!Globals.MustExit)
 {
  if ((Count++)==50)
  {
   Count=0;

   stamp1= stamp2;
   stamp2= KeQueryPerformanceCounter (&freq);

   PPJOY_DBGPRINT (FILE_ASYNC|PPJOY_BABBLE2, ("PPJoy_ScanThread Loop Count= 50 - delay = %I64uus",((stamp2.QuadPart-stamp1.QuadPart)*1000000/freq.QuadPart)) );
  }

  ntStatus= KeDelayExecutionThread(KernelMode,FALSE,&delay);
  if (!NT_SUCCESS (ntStatus))
  {
   PPJOY_DBGPRINT (FILE_PPORTJOY|PPJOY_ERROR, ("KeDelayExecutionThread() result  ntStatus(0x%x)",ntStatus) );
   return;
  }

  KeStallExecutionProcessor (14);
  KeStallExecutionProcessor (14);

  KeStallExecutionProcessor (14);
  KeStallExecutionProcessor (14);

  KeStallExecutionProcessor (14);
  KeStallExecutionProcessor (14);

  KeStallExecutionProcessor (14);

  KeStallExecutionProcessor (1400);

 }

 PPJOY_EXITPROC (FILE_ASYNC|PPJOY_FEXIT_STATUSOK, "ScanThread",STATUS_SUCCESS);

 PsTerminateSystemThread (0);
}


/**************************************************************************/
/* Create thread to do background read of joystick data. The thread will  */
/* sit (more or less) idle until a relevant joystick unit is started.     */
/**************************************************************************/
NTSTATUS PPJoy_StartScanThread (void)
{
 NTSTATUS			ntStatus;
 OBJECT_ATTRIBUTES	ObjAttrib;
 HANDLE				ThreadHandle;

 PAGED_CODE ();

 PPJOY_DBGPRINT (FILE_ASYNC|PPJOY_FENTRY, ("PPJoy_StartScanThread") );

 return STATUS_UNSUCCESSFUL;

// PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("KeQueryTimeIncrement() return value is %d",KeQueryTimeIncrement()) );
/// NTSTATUS  KeDelayExecutionThread(KernelMode,FALSE,   IN PLARGE_INTEGER  Interval );
/// VOID  KeStallExecutionProcessor( IN ULONG  MicroSeconds  );

 InitializeObjectAttributes (&ObjAttrib,NULL,OBJ_KERNEL_HANDLE,NULL,NULL);		// Not sure why we need this - following example from PARCLASS' thread.c
 ntStatus= PsCreateSystemThread (&ThreadHandle,THREAD_ALL_ACCESS,&ObjAttrib,NULL,NULL,&PPJoy_ScanThread,NULL/*Context*/);

 if (NT_SUCCESS (ntStatus))
 {
  /* Now we get a object reference to this handle. */
  ntStatus= ObReferenceObjectByHandle (ThreadHandle,THREAD_ALL_ACCESS,NULL,KernelMode,&(Globals.ThreadObject),NULL);
  ZwClose(ThreadHandle);

  /* If we could not get object handle - cause thread to exit */
  if (!NT_SUCCESS (ntStatus))
  {
   PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_ERROR, ("ObReferenceObjectByHandle() result was 0x%X - killing thread",ntStatus) );
   Globals.MustExit= 1;
  }
 }
 else
 {
  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_ERROR, ("PsCreateSystemThread() result was 0x%X - thread not created",ntStatus) );
 }
  
 PPJOY_EXITPROC (FILE_ASYNC|PPJOY_FEXIT_STATUSOK, "StartScanThread",ntStatus);

 return ntStatus;
}

#endif

#if 0

 LARGE_INTEGER	stamp1;
 LARGE_INTEGER	stamp2;
 LARGE_INTEGER	freq;
 LARGE_INTEGER	delay;
 NTSTATUS		ntStatus;

 PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("KeQueryTimeIncrement() return value is %d",KeQueryTimeIncrement()) );
 
 delay.QuadPart= -10;	// Should be 1us. Lets see how close we can come to this :-)

 stamp1= KeQueryPerformanceCounter (&freq);  ntStatus= KeDelayExecutionThread(KernelMode,FALSE,&delay);  stamp2= KeQueryPerformanceCounter (&freq);
 PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: delay= %ldus",delay.QuadPart/-10) );  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: delta= %ldus (perf counter)",((stamp2.QuadPart-stamp1.QuadPart)*1000000/freq.QuadPart)) );  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: freq= %ld",freq.QuadPart) );  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: ntstatus= %X\n",ntStatus) );

 delay.QuadPart= -100;	// Should be 1us. Lets see how close we can come to this :-)

 stamp1= KeQueryPerformanceCounter (&freq);  ntStatus= KeDelayExecutionThread(KernelMode,FALSE,&delay);  stamp2= KeQueryPerformanceCounter (&freq);
 PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: delay= %ldus",delay.QuadPart/-10) );  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: delta= %ldus (perf counter)",((stamp2.QuadPart-stamp1.QuadPart)*1000000/freq.QuadPart)) );  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: freq= %ld",freq.QuadPart) );  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: ntstatus= %X\n",ntStatus) );

 delay.QuadPart= -1000;	// Should be 1us. Lets see how close we can come to this :-)

 stamp1= KeQueryPerformanceCounter (&freq);  ntStatus= KeDelayExecutionThread(KernelMode,FALSE,&delay);  stamp2= KeQueryPerformanceCounter (&freq);
 PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: delay= %ldus",delay.QuadPart/-10) );  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: delta= %ldus (perf counter)",((stamp2.QuadPart-stamp1.QuadPart)*1000000/freq.QuadPart)) );  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: freq= %ld",freq.QuadPart) );  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: ntstatus= %X\n",ntStatus) );

 delay.QuadPart= -10000;	// Should be 1us. Lets see how close we can come to this :-)

 stamp1= KeQueryPerformanceCounter (&freq);  ntStatus= KeDelayExecutionThread(KernelMode,FALSE,&delay);  stamp2= KeQueryPerformanceCounter (&freq);
 PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: delay= %ldus",delay.QuadPart/-10) );  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: delta= %ldus (perf counter)",((stamp2.QuadPart-stamp1.QuadPart)*1000000/freq.QuadPart)) );  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: freq= %ld",freq.QuadPart) );  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: ntstatus= %X\n",ntStatus) );

 delay.QuadPart= -100000;	// Should be 1us. Lets see how close we can come to this :-)

 stamp1= KeQueryPerformanceCounter (&freq);  ntStatus= KeDelayExecutionThread(KernelMode,FALSE,&delay);  stamp2= KeQueryPerformanceCounter (&freq);
 PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: delay= %ldus",delay.QuadPart/-10) );  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: delta= %ldus (perf counter)",((stamp2.QuadPart-stamp1.QuadPart)*1000000/freq.QuadPart)) );  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: freq= %ld",freq.QuadPart) );  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: ntstatus= %X\n",ntStatus) );

 delay.QuadPart= -1000000;	// Should be 1us. Lets see how close we can come to this :-)

 stamp1= KeQueryPerformanceCounter (&freq);  ntStatus= KeDelayExecutionThread(KernelMode,FALSE,&delay);  stamp2= KeQueryPerformanceCounter (&freq);
 PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: delay= %ldus",delay.QuadPart/-10) );  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: delta= %ldus (perf counter)",((stamp2.QuadPart-stamp1.QuadPart)*1000000/freq.QuadPart)) );  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: freq= %ld",freq.QuadPart) );  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: ntstatus= %X\n",ntStatus) );

 delay.QuadPart= -10000000;	// Should be 1us. Lets see how close we can come to this :-)

 stamp1= KeQueryPerformanceCounter (&freq);  ntStatus= KeDelayExecutionThread(KernelMode,FALSE,&delay);  stamp2= KeQueryPerformanceCounter (&freq);
 PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: delay= %ldus",delay.QuadPart/-10) );  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: delta= %ldus (perf counter)",((stamp2.QuadPart-stamp1.QuadPart)*1000000/freq.QuadPart)) );  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: freq= %ld",freq.QuadPart) );  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: ntstatus= %X\n",ntStatus) );

 delay.QuadPart= -160000;	// Should be 1us. Lets see how close we can come to this :-)

 stamp1= KeQueryPerformanceCounter (&freq);  ntStatus= KeDelayExecutionThread(KernelMode,FALSE,&delay);  stamp2= KeQueryPerformanceCounter (&freq);
 PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: delay= %ldus",delay.QuadPart/-10) );  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: delta= %ldus (perf counter)",((stamp2.QuadPart-stamp1.QuadPart)*1000000/freq.QuadPart)) );  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: freq= %ld",freq.QuadPart) );  PPJOY_DBGPRINT (FILE_ASYNC | PPJOY_BABBLE2, ("StartScanThread: ntstatus= %X\n",ntStatus) );

#endif

