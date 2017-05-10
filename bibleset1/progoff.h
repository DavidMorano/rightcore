/* progoff */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */


#ifndef	PROGOFF_INCLUDE
#define	PROGOFF_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<bfile.h>
#include	<localmisc.h>

#include	"defs.h"


#ifdef	__cplusplus
extern "C" {
#endif

extern int	progoffbegin(struct proginfo *,bfile *) ;
extern int	progoffcomment(struct proginfo *,bfile *,const char *,int) ;
extern int	progoffdss(struct proginfo *,bfile *,
			const char *,const char *) ;
extern int	progoffdsn(struct proginfo *,bfile *,
			const char *,int) ;
extern int	progoffsrs(struct proginfo *,bfile *,
			const char *,const char *,const char *) ;
extern int	progoffsrn(struct proginfo *,bfile *,
			const char *,const char *,int) ;
extern int	progoffhf(struct proginfo *,bfile *,
			const char *,const char *,const char *,const char *) ;
extern int	progoffsetbasefont(struct proginfo *,bfile *) ;
extern int	progoffwrite(struct proginfo *,bfile *,const char *,int) ;
extern int	progofftcadd(struct proginfo *,bfile *,int,const char *) ;
extern int	progofftcmk(struct proginfo *,bfile *,int) ;
extern int	progoffend(struct proginfo *) ;

#ifdef	__cplusplus
}
#endif


#endif /* PROGOFF_INCLUDE */



