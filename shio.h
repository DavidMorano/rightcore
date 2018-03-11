/* shio */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	SHIO_INCLUDE
#define	SHIO_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdarg.h>

#include	<bfile.h>
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

#define	SHIO_MAGIC	0x43628192
#define	SHIO		struct shio_head
#define	SHIO_FL		struct shio_flags


/* these are control commands */

#define	SHIO_CNOP		0	/* no-operation */
#define	SHIO_CSETBUFWHOLE	1	/* use whole buffering */
#define	SHIO_CSETBUFLINE	2	/* perform line-buffer flushing */
#define	SHIO_CSETBUFNONE	3	/* do not buffer */
#define	SHIO_CSETBUFDEF		4	/* buffer as default */
#define	SHIO_CFD		5	/* get the underlying FD */
#define	SHIO_CSETFLAGS		6	/* set flags */
#define	SHIO_CGETFLAGS		7	/* get flags */
#define	SHIO_CNONBLOCK		8	/* nonblocking mode */
#define	SHIO_CSTAT		9	/* stat */

/* flags */

#define	SHIO_FBUFWHOLE		(1<<0)
#define	SHIO_FBUFLINE		(1<<1)
#define	SHIO_FBUFNONE		(1<<2)
#define	SHIO_FBUFDEF		(1<<3)


enum shiobms {
	shiobm_all,
	shiobm_whole,
	shiobm_line,
	shiobm_none,
	shiobm_overlast
} ;

struct shio_flags {
	uint		sfio:1 ;
	uint		stdfname:1 ;
	uint		bufwhole:1 ;
	uint		bufline:1 ;
	uint		bufnone:1 ;
	uint		bufdef:1 ;
	uint		terminal:1 ;
	uint		nullfile:1 ;
} ;

struct shio_head {
	uint		magic ;
	SHIO_FL		f ;
	void		*outstore ;	/* for OUTSORE if needed */
	void		*fp ;
	bfile		*bfp ;		/* memory-allocated */
	const char	*fname ;	/* memory allocated */
} ;


#if	(! defined(SHIO_MASTER)) || (SHIO_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	shio_opene(SHIO *,const char *,const char *,mode_t,int) ;
extern int	shio_opentmp(SHIO *,mode_t) ;
extern int	shio_open(SHIO *,const char *,const char *,mode_t) ;
extern int	shio_reade(SHIO *,void *,int,int,int) ;
extern int	shio_read(SHIO *,void *,int) ;
extern int	shio_readline(SHIO *,char *,int) ;
extern int	shio_readlines(SHIO *,char *,int,int *) ;
extern int	shio_readlinetimed(SHIO *,char *,int,int) ;
extern int	shio_write(SHIO *,const void *,int) ;
extern int	shio_println(SHIO *,cchar *,int) ;
extern int	shio_print(SHIO *,cchar *,int) ;
extern int	shio_printline(SHIO *,cchar *,int) ;
extern int	shio_vprintf(SHIO *,const char *,va_list) ;
extern int	shio_printf(SHIO *,const char *,...) ;
extern int	shio_putc(SHIO *,int) ;
extern int	shio_seek(SHIO *,offset_t,int) ;
extern int	shio_flush(SHIO *) ;
extern int	shio_control(SHIO *,int,...) ;
extern int	shio_getfd(SHIO *) ;
extern int	shio_getlines(SHIO *) ;
extern int	shio_reserve(SHIO *,int) ;
extern int	shio_isterm(SHIO *) ;
extern int	shio_isseekable(SHIO *) ;
extern int	shio_stat(SHIO *,struct ustat *) ;
extern int	shio_writefile(SHIO *,const char *) ;
extern int	shio_close(SHIO *) ;

extern int	shio_readintr(SHIO *,void *,int,int,volatile int **) ;

#ifdef	__cplusplus
}
#endif

#endif /* SHIO_MASTER */

#endif /* SHIO_INCLUDE */


