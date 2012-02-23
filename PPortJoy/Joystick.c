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
/**  Joystick.c Routines to initialise and read the various joystick      **/
/**             interfaces.                                               **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/**                                                                       **/
/***************************************************************************/

#include "PPortJoy.h"

#include "BitNames.h"
#include "Joystick.h"
#include "JoyDefs.h"

/**************************************************************************/
/* Declare the code segments into which each routine will be placed       */
/* Used to declare pagable and initialisation code.                       */
/**************************************************************************/

#ifdef ALLOC_PRAGMA
    #pragma alloc_text (PAGE, InitIanHarries)
    #pragma alloc_text (PAGE, InitTurboGraFX)
    #pragma alloc_text (PAGE, InitTheMaze)
    #pragma alloc_text (PAGE, InitLinux0802)
    #pragma alloc_text (PAGE, InitLinuxDB9c)
    #pragma alloc_text (PAGE, InitTormod)
    #pragma alloc_text (PAGE, InitDirectPro)
    #pragma alloc_text (PAGE, InitLPTJoystick)
    #pragma alloc_text (PAGE, InitCHAMPGames)
    #pragma alloc_text (PAGE, InitSTFormat)
    #pragma alloc_text (PAGE, InitSNESKey2600)
    #pragma alloc_text (PAGE, InitAmiga4Play)
    #pragma alloc_text (PAGE, InitPCAE)
    #pragma alloc_text (PAGE, InitGenesisLin)
    #pragma alloc_text (PAGE, InitGenesisDPP)
    #pragma alloc_text (PAGE, InitGenesisNTP)
    #pragma alloc_text (PAGE, InitSNESPad)
    #pragma alloc_text (PAGE, InitGenesisCC)
    #pragma alloc_text (PAGE, InitGenesisSNES)
    #pragma alloc_text (PAGE, InitPSXStandard)
    #pragma alloc_text (PAGE, InitNullFunc)
    #pragma alloc_text (PAGE, InitLinuxGamecon)
    #pragma alloc_text (PAGE, InitLPTSwitch)
    #pragma alloc_text (PAGE, InitFMSBuddyBox)
    #pragma alloc_text (PAGE, InitPowerPadLin)
    #pragma alloc_text (PAGE, InitGenesisDPP6)
#endif



/* Offsets for the various registers */
#define	DATAPORT(X)					((X)+0)
#define	STATUSPORT(X)				((X)+1)
#define	CONTROLPORT(X)				((X)+2)

/* Precomputed values for the Control register for forward and reverse    */
/* mode respectively. In all cases, ensure that all control output lines  */
/* are at logical 1 (+5v) at the parallel port connector. Some interfaces */
/* need this.                                                             */
#define	CONTROL_FORWARD			0x04
#define	CONTROL_REVERSE			0x24
#define	CONTROL_INTERRUPT		0x10

#define GENESISLIN_SEL_HIGH		0x24
#define GENESISLIN_SEL_LOW		0x26

#define GENESISDPP_SEL_HIGH		0xFF
#define GENESISDPP_SEL_LOW		0xFE

#define GENESISNTP_SEL_HIGH		0x24
#define GENESISNTP_SEL_LOW		0x25

#define GENESISCC_SEL_HIGH		0x24
#define GENESISCC_SEL_LOW		0x22

#define	GENESISSNES_SEL_HIGH	0x04
#define	GENESISSNES_SEL_LOW		0x06

#define	SNESPAD_POWER			0xFC
#define SNESPAD_CLOCK			0x01
#define	SNESPAD_LATCH			0x02


void ToggleSpeaker (void)
{
 UCHAR SpeakerState;

 SpeakerState= READ_PORT_UCHAR ((PUCHAR)0x61);
 SpeakerState= (SpeakerState&0xFE)^2;
 WRITE_PORT_UCHAR ((PUCHAR)0x61,SpeakerState);
}

/**************************************************************************/
/* Interface initialisation routines                                      */
/**************************************************************************/

void InitIanHarries (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Standard forward mode */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),CONTROL_FORWARD);

 /* Set D3-D7 to +5V to power interface */
 WRITE_PORT_UCHAR (DATAPORT(BasePort),0xF8);
}

void InitTurboGraFX (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Standard forward mode */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),CONTROL_FORWARD);

 /* Set D7 to +5V to power interface, D0-D6 to +5V to disable all sticks */
 WRITE_PORT_UCHAR (DATAPORT(BasePort),0xFF);
}

void InitTheMaze (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Standard forward mode */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),CONTROL_FORWARD);
}

void InitLinux0802 (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Standard reverse mode for UnitNumber 1 (modprode device 9) */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),CONTROL_REVERSE);
}

void InitLinuxDB9c (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Standard reverse mode */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),CONTROL_REVERSE);
}

void InitTormod (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Standard forward mode */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),CONTROL_FORWARD);

 /* Set D0-D7 to +5V to power interface */
 WRITE_PORT_UCHAR (DATAPORT(BasePort),0xFF);
}

void InitDirectPro (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Standard forward mode */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),CONTROL_FORWARD);

 /* Set D0 and D1 to +5V to power interface */
 WRITE_PORT_UCHAR (DATAPORT(BasePort),0x03);
}

void InitLPTJoystick (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Standard forward mode - not really neccessary for this interface */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),CONTROL_FORWARD);
}

void InitCHAMPGames  (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Standard forward mode - not really neccessary for this interface */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),CONTROL_FORWARD);
}

void InitSTFormat (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Standard reverse mode */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),CONTROL_REVERSE);
}

void InitSNESKey2600 (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Standard forward mode */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),CONTROL_FORWARD);

 /* Set D0, D1 and D2 to +5V to power interface */
 WRITE_PORT_UCHAR (DATAPORT(BasePort),0x07);
}

void InitAmiga4Play (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Standard reverse mode */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),CONTROL_REVERSE);
}

void InitPCAE (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Standard reverse mode */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),CONTROL_REVERSE);
}

void InitGenesisLin (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Set port in reverse mode, make sure that select line is high */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),GENESISLIN_SEL_HIGH);
}

void InitGenesisDPP (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Standard forward mode */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),CONTROL_FORWARD);
 /* Set lines to power interface; select line high */
 WRITE_PORT_UCHAR (DATAPORT(BasePort),GENESISDPP_SEL_HIGH);
}

void InitGenesisNTP (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Set port in reverse mode, make sure that select line is high */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),GENESISNTP_SEL_HIGH);
}

void InitSNESPad (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Standard forward mode */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),CONTROL_FORWARD);

 /* Set D2 - D7 to +5V to power interface */
 WRITE_PORT_UCHAR (DATAPORT(BasePort),SNESPAD_POWER);
}

void InitGenesisCC (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Set port in reverse mode, make sure that both select lines are high */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),GENESISCC_SEL_HIGH);
}

void InitGenesisSNES (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Set port in forward mode, make sure that select line is high */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),GENESISSNES_SEL_HIGH);

 /* Power on interface. Diagram shows D0-D2 - we will power all of them  */
 WRITE_PORT_UCHAR (DATAPORT(BasePort),0xFF);
}

void InitPSXStandard (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Standard forward mode, with all lines high */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),CONTROL_FORWARD);

 /* Power on interface; set all interfaces lines high as well. */
 WRITE_PORT_UCHAR (DATAPORT(BasePort),0xFF);
}

void InitNullFunc (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Nothing to do */
}

void InitLinuxGamecon (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Standard forward mode */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),CONTROL_FORWARD);
}

void InitLPTSwitch (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Standard forward mode, with all Control and Output lines high */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),CONTROL_FORWARD);
 WRITE_PORT_UCHAR (DATAPORT(BasePort),0xFF);
}

void InitFMSBuddyBox (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Standard forward mode, with all Control lines high, Interrupt ON */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),CONTROL_FORWARD|CONTROL_INTERRUPT);
}

void InitPowerPadLin (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* No particular setup needed - just set it to something know */
 /* Standard forward mode, with all Control and Output lines high */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),CONTROL_FORWARD);
 WRITE_PORT_UCHAR (DATAPORT(BasePort),0xFF);
}

void InitGenesisDPP6 (PUCHAR BasePort, UCHAR JoyType)
{
 PAGED_CODE();

 /* Standard forward mode */
 WRITE_PORT_UCHAR (CONTROLPORT(BasePort),CONTROL_FORWARD);
 /* Set lines to power interface; select line high */
 WRITE_PORT_UCHAR (DATAPORT(BasePort),GENESISDPP_SEL_HIGH);
}


/**************************************************************************/
/* Interface read routines                                                */
/**************************************************************************/

void ReadIanHarries (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		Data;
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;

 /* Exit if invalid UnitNumber, leave RawInput unchanged */
 if (UnitNumber>0)
  return;

 /* Read the data we need from the status port */
 Data= READ_PORT_UCHAR (STATUSPORT(BasePort));
 PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("IANHARRIES: Read parallel port 0x%p status =0x%x",BasePort,Data) );

 BUTTON1(RawInput,IANHARRIES_FIRE1(Data))

 AXISLR (RawInput,IANHARRIES_LEFT(Data),IANHARRIES_RIGHT(Data))
 AXISUD (RawInput,IANHARRIES_UP(Data),IANHARRIES_DOWN(Data))
}

