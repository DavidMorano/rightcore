/* htm */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	HTM_INCLUDE
#define	HTM_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<stdarg.h>

#include	<shio.h>
#include	<localmisc.h>


/* miscellaneous */

#ifndef	STDINFNAME
#define	STDINFNAME	"*STDIN*"
#endif

#ifndef	STDOUTFNAME
#define	STDOUTFNAME	"*STDOUT*"
#endif

#ifndef	STDERRFNAME
#define	STDERRFNAME	"*STDERR*"
#endif

#ifndef	STDNULLFNAME
#define	STDNULLFNAME	"*STDNULL*"
#endif

#define	HTM_MAGIC	0x43628193
#define	HTM		struct htm_head
#define	HTM_FL		struct htm_flags


/* these are control commands */

/* flags */

#define	HTM_FBUFWHOLE		(1<<0)
#define	HTM_FBUFLINE		(1<<1)
#define	HTM_FBUFNONE		(1<<2)
#define	HTM_FBUFDEF		(1<<3)


struct htm_flags {
	uint		dummy:1 ;
} ;

struct htm_head {
	uint		magic ;
	HTM_FL		f ;
	SHIO		*ofp ;
	int		wlen ;
} ;


#if	(! defined(HTM_MASTER)) || (HTM_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	htm_start(HTM *,SHIO *,const char *) ;
extern int	htm_finish(HTM *) ;
extern int	htm_headbegin(HTM *,const char *) ;
extern int	htm_headend(HTM *) ;
extern int	htm_bodybegin(HTM *,const char *) ;
extern int	htm_bodyend(HTM *) ;
extern int	htm_tagbegin(HTM *,cchar *,cchar *,cchar *,cchar *(*)[2]) ;
extern int	htm_tagend(HTM *,const char *) ;
extern int	htm_abegin(HTM *,cchar *,cchar *,cchar *,cchar *) ;
extern int	htm_aend(HTM *) ;

extern int	htm_textbegin(HTM *,cchar *,cchar *,cchar *,
			int,int,cchar *(*)[2]) ;
extern int	htm_textend(HTM *) ;

extern int	htm_hr(HTM *,const char *,const char *) ;
extern int	htm_br(HTM *,const char *,const char *) ;
extern int	htm_img(HTM *,cchar *,cchar *,cchar *,cchar *,cchar *,int,int) ;

extern int	htm_write(HTM *,const void *,int) ;
extern int	htm_printline(HTM *,const char *,int) ;
extern int	htm_vprintf(HTM *,const char *,va_list) ;
extern int	htm_printf(HTM *,const char *,...) ;
extern int	htm_putc(HTM *,int) ;
extern int	htm_seek(HTM *,offset_t,int) ;
extern int	htm_flush(HTM *) ;
extern int	htm_reserve(HTM *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* HTM_MASTER */

#endif /* HTM_INCLUDE */


