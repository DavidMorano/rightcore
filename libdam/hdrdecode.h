/* hdrdecode */

/* mail-header value string decoder */


/* revision history:

	= 1998-02-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	HDRDECODE_INCLUDE
#define	HDRDECODE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<b64decoder.h>
#include	<qpdecoder.h>
#include	<chartrans.h>
#include	<localmisc.h>


#define	HDRDECODE_MAGIC		0x13f3c202
#define	HDRDECODE		struct hdrdecode_head
#define	HDRDECODE_FL		struct hdrdecode_flags
#define	HDRDECODE_CSLEN		32


struct hdrdecode_flags {
	uint		space:1 ;
	uint		ct:1 ;
} ;

struct hdrdecode_head {
	uint		magic ;
	HDRDECODE_FL	f ;
	B64DECODER	*b64decoder ;
	QPDECODER	*qpdecoder ;
	CHARTRANS	*chartrans ;
	cchar		*pr ;
	char		cs[HDRDECODE_CSLEN+1] ;
} ;


#if	(! defined(HDRDECODE_MASTER)) || (HDRDECODE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int hdrdecode_start(HDRDECODE *,cchar *) ;
extern int hdrdecode_proc(HDRDECODE *,wchar_t *,int,cchar *,int) ;
extern int hdrdecode_finish(HDRDECODE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* HDRDECODE_MASTER */

#endif /* HDRDECODE_INCLUDE */