void ReadTurboGraFX (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		Data;
 UCHAR		Control;
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;
 UCHAR		JoyType=	DeviceExtension->Config.JoyType;

 /* Exit if invalid UnitNumber, leave RawInput unchanged */
 if (UnitNumber>6)
  return;

 /* Set Dx to 0 to enable UnitNumber x, all other bits should be 1 (+5v) */
 Data= (UCHAR) ~(1<<UnitNumber);
 WRITE_PORT_UCHAR (DATAPORT(BasePort),Data);

 /* Read the data we need from the status port */
 Data= READ_PORT_UCHAR (STATUSPORT(BasePort));
 PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("TURBOGRAFX: Read parallel port 0x%p data =0x%x for stick %d",
												 BasePort,Data,UnitNumber) );
 /* Read data we need from control port - will not work on all cards */
 Control= READ_PORT_UCHAR (CONTROLPORT(BasePort));
 PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("TURBOGRAFX: Read parallel port 0x%p ctrl data =0x%x for stick %d",
												 BasePort,Control,UnitNumber) );

 BUTTON1(RawInput,TURBOGFX_FIRE1(Data))
 BUTTON2(RawInput,TURBOGFX_FIRE2(Control))
 BUTTON3(RawInput,TURBOGFX_FIRE3(Control))

 if (JoyType==IF_TURBOGFX45)				/* Do we want buttons 4 and 5 swapped? */
 {
  BUTTON4(RawInput,TURBOGFX_FIRE5(Control))
  BUTTON5(RawInput,TURBOGFX_FIRE4(Control))
 }
 else
 {
  BUTTON4(RawInput,TURBOGFX_FIRE4(Control))
  BUTTON5(RawInput,TURBOGFX_FIRE5(Control))
 }

 AXISLR (RawInput,TURBOGFX_LEFT(Data),TURBOGFX_RIGHT(Data))
 AXISUD (RawInput,TURBOGFX_UP(Data),TURBOGFX_DOWN(Data))
}

void ReadTheMaze (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		Data;
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;

 /* Exit if invalid UnitNumber, leave RawInput unchanged */
 if (UnitNumber>0)
  return;

 /* Read the data we need from the status port */
 Data= READ_PORT_UCHAR (STATUSPORT(BasePort));
 PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("THEMAZE: Read parallel port 0x%p data =0x%x",BasePort,Data) );

 BUTTON1(RawInput,THEMAZE_FIRE1(Data))
 BUTTON2(RawInput,THEMAZE_FIRE2(Data))
 BUTTON3(RawInput,THEMAZE_FIRE3(Data))

 AXISLR (RawInput,THEMAZE_LEFT(Data),THEMAZE_RIGHT(Data))
 AXISUD (RawInput,THEMAZE_UP(Data),THEMAZE_DOWN(Data))
}

void ReadLinux0802 (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		Data;
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;

 if (UnitNumber==0)
 {
  /* Read the data we need from the status port */
  Data= READ_PORT_UCHAR (STATUSPORT(BasePort));
  PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("LINUX0802: Read parallel port 0x%p data =0x%x for stick 0",BasePort,Data) );

  BUTTON1(RawInput,LIN0802_1_FIRE1(Data))

  AXISLR (RawInput,LIN0802_1_LEFT(Data),LIN0802_1_RIGHT(Data))
  AXISUD (RawInput,LIN0802_1_UP(Data),LIN0802_1_DOWN(Data))
 }
 else if (UnitNumber==1)
 {
  /* Read the data we need from the data port - only works in reverse mode */
  Data= READ_PORT_UCHAR (DATAPORT(BasePort));
  PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("LINUX0802: Read parallel port 0x%p data =0x%x for stick 1",BasePort,Data) );

  BUTTON1(RawInput,LIN0802_2_FIRE1(Data))

  AXISLR (RawInput,LIN0802_2_LEFT(Data),LIN0802_2_RIGHT(Data))
  AXISUD (RawInput,LIN0802_2_UP(Data),LIN0802_2_DOWN(Data))
 }
}

void ReadLinuxDB9c (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		Data;
 UCHAR		Status;
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;

 /* Exit if invalid UnitNumber, leave RawInput unchanged */
 if (UnitNumber>0)
  return;

 /* Read the data we need from the data port - only works in reverse mode */
 Data= READ_PORT_UCHAR (DATAPORT(BasePort));
 PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("LINUXDB9C: Read parallel port 0x%p data =0x%x",BasePort,Data) );

 BUTTON1(RawInput,LINDB9_FIRE1(Data))
 BUTTON2(RawInput,LINDB9_FIRE2(Data))
 BUTTON3(RawInput,LINDB9_FIRE3(Data))
 BUTTON4(RawInput,LINDB9_FIRE4(Data))

 AXISLR (RawInput,LINDB9_LEFT(Data),LINDB9_RIGHT(Data))
 AXISUD (RawInput,LINDB9_UP(Data),LINDB9_DOWN(Data))

/* The following is only compiled if the flag is set to use the status port */
/* for extra buttons for the LinuxDB9c interface.                           */
#ifdef LINUXDB9_EXTRABUTTONS

 /* Read the data we need from the status port */
 Status= READ_PORT_UCHAR (STATUSPORT(BasePort));
 PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("LINUXDB9C: Read parallel port 0x%p status data =0x%x",BasePort,Status) );

 BUTTON5(RawInput,LINDB9_FIRE5(Status))
 BUTTON6(RawInput,LINDB9_FIRE6(Status))
 BUTTON7(RawInput,LINDB9_FIRE7(Status))
 BUTTON8(RawInput,LINDB9_FIRE8(Status))

#endif
}

void ReadTormod (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		Data;
 UCHAR		Control;
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;

 /* Exit if invalid UnitNumber, leave RawInput unchanged */
 if (UnitNumber>0)
  return;

  /* Read the data we need from the status port */
 Data= READ_PORT_UCHAR (STATUSPORT(BasePort));
 PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("TORMOD: Read parallel port 0x%p data =0x%x",BasePort,Data) );

 /* Read data we need from control port - will not work on all cards */
 Control= READ_PORT_UCHAR (CONTROLPORT(BasePort));
 PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("TORMOD: Read parallel port 0x%p ctrl data =0x%x",BasePort,Control) );

 BUTTON1(RawInput,TORMOD_FIRE1(Data))
 BUTTON2(RawInput,TORMOD_FIRE2(Control))

 AXISLR (RawInput,TORMOD_LEFT(Data),TORMOD_RIGHT(Data))
 AXISUD (RawInput,TORMOD_UP(Data),TORMOD_DOWN(Data))

 BUTTON3(RawInput,TORMOD_FIRE3(Control))
 BUTTON4(RawInput,TORMOD_FIRE4(Control))
}

void ReadDirectPro (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		Data;
 UCHAR		Control;
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;
 
 /* Exit if invalid UnitNumber, leave RawInput unchanged */
 if (UnitNumber>0)
  return;

  /* Read the data we need from the status port */
 Data= READ_PORT_UCHAR (STATUSPORT(BasePort));
 PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("DIRECTPRO: Read parallel port 0x%p data =0x%x",BasePort,Data) );

 /* Read data we need from control port - will not work on all cards */
 Control= READ_PORT_UCHAR (CONTROLPORT(BasePort));
 PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("DIRECTPRO: Read parallel port 0x%p ctrl data =0x%x",BasePort,Control) );

 BUTTON1(RawInput,DPPJOY_FIRE1(Data))
 BUTTON2(RawInput,DPPJOY_FIRE2(Data))

 AXISLR (RawInput,DPPJOY_LEFT(Control),DPPJOY_RIGHT(Control))
 AXISUD (RawInput,DPPJOY_UP(Control),DPPJOY_DOWN(Control))
}

void ReadLPTJoystick (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		Data;
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;

 /* Exit if invalid UnitNumber, leave RawInput unchanged */
 if (UnitNumber>0)
  return;

  /* Read the data we need from the status port */
 Data= READ_PORT_UCHAR (STATUSPORT(BasePort));
 PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("LPTJoystick: Read parallel port 0x%p data =0x%x",BasePort,Data) );

 BUTTON1(RawInput,LPTJOY_FIRE1(Data))

 AXISLR (RawInput,LPTJOY_LEFT(Data),LPTJOY_RIGHT(Data))
 AXISUD (RawInput,LPTJOY_UP(Data),LPTJOY_DOWN(Data))
}

void ReadCHAMPGames (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		Data;
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;

 /* Exit if invalid UnitNumber, leave RawInput unchanged */
 if (UnitNumber>0)
  return;

  /* Read the data we need from the status port */
 Data= READ_PORT_UCHAR (STATUSPORT(BasePort));
 PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("CHAMPGames: Read parallel port 0x%p data =0x%x",BasePort,Data) );

 BUTTON1(RawInput,CHAMPG_FIRE1(Data))

 AXISLR (RawInput,CHAMPG_LEFT(Data),CHAMPG_RIGHT(Data))
 AXISUD (RawInput,CHAMPG_UP(Data),CHAMPG_DOWN(Data))
}

void ReadSTFormat (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		Data;
 UCHAR		Status;
 UCHAR		Control;
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;

 /* Read the data we need from the data port - only works in reverse mode */
 Data= READ_PORT_UCHAR (DATAPORT(BasePort));
 PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("STFORMAT: Read parallel port 0x%p data =0x%x",BasePort,Data) );

 if (UnitNumber==0)
 {
  /* Read the data we need from the control port */
  Control= READ_PORT_UCHAR (STATUSPORT(BasePort));
  PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("STFORMAT: Read parallel port 0x%p control =0x%x",BasePort,Control) );

  BUTTON1(RawInput,STFORMAT1_FIRE1(Control))

  AXISLR (RawInput,STFORMAT1_LEFT(Data),STFORMAT1_RIGHT(Data))
  AXISUD (RawInput,STFORMAT1_UP(Data),STFORMAT1_DOWN(Data))
 }
 else if (UnitNumber==1)
 {
  /* Read the data we need from the status port */
  Status= READ_PORT_UCHAR (STATUSPORT(BasePort));
  PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("STFORMAT: Read parallel port 0x%p status =0x%x",BasePort,Status) );

  BUTTON1(RawInput,STFORMAT2_FIRE1(Status))

  AXISLR (RawInput,STFORMAT2_LEFT(Data),STFORMAT2_RIGHT(Data))
  AXISUD (RawInput,STFORMAT2_UP(Data),STFORMAT2_DOWN(Data))
 }
}

