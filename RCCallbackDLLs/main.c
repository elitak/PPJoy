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


#include <stdio.h>

#include <windows.h>
#include <mmsystem.h>
#include <math.h>

#include "jr.h"
#include "futaba.h"

#define PW_FUTABA 6.623
#define PW_JR     7.340
#define PPM_MIN    30.0
#define PPM_MAX   100.0
#define PPM_TRIG    200

//#define FUTABA_PCM
//#define FUTABA_PPM
//#define JR_PCM
//#define JR_PPM

#define	NUM_CHANNELS	16

typedef int (_cdecl *AsyncDLL_Callback_Proto) (int NumAnalog, int *Analog, int AnalogMin, int AnalogMax, int NumDigital, char *Digital);

AsyncDLL_Callback_Proto	UpdatePPJoy;

//---------------------------------------------------------------------------
static HWAVEIN       waveIn;         // WAVE OUTハンドル
static WAVEFORMATEX  waveFmt;
static WAVEHDR      *waveBuf[2];      // WAVEHDR構造体へのポインタ
static volatile BOOL waveRecording;
static const int     waveBufSize = 1024;
static int Position[NUM_CHANNELS]={512,512,512,512,512,512,512,512,512,512,512,512,512,512,512,512};
//---------------------------------------------------------------------------

static int LastPos[NUM_CHANNELS]={512,512,512,512,512,512,512,512,512,512,512,512,512,512,512,512};

/**/ COORD	SaveCursorPosition; /**/

void UpdateJoystick (void)
{
 if (!memcmp(Position,LastPos,sizeof(Position)))
  return;

 memcpy (LastPos,Position,sizeof(Position));
 UpdatePPJoy (NUM_CHANNELS,LastPos,0,1023,0,NULL);

 return;
}

////////////////// Here's the beef //////////////////////////////////////////

#ifdef JR_PCM
static void __fastcall ProcessPulse(int width, BOOL input)
{
    static int sync = 0;

    static unsigned int bitcount = 0;
    static unsigned int bitstream = 0;
    static int data[30];
    static int datacount = 0;

    if (sync == 0 && (int)floor(2.0 * width / PW_JR + 0.5) == 5) {
        sync = 1;
        bitstream = 0;
        bitcount = -1;
        datacount = 0;
        return;
    }

    if (!sync) return;

    width = (int)floor((double)width / PW_JR + 0.5);
    bitstream = ((bitstream << 1) + 1) << (width - 1);
    bitcount += width;

    if (bitcount >= 8) {
        bitcount -= 8;
        if ((data[datacount++] = jr_symbol[(bitstream >> bitcount) & 0xFF]) < 0) {
            sync = 0;
            return;
        }
    }
	else
     return;

    switch (datacount) {
        case 3:  Position[2] = 1023 - ((data[1] << 5) | data[2]);	UpdateJoystick(); break;
        case 6:  Position[0] = 1023 - ((data[4] << 5) | data[5]);	UpdateJoystick(); break;
        case 11: Position[5] = 1023 - ((data[9] << 5) | data[10]);	UpdateJoystick(); break;
        case 14: Position[6] = 1023 - ((data[12] << 5) | data[13]);	UpdateJoystick(); break; /* Guess */
        case 18: Position[3] = 1023 - ((data[16] << 5) | data[17]);	UpdateJoystick(); break;
        case 21: Position[1] = 1023 - ((data[19] << 5) | data[20]);	UpdateJoystick(); break;
        case 26: Position[4] = 1023 - ((data[24] << 5) | data[25]);	UpdateJoystick(); break;
        case 28: Position[7] = 1023 - ((data[26] << 5) | data[27]);	UpdateJoystick(); break; /* Guess */
        case 30: sync = 0; break;
    }
}
#endif

//---------------------------------------------------------------------------

