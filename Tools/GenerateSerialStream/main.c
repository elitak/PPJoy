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


#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <math.h>

BYTE	SwapBitOrder[256]=
	{0x00,0x80,0x40,0xC0,0x20,0xA0,0x60,0xE0,0x10,0x90,0x50,0xD0,0x30,0xB0,0x70,0xF0,
	 0x08,0x88,0x48,0xC8,0x28,0xA8,0x68,0xE8,0x18,0x98,0x58,0xD8,0x38,0xB8,0x78,0xF8,
	 0x04,0x84,0x44,0xC4,0x24,0xA4,0x64,0xE4,0x14,0x94,0x54,0xD4,0x34,0xB4,0x74,0xF4,
	 0x0C,0x8C,0x4C,0xCC,0x2C,0xAC,0x6C,0xEC,0x1C,0x9C,0x5C,0xDC,0x3C,0xBC,0x7C,0xFC,
	 0x02,0x82,0x42,0xC2,0x22,0xA2,0x62,0xE2,0x12,0x92,0x52,0xD2,0x32,0xB2,0x72,0xF2,
	 0x0A,0x8A,0x4A,0xCA,0x2A,0xAA,0x6A,0xEA,0x1A,0x9A,0x5A,0xDA,0x3A,0xBA,0x7A,0xFA,
	 0x06,0x86,0x46,0xC6,0x26,0xA6,0x66,0xE6,0x16,0x96,0x56,0xD6,0x36,0xB6,0x76,0xF6,
	 0x0E,0x8E,0x4E,0xCE,0x2E,0xAE,0x6E,0xEE,0x1E,0x9E,0x5E,0xDE,0x3E,0xBE,0x7E,0xFE,
	 0x01,0x81,0x41,0xC1,0x21,0xA1,0x61,0xE1,0x11,0x91,0x51,0xD1,0x31,0xB1,0x71,0xF1,
	 0x09,0x89,0x49,0xC9,0x29,0xA9,0x69,0xE9,0x19,0x99,0x59,0xD9,0x39,0xB9,0x79,0xF9,
	 0x05,0x85,0x45,0xC5,0x25,0xA5,0x65,0xE5,0x15,0x95,0x55,0xD5,0x35,0xB5,0x75,0xF5,
	 0x0D,0x8D,0x4D,0xCD,0x2D,0xAD,0x6D,0xED,0x1D,0x9D,0x5D,0xDD,0x3D,0xBD,0x7D,0xFD,
	 0x03,0x83,0x43,0xC3,0x23,0xA3,0x63,0xE3,0x13,0x93,0x53,0xD3,0x33,0xB3,0x73,0xF3,
	 0x0B,0x8B,0x4B,0xCB,0x2B,0xAB,0x6B,0xEB,0x1B,0x9B,0x5B,0xDB,0x3B,0xBB,0x7B,0xFB,
	 0x07,0x87,0x47,0xC7,0x27,0xA7,0x67,0xE7,0x17,0x97,0x57,0xD7,0x37,0xB7,0x77,0xF7,
	 0x0F,0x8F,0x4F,0xCF,0x2F,0xAF,0x6F,0xEF,0x1F,0x9F,0x5F,0xDF,0x3F,0xBF,0x7F,0xFF};

