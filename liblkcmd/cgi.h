/* cgi */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	CGI_INCLUDE
#define	CGI_INCLUDE	1


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

#define	CGI_MAGIC	0x43628195
#define	CGI		struct cgi_head
#define	CGI_FL		struct cgi_flags


/* these are control commands */

/* flags */

#define	CGI_FBUFWHOLE		(1<<0)
#define	CGI_FBUFLINE		(1<<1)
#define	CGI_FBUFNONE		(1<<2)
#define	CGI_FBUFDEF		(1<<3)


struct cgi_flags {
	uint		dummy:1 ;
} ;

struct cgi_head {
	uint		magic ;
	CGI_FL		f ;
	SHIO		*ofp ;
	int		wlen ;
} ;


#if	(! defined(CGI_MASTER)) || (CGI_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	cgi_start(CGI *,SHIO *) ;
extern int	cgi_finish(CGI *) ;
extern int	cgi_eoh(CGI *) ;
extern int	cgi_hdrdate(CGI *,time_t) ;
extern int	cgi_hdr(CGI *,const char *,const char *,int) ;

extern int	cgi_write(CGI *,const void *,int) ;
extern int	cgi_printline(CGI *,const char *,int) ;
extern int	cgi_vprintf(CGI *,const char *,va_list) ;
extern int	cgi_printf(CGI *,const char *,...) ;
extern int	cgi_putc(CGI *,int) ;
extern int	cgi_seek(CGI *,offset_t,int) ;
extern int	cgi_flush(CGI *) ;
extern int	cgi_reserve(CGI *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* CGI_MASTER */

#endif /* CGI_INCLUDE */


