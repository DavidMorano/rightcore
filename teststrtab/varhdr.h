/* varhdr */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	VARHDR_INCLUDE
#define	VARHDR_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


#define	VARHDR			struct varhdr
#define	VARHDR_FSUF		"vi"
#define	VARHDR_MAGICSTR		"VARIND"
#define	VARHDR_MAGICLEN		sizeof(VARHDR_MAGICSTR)
#define	VARHDR_MAGICSIZE	16
#define	VARHDR_VERSION		0


struct varhdr {
	uint		fsize ;
	uint		wtime ;
	uint		ksoff ;
	uint		kslen ;
	uint		vsoff ;
	uint		vslen ;
	uint		rtoff ;
	uint		rtlen ;
	uint		itoff ;
	uint		itlen ;
	uint		nvars ;
	uint		nskip ;
	char		vetu[4] ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int varhdr(VARHDR *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* VARHDR_INCLUDE */