HANDLE OpenSerialPort (char *SerialPortName, int BaudRate, char DataBits, char Parity, char StopBits, char RTSState, char DTRState, char CTSFlow, char DSRFlow)
{
 DCB			SerialDCB;
 COMMTIMEOUTS	SerialTimeouts;
 HANDLE			hSerialPort;

 hSerialPort= CreateFile (SerialPortName,GENERIC_READ|GENERIC_WRITE,0,NULL,
//					OPEN_EXISTING,FILE_FLAG_OVERLAPPED,NULL);
					OPEN_EXISTING,0,NULL);
 if (hSerialPort==INVALID_HANDLE_VALUE)
 {
  printf ("Error %d opening port",GetLastError());
  return INVALID_HANDLE_VALUE;
 }

 if (!GetCommState(hSerialPort,&SerialDCB))
 {
  printf ("Error %d getting serial DCB",GetLastError());
  CloseHandle(hSerialPort);
  return INVALID_HANDLE_VALUE;
 }
 
 SerialDCB.BaudRate= BaudRate;
 SerialDCB.ByteSize= (BYTE) DataBits;
 SerialDCB.Parity= (BYTE) Parity;
 SerialDCB.StopBits= (BYTE) StopBits;

 SerialDCB.fBinary= TRUE;
 SerialDCB.fParity= TRUE; 

 SerialDCB.fOutxCtsFlow= CTSFlow;
 SerialDCB.fRtsControl= RTSState;
 SerialDCB.fOutxDsrFlow= DSRFlow;
 SerialDCB.fDtrControl= DTRState;
 SerialDCB.fDsrSensitivity= FALSE;

 SerialDCB.fOutX= FALSE;
 SerialDCB.fInX= FALSE;
 SerialDCB.fErrorChar= FALSE;
 SerialDCB.fNull= FALSE;
 SerialDCB.fAbortOnError= FALSE;

 if (!SetCommState(hSerialPort,&SerialDCB))
 {
  printf ("Error %d setting serial DCB",GetLastError());
  CloseHandle(hSerialPort);
  return INVALID_HANDLE_VALUE;
 }

 SerialTimeouts.ReadIntervalTimeout= MAXDWORD;
 SerialTimeouts.ReadTotalTimeoutMultiplier= MAXDWORD;
 SerialTimeouts.ReadTotalTimeoutConstant= 5000;			// 5 seconds
 SerialTimeouts.WriteTotalTimeoutMultiplier= 1000;		// 1 second
 SerialTimeouts.WriteTotalTimeoutConstant= 1000;		// 1 second

 if (!SetCommTimeouts(hSerialPort,&SerialTimeouts))
 {
  printf ("Error %d setting timeouts",GetLastError());
  CloseHandle(hSerialPort);
  return INVALID_HANDLE_VALUE;
 }
 
 if (!SetCommMask(hSerialPort,EV_RXCHAR))
 {
  printf ("Error %d setting event mask",GetLastError());
  CloseHandle(hSerialPort);
  hSerialPort= INVALID_HANDLE_VALUE;
  return 0;
 }

 printf ("Successfully opened port\n");
 return hSerialPort;
}


//#define	PORTNAME	"COM2"
#define	NUMSECONDS	600
#define	NUMSTEPS	60
#define	PI			3.1415926535


