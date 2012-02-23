#include <stdio.h>
#include <conio.h>
#include <windows.h>

#include <winioctl.h>

#include "ppjioctl.h"
#include "ppjioctl_devname.h"

#define	NUM_ANALOG	8		/* Number of analog values which we will provide */
#define	NUM_DIGITAL	16		/* Number of digital values which we will provide */

#pragma pack(push,1)		/* All fields in structure must be byte aligned. */
typedef struct
{
 unsigned long	Signature;				/* Signature to identify packet to PPJoy IOCTL */
 char			NumAnalog;				/* Num of analog values we pass */
 long			Analog[NUM_ANALOG];		/* Analog values */
 char			NumDigital;				/* Num of digital values we pass */
 char			Digital[NUM_DIGITAL];	/* Digital values */
}	JOYSTICK_STATE;
#pragma pack(pop)

int main (int argc, char **argv)
{
 HANDLE				h;
 char				ch;
 JOYSTICK_STATE		JoyState;

 DWORD				RetSize;
 DWORD				rc;

 long				*Analog;
 char				*Digital;

 char				*DevName;

 DevName= PPJOY_IOCTL_DEVNAME;
 if (argc==2)
  DevName= argv[1];

 /* Open a handle to the control device for the first virtual joystick. */
 h= CreateFile(DevName,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);

 /* Make sure we could open the device! */
 if (h==INVALID_HANDLE_VALUE)
 {
  printf ("CreateFile failed with error code %d trying to open %s device\n",GetLastError(),DevName);
  return 1;
 }

 /* Initialise the IOCTL data structure */
 JoyState.Signature= JOYSTICK_STATE_V1;
 JoyState.NumAnalog= NUM_ANALOG;	/* Number of analog values */
 Analog= JoyState.Analog;			/* Keep a pointer to the analog array for easy updating */
 JoyState.NumDigital= NUM_DIGITAL;	/* Number of digital values */
 Digital= JoyState.Digital;			/* Digital array */

 printf ("PPJoy virtual joystick demonstration program -- controlling virtual joystick 1.\n\n");
 printf ("Press 1,2,3,4,5,6,7,8,9,0,z,x,c,v,b,n keys for buttons 1-16.\n");
 printf ("Press q,w,e,r,t,y,u,i keys for up (max) values for axis 1-16.\n");
 printf ("Press a,s,d,f,g,h,j,k keys for down (min) values for axis 1-16.\n");
 printf ("\nPress ESC to exit...");

 while ((ch=_getch())!=27)
 {
  /* On each iteration clear position buffer: Analog in centre, buttons not pressed */
  Analog[0]= Analog[1]= Analog[2]= Analog[3]= Analog[4]= Analog[5]= Analog[6]= Analog[7]= (PPJOY_AXIS_MIN+PPJOY_AXIS_MAX)/2;
  memset (Digital,0,sizeof(JoyState.Digital));
  
  /* Convert input to lower case */
  if ((ch>='A')&&(ch<='Z'))
   ch|=0x20;

  /* Very simple demo app! */
  switch (ch)
  {
   /* 1,2,3,4,5,6,7,8,9,0,z,x,c,v,b,n are buttons */
   case	'1':	Digital[0]= 1;	break;
   case	'2':	Digital[1]= 1;	break;
   case	'3':	Digital[2]= 1;	break;
   case	'4':	Digital[3]= 1;	break;
   case	'5':	Digital[4]= 1;	break;
   case	'6':	Digital[5]= 1;	break;
   case	'7':	Digital[6]= 1;	break;
   case	'8':	Digital[7]= 1;	break;
   case	'9':	Digital[8]= 1;	break;
   case	'0':	Digital[9]= 1;	break;
   case	'z':	Digital[10]= 1;	break;
   case	'x':	Digital[11]= 1;	break;
   case	'c':	Digital[12]= 1;	break;
   case	'v':	Digital[13]= 1;	break;
   case	'b':	Digital[14]= 1;	break;
   case	'n':	Digital[15]= 1;	break;

   /* q,w,e,r,t,y,u,i are "up" for each axis */
   case	'q':	Analog[0]= PPJOY_AXIS_MAX;	break;
   case	'w':	Analog[1]= PPJOY_AXIS_MAX;	break;
   case	'e':	Analog[2]= PPJOY_AXIS_MAX;	break;
   case	'r':	Analog[3]= PPJOY_AXIS_MAX;	break;
   case	't':	Analog[4]= PPJOY_AXIS_MAX;	break;
   case	'y':	Analog[5]= PPJOY_AXIS_MAX;	break;
   case	'u':	Analog[6]= PPJOY_AXIS_MAX;	break;
   case	'i':	Analog[7]= PPJOY_AXIS_MAX;	break;

   /* a,s,d,f,g,h,j,k are "down" for each axis */
   case	'a':	Analog[0]= PPJOY_AXIS_MIN;	break;
   case	's':	Analog[1]= PPJOY_AXIS_MIN;	break;
   case	'd':	Analog[2]= PPJOY_AXIS_MIN;	break;
   case	'f':	Analog[3]= PPJOY_AXIS_MIN;	break;
   case	'g':	Analog[4]= PPJOY_AXIS_MIN;	break;
   case	'h':	Analog[5]= PPJOY_AXIS_MIN;	break;
   case	'j':	Analog[6]= PPJOY_AXIS_MIN;	break;
   case	'k':	Analog[7]= PPJOY_AXIS_MIN;	break;
  }

  /* Send request to PPJoy for processing. */
  /* Currently there is no Return Code from PPJoy, this may be added at a */
  /* later stage. So we pass a 0 byte output buffer.                      */
  if (!DeviceIoControl(h,IOCTL_PPORTJOY_SET_STATE,&JoyState,sizeof(JoyState),NULL,0,&RetSize,NULL))
  {
   rc= GetLastError();
   if (rc==2)
   {
    printf ("Underlying joystick device deleted. Exiting read loop\n");
    break;
   }
   printf ("DeviceIoControl error %d\n",rc);
  }
 }

 CloseHandle(h);
 return 0;
}