/*  $Id: xstack.h,v 1.1 2002-09-04 19:54:10 simpson Exp $

    Xbase project source code
  
    This file conatains a header file for the xbStack object which
    is used for handling expressions.

    Copyright (C) 1997  Startech, Gary A. Kunkel   

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
    V 1.6b  4/8/98     - Numeric index keys 
*/

/*! \file xstack.h
*/

#ifndef __XB_STACK_H__
#define __XB_STACK_H__

#ifdef __GNUG__
#pragma interface
#endif

#include <xbase/xtypes.h>

//! xbStackElement struct
/*!
*/

struct xbStackElement{
   xbStackElement * Previous;
   xbStackElement * Next;
   void * UserPtr;
};

//! xbStack class
/*!
*/

class XBDLLEXPORT xbStack{

 public:
   xbStack( void );
   void    InitStack( void );
   void *  Pop( void );
   xbShort Push( void * );
   //! Short description.
   /*!
   */
   xbShort GetStackDepth( void ) { return StackDepth; }
   void    DumpStack( void );

 protected:
   xbShort StackDepth;
   xbStackElement * First;
   xbStackElement * Last;
   xbStackElement * Free;          /* points to free stack items */
   xbStackElement * GetStackElement( void );
   void FreeStackElement( xbStackElement * );
};

#endif               // __XB_STACK_H__
