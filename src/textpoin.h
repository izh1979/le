/* 
 * Copyright (c) 1993-1997 by Alexander V. Lukyanov (lav@yars.free.net)
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330, 
 * Boston, MA 02111-1307, USA. 
 */

#define  COLUNDEFINED      1
#define  LINEUNDEFINED     2

class TextPoint
{
   offs  offset;
   num   line,col;
   int   flags;

   TextPoint   *next;
   static
   TextPoint   *base;

   void  AddTextPoint();
   void  DeleteTextPoint();
   
   void  FindOffset();
   void  FindLineCol();

public:
   offs  Offset()
   {
      return(offset);
   }
   num   Line()
   {
      if(flags&LINEUNDEFINED)
         FindLineCol();
      return(line);
   }
   num   Col()
   {
      if(flags&(COLUNDEFINED|LINEUNDEFINED))
         FindLineCol();
      return(col);
   }

   TextPoint();
   TextPoint(offs);
   TextPoint(num,num);
   TextPoint(const TextPoint&);
   TextPoint(offs,num,num);
   
   ~TextPoint()
   {
      DeleteTextPoint();
   }
   
   TextPoint   operator=(const TextPoint& tp);
   TextPoint   operator+=(num shift);
   TextPoint   operator-=(num shift)
   {
      return(*this+=-shift);
   }
   operator offs()
   {
      return(offset);
   }
   
   static   void  ResetTextPoints();
   static   void  OrFlags(int mask);

   friend   int   InsertBlock(char *,num,char *,num);
   friend   int   DeleteBlock(num,num);
};

extern TextPoint  CurrentPos;
extern TextPoint  ScreenTop;
extern TextPoint  BlockBegin;
extern TextPoint  BlockEnd;
extern TextPoint  TextEnd;
extern TextPoint  TextBegin;

#define ScrPtr  ScreenTop.Offset()
#define ScrLine ScreenTop.Line()