void ReadSNESKey2600 (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		Data;
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;

 /* Exit if invalid UnitNumber, leave RawInput unchanged */
 if (UnitNumber>0)
  return;

  /* Read the data we need from the status port */
 Data= READ_PORT_UCHAR (STATUSPORT(BasePort));
 PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("SNESKey2600: Read parallel port 0x%p data =0x%x",BasePort,Data) );

 BUTTON1(RawInput,SNES2600_FIRE1(Data))

 AXISLR (RawInput,SNES2600_LEFT(Data),SNES2600_RIGHT(Data))
 AXISUD (RawInput,SNES2600_UP(Data),SNES2600_DOWN(Data))
}

void ReadAmiga4Play (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		Data;
 UCHAR		Status;
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;

 /* Read the data we need from the data port - only works in reverse mode */
 Data= READ_PORT_UCHAR (DATAPORT(BasePort));
 PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("Amiga4Player: Read parallel port 0x%p data =0x%x",BasePort,Data) );

 /* Read the data we need from the status port */
 Status= READ_PORT_UCHAR (STATUSPORT(BasePort));
 PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("Amiga4Player: Read parallel port 0x%p status =0x%x",BasePort,Status) );

 if (UnitNumber==0)
 {
  BUTTON1(RawInput,AMIGA4_1_FIRE1(Status))
  BUTTON2(RawInput,AMIGA4_1_FIRE2(Status))

  AXISLR (RawInput,AMIGA4_1_LEFT(Data),AMIGA4_1_RIGHT(Data))
  AXISUD (RawInput,AMIGA4_1_UP(Data),AMIGA4_1_DOWN(Data))
 }
 else if (UnitNumber==1)
 {
  BUTTON1(RawInput,AMIGA4_2_FIRE1(Status))
  BUTTON2(RawInput,AMIGA4_2_FIRE2(Status))

  AXISLR (RawInput,AMIGA4_2_LEFT(Data),AMIGA4_2_RIGHT(Data))
  AXISUD (RawInput,AMIGA4_2_UP(Data),AMIGA4_2_DOWN(Data))
 }
}

void ReadPCAE (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		Data;
 UCHAR		Joy;
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;

 /* Read the data we need from the data port - only works in reverse mode */
 Data= READ_PORT_UCHAR (DATAPORT(BasePort));
 PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("PCAE: Read parallel port 0x%p data =0x%x",BasePort,Data) );

 /* Read the data we need from the joystick port */
 Joy= READ_PORT_UCHAR ((UCHAR*)0x201);
 PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("PCAE: Read joystick port 0x%p status =0x%x",BasePort,Joy) );

 if (UnitNumber==0)
 {
  BUTTON1(RawInput,PCAE1_FIRE1(Joy))

  AXISLR (RawInput,PCAE1_LEFT(Data),PCAE1_RIGHT(Data))
  AXISUD (RawInput,PCAE1_UP(Data),PCAE1_DOWN(Data))
 }
 else if (UnitNumber==1)
 {
  BUTTON1(RawInput,PCAE2_FIRE1(Joy))

  AXISLR (RawInput,PCAE2_LEFT(Data),PCAE2_RIGHT(Data))
  AXISUD (RawInput,PCAE2_UP(Data),PCAE2_DOWN(Data))
 }
}

void ReadGenesisLin (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		DataLow;
 UCHAR		DataHigh;
 UCHAR		DataP3;
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;
 UCHAR		JoySubType=	DeviceExtension->Config.JoySubType;
 ULONG		BitDelay=	DeviceExtension->Timing->genesis.BitDelay;
 ULONG		Bit6Delay=	DeviceExtension->Timing->genesis.Bit6Delay;

 GENESIS_SCAN_SKELETON(														\
	/*JOYSUBTYPE*/	JoySubType,												\
	/*SEL_PORT*/	CONTROLPORT(BasePort),									\
	/*SET_SEL_LOW*/	GENESISLIN_SEL_LOW,										\
	/*SET_SEL_HIGH*/GENESISLIN_SEL_HIGH,									\
	/*READ_LOW*/	DataLow= READ_PORT_UCHAR (DATAPORT(BasePort));,			\
	/*READ_HIGH*/	DataHigh= READ_PORT_UCHAR (DATAPORT(BasePort));,		\
	/*READ_P3*/		DataP3= READ_PORT_UCHAR (DATAPORT(BasePort));,			\
	/*BIT_DELAY*/	BitDelay,												\
	/*BIT6_DELAY*/	Bit6Delay												)

 PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("GenesisLin: Read parallel port 0x%p data =0x%X, 0x%X, 0x%X",BasePort,DataLow,DataHigh,DataP3) );

 GENESIS_REPORT_SKELETON(																	\
	/*RAWINPUT*/	RawInput,																	\
	/*JOYSUBTYPE*/	JoySubType,																\
	/*LEFT_LOW*/	LINDB9_LEFT(DataLow),		/*RIGHT_LOW*/	LINDB9_RIGHT(DataLow),		\
	/*FIRE1_LOW*/	LINDB9_FIRE1(DataLow),		/*FIRE2_LOW*/	LINDB9_FIRE2(DataLow),		\
	/*UP_HIGH*/		LINDB9_UP(DataHigh),		/*DOWN_HIGH*/	LINDB9_DOWN(DataHigh),		\
	/*LEFT_HIGH*/	LINDB9_LEFT(DataHigh),		/*RIGHT_HIGH*/	LINDB9_RIGHT(DataHigh),		\
	/*FIRE1_HIGH*/	LINDB9_FIRE1(DataHigh),		/*FIRE2_HIGH*/	LINDB9_FIRE2(DataHigh),		\
	/*UP_P3*/		LINDB9_UP(DataP3),			/*DOWN_P3*/		LINDB9_DOWN(DataP3),		\
	/*LEFT_P3*/		LINDB9_LEFT(DataP3),		/*RIGHT_P3*/	LINDB9_RIGHT(DataP3)		)
}

void ReadGenesisDPP (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		StatusLow;
 UCHAR		StatusHigh;
 UCHAR		StatusP3;
 UCHAR		ControlHigh;
 UCHAR		ControlP3;
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;
 UCHAR		JoySubType=	DeviceExtension->Config.JoySubType;
 ULONG		BitDelay=	DeviceExtension->Timing->genesis.BitDelay;
 ULONG		Bit6Delay=	DeviceExtension->Timing->genesis.Bit6Delay;

 GENESIS_SCAN_SKELETON(														\
	/*JOYSUBTYPE*/	JoySubType,												\
	/*SEL_PORT*/	DATAPORT(BasePort),										\
	/*SET_SEL_LOW*/	GENESISDPP_SEL_LOW,										\
	/*SET_SEL_HIGH*/GENESISDPP_SEL_HIGH,									\
	/*READ_LOW*/	StatusLow= READ_PORT_UCHAR (STATUSPORT(BasePort));,		\
	/*READ_HIGH*/	StatusHigh= READ_PORT_UCHAR (STATUSPORT(BasePort));		\
					ControlHigh= READ_PORT_UCHAR (CONTROLPORT(BasePort));,	\
	/*READ_P3*/		StatusP3= READ_PORT_UCHAR (STATUSPORT(BasePort));		\
					ControlP3= READ_PORT_UCHAR (CONTROLPORT(BasePort));,	\
	/*BIT_DELAY*/	BitDelay,												\
	/*BIT6_DELAY*/	Bit6Delay												)

 PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("GenesisDPP: Read parallel port 0x%p status =0x%X, 0x%X, 0x%X Ctl= 0x%X 0x%X",BasePort,StatusLow,StatusHigh,StatusP3,ControlHigh,ControlP3) );

 GENESIS_REPORT_SKELETON(																	\
	/*RAWINPUT*/	RawInput,																\
	/*JOYSUBTYPE*/	JoySubType,																\
	/*LEFT_LOW*/	DPPGEN_LEFT(StatusLow),		/*RIGHT_LOW*/	DPPGEN_RIGHT(StatusLow),	\
	/*FIRE1_LOW*/	DPPGEN_FIRE1(StatusLow),	/*FIRE2_LOW*/	DPPGEN_FIRE2(StatusLow),	\
	/*UP_HIGH*/		DPPGEN_UP(ControlHigh),		/*DOWN_HIGH*/	DPPGEN_DOWN(ControlHigh),	\
	/*LEFT_HIGH*/	DPPGEN_LEFT(StatusHigh),	/*RIGHT_HIGH*/	DPPGEN_RIGHT(StatusHigh),	\
	/*FIRE1_HIGH*/	DPPGEN_FIRE1(StatusHigh),	/*FIRE2_HIGH*/	DPPGEN_FIRE2(StatusHigh),	\
	/*UP_P3*/		DPPGEN_UP(ControlP3),		/*DOWN_P3*/		DPPGEN_DOWN(ControlP3),		\
	/*LEFT_P3*/		DPPGEN_LEFT(StatusP3),		/*RIGHT_P3*/	DPPGEN_RIGHT(StatusP3)		)
}

