/* char */

/* fast character text and conversion facility */


/* revision history:

	= 1998-04-05, David A­D­ Morano
	This module was adapted from assembly lanauge.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This file (the header file) essentially supplies macros that provide
	the calling interface for this facility.  These macros provide super
	fast character test and conversion functions.  This level of speed is
	actually rarely needed since the normal corresponding UNIX®-supplied
	character subroutes are already very fast, but they are here for
	speacial needs that may arise.

	Only 8-bit characters are supported (ISO-Latin-1 character set).  For
	other character sets, use the system-supplied facilities.

	Extra-note: Note that non-breaking-white-space (NBSP) characters are
	*not* considered to be white-space!


******************************************************************************/


#ifndef	CHAR_INCLUDE
#define	CHAR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>


/* support tables; not accessed directly by callers  */

extern const unsigned char	char_iswhite[] ;
extern const unsigned char	char_islc[] ;
extern const unsigned char	char_isuc[] ;
extern const unsigned char	char_tolc[] ;
extern const unsigned char	char_touc[] ;
extern const unsigned char	char_tofc[] ;
extern const unsigned char	char_toval[] ;
extern const short		char_dictorder[] ;


/* test routines */

#define	CHAR_SPACETAB(c)	(((c) == ' ') || ((c) == '\t'))
#define	CHAR_ISWHITE(c)		(char_iswhite[(c) & 0xff])
#define	CHAR_ISLC(c)		(char_islc[(c) & 0xff])
#define	CHAR_ISUC(c)		(char_isuc[(c) & 0xff])


/* character manipulation routines */

/* our super-fast conversions */
#define	CHAR_TOLC(c)		(char_tolc[(c) & 0xff])
#define	CHAR_TOUC(c)		(char_touc[(c) & 0xff])
#define	CHAR_TOFC(c)		(char_tofc[(c) & 0xff])
#define	CHAR_TOVAL(c)		(char_toval[(c) & 0xff])


/* dictionary-collating-ordinal */
#define	CHAR_DICTORDER(c)	(char_dictorder[(c) & 0xff])


#endif /* CHAR_INCLUDE */


