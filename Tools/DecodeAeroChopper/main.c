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
#include <string.h>

void main (int argc, char **argv)
{
 FILE			*File;
 unsigned char	Cell[5];
 int			CellCount;
 char			AuxString[33];

 if (argc!=2)
 {
  printf ("Usage: %s <AerochopperCaptureBin>\n",argv[0]);
  return;
 }

 File=fopen (argv[1],"rb");
 if (!File)
 {
  printf ("Unable to open file %s\n",argv[1]);
  return;
 }

 CellCount= 0;
 memset (AuxString,'-',32);
 AuxString[32]= 0;

 while (fread (Cell,sizeof(Cell),1,File))
 {
  // Check the cell header
  if (!(Cell[0]&0x0F))	// Check cell header ends on 1111 binary
  {
   printf ("Invalid cell header %02X - value not xF\n",Cell[0]);
   break;
  }
  if (!(CellCount&0x03)) // Check fixed supercell markers
  {
   if ( (Cell[0]>>4)!=(CellCount>>1) )
   {
    printf ("Invalid cell header %02X - supercell counter mismatch\n",Cell[0]);
	break;
   }
  }
  if ( ((Cell[0]>>4)&0x01) != (CellCount&0x01) )
  {
   printf ("Invalid cell header %02X - even/odd sequence error\n",Cell[0]);
   break;
  }

/*
  // Now display the meat:
  if (CellCount&0x03)
   printf ("%02d: %02X %02X %02X %02X %*d\n",CellCount+1,Cell[1],Cell[2],Cell[3],Cell[4],CellCount+1,Cell[0]>>5);
  else
   printf ("%02d: %02X %02X %02X %02X %*c\n",CellCount+1,Cell[1],Cell[2],Cell[3],Cell[4],CellCount+1,'-');
*/

  if (CellCount&0x03)
   AuxString[CellCount]= (Cell[0]>>5)+'0';
  printf ("%02d: %02X %02X %02X %02X %s\n",CellCount+1,Cell[1],Cell[2],Cell[3],Cell[4],AuxString);

  CellCount= (CellCount+1)%32;
 }

 if (!feof(File))
  printf ("Error reading file!\n");

 fclose (File);
}