/* pcsnso */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	PCSNSO_INCLUDE
#define	PCSNSO_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<ids.h>
#include	<pcsnsc.h>
#include	<localmisc.h>
#include	"pcsnsreq.h"


#define	PCSNSO_MAGIC	0x99889298
#define	PCSNSO		struct pcsnso_head
#define	PCSNSO_CUR	struct pcsnso_c
#define	PCSNSO_OBJ	struct pcsnso_obj
#define	PCSNSO_FL	struct pcsnso_flags
#define	PCSNSO_PWD	struct pcsnso_pwd
#define	PCSNSO_TO	7

/* query options */

#define	PCSNSO_ONOSERV	(1<<0)		/* do not call the server */
#define	PCSNSO_OPREFIX	(1<<1)		/* prefix match */


struct pcsnso_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct pcsnso_c {
	uint		*verses ;		/* file-offsets to tags */
	uint		nverses ;
	int		i ;
} ;

struct pcsnso_flags {
	uint		id:1 ;			/* text-index */
	uint		client:1 ;
} ;

struct pcsnso_pwd {
	struct passwd	pw ;
	char		*pwbuf ;
	int		pwlen ;
} ;

struct pcsnso_head {
	uint		magic ;
	cchar		*a ;			/* memory allocation */
	cchar		*pr ;			/* stored argument */
	PCSNSO_FL	f, open ;
	PCSNSO_PWD	pwd ;
	IDS		id ;
	PCSNSC		client ;
	time_t		ti_lastcheck ;
	time_t		ti_tind ;		/* text-index */
	int		ncursors ;
	int		opts ;
} ;


#if	(! defined(PCSNSO_MASTER)) || (PCSNSO_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int pcsnso_open(PCSNSO *,cchar *) ;
extern int pcsnso_setopts(PCSNSO *,int) ;
extern int pcsnso_get(PCSNSO *,char *,int,cchar *,int) ;
extern int pcsnso_curbegin(PCSNSO *,PCSNSO_CUR *) ;
extern int pcsnso_read(PCSNSO *,PCSNSO_CUR *,char *,int,int) ;
extern int pcsnso_curend(PCSNSO *,PCSNSO_CUR *) ;
extern int pcsnso_audit(PCSNSO *) ;
extern int pcsnso_close(PCSNSO *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PCSNSO_MASTER */

#endif /* PCSNSO_INCLUDE */


