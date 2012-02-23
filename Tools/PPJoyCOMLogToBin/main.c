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

int IsHexChar (char ch)
{
 if ((ch>='0')&&(ch<='9'))
  return 1;
 ch|= 0x20;
 if ((ch>='a')&&(ch<='f'))
  return 1;
 return 0;
}

unsigned char HexValue (char ch)
{
 if ((ch>='0')&&(ch<='9'))
  return ch-'0';
 ch|= 0x20;
 if ((ch>='a')&&(ch<='f'))
  return ch-'a'+10;
 return 0;
}

void main (int argc, char** argv)
{
 FILE	*InFile;
 FILE	*OutFile;
 char	NewLine[256];
 char	*Next;
 unsigned char	HiNibble;
 unsigned char	LoNibble;

 if (argc!=3)
 {
  printf ("Usage: %s LogFile BinFile\n",argv[0]);
  return;
 }

 InFile= fopen (argv[1],"r");
 if (!InFile)
 {
  printf ("Cannot open input file %s\n",argv[1]);
  return;
 }
 OutFile= fopen (argv[2],"wb");
 if (!OutFile)
 {
  printf ("Cannot open output file %s\n",argv[2]);
  fclose (InFile);
  return;
 }

 while (fgets(NewLine,sizeof(NewLine),InFile))
 {
  if (strlen(NewLine)<9)
   continue;
  if (NewLine[8]!=':')
   continue;

  Next= NewLine+10;

  while (1)
  {
   if (!IsHexChar(*Next))
   {
    printf ("Invalid character %c in line %s\n",*Next,NewLine);
	goto Exit;
   }
  
   HiNibble= HexValue(*(Next++));

   if (!IsHexChar(*Next))
   {
    printf ("Invalid character %c in line %s\n",*Next,NewLine);
	goto Exit;
   }
  
   LoNibble= HexValue(*(Next++));
  
   LoNibble|= HiNibble<<4;
   fwrite (&LoNibble,1,1,OutFile);

   if (*Next==' ')
   {
	Next++;
	continue;
   }
   break;
  }
 }
 
Exit:
 fclose (InFile);
 fclose (OutFile);
}