#ifdef FUTABA_PCM
static void __fastcall ProcessPulse(int width, BOOL input)
{
    static int sync = 0;

    static unsigned int bit = 0;
    static unsigned int bitcount = 0;
    static unsigned int bitstream = 0;

    static int data[32];
    static int datacount = 0;

    width = (int)floor(width / PW_FUTABA + 0.5);

    if (sync == 0 && width == 18) {
        sync = 1;
        bit = 0;
        bitstream = 0;
        bitcount = 0;
        datacount = 0;
        return;
    }

    if (!sync) return;

    bitstream = (bitstream << width) | (bit >> (32 - width));
    bit ^= 0xFFFFFFFF;
    bitcount += width;

    if (sync == 1) {
        if (bitcount >= 6) {
            bitcount -= 6;
            if (((bitstream >> bitcount) & 0x3F) == 0x03) {
                sync = 2;
                datacount = 0;
            } else if (((bitstream >> bitcount) & 0x3F) == 0x00) {
                sync = 2;
                datacount = 16;
                bitcount -= 2;
            } else {
                sync = 0;
            }
        }
        return;
    }

    if (bitcount >= 10) {
        bitcount -= 10;
        if ((data[datacount++] = futaba_symbol[(bitstream >> bitcount) & 0x3FF]) < 0) {
            sync = 0;
            return;
        }
    }
	else
     return;

    switch (datacount) {
        case 3:  if ((data[0] >> 4) != 0)  Position[2] = (data[1] << 4) | (data[2] >> 2);	UpdateJoystick(); break;
        case 7:                            Position[3] = (data[5] << 4) | (data[6] >> 2);	UpdateJoystick(); break;
        case 11: if ((data[0] >> 4) != 0)  Position[4] = (data[9] << 4) | (data[10] >> 2);	UpdateJoystick(); break;
        case 15:                           Position[6] = (data[13] << 4) | (data[14] >> 2);	UpdateJoystick(); sync = 0; break;
        case 19:                           Position[1] = (data[17] << 4) | (data[18] >> 2);	UpdateJoystick(); break;
        case 23: if ((data[16] >> 4) != 1) Position[0] = (data[21] << 4) | (data[22] >> 2);	UpdateJoystick(); break;
        case 27:                           Position[5] = (data[25] << 4) | (data[26] >> 2);	UpdateJoystick(); break;
        case 31:                           Position[7] = (data[29] << 4) | (data[30] >> 2);	UpdateJoystick(); break;
        case 32: sync = 0;

//			/**/{ int cnt;
//				  
//			      SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),SaveCursorPosition);
//
//			      for (cnt=1 ;cnt<15;cnt+=2) printf ("%4d ",(data[cnt] << 4) | (data[cnt+1] >> 2));
//			      for (cnt=17;cnt<31;cnt+=2) printf ("%4d ",(data[cnt] << 4) | (data[cnt+1] >> 2));
//				  printf ("\n");
//			} 
//			/**/

			 break;
    }

// 1.wav = Channel 1 Aileron servo
// 2.wav = Channel 2 Elevator servo
// 3.wav = Channel 3 Throttle servo
// 4.wav = Channel 4 Rudder servo
// 5.wav = Channel 5 Gyro sensitivity (2 pos. switch)		data[9]+data[10], values 40, 984 - looks right
// 6.wav = Channel 6 Pitch Servo							data[25]+data[26]
// 7.wav = Channel 7 Spare servo (3 pos. switch)			data[13]+data[14], values 40, 512 and 984 - looks right!
// 8.wav = Channel 8 Spare servo							data[29]+data[30]
// 9.wav = Channel 9 Spare servo (2 pos. switch)


}
//---------------------------------------------------------------------------
#endif

#ifdef FUTABA_PPM
static void __fastcall ProcessPulse(int width, BOOL input)
{
    static int sync = 0;

    int newdata;
    static int data[14];
    static int datacount = 0;

    if (sync == 0 && width > PPM_TRIG) {
        sync = 1;
        datacount = 0;
    }

    if (!sync) return;

    newdata = (int) ((width - PPM_MIN) / (PPM_MAX - PPM_MIN) * 1024);
    if (newdata < 0) newdata = 0;
    else if (newdata > 1023) newdata = 1023;

    if (data[datacount] - newdata > 100) data[datacount] -= 100;
    else if (newdata - data[datacount] > 100) data[datacount] += 100;
    else data[datacount] = (data[datacount] + newdata) / 2;
    datacount++;

    switch (datacount) {
        case 3:  Position[2] = data[2];		UpdateJoystick(); break;
        case 5:  Position[1] = data[4];		UpdateJoystick(); break;
        case 7:  Position[3] = data[6];		UpdateJoystick(); break;
        case 9:  Position[0] = data[8];		UpdateJoystick(); break;
        case 11: Position[4] = data[10];	UpdateJoystick(); break;
        case 13: Position[5] = data[12];	UpdateJoystick(); break;
        case 15: Position[6] = data[14];	UpdateJoystick(); break;
        case 17: Position[7] = data[16];	UpdateJoystick(); sync = 0; break;
    }
}
#endif