void ReadGenesisNTP (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		DataLow;
 UCHAR		DataHigh;
 UCHAR		DataP3;
 UCHAR		StatusLow;
 UCHAR		StatusHigh;
 UCHAR		StatusP3;
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;
 UCHAR		JoySubType=	DeviceExtension->Config.JoySubType;
 ULONG		BitDelay=	DeviceExtension->Timing->genesis.BitDelay;
 ULONG		Bit6Delay=	DeviceExtension->Timing->genesis.Bit6Delay;
 
 GENESIS_SCAN_SKELETON(														\
	/*JOYSUBTYPE*/	JoySubType,												\
	/*SEL_PORT*/	CONTROLPORT(BasePort),									\
	/*SET_SEL_LOW*/	GENESISNTP_SEL_LOW,										\
	/*SET_SEL_HIGH*/GENESISNTP_SEL_HIGH,									\
	/*READ_LOW*/	DataLow= READ_PORT_UCHAR (DATAPORT(BasePort));			\
					StatusLow= READ_PORT_UCHAR (STATUSPORT(BasePort));,		\
	/*READ_HIGH*/	DataHigh= READ_PORT_UCHAR (DATAPORT(BasePort));			\
					StatusHigh= READ_PORT_UCHAR (STATUSPORT(BasePort));,	\
	/*READ_P3*/		DataP3= READ_PORT_UCHAR (DATAPORT(BasePort));			\
					StatusP3= READ_PORT_UCHAR (STATUSPORT(BasePort));,		\
	/*BIT_DELAY*/	BitDelay,												\
	/*BIT6_DELAY*/	Bit6Delay												)

 PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("GenesisNTP: Read parallel port 0x%p data= 0x%X, 0x%X, 0x%X  , Status= 0x%X, 0x%X, 0x%X ",BasePort,DataLow,DataHigh,DataP3,StatusLow,StatusHigh,StatusP3) );

 if (UnitNumber==0)
 {
  GENESIS_REPORT_SKELETON(																	\
	/*RAWINPUT*/	RawInput,																\
	/*JOYSUBTYPE*/	JoySubType,																\
	/*LEFT_LOW*/	NTPGEN1_LEFT(DataLow),		/*RIGHT_LOW*/	NTPGEN1_RIGHT(DataLow),		\
	/*FIRE1_LOW*/	NTPGEN1_FIRE1(DataLow),		/*FIRE2_LOW*/	NTPGEN1_FIRE2(DataLow),		\
	/*UP_HIGH*/		NTPGEN1_UP(DataHigh),		/*DOWN_HIGH*/	NTPGEN1_DOWN(DataHigh),		\
	/*LEFT_HIGH*/	NTPGEN1_LEFT(DataHigh),		/*RIGHT_HIGH*/	NTPGEN1_RIGHT(DataHigh),	\
	/*FIRE1_HIGH*/	NTPGEN1_FIRE1(DataHigh),	/*FIRE2_HIGH*/	NTPGEN1_FIRE2(DataHigh),	\
	/*UP_P3*/		NTPGEN1_UP(DataP3),			/*DOWN_P3*/		NTPGEN1_DOWN(DataP3),		\
	/*LEFT_P3*/		NTPGEN1_LEFT(DataP3),		/*RIGHT_P3*/	NTPGEN1_RIGHT(DataP3)		)
 }
 else if (UnitNumber==1)
 {
  GENESIS_REPORT_SKELETON(																	\
	/*RAWINPUT*/	RawInput,																\
	/*JOYSUBTYPE*/	JoySubType,																\
	/*LEFT_LOW*/	NTPGEN2_LEFT(StatusLow),	/*RIGHT_LOW*/	NTPGEN2_RIGHT(StatusLow),	\
	/*FIRE1_LOW*/	NTPGEN2_FIRE1(StatusLow),	/*FIRE2_LOW*/	NTPGEN2_FIRE2(StatusLow),	\
	/*UP_HIGH*/		NTPGEN2_UP(DataHigh),		/*DOWN_HIGH*/	NTPGEN2_DOWN(DataHigh),		\
	/*LEFT_HIGH*/	NTPGEN2_LEFT(StatusHigh),	/*RIGHT_HIGH*/	NTPGEN2_RIGHT(StatusHigh),	\
	/*FIRE1_HIGH*/	NTPGEN2_FIRE1(StatusHigh),	/*FIRE2_HIGH*/	NTPGEN2_FIRE2(StatusHigh),	\
	/*UP_P3*/		NTPGEN2_UP(DataP3),			/*DOWN_P3*/		NTPGEN2_DOWN(DataP3),		\
	/*LEFT_P3*/		NTPGEN2_LEFT(StatusP3),		/*RIGHT_P3*/	NTPGEN2_RIGHT(StatusP3)		)
 }
}

void ReadGenesisCC (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		DataLow;
 UCHAR		DataHigh;
 UCHAR		DataP3;
 UCHAR		StatusLow;
 UCHAR		StatusHigh;
 UCHAR		StatusP3;
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;
 UCHAR		JoySubType=	DeviceExtension->Config.JoySubType;
 ULONG		BitDelay=	DeviceExtension->Timing->genesis.BitDelay;
 ULONG		Bit6Delay=	DeviceExtension->Timing->genesis.Bit6Delay;
 
 GENESIS_SCAN_SKELETON(														\
	/*JOYSUBTYPE*/	JoySubType,												\
	/*SEL_PORT*/	CONTROLPORT(BasePort),									\
	/*SET_SEL_LOW*/	GENESISCC_SEL_LOW,										\
	/*SET_SEL_HIGH*/GENESISCC_SEL_HIGH,										\
	/*READ_LOW*/	DataLow= READ_PORT_UCHAR (DATAPORT(BasePort));			\
					StatusLow= READ_PORT_UCHAR (STATUSPORT(BasePort));,		\
	/*READ_HIGH*/	DataHigh= READ_PORT_UCHAR (DATAPORT(BasePort));			\
					StatusHigh= READ_PORT_UCHAR (STATUSPORT(BasePort));,	\
	/*READ_P3*/		DataP3= READ_PORT_UCHAR (DATAPORT(BasePort));			\
					StatusP3= READ_PORT_UCHAR (STATUSPORT(BasePort));,		\
	/*BIT_DELAY*/	BitDelay,												\
	/*BIT6_DELAY*/	Bit6Delay												)

 PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("GenesisCC: Read parallel port 0x%p data= 0x%X, 0x%X, 0x%X  , Status= 0x%X, 0x%X, 0x%X ",BasePort,DataLow,DataHigh,DataP3,StatusLow,StatusHigh,StatusP3) );

 if (UnitNumber==0)
 {
  GENESIS_REPORT_SKELETON(																	\
	/*RAWINPUT*/	RawInput,																\
	/*JOYSUBTYPE*/	JoySubType,																\
	/*LEFT_LOW*/	CCGEN1_LEFT(StatusLow),		/*RIGHT_LOW*/	CCGEN1_RIGHT(StatusLow),	\
	/*FIRE1_LOW*/	CCGEN1_FIRE1(StatusLow),	/*FIRE2_LOW*/	CCGEN1_FIRE2(DataLow),		\
	/*UP_HIGH*/		CCGEN1_UP(StatusHigh),		/*DOWN_HIGH*/	CCGEN1_DOWN(StatusHigh),	\
	/*LEFT_HIGH*/	CCGEN1_LEFT(StatusHigh),	/*RIGHT_HIGH*/	CCGEN1_RIGHT(StatusHigh),	\
	/*FIRE1_HIGH*/	CCGEN1_FIRE1(StatusHigh),	/*FIRE2_HIGH*/	CCGEN1_FIRE2(DataHigh),		\
	/*UP_P3*/		CCGEN1_UP(StatusP3),		/*DOWN_P3*/		CCGEN1_DOWN(StatusP3),		\
	/*LEFT_P3*/		CCGEN1_LEFT(StatusP3),		/*RIGHT_P3*/	CCGEN1_RIGHT(StatusP3)		)
 }
 else if (UnitNumber==1)
 {
  GENESIS_REPORT_SKELETON(																	\
	/*RAWINPUT*/	RawInput,																\
	/*JOYSUBTYPE*/	JoySubType,																\
	/*LEFT_LOW*/	CCGEN2_LEFT(DataLow),		/*RIGHT_LOW*/	CCGEN2_RIGHT(DataLow),		\
	/*FIRE1_LOW*/	CCGEN2_FIRE1(DataLow),		/*FIRE2_LOW*/	CCGEN2_FIRE2(DataLow),		\
	/*UP_HIGH*/		CCGEN2_UP(DataHigh),		/*DOWN_HIGH*/	CCGEN2_DOWN(DataHigh),		\
	/*LEFT_HIGH*/	CCGEN2_LEFT(DataHigh),		/*RIGHT_HIGH*/	CCGEN2_RIGHT(DataHigh),		\
	/*FIRE1_HIGH*/	CCGEN2_FIRE1(DataHigh),		/*FIRE2_HIGH*/	CCGEN2_FIRE2(DataHigh),		\
	/*UP_P3*/		CCGEN2_UP(DataP3),			/*DOWN_P3*/		CCGEN2_DOWN(DataP3),		\
	/*LEFT_P3*/		CCGEN2_LEFT(DataP3),		/*RIGHT_P3*/	CCGEN2_RIGHT(DataP3)		)
 }
}

