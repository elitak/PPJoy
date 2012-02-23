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
#include <stdlib.h>
#include <string.h>


void main (int argc, char** argv)
{
 FILE	*InFile= NULL;
 FILE	*OutFile= NULL;
 char	*CopyFlags= NULL;
 char	*SourceRow= NULL;
 int	RowLength;
 int	Count;
 int	ColNumber;

 if (argc<5)
 {
  printf ("Usage: %s <InFile> <OutFile> <RowLength> <CopyCol1> [<CopyCol2> [<CopyCol3> [...]]]\nColumn numbers start at 1\n",argv[0]);
  return;
 }

/****** Setup copy structures ******/

 RowLength= atoi(argv[3]);
 if (!RowLength)
 {
  printf ("Row length must be greater than Zero!!!\n");
  goto Exit;
 }

 CopyFlags= malloc (RowLength);
 SourceRow= malloc (RowLength);

 if ((!CopyFlags)||(!SourceRow))
 {
  printf ("Error allocating working memory\n");
  goto Exit;
 }

 memset (CopyFlags,0,RowLength);
 for (Count=4;Count<argc;Count++)
 {
  ColNumber= atoi(argv[Count]);
  printf ("Adding column number %d\n",ColNumber);
  if ((ColNumber<1)||(ColNumber>RowLength))
  {
   printf ("Invalid column number %d\n",ColNumber);
   goto Exit;
  }
  CopyFlags[ColNumber-1]= 1;
 }

/****** Open source and destination files ******/

 InFile= fopen (argv[1],"rb");
 if (!InFile)
 {
  printf ("Cannot open input file %s\n",argv[1]);
  goto Exit;
 }
 OutFile= fopen (argv[2],"wb");
 if (!OutFile)
 {
  printf ("Cannot open output file %s\n",argv[2]);
  goto Exit;
 }

/****** Perform column copy ******/

 while (fread(SourceRow,RowLength,1,InFile))
 {
  for (Count=0;Count<RowLength;Count++)
   if (CopyFlags[Count])
    if (!fwrite (SourceRow+Count,1,1,OutFile))
	{
     printf ("Error writing to output file\n");
	 goto Exit;
	}
 }
 
/****** Cleanup ******/

Exit:
 if (InFile) fclose (InFile);
 if (OutFile) fclose (OutFile);
 if (CopyFlags) free (CopyFlags);
 if (SourceRow) free (SourceRow);
}