void main (int argc, char **argv)
{
 HANDLE	hSerial;
 BYTE	Buffer[32];
 DWORD	NumWritten;
 int	Count1;
 int	Count2;
 int	Choice;
 int	SeqNumber= 0;

 char	PortName[32];
 char	TempChar;

 double	xpos;
 double	ypos;
 double	zpos;
 double	rpos;
 
 if (argc<2)
 {
  printf ("Enter serial port name: ");
  scanf ("%s",PortName);
 }
 else
 {
  strcpy (PortName,argv[1]);
 }

 if (argc<3)
 {
  printf ("\n");
  printf ("Choose protocol to send:\n");
  printf ("========================\n\n");
  printf ("1. FMS9600\n");
  printf ("2. FMS9600 generic\n");
  printf ("3. FMS19200\n");
  printf ("4. Zhen Hua\n");
  printf ("5. AeroChopper\n");
  printf ("\n\nChoice:");

  scanf ("%d",&Choice);
  printf ("\n");
 }
 else
 {
  Choice= atoi (argv[2]);
 }

 if ((Choice<1)||(Choice>5))
 {
  printf ("Don't be silly!\n");
  return;
 }

 printf ("Trying to open %s\n",PortName);
 hSerial= INVALID_HANDLE_VALUE;
 if ((Choice==1)||(Choice==2))
  hSerial= OpenSerialPort (PortName,9600,8,NOPARITY,ONESTOPBIT,RTS_CONTROL_ENABLE,DTR_CONTROL_DISABLE,FALSE,FALSE);
 if ((Choice==3)||(Choice==4))
  hSerial= OpenSerialPort (PortName,19200,8,NOPARITY,ONESTOPBIT,RTS_CONTROL_ENABLE,DTR_CONTROL_DISABLE,FALSE,FALSE);
 if (Choice==5)
  hSerial= OpenSerialPort (PortName,4800,8,NOPARITY,ONESTOPBIT,RTS_CONTROL_ENABLE,DTR_CONTROL_DISABLE,FALSE,FALSE);

 if (hSerial==INVALID_HANDLE_VALUE)
 {
  printf ("Cannot open port, exiting\n");
  return;
 }

 printf ("Selected protocol: ");
 switch (Choice)
 {
  case 1:	printf ("FMS9600\n");			break;
  case 2:	printf ("FMS9600 generic\n");	break;
  case 3:	printf ("FMS19200\n");			break;
  case 4:	printf ("Zhen Hua\n");			break;
  case 5:	printf ("AeroChopper\n");		break;
  default:	printf ("UNKNOWN!\n");
 }

 printf ("Running for %d seconds, %d steps\n",NUMSECONDS,NUMSTEPS);

 for (Count1=0;Count1<NUMSECONDS;Count1++)
 {
  for (Count2=0;Count2<NUMSTEPS;Count2++)
  {
   xpos= (sin (((double)Count2)/NUMSTEPS*PI*2)+1)/2;
   ypos= (cos (((double)Count2)/NUMSTEPS*PI*2)+1)/2;
   zpos= ((double)Count2)/NUMSTEPS;
   rpos= 1-zpos;

//   printf ("%d: %.2f,%.2f,%.2f,%.2f\n",Count2,xpos,ypos,zpos,rpos);

   switch (Choice)
   {
    case 1:	// FMS9600
			Buffer[0]= 0xF0+6;
			Buffer[1]= Count2&0x0F;
			Buffer[2]= (BYTE)(xpos*100+100);
			Buffer[3]= (BYTE)(ypos*100+100);
			Buffer[4]= (BYTE)(zpos*100+100);
			Buffer[5]= (BYTE)(rpos*100+100);
			Buffer[6]= (BYTE)(zpos*100+100);
			Buffer[7]= (BYTE)(rpos*100+100);
			NumWritten= 8;
			break;

    case 2:	// FMS9600 alt
			Buffer[0]= 0xF0;
			Buffer[1]= Count2&0x0F;
			Buffer[2]= (BYTE)(xpos*0xEF);
			Buffer[3]= (BYTE)(ypos*0xEF);
			Buffer[4]= (BYTE)(zpos*0XEF);
			Buffer[5]= (BYTE)(rpos*0XEF);
			Buffer[6]= (BYTE)(zpos*0XEF);
			Buffer[7]= (BYTE)(rpos*0XEF);
			NumWritten= 8;
			break;

    case 3:	// FMS19200
			Buffer[0]= 0xFF;
			Buffer[1]= (BYTE)(xpos*0xFE);
			Buffer[2]= (BYTE)(ypos*0xFE);
			Buffer[3]= (BYTE)(zpos*0XFE);
			Buffer[4]= (BYTE)(rpos*0XFE);
			Buffer[5]= (BYTE)(zpos*0XFE);
			Buffer[6]= (BYTE)(rpos*0XFE);
			NumWritten= 7;
			break;

    case 4:	// Zhen Hua
			Buffer[0]= 0xEF;
			Buffer[1]= SwapBitOrder[(BYTE)(xpos*0xEF)];
			Buffer[2]= SwapBitOrder[(BYTE)(ypos*0xEF)];
			Buffer[3]= SwapBitOrder[(BYTE)(zpos*0XEF)];
			Buffer[4]= SwapBitOrder[(BYTE)(rpos*0XEF)];
			Buffer[5]= SwapBitOrder[(BYTE)(zpos*0XEF)];
			Buffer[6]= SwapBitOrder[(BYTE)(rpos*0XEF)];
			NumWritten= 7;
			break;

    case 5:	// AeroChopper
			Buffer[0]= 0x0F+((SeqNumber&0x01)<<4)+((SeqNumber&0x1C)<<3);
			Buffer[1]= (BYTE)(xpos*0xFE);
			Buffer[2]= (BYTE)(ypos*0xFE);
			Buffer[3]= (BYTE)(zpos*0XFE);
			Buffer[4]= (BYTE)(rpos*0XFE);
			NumWritten= 5;
			SeqNumber= (SeqNumber+1)%32;
			break;
   }
   if (!WriteFile (hSerial,Buffer,NumWritten,&NumWritten,NULL))
    printf ("Error %d writing to the serial port\n",GetLastError());
   
#if 1
   Sleep (1000/NUMSTEPS);
   if (_kbhit())
   {
    TempChar= _getch();
    WriteFile (hSerial,&TempChar,1,&NumWritten,NULL);
   }
#else
   printf ("Press any key...");
   getch();
   printf ("\n");
#endif
  }
 }

#if 0
 printf ("Press any key...");
 getch();
#endif

 CloseHandle (hSerial);
}


	 
 