void ReadGenesisSNES (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		ControlLow;
 UCHAR		ControlHigh;
 UCHAR		StatusLow;
 UCHAR		StatusHigh;
 UCHAR		StatusP3;
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;
 UCHAR		JoySubType=	DeviceExtension->Config.JoySubType;
 ULONG		BitDelay=	DeviceExtension->Timing->genesis.BitDelay;
 ULONG		Bit6Delay=	DeviceExtension->Timing->genesis.Bit6Delay;
 
 GENESIS_SCAN_SKELETON(														\
	/*JOYSUBTYPE*/	JoySubType,												\
	/*SEL_PORT*/	CONTROLPORT(BasePort),									\
	/*SET_SEL_LOW*/	GENESISSNES_SEL_LOW,									\
	/*SET_SEL_HIGH*/GENESISSNES_SEL_HIGH,									\
	/*READ_LOW*/	ControlLow= READ_PORT_UCHAR (CONTROLPORT(BasePort));	\
					StatusLow= READ_PORT_UCHAR (STATUSPORT(BasePort));,		\
	/*READ_HIGH*/	ControlHigh= READ_PORT_UCHAR (CONTROLPORT(BasePort));	\
					StatusHigh= READ_PORT_UCHAR (STATUSPORT(BasePort));,	\
	/*READ_P3*/		StatusP3= READ_PORT_UCHAR (STATUSPORT(BasePort));,		\
	/*BIT_DELAY*/	BitDelay,												\
	/*BIT6_DELAY*/	Bit6Delay												)

 PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("GenesisSNES: Read parallel port 0x%p Control= 0x%X, 0x%X , Status= 0x%X, 0x%X, 0x%X ",BasePort,ControlLow,ControlHigh,StatusLow,StatusHigh,StatusP3) );

 GENESIS_REPORT_SKELETON(																	\
	/*RAWINPUT*/	RawInput,																\
	/*JOYSUBTYPE*/	JoySubType,																\
	/*LEFT_LOW*/	SNESGEN_LEFT(StatusLow),	/*RIGHT_LOW*/	SNESGEN_RIGHT(StatusLow),	\
	/*FIRE1_LOW*/	SNESGEN_FIRE1(StatusLow),	/*FIRE2_LOW*/	SNESGEN_FIRE2(ControlLow),	\
	/*UP_HIGH*/		SNESGEN_UP(StatusHigh),		/*DOWN_HIGH*/	SNESGEN_DOWN(StatusHigh),	\
	/*LEFT_HIGH*/	SNESGEN_LEFT(StatusHigh),	/*RIGHT_HIGH*/	SNESGEN_RIGHT(StatusHigh),	\
	/*FIRE1_HIGH*/	SNESGEN_FIRE1(StatusHigh),	/*FIRE2_HIGH*/	SNESGEN_FIRE2(ControlHigh),	\
	/*UP_P3*/		SNESGEN_UP(StatusP3),		/*DOWN_P3*/		SNESGEN_DOWN(StatusP3),		\
	/*LEFT_P3*/		SNESGEN_LEFT(StatusP3),		/*RIGHT_P3*/	SNESGEN_RIGHT(StatusP3)		)
}

void ReadSNESPad (IN PDEVICE_EXTENSION DeviceExtension)
{
 char		Data[16];
 int		Count;
 int		TotalBits;
 int		PadMask;
 int		DPPMaskArray[]=		{PIN10_MASK,PIN12_MASK,PIN13_MASK,PIN15_MASK,PIN11_MASK};
 int		LinuxMaskArray[]=	{PIN10_MASK,PIN11_MASK,PIN12_MASK,PIN13_MASK,PIN15_MASK};
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;
 UCHAR		JoyType=	DeviceExtension->Config.JoyType;
 UCHAR		JoySubType=	DeviceExtension->Config.JoySubType;
 ULONG		BitDelay=	DeviceExtension->Timing->snes.BitDelay;
 ULONG		SetupDelay=	DeviceExtension->Timing->snes.SetupDelay;

 if (UnitNumber>4)
  return;

 if (JoyType==IF_SNESPADDPP)
  PadMask= DPPMaskArray[UnitNumber];
 else if (JoyType==IF_SNESPADLIN)
  PadMask= LinuxMaskArray[UnitNumber];

 if (JoySubType==SUBTYPE_SNES_NES)
  TotalBits= 8;
 else
  TotalBits= 16;

 /* Latch new samples into shift registers in Pad */
 WRITE_PORT_UCHAR (DATAPORT(BasePort),SNESPAD_POWER|SNESPAD_CLOCK|SNESPAD_LATCH);
 KeStallExecutionProcessor (SetupDelay);
 WRITE_PORT_UCHAR (DATAPORT(BasePort),SNESPAD_POWER|SNESPAD_CLOCK);

 for (Count=0;Count<TotalBits;Count++)					/* Loop for each data sample to be read */
 {
  KeStallExecutionProcessor (BitDelay);
  WRITE_PORT_UCHAR (DATAPORT(BasePort),SNESPAD_POWER);
  Data[Count]= READ_PORT_UCHAR (STATUSPORT(BasePort)) ^ STATUS_INVMASK;
  KeStallExecutionProcessor (BitDelay);
  WRITE_PORT_UCHAR (DATAPORT(BasePort),SNESPAD_POWER|SNESPAD_CLOCK);
 }

 SNES_REPORT_SKELETON(																		\
	/*RAWINPUT*/	RawInput,																\
	/*JOYSUBTYPE*/	JoySubType,																\
	/*INPUTARRAY*/	Data,																	\
	/*BITMASK*/		PadMask																	)
}

/* Entry condition: SEL is LOW, CLOCK is HIGH */
/* Exit condition: SEL is LOW, CLOCK is HIGH */
UCHAR PSX_Do_Byte_IO (UCHAR WriteData, PUCHAR WritePort, UCHAR BaseValue, UCHAR ClockHigh, UCHAR ClockLow,
					  UCHAR CmdHigh, UCHAR CmdLow, PUCHAR ReadPort, UCHAR BitMask, UCHAR InvMask, ULONG BitDelay)
{
 UCHAR	ReadBuffer;
 UCHAR	CommandValue;
 UCHAR	BitCount;

 ReadBuffer= 0;

 for (BitCount=0;BitCount<8;BitCount++)
 {
  /* Precompute value to write for current bit; excluding clock */
  CommandValue= BaseValue|((WriteData&1)?CmdHigh:CmdLow);
  WriteData>>= 1;

  /* Output data bit and take clock low. */
  /* If we need to split, first output data, then clock change */
  KeStallExecutionProcessor (BitDelay);
  WRITE_PORT_UCHAR (WritePort,(UCHAR)(CommandValue|ClockLow));
  KeStallExecutionProcessor (BitDelay);

  /* Read incoming data bit */
  ReadBuffer>>= 1;
  if ((READ_PORT_UCHAR(ReadPort)^InvMask)&BitMask)
   ReadBuffer|= 0x80;

  /* Take clock high while retaining data bit */
  WRITE_PORT_UCHAR (WritePort,(UCHAR)(CommandValue|ClockHigh));
 }

 return ReadBuffer;
}

UCHAR PSX_Wait_For_ACK (PUCHAR AckPort, UCHAR AckBitMask, UCHAR AckInvMask, ULONG MaxWaitUS)
{
 ULONG	Count;

 for (Count=0;Count<MaxWaitUS;Count++)
 {
  if (!((READ_PORT_UCHAR(AckPort)^AckInvMask)&AckBitMask))		// Wait for ACK line to go LOW
   return 1;
  KeStallExecutionProcessor (1);
 }
 return 0;
}

/* Entry and exit: Sel, Clock high */
void PSX_Do_Packet_IO (PUCHAR WritePort, UCHAR BaseValue, UCHAR SelHigh, UCHAR SelLow, UCHAR ClockHigh, UCHAR ClockLow, UCHAR CmdHigh, UCHAR CmdLow,
					   PUCHAR ReadPort, UCHAR ReadBitMask, UCHAR ReadInvMask, PUCHAR AckPort, UCHAR AckBitMask, UCHAR AckInvMask,
					   UCHAR *ReadBuffer, UCHAR ReadLen, UCHAR *WriteBuffer, UCHAR WriteLen, PTIMING_PSX Timing)
{
 UCHAR	ReadValue;
 UCHAR	WriteValue;
 UCHAR	Length;
 UCHAR	Count;
 ULONG	BitDelay=	Timing->BitDelay;
 ULONG	AckXDelay=	Timing->AckXDelay;

 WRITE_PORT_UCHAR (WritePort,(UCHAR)(BaseValue|SelLow|ClockHigh));
 KeStallExecutionProcessor (Timing->SelDelay);

 /* Perform first three bytes of IO */
 WriteValue= (WriteLen>0)?WriteBuffer[0]:0x01;	/* Default to tap1 start transfer */
 PSX_Do_Byte_IO(WriteValue,WritePort,(UCHAR)(BaseValue|SelLow),ClockHigh,ClockLow,CmdHigh,CmdLow,ReadPort,ReadBitMask,ReadInvMask,BitDelay);
 PSX_Wait_For_ACK(AckPort,AckBitMask,AckInvMask,Timing->Ack1Delay);

 WriteValue= (WriteLen>1)?WriteBuffer[1]:0x42;
 ReadValue= PSX_Do_Byte_IO(WriteValue,WritePort,(UCHAR)(BaseValue|SelLow),ClockHigh,ClockLow,CmdHigh,CmdLow,ReadPort,ReadBitMask,ReadInvMask,BitDelay);
 PSX_Wait_For_ACK(AckPort,AckBitMask,AckInvMask,AckXDelay);
 if (ReadLen>0) ReadBuffer[0]= ReadValue;
 Length= (ReadValue&0x0F)*2 + 3;

 WriteValue= (WriteLen>2)?WriteBuffer[2]:0x00;
 ReadValue= PSX_Do_Byte_IO(WriteValue,WritePort,(UCHAR)(BaseValue|SelLow),ClockHigh,ClockLow,CmdHigh,CmdLow,ReadPort,ReadBitMask,ReadInvMask,BitDelay);
 if (ReadLen>1) ReadBuffer[1]= ReadValue;

 if (WriteLen>Length) Length= WriteLen;
 for (Count=3;Count<Length;Count++)
 {
  PSX_Wait_For_ACK(AckPort,AckBitMask,AckInvMask,AckXDelay);

  WriteValue= (WriteLen>Count)?WriteBuffer[Count]:0x00;
  ReadValue= PSX_Do_Byte_IO(WriteValue,WritePort,(UCHAR)(BaseValue|SelLow),ClockHigh,ClockLow,CmdHigh,CmdLow,ReadPort,ReadBitMask,ReadInvMask,BitDelay);
  if (ReadLen>=Count) ReadBuffer[Count-1]= ReadValue;
 }
 KeStallExecutionProcessor (Timing->TailDelay);
 WRITE_PORT_UCHAR (WritePort,(UCHAR)(BaseValue|SelHigh|ClockHigh));

 if (ReadBuffer[1]!=0x5A)
 {
  if (PPJoy_RuntimeDebug&RTDEBUG_PSXERR)
   ToggleSpeaker();

#ifdef TRAP
  DbgPrint ("INVALID PACKET RECEIVED:\n");

  DbgPrint ("Write buffer contents: ");
  for (Count=0; Count<WriteLen; Count++)
   DbgPrint ("%02X ",WriteBuffer[Count]);
  DbgPrint ("\n");

  DbgPrint ("Read buffer contents: ");
  for (Count=0; Count<Length-1; Count++)
   DbgPrint ("%02X ",ReadBuffer[Count]);
  DbgPrint ("\n");
#endif
 }
}

