#pragma hdrstop
/*  $Id: stack.cpp,v 1.2 2002-09-05 13:19:20 hostovic Exp $

    Xbase project source code

    This file contains logic for handling basic stack functions.

    Copyright (C) 1997 Startech, Gary A. Kunkel   

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Contact:

      Mail:

        Technology Associates, Inc.
        XBase Project
        1455 Deming Way #11
        Sparks, NV 89434
        USA

      Email:

        xbase@techass.com

      See our website at:

        xdb.sourceforge.net


    V 1.0   10/10/97   - Initial release of software
    V 1.5   1/2/97     - Added memo field support
    V 1.6a  4/1/98     - Added expression support
    V 1.6b  4/8/98     - Added expression support
*/

#ifdef __GNUG__
  #pragma implementation "xstack.h"
#endif

#ifdef __WIN32__
#include <xbase/xbconfigw32.h>
#else
#include <xbase/xbconfig.h>
#endif

#include <xbase/xbase.h>

#include <string.h>
#include <stdlib.h>
#include <iostream.h>

#include <xbase/xstack.h>

/*! \file stack.cpp
*/

/*************************************************************************/
//! Short description.
/*!
*/
xbStack::xbStack( void )
{
   First      = NULL;
   Last       = NULL;
   Free       = NULL;
   StackDepth = 0;;
}
/*************************************************************************/
//! Short description.
/*!
*/
void xbStack::InitStack( void )
{
   if( !First || !Last ) return;
   if( Free )
   {
      Last->Next     = Free;
      Free->Previous = Last;
   }
   Free  = First;
   First = NULL;
   Last  = NULL;
   StackDepth = 0;
   return;
}
/*************************************************************************/
//! Short description.
/*!
*/
xbStackElement * xbStack::GetStackElement( void )
{
   xbStackElement * Temp;

   /* check the free chain for an empty Stack element, if not found,
      allocate one from memory */

   if( Free )
   {
      Temp = Free;
      Free = Free->Next;
   }
   else 

    if((( Temp = (xbStackElement *) malloc( sizeof( xbStackElement )))==NULL))
      return NULL;

   memset( Temp, 0x00, sizeof( xbStackElement ));
   return Temp;
}
/*************************************************************************/
//! Short description.
/*!
  \param e
*/
void xbStack::FreeStackElement( xbStackElement * e )
{
   e->Previous = NULL;
   e->Next = Free;
   if( Free ) Free->Previous = e;
   Free = e;
}
/*************************************************************************/
//! Short description.
/*!
  \param p
*/
xbShort xbStack::Push( void * p )
{
   xbStackElement * Temp;
   if(( Temp = GetStackElement()) == NULL )
      return 102;

   Temp->UserPtr = p;
   if( !First )
   {
      First = Temp;
      Last  = Temp;
      StackDepth = 1;
   }
   else
   {
      Last->Next = Temp;
      Temp->Previous = Last;
      Last = Temp;
      StackDepth++;
   }
   return 0;
}
/*************************************************************************/
//! Short description.
/*!
*/
void * xbStack::Pop( void )
{
   void * p;
   xbStackElement * Save;

   if( StackDepth == 0 ) 
      return NULL;
   else
   {
      p = Last->UserPtr;
      if( StackDepth == 1 )
      {
         FreeStackElement( First );
         First = NULL;
         Last  = NULL;
      }
      else  /* number of items in Stack must be > 1 */
      {
         Last->Previous->Next = NULL;
         Save = Last;
         Last = Last->Previous;
         FreeStackElement( Save );
      }
      StackDepth--;
      return p;
   }
}
/*************************************************************************/
//! Short description.
/*!
*/
#ifdef XBASE_DEBUG
void xbStack::DumpStack( void )
{
   xbStackElement * e;
   if( StackDepth == 0 )
   {
      cout << "\nStack is empty...";
      return;
   }

   cout << "\nThere are " << StackDepth << " entries.";
   cout << "\nFirst = " << First << "  Last = " << Last;

   e = First;
   while( e )
   {
      cout << "\n*****************************";
      cout << "\nThis      = " << e;
      cout << "\nNext      = " << e->Next;
      cout << "\nPrevious  = " << e->Previous;
      cout << "\nUser Pointer = " << e->UserPtr;
      e = e->Next;
   }
   cout << "\nFree list follows...";
   e = Free;
   while( e )
   {
      cout << "\n*****************************";
      cout << "\nThis      = " << e;
      cout << "\nNext      = " << e->Next;
      cout << "\nPrevious  = " << e->Previous;
      cout << "\nUser Pointer = " << e->UserPtr;
      e = e->Next;
   }
   return;
}
#endif
/*************************************************************************/
