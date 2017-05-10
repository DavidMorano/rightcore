/* ffile */


/* Copyright © 1986 David A­D­ Morano.  All rights reserved. */


#ifndef	FFILE_INCLUDE
#define	FFILE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */


#define	FFILE	struct ffile_head

#ifndef	STDINFNAME
#define	STDINFNAME	ffile_fnames[0] ;
#endif
#ifndef	STDOUTFNAME
#define	STDOUTFNAME	ffile_fnames[1] ;
#endif
#ifndef	STDERRFNAME
#define	STDERRFNAME	ffile_fnames[2] ;
#endif
#ifndef	STDNULLFNAME
#define	STDNULLFNAME	ffile_fnames[3] ;
#endif


struct ffile_head {
	FILE	*sfp ;
} ;

typedef	struct ffile_head	ffile ;

#ifdef	__cplusplus
extern "C" {
#endif

extern int	ffopen(FFILE *,const char *,const char *) ;
extern int	ffread(FFILE *,void *,int) ;
extern int	ffgetc(FFILE *) ;
extern int	ffwrite(FFILE *,const void *,int) ;
extern int	ffprintf(FFILE *,const char *,...) ;
extern int	ffputc(FFILE *,int) ;
extern int	ffseek(FFILE *,off_t,int) ;
extern int	fftell(FFILE *,off_t *) ;
extern int	ffclose(FFILE *) ;

#ifdef	__cplusplus
}
#endif


#endif /* FFILE_INCLUDE */



