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

#include <config.h>
#include <limits.h>
#include <stdlib.h>
#include "edit.h"

TextPoint   CurrentPos;
TextPoint   ScreenTop;
TextPoint   TextBegin;
TextPoint   TextEnd;
TextPoint   BlockBegin;
TextPoint   BlockEnd;

TextPoint   *TextPoint::base=NULL;

void  TextPoint::AddTextPoint()
{
   next=base;
   base=this;
}
void  TextPoint::DeleteTextPoint()
{
   for(TextPoint**scan=&base; *scan; scan=&((*scan)->next))
   {
      if(*scan==this)
      {
         *scan=(*scan)->next;
         return;
      }
   }
}

TextPoint::TextPoint(offs o)
{
   offset=o;
   flags=COLUNDEFINED|LINEUNDEFINED;
   line=col=0;
   AddTextPoint();
   FindLineCol();
}
TextPoint::TextPoint(offs o,num l,num c)
{
   offset=o;
   if(l==-1)
   {
      flags=COLUNDEFINED|LINEUNDEFINED;
      line=col=0;
   }
   else if(c==-1)
   {
      flags=COLUNDEFINED;
      line=l;
      col=0;
   }
   else
   {
      flags=0;
      line=l;
      col=c;
   }
   AddTextPoint();
   FindLineCol();
}

TextPoint::TextPoint()
{
   offset=line=col=flags=0;
   AddTextPoint();
}
TextPoint::TextPoint(num l,num c)
{
   offset=0;
   line=l;
   col=c;
   flags=0;
   FindOffset();
   AddTextPoint();
}
TextPoint::TextPoint(const TextPoint& tp)
{
   offset=tp.offset;
   line=tp.line;
   col=tp.col;
   flags=tp.flags;
   AddTextPoint();
}

TextPoint   TextPoint::operator=(const TextPoint& tp)
{
   offset=tp.offset;
   line=tp.line;
   col=tp.col;
   flags=tp.flags;
   return(*this);
}

/*int   TextPoint::SetDragMode(int mode)
{
   int   oldmode=(flags&DRAG)?1:0;
   flags&=~DRAG;
   if(mode)
      flags|=DRAG;
   return(oldmode);
}*/

void  TextPoint::FindOffset()
{
   if(line<0)
   {
      col=line=offset=0;
      flags&=~(LINEUNDEFINED|COLUNDEFINED);
      return;
   }
   if(col<0)
   {
      col=0;
   }

   TextPoint   *scan=base;
   TextPoint   *found=NULL;
   for( ; scan; scan=scan->next)
   {
      if(!(scan->flags&LINEUNDEFINED))
      {
         if(!found || abs(this->line-scan->line)<abs(found->line-scan->line)
         || (scan->line==this->line && !(scan->flags&COLUNDEFINED)
             && scan->offset<found->offset))
         {
            found=scan;
         }
      }
   }

   offs  o;
   num   l,c;

   if(found)
   {
      if(found->flags&COLUNDEFINED)
      {
         o=LineBegin(found->offset);
         c=0;
      }
      else
      {
         o=found->offset;
         c=found->col;
      }
      l=found->line;
   }
   else
      o=c=l=0;

   if(l>line)
   {
      while(l>line)
      {
         o=PrevLine(o);
         l--;
      }
      c=0;
   }
   else if(l<line)
   {
      if(EofAt(o))
      {
         offset=o;
         line=l;
         col=c;
         flags&=~(COLUNDEFINED|LINEUNDEFINED);
         return;
      }
      while(l<line)
      {
         o=LineEnd(o);
         if(EofAt(o))
         {
            offset=o;
            line=l;
            col=0;
            flags|=COLUNDEFINED;
            flags&=~LINEUNDEFINED;
            return;
         }
         o=NextLine(o);
         l++;
         if(EofAt(o))
         {
            offset=o;
            line=l;
            col=0;
            flags&=~(COLUNDEFINED|LINEUNDEFINED);
            return;
         }
      }
      c=0;
   }
   while(c>col)
   {
      if(CharAt(o-1)=='\t')
      {
         o=LineBegin(o);
         c=0;
         break;
      }
      c--;
      o--;
   }
   while(c<col)
   {
      if(EolAt(o))
         break;
      if(CharAt(o)=='\t')
         c=Tabulate(c);
      else
         c++;
      o++;
   }
   col=c;
   offset=o;
}

void  TextPoint::FindLineCol()
{
   if(offset<0)
   {
      col=line=offset=0;
      flags&=~(COLUNDEFINED|LINEUNDEFINED);
      return;
   }

   TextPoint   *found=NULL;
   num	 dist=INT_MAX;
   for(TextPoint *scan=base; scan; scan=scan->next)
   {
      if(!(scan->flags&LINEUNDEFINED))
      {
	 if(!found || abs(this->offset-scan->offset)<dist)
	 {
	    dist=abs(this->offset-scan->offset);
	    found=scan;
	 }
      }
   }

   offs  o;
   num   l,c;

   if(found)
   {
      if(found->flags&COLUNDEFINED)
      {
         o=LineBegin(found->offset);
         c=0;
      }
      else
      {
         o=found->offset;
         c=found->col;
      }
      l=found->line;
   }
   else
      o=c=l=0;

   while(o>offset)
   {
      if(BolAt(o) || CharAt(o-1)=='\t')
         break;
      c--;
      o--;
   }
   if(o>offset)
   {
      o=LineBegin(o);
      c=0;
      while(o>offset)
      {
         o=PrevLine(o);
         l--;
      }
   }
   while(o<offset && !EofAt(o))
   {
      if(BolAt(o+1))
      {
         l++;
         c=0;
      }
      else if(CharAt(o)=='\t')
         c=Tabulate(c);
      else
         c++;
      o++;
   }
   col=c;
   line=l;
   offset=o;
   flags&=~(COLUNDEFINED|LINEUNDEFINED);
}

TextPoint   TextPoint::operator+=(num shift)
{
   TextPoint   res(offset+shift);
   res.FindLineCol();
   *this=res;
   return(*this);
}

void  TextPoint::ResetTextPoints()
{
   for(TextPoint *scan=TextPoint::base; scan; scan=scan->next)
      scan->line=scan->col=scan->offset=scan->flags=0;
}
void  TextPoint::OrFlags(int mask)
{
   mask&=COLUNDEFINED|LINEUNDEFINED;
   for(TextPoint *scan=TextPoint::base; scan; scan=scan->next)
      scan->flags|=mask;
}