void PSX_Make_Report (PDEVICE_EXTENSION DeviceExtension, UCHAR *Packet)
{
 UCHAR			Type;
 PRAWINPUT RawInput=	&(DeviceExtension->RawInput);

 if (Packet[1]!=0x5A)
 {
  PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_WARN, ("PSX_Make_Report: Invalid packet received.") );
  return;
 }

 DeviceExtension->ActiveMap= 0;
 Type= Packet[0];

 if ((Type==0x41)||(Type==0x53)||(Type==0x73))
 {
  BUTTON1(RawInput,(Packet[3]&0x08))
  BUTTON2(RawInput,(Packet[3]&0x04))
  BUTTON3(RawInput,(Packet[3]&0x02))
  BUTTON4(RawInput,(Packet[3]&0x01))

  BUTTON5(RawInput,(Packet[2]&0x08))
  BUTTON6(RawInput,(Packet[2]&0x01))
  BUTTON7(RawInput,(Packet[3]&0x80))
  BUTTON8(RawInput,(Packet[3]&0x40))
  BUTTON9(RawInput,(Packet[3]&0x20))
  BUTTON10(RawInput,(Packet[3]&0x10))

  if (Type==0x41)
  {
   AXISLR (RawInput,(Packet[2]&0x80),(Packet[2]&0x20))
   AXISUD (RawInput,(Packet[2]&0x10),(Packet[2]&0x40))

   return;
  }

  DeviceExtension->ActiveMap= 1;

  /* These macros map to buttons D0 to D3 */
  AXISLR (RawInput,(Packet[2]&0x80),(Packet[2]&0x20))
  AXISUD (RawInput,(Packet[2]&0x10),(Packet[2]&0x40))

  /* Wrong wrong wrong. These buttons enf up D15-D18 */
  /*
  BUTTON11(RawInput,(Packet[2]&0x80))
  BUTTON12(RawInput,(Packet[2]&0x40))
  BUTTON13(RawInput,(Packet[2]&0x20))
  BUTTON14(RawInput,(Packet[2]&0x10))
  */

  if (Type==0x73)
  {
   BUTTON11(RawInput,(Packet[2]&0x04))
   BUTTON12(RawInput,(Packet[2]&0x02))
  }

  AXISX2_UANALOG (RawInput,(Packet[4]),0x00,0xFF)
  AXISY2_UANALOG (RawInput,(Packet[5]),0x00,0xFF)
  AXISLR_UANALOG (RawInput,(Packet[6]),0x00,0xFF)
  AXISUD_UANALOG (RawInput,(Packet[7]),0x00,0xFF)

  return;
 }
}

void ReadPSXPBLIB (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		ReadPacket[32];
 UCHAR		WritePacket[32];
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;

 WritePacket[0]= 0x01;
 WritePacket[1]= 0x42;

 RtlZeroMemory(&ReadPacket,sizeof(ReadPacket));

 if (UnitNumber==0)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXLIB1_BASE,PSXLIB1_SEL,0,PSXLIB1_CLK,0,PSXLIB1_CMD,0,
					STATUSPORT(BasePort),PSXLIB1_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXLIB1_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }
 else if (UnitNumber==1)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXLIB2_BASE,PSXLIB2_SEL,0,PSXLIB2_CLK,0,PSXLIB2_CMD,0,
					STATUSPORT(BasePort),PSXLIB2_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXLIB2_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }
 else if (UnitNumber==2)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXLIB3_BASE,PSXLIB3_SEL,0,PSXLIB3_CLK,0,PSXLIB3_CMD,0,
					STATUSPORT(BasePort),PSXLIB3_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXLIB3_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }
 else if (UnitNumber==3)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXLIB4_BASE,PSXLIB4_SEL,0,PSXLIB4_CLK,0,PSXLIB4_CMD,0,
					STATUSPORT(BasePort),PSXLIB4_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXLIB4_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }
 else if (UnitNumber==4)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXLIB5_BASE,PSXLIB5_SEL,0,PSXLIB5_CLK,0,PSXLIB5_CMD,0,
					STATUSPORT(BasePort),PSXLIB5_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXLIB5_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }
 else if (UnitNumber==5)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXLIB6_BASE,PSXLIB6_SEL,0,PSXLIB6_CLK,0,PSXLIB6_CMD,0,
					STATUSPORT(BasePort),PSXLIB6_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXLIB6_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }
 else if (UnitNumber==6)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXLIB7_BASE,PSXLIB7_SEL,0,PSXLIB7_CLK,0,PSXLIB7_CMD,0,
					STATUSPORT(BasePort),PSXLIB7_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXLIB7_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }
 else if (UnitNumber==7)
 {
  PSX_Do_Packet_IO (CONTROLPORT(BasePort),PSXLIB8_BASE,PSXLIB8_SEL,0,0,PSXLIB8_CLK,0,PSXLIB8_CMD,
					STATUSPORT(BasePort),PSXLIB8_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXLIB8_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }

 PSX_Make_Report (DeviceExtension,ReadPacket);
}

void ReadPSXMegaTap (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		ReadPacket[32];
 UCHAR		WritePacket[32];
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;

 WritePacket[0]= 0x01;
 WritePacket[1]= 0x42;

 RtlZeroMemory(&ReadPacket,sizeof(ReadPacket));

 if (UnitNumber==0)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXLIB1_BASE,PSXLIB1_SEL,0,PSXLIB1_CLK,0,PSXLIB1_CMD,0,
					STATUSPORT(BasePort),PSXLIB1_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXLIB1_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }
 else if (UnitNumber==1)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXLIB3_BASE,PSXLIB3_SEL,0,PSXLIB3_CLK,0,PSXLIB3_CMD,0,
					STATUSPORT(BasePort),PSXLIB3_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXLIB3_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }
 else if (UnitNumber==2)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXLIB4_BASE,PSXLIB4_SEL,0,PSXLIB4_CLK,0,PSXLIB4_CMD,0,
					STATUSPORT(BasePort),PSXLIB4_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXLIB4_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }
 else if (UnitNumber==3)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXLIB5_BASE,PSXLIB5_SEL,0,PSXLIB5_CLK,0,PSXLIB5_CMD,0,
					STATUSPORT(BasePort),PSXLIB5_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXLIB5_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }
 else if (UnitNumber==4)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXLIB6_BASE,PSXLIB6_SEL,0,PSXLIB6_CLK,0,PSXLIB6_CMD,0,
					STATUSPORT(BasePort),PSXLIB6_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXLIB6_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }

 PSX_Make_Report (DeviceExtension,ReadPacket);
}

void ReadPSXDirectPad (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		ReadPacket[32];
 UCHAR		WritePacket[32];
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;

 WritePacket[0]= 0x01;
 WritePacket[1]= 0x42;

 RtlZeroMemory(&ReadPacket,sizeof(ReadPacket));

 if (UnitNumber==0)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXLIB1_BASE,PSXLIB1_SEL,0,PSXLIB1_CLK,0,PSXLIB1_CMD,0,
					STATUSPORT(BasePort),PSXLIB1_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXLIB1_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }
 else if (UnitNumber==1)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXLIB2_BASE,PSXLIB2_SEL,0,PSXLIB2_CLK,0,PSXLIB2_CMD,0,
					STATUSPORT(BasePort),PSXLIB2_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXLIB2_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }

 PSX_Make_Report (DeviceExtension,ReadPacket);
}

void ReadPSXLinux (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		ReadPacket[32];
 UCHAR		WritePacket[32];
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;

 WritePacket[0]= 0x01;
 WritePacket[1]= 0x42;

 RtlZeroMemory(&ReadPacket,sizeof(ReadPacket));

 if (UnitNumber==0)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXLINUX1_BASE,PSXLINUX1_SEL,0,PSXLINUX1_CLK,0,PSXLINUX1_CMD,0,
					STATUSPORT(BasePort),PSXLINUX1_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXLINUX1_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }
 else if (UnitNumber==1)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXLINUX2_BASE,PSXLINUX2_SEL,0,PSXLINUX2_CLK,0,PSXLINUX2_CMD,0,
					STATUSPORT(BasePort),PSXLINUX2_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXLINUX2_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }
 else if (UnitNumber==2)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXLINUX3_BASE,PSXLINUX3_SEL,0,PSXLINUX3_CLK,0,PSXLINUX3_CMD,0,
					STATUSPORT(BasePort),PSXLINUX3_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXLINUX3_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }
 else if (UnitNumber==3)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXLINUX4_BASE,PSXLINUX4_SEL,0,PSXLINUX4_CLK,0,PSXLINUX4_CMD,0,
					STATUSPORT(BasePort),PSXLINUX4_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXLINUX4_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }
 else if (UnitNumber==4)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXLINUX5_BASE,PSXLINUX5_SEL,0,PSXLINUX5_CLK,0,PSXLINUX5_CMD,0,
					STATUSPORT(BasePort),PSXLINUX5_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXLINUX5_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }

 PSX_Make_Report (DeviceExtension,ReadPacket);
}

