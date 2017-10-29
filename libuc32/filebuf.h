/* filebuf */

/* support some buffered file operations */


/* revision history:

	= 2002-04-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This little object supports some buffered file operations for
        low-overhead buffered I-O requirements.


*******************************************************************************/


#ifndef	FILEBUF_INCLUDE
#define	FILEBUF_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<localmisc.h>


/* local defines */

#define	FILEBUF		struct filebuf_head
#define	FILEBUF_FL	struct filebuf_flags
#define	FILEBUF_ONET	O_NETWORK	/* specify network FD */


struct filebuf_flags {
	uint		net:1 ;		/* network FD */
	uint		write:1 ;	/* we are writing (othewise reading) */
} ;

struct filebuf_head {
	offset_t	off ;		/* virtual file pointer */
	FILEBUF_FL	f ;
	char		*buf ;		/* constant */
	char		*bp ;		/* goes up with use */
	int		bufsize ;	/* buffer size (constant) */
	int		len ;		/* length of valid-data in buffer */
	int		fd ;
} ;


#if	(! defined(FILEBUF_MASTER)) || (FILEBUF_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	filebuf_start(FILEBUF *,int,offset_t,int,int) ;
extern int	filebuf_read(FILEBUF *,void *,int,int) ;
extern int	filebuf_readp(FILEBUF *,void *,int,offset_t,int) ;
extern int	filebuf_readline(FILEBUF *,char *,int,int) ;
extern int	filebuf_readlines(FILEBUF *,char *,int,int,int *) ;
extern int	filebuf_write(FILEBUF *,const void *,int) ;
extern int	filebuf_writeto(FILEBUF *,const void *,int,int) ;
extern int	filebuf_print(FILEBUF *,const char *,int) ;
extern int	filebuf_printf(FILEBUF *,const char *,...) ;
extern int	filebuf_reserve(FILEBUF *,int) ;
extern int	filebuf_update(FILEBUF *,offset_t,const char *,int) ;
extern int	filebuf_invalidate(FILEBUF *) ;
extern int	filebuf_flush(FILEBUF *) ;
extern int	filebuf_adv(FILEBUF *,int) ;
extern int	filebuf_seek(FILEBUF *,offset_t,int) ;
extern int	filebuf_tell(FILEBUF *,offset_t *) ;
extern int	filebuf_poll(FILEBUF *,int) ;
extern int	filebuf_finish(FILEBUF *) ;

#ifdef	__cplusplus
}
#endif

#endif /* FILEBUF_MASTER */

#endif /* FILEBUF_INCLUDE */