#ifdef JR_PPM
static void __fastcall ProcessPulse(int width, BOOL input)
{
    static int sync = 0;

    int newdata;
    static int data[14];
    static int datacount = 0;

    if (sync == 0 && width > PPM_TRIG) {
        sync = 1;
        datacount = 0;
    }

    if (!sync) return;

    newdata = (int) (1024 - (width - PPM_MIN) / (PPM_MAX - PPM_MIN) * 1024);
    if (newdata < 0) newdata = 0;
    else if (newdata > 1023) newdata = 1023;

    if (data[datacount] - newdata > 100) data[datacount] -= 100;
    else if (newdata - data[datacount] > 100) data[datacount] += 100;
    else data[datacount] = (data[datacount] + newdata) / 2;
    datacount++;

    switch (datacount) {
        case 3:  Position[3] = data[2];		UpdateJoystick(); break;
        case 5:  Position[2] = data[4];		UpdateJoystick(); break;
        case 7:  Position[1] = data[6];		UpdateJoystick(); break;
        case 9:  Position[0] = data[8];		UpdateJoystick(); break;
        case 11: Position[4] = data[10];	UpdateJoystick(); break;
        case 13: Position[5] = data[12];	UpdateJoystick(); break;
        case 15: Position[6] = data[14];	UpdateJoystick(); break;
        case 17: Position[7] = data[16];	UpdateJoystick(); sync = 0; break;
    }
}
#endif

static void __fastcall ProcessData(int i)
{
    static double min = 0;
    static double max = 0;
    static int high = 0;
    static int low = 0;
    double threshold;

//    max *= 0.9999;
//    min *= 0.9999;
    max -= 0.1;
    min += 0.1;
    if (max < min) max = min + 1;

    if (i> max) max = i;
    else if (i < min) min = i;
    threshold = (min + max) / 2;

    if (i > threshold) {
	high++;
        if (low) {
            ProcessPulse(low, FALSE);
            low = 0;
        }
    } else {
        low++;
        if (high) {
            ProcessPulse(high, TRUE);
            high = 0;
        }
    }
}
//---------------------------------------------------------------------------

static void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, void *lpUser, WAVEHDR *buf, DWORD Reserved)
{
    int i;
    if (uMsg == WIM_DATA) {
        int Size = waveFmt.nBlockAlign;
        int Length = buf->dwBytesRecorded / Size;
        if (Size == 1) {
            for (i = 0; i < Length; i++) {
                ProcessData((unsigned char)buf->lpData[i]);
            }
        } else if (Size == 2) {
            for (i = 0; i < Length; i++) {
                ProcessData(((signed short*)(buf->lpData))[i]);
            }
        }
        if (waveRecording) waveInAddBuffer(waveIn, waveBuf[buf->dwUser], sizeof(WAVEHDR));
    }
}

//---------------------------------------------------------------------------

void StartPropo(void)
{
    int i;
    waveRecording = TRUE;
    waveFmt.wFormatTag = WAVE_FORMAT_PCM;
    waveFmt.nChannels = 1;
    waveFmt.nSamplesPerSec = 44100;
    waveFmt.wBitsPerSample =16;
    waveFmt.nBlockAlign = waveFmt.wBitsPerSample / 8 * waveFmt.nChannels;
    waveFmt.nAvgBytesPerSec = waveFmt.nSamplesPerSec * waveFmt.nBlockAlign;
    waveFmt.cbSize = 0;
    waveInOpen (&waveIn, WAVE_MAPPER, &waveFmt, (DWORD)(waveInProc), 0, CALLBACK_FUNCTION);
    for (i = 0; i < 2; i++) {
        waveBuf[i] = (WAVEHDR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WAVEHDR));
        waveBuf[i]->lpData = (char*)HeapAlloc(GetProcessHeap(), 0, waveBufSize);
        waveBuf[i]->dwBufferLength = waveBufSize;
        waveBuf[i]->dwUser = i;
        waveInPrepareHeader (waveIn, waveBuf[i], sizeof(WAVEHDR));
        waveInAddBuffer (waveIn, waveBuf[i], sizeof(WAVEHDR));
    }
    waveInStart (waveIn);
}
//---------------------------------------------------------------------------

void StopPropo(void)
{
    int i;

    waveRecording = FALSE;
    waveInStop (waveIn);
    for (i = 0; i < 2 ;i++) {
        waveInUnprepareHeader (waveIn, waveBuf[i], sizeof(WAVEHDR));
        HeapFree(GetProcessHeap(), 0, waveBuf[i]->lpData);
        HeapFree(GetProcessHeap(), 0, waveBuf[i]);
    }
    waveInClose (waveIn);
}

////////////////// Here's the beef //////////////////////////////////////////

int __stdcall InitDLL (AsyncDLL_Callback_Proto CallbackFunc)
{
 if (!CallbackFunc)
  return 0;

 UpdatePPJoy= CallbackFunc;
 StartPropo();

 return 1;
}

int __stdcall Cleanup (void)
{
 StopPropo();
 return 1;
}