void ReadPSXNTPadXP (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		ReadPacket[32];
 UCHAR		WritePacket[32];
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;

 WritePacket[0]= 0x01;
 WritePacket[1]= 0x42;

 RtlZeroMemory(&ReadPacket,sizeof(ReadPacket));

 if (UnitNumber==0)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXNTPAD1_BASE,PSXNTPAD1_SEL,0,PSXNTPAD1_CLK,0,PSXNTPAD1_CMD,0,
					STATUSPORT(BasePort),PSXNTPAD1_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXNTPAD1_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }
 else if (UnitNumber==1)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXNTPAD2_BASE,PSXNTPAD2_SEL,0,PSXNTPAD2_CLK,0,PSXNTPAD2_CMD,0,
					STATUSPORT(BasePort),PSXNTPAD2_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXNTPAD2_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }
 else if (UnitNumber==2)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXNTPAD3_BASE,PSXNTPAD3_SEL,0,PSXNTPAD3_CLK,0,PSXNTPAD3_CMD,0,
					STATUSPORT(BasePort),PSXNTPAD3_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXNTPAD3_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }
 else if (UnitNumber==3)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXNTPAD4_BASE,PSXNTPAD4_SEL,0,PSXNTPAD4_CLK,0,PSXNTPAD4_CMD,0,
					STATUSPORT(BasePort),PSXNTPAD4_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXNTPAD4_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }
 else if (UnitNumber==4)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXNTPAD5_BASE,PSXNTPAD5_SEL,0,PSXNTPAD5_CLK,0,PSXNTPAD5_CMD,0,
					STATUSPORT(BasePort),PSXNTPAD5_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXNTPAD5_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }
 else if (UnitNumber==5)
 {
  PSX_Do_Packet_IO (DATAPORT(BasePort),PSXNTPAD6_BASE,PSXNTPAD6_SEL,0,PSXNTPAD6_CLK,0,PSXNTPAD6_CMD,0,
					STATUSPORT(BasePort),PSXNTPAD6_DATA,STATUS_INVMASK,STATUSPORT(BasePort),PSXNTPAD6_ACK,STATUS_INVMASK,
					ReadPacket,sizeof(ReadPacket),WritePacket,2,(PTIMING_PSX)DeviceExtension->Timing);
 }

 PSX_Make_Report (DeviceExtension,ReadPacket);
}

void ReadNullFunc (IN PDEVICE_EXTENSION DeviceExtension)
{
 /* Nothing to do here - data updated elsewhere */
}

void ReadLinuxGamecon (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		Mask;
 char		Status;
 char		Status2;
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;

 UCHAR	Masks[5]= {GAMECONJOY0_DATA,GAMECONJOY1_DATA,GAMECONJOY2_DATA,GAMECONJOY3_DATA,GAMECONJOY4_DATA};
 if (UnitNumber>4)
  return;

 Mask= Masks[UnitNumber];

 WRITE_PORT_UCHAR (DATAPORT(BasePort),0xFF&~GAMECONJOY_UP);
 Status= READ_PORT_UCHAR (STATUSPORT(BasePort))^STATUS_INVMASK;

 WRITE_PORT_UCHAR (DATAPORT(BasePort),0xFF&~GAMECONJOY_DOWN);
 Status2= READ_PORT_UCHAR (STATUSPORT(BasePort))^STATUS_INVMASK;

 AXISUD (RawInput,Status&Mask,Status2&Mask)

 WRITE_PORT_UCHAR (DATAPORT(BasePort),0xFF&~GAMECONJOY_LEFT);
 Status= READ_PORT_UCHAR (STATUSPORT(BasePort))^STATUS_INVMASK;

 WRITE_PORT_UCHAR (DATAPORT(BasePort),0xFF&~GAMECONJOY_RIGHT);
 Status2= READ_PORT_UCHAR (STATUSPORT(BasePort))^STATUS_INVMASK;

 AXISLR (RawInput,Status&Mask,Status2&Mask)

 WRITE_PORT_UCHAR (DATAPORT(BasePort),0xFF&~GAMECONJOY_FIRE1);
 Status= READ_PORT_UCHAR (STATUSPORT(BasePort))^STATUS_INVMASK;
 BUTTON1(RawInput,Status&Mask)

 WRITE_PORT_UCHAR (DATAPORT(BasePort),0xFF&~GAMECONJOY_FIRE2);
 Status= READ_PORT_UCHAR (STATUSPORT(BasePort))^STATUS_INVMASK;
 BUTTON2(RawInput,Status&Mask)
}

void ReadLPTSwitch (IN PDEVICE_EXTENSION DeviceExtension)
{
 int		Count;
 UCHAR		*Buffer;
 UCHAR		Status;
 UCHAR		Data;

 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;
 ULONG		RowDelay=	DeviceExtension->Timing->lptswitch.RowDelay;

 if (UnitNumber>1)
  return;

 Buffer= RawInput->Digital;

 if (UnitNumber==0)
 {
  for (Count=0;Count<6;Count++)
  {
   Data= (UCHAR)~(1<<Count);
   WRITE_PORT_UCHAR (DATAPORT(BasePort),Data);
   KeStallExecutionProcessor (RowDelay);
   Status= READ_PORT_UCHAR (STATUSPORT(BasePort))^STATUS_INVMASK;
   PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("ReadLPTSwitch: Wrote 0x%02X to data port, read 0x%02X from the status port.",Data,Status) );

   *(Buffer++)= !(Status&PIN10_MASK);
   *(Buffer++)= !(Status&PIN11_MASK);
   *(Buffer++)= !(Status&PIN12_MASK);
   *(Buffer++)= !(Status&PIN13_MASK);
   *(Buffer++)= !(Status&PIN15_MASK);
  }
  WRITE_PORT_UCHAR (DATAPORT(BasePort),0xFF);
 }

 if (UnitNumber==1)
 {
  for (Count=6;Count<8;Count++)
  {
   Data= (UCHAR)~(1<<Count);
   WRITE_PORT_UCHAR (DATAPORT(BasePort),Data);
   KeStallExecutionProcessor (RowDelay);
   Status= READ_PORT_UCHAR (STATUSPORT(BasePort))^STATUS_INVMASK;
   PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("ReadLPTSwitch: Wrote 0x%02X to data port, read 0x%02X from the status port.",Data,Status) );

   *(Buffer++)= !(Status&PIN10_MASK);
   *(Buffer++)= !(Status&PIN11_MASK);
   *(Buffer++)= !(Status&PIN12_MASK);
   *(Buffer++)= !(Status&PIN13_MASK);
   *(Buffer++)= !(Status&PIN15_MASK);
  }
  WRITE_PORT_UCHAR (DATAPORT(BasePort),0xFF);

  for (Count=3;Count>=0;Count--)
  {
   Data= ((~(1<<Count))^CONTROL_INVMASK) & 0x0F;
   WRITE_PORT_UCHAR (CONTROLPORT(BasePort),Data);
   KeStallExecutionProcessor (RowDelay);
   Status= READ_PORT_UCHAR (STATUSPORT(BasePort))^STATUS_INVMASK;
   PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("ReadLPTSwitch: Wrote 0x%02X to contol port, read 0x%02X from the status port.",Data,Status) );

   *(Buffer++)= !(Status&PIN10_MASK);
   *(Buffer++)= !(Status&PIN11_MASK);
   *(Buffer++)= !(Status&PIN12_MASK);
   *(Buffer++)= !(Status&PIN13_MASK);
   *(Buffer++)= !(Status&PIN15_MASK);
  }
  WRITE_PORT_UCHAR (CONTROLPORT(BasePort),CONTROL_FORWARD);
 }
}

BOOLEAN ISR_FMSBuddyBox (PKINTERRUPT IntObj, IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR			Status;
 PRAWINPUT		RawInput=	&(DeviceExtension->RawInput);
 PUCHAR			BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR			UnitNumber=	DeviceExtension->Config.UnitNumber;

 LARGE_INTEGER	TickCount;
 LARGE_INTEGER	ElapsedTicks;
 ULONG			Value;

 LARGE_INTEGER	MinPulseTicks=  DeviceExtension->Param.PPMBuddyBox.MinPulseTicks;
 LARGE_INTEGER	MaxPulseTicks=  DeviceExtension->Param.PPMBuddyBox.MaxPulseTicks;
 LARGE_INTEGER	MinSyncTicks=	DeviceExtension->Param.PPMBuddyBox.MinSyncTicks;

 TickCount= KeQueryPerformanceCounter(NULL);
 ElapsedTicks.QuadPart= TickCount.QuadPart - DeviceExtension->Param.PPMBuddyBox.LastTickCount.QuadPart;
 DeviceExtension->Param.PPMBuddyBox.LastTickCount= TickCount;

 /* Read the status port. We need to do this to receive the next interrupt!!! */
 Status= READ_PORT_UCHAR (STATUSPORT(BasePort));

 /* Routine to provide audio feedback of interrupt */
 if (PPJoy_RuntimeDebug&RTDEBUG_FMS)
  ToggleSpeaker();

 /* Can perhaps do something to make NextIndex -1 or something to prevent   */
 /* any data from being stored until we find the first matching sync pulse? */
 
 /* Do we clear axis positions after N sync pulses with no data?            */

 if (ElapsedTicks.QuadPart>MinSyncTicks.QuadPart)
 {
  DeviceExtension->Param.PPMBuddyBox.NextIndex= 0;
  goto Exit;
 }

// PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("Past reset - tick count= %d",ElapsedTicks.LowPart) );

 if (DeviceExtension->Param.PPMBuddyBox.NextIndex>=MAX_ANALOG_RAW)
  goto Exit;

 if  (ElapsedTicks.QuadPart<MinPulseTicks.QuadPart)
  ElapsedTicks.QuadPart= MinPulseTicks.QuadPart;
 else if  (ElapsedTicks.QuadPart>MaxPulseTicks.QuadPart)
  ElapsedTicks.QuadPart= MaxPulseTicks.QuadPart;
 
 AXIS_UANALOG_64(RawInput,DeviceExtension->Param.PPMBuddyBox.NextIndex,ElapsedTicks.QuadPart,MinPulseTicks.QuadPart,MaxPulseTicks.QuadPart)
 //{ (RAWINPUT)->Analog[IDX]= ((DATA-MIN)*(PPJOY_AXIS_MAX-PPJOY_AXIS_MIN))/(MAX-MIN) + PPJOY_AXIS_MIN; }
 DeviceExtension->Param.PPMBuddyBox.NextIndex++;

Exit:
 return TRUE;
}

void ReadPowerPadLin (IN PDEVICE_EXTENSION DeviceExtension)
{
 char		Data[8];
 int		Count;
 int		Mask[2];

 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;
 ULONG		BitDelay=	DeviceExtension->Timing->snes.BitDelay;

 if (UnitNumber==0)
 {
  Mask[0]= PIN10_MASK;
  Mask[1]= PIN11_MASK;
 }
 else if (UnitNumber==1)
 {
  Mask[0]= PIN12_MASK;
  Mask[1]= PIN13_MASK;
 }
 else if (UnitNumber>2)
  return;

 /* Latch new samples into shift registers in Pad */
 WRITE_PORT_UCHAR (DATAPORT(BasePort),SNESPAD_POWER|SNESPAD_CLOCK|SNESPAD_LATCH);
 KeStallExecutionProcessor (DeviceExtension->Timing->snes.SetupDelay);
 WRITE_PORT_UCHAR (DATAPORT(BasePort),SNESPAD_POWER|SNESPAD_CLOCK);

 for (Count=0;Count<8;Count++)					/* Loop for each data sample to be read */
 {
  KeStallExecutionProcessor (BitDelay);
  WRITE_PORT_UCHAR (DATAPORT(BasePort),SNESPAD_POWER);
  Data[Count]= READ_PORT_UCHAR (STATUSPORT(BasePort)) ^ STATUS_INVMASK;
  KeStallExecutionProcessor (BitDelay);
  WRITE_PORT_UCHAR (DATAPORT(BasePort),SNESPAD_POWER|SNESPAD_CLOCK);
 }

//                                     0 0 1 1 0 0 0 1 0 0 0 1
//static unsigned char ppad_bytes[] = {1,0,1,0,2,4,7,3,3,5,6,2};
//static unsigned char ppad_mask[]=   {1,1,2,2,1,1,1,2,1,1,1,2};

 AXISUD(RawInput,Data[1]&Mask[0],Data[0]&Mask[0])
 AXISLR(RawInput,Data[1]&Mask[1],Data[0]&Mask[1])
 BUTTON1(RawInput,Data[2]&Mask[0])
 BUTTON2(RawInput,Data[4]&Mask[0])
 BUTTON3(RawInput,Data[7]&Mask[0])
 BUTTON4(RawInput,Data[3]&Mask[1])
 BUTTON5(RawInput,Data[3]&Mask[0])
 BUTTON6(RawInput,Data[5]&Mask[0])
 BUTTON7(RawInput,Data[6]&Mask[0])
 BUTTON8(RawInput,Data[2]&Mask[1])
}

void ReadGenesisDPP6 (IN PDEVICE_EXTENSION DeviceExtension)
{
 UCHAR		StatusLow;
 UCHAR		StatusHigh;
 UCHAR		StatusHighUp;
 UCHAR		StatusP3;
 UCHAR		StatusP3Up;
 PRAWINPUT	RawInput=	&(DeviceExtension->RawInput);
 PUCHAR		BasePort=	(PUCHAR)(ULONG_PTR) DeviceExtension->Config.PortAddress;
 UCHAR		UnitNumber=	DeviceExtension->Config.UnitNumber;
 UCHAR		JoySubType=	DeviceExtension->Config.JoySubType;
 ULONG		BitDelay=	DeviceExtension->Timing->genesis.BitDelay;
 ULONG		Bit6Delay=	DeviceExtension->Timing->genesis.Bit6Delay;

 GENESIS_SCAN_SKELETON(														\
	/*JOYSUBTYPE*/	JoySubType,												\
	/*SEL_PORT*/	DATAPORT(BasePort),										\
	/*SET_SEL_LOW*/	GENESISDPP_SEL_LOW,										\
	/*SET_SEL_HIGH*/GENESISDPP_SEL_HIGH,									\
	/*READ_LOW*/	WRITE_PORT_UCHAR (CONTROLPORT(BasePort),0x0e);/* LEFT */\
					KeStallExecutionProcessor (1);							\
					StatusLow= READ_PORT_UCHAR (STATUSPORT(BasePort));,		\
	/*READ_HIGH*/	StatusHigh= READ_PORT_UCHAR (STATUSPORT(BasePort));		\
					WRITE_PORT_UCHAR (CONTROLPORT(BasePort),0x0d);/*UP*/	\
					KeStallExecutionProcessor (1);							\
					StatusHighUp= READ_PORT_UCHAR (STATUSPORT(BasePort));,	\
	/*READ_P3*/		StatusP3Up= READ_PORT_UCHAR (STATUSPORT(BasePort));		\
					WRITE_PORT_UCHAR (CONTROLPORT(BasePort),0x0e);/* LEFT */\
					KeStallExecutionProcessor (1);							\
					StatusP3= READ_PORT_UCHAR (STATUSPORT(BasePort));,		\
	/*BIT_DELAY*/	BitDelay,												\
	/*BIT6_DELAY*/	Bit6Delay												)

	PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_BABBLE2, ("GenesisDPP6: Read parallel port 0x%p StatusLowL =0x%X, StatusHighL,U 0x%X,0x%X StatusP3L,U 0x%X,0x%X",BasePort,StatusLow,StatusHigh,StatusHighUp,StatusP3,StatusP3Up) );

 GENESIS_REPORT_SKELETON(																	\
	/*RAWINPUT*/	RawInput,																\
	/*JOYSUBTYPE*/	JoySubType,																\
	/*LEFT_LOW*/	DPP6GEN_LEFT(StatusLow),	/*RIGHT_LOW*/	DPP6GEN_RIGHT(StatusLow),	\
	/*FIRE1_LOW*/	DPP6GEN_FIRE1(StatusLow),	/*FIRE2_LOW*/	DPP6GEN_FIRE2(StatusLow),	\
	/*UP_HIGH*/		DPP6GEN_UP(StatusHighUp),	/*DOWN_HIGH*/	DPP6GEN_DOWN(StatusHigh),	\
	/*LEFT_HIGH*/	DPP6GEN_LEFT(StatusHigh),	/*RIGHT_HIGH*/	DPP6GEN_RIGHT(StatusHigh),	\
	/*FIRE1_HIGH*/	DPP6GEN_FIRE1(StatusHigh),	/*FIRE2_HIGH*/	DPP6GEN_FIRE2(StatusHigh),	\
	/*UP_P3*/		DPP6GEN_UP(StatusP3Up),		/*DOWN_P3*/		DPP6GEN_DOWN(StatusP3),		\
	/*LEFT_P3*/		DPP6GEN_LEFT(StatusP3),		/*RIGHT_P3*/	DPP6GEN_RIGHT(StatusP3)		)
}

/**************************************************************************/
/* Routine to read joystick data for this device object. Before read,     */
/* aquire spinlock to ensure exclusive access, after read release the     */
/* spinlock again.                                                        */
/**************************************************************************/
NTSTATUS PPJoy_UpdateJoystickData (IN PDEVICE_EXTENSION DeviceExtension)
{
// PUCHAR		Port;
 UCHAR		Data;
 KIRQL		OldIrql;

 PPJOY_DBGPRINT (FILE_JOYSTICK|PPJOY_FENTRY, ("PPJoy_UpdateJoystickData (DeviceExt=0x%p)",DeviceExtension) );

 /* Acquire the global spinlock. We are now at dispatch level.        */
 /* This is to ensure no one touches the port while we are reading it */
 /* We currently do this at a global level. An optimisation would be  */
 /* to lock on a per-port basis. But too much effort for now.         */
 KeAcquireSpinLock (&Globals.SpinLock,&OldIrql);

 if ((DeviceExtension->Config.JoyType<=PPJOY_MAX_JOYTYPE)&&(JoyDefs[DeviceExtension->Config.JoyType].ReadFunc))
 {
  JoyDefs[DeviceExtension->Config.JoyType].ReadFunc(DeviceExtension);
 }
 else
 {
  PPJOY_DBGPRINT (FILE_JOYSTICK | PPJOY_ERROR, ("No such joystick type: %d",DeviceExtension->Config.JoyType) );
 }

 /* After this we are back at the old Irql level */
 KeReleaseSpinLock (&Globals.SpinLock,OldIrql);

 PPJOY_DBGPRINT (FILE_IOCTL|PPJOY_FEXIT, ("PPJoy_UpdateJoystickData") );

 return STATUS_SUCCESS;
}
