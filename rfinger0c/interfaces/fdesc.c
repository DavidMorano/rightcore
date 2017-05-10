/* fdesc */

/* interface */


#define	F_DEBUG		0
#define	F_DEBUGFIELD	0


/* revision history :

	- 96/02/01, Dave Morano

	This subroutine was adopted for use from the DWD program.


	- 03/11/04, Dave Morano

	I don't know where all this has been (apparently "around") but
	I grabbed it from the FDESC object !



*/


/******************************************************************************

	This is an interface module.



******************************************************************************/


#define	FDESC_MASTER	0


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>

#include	"misc.h"
#include	"fdesc.h"
#include	"dialer.h"



/* local defines */

#define	FDESC_MODNAME	"fdesc"

#define	FDESC_MAGIC	31415926

#define	TI_FILECHECK	3
#define	LINELEN		((4 * 1024) + 1)
#define	ARGBUFLEN	(MAXPATHLEN + 35)



/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	starmat(const char *,const char *) ;
extern int	getpwd(char *,int) ;


/* external variables */


/* forward references */


/* global variables (module information) */

struct sinterface	fdesc = {
	FDESC_MODNAME,
	sizeof(FDESC),
} ;


/* local variables */

static const unsigned char 	fterms[32] = {
	    0x00, 0x00, 0x00, 0x00,
	    0x08, 0x10, 0x00, 0x24,
	    0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00,
	    0x00, 0x00, 0x00, 0x00,
} ;





int fdesc_init(op,ap)
FDESC		*op ;
FDESC_ARGS	*ap ;
{
	int	rs = SR_OK ;


	if (op == NULL)
	    return SR_FAULT ;

	memset(op,0,sizeof(FDESC)) ;

	if (ap != NULL) {

		op->fd = ap->fd ;
		op->pid = ap->pid ;
		op->flags = ap->flags ;

	}

	return rs ;
}


int fdesc_read(op,buf,buflen,to)
FDESC		*op ;
char		buf[] ;
int		buflen ;
int		to ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != FDESC_MAGIC)
	    return SR_NOTOPEN ;

	rs = u_reade(op->fd,buf,buflen,to,0) ;

	return rs ;
}


int fdesc_write(op,buf,buflen)
FDESC		*op ;
const char	buf[] ;
int		buflen ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != FDESC_MAGIC)
	    return SR_NOTOPEN ;

	rs = uc_writen(op->fd,buf,buflen) ;

	return rs ;
}


int fdesc_shutdown(op,cmd)
FDESC		*op ;
int		cmd ;
{
	int	rs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != FDESC_MAGIC)
	    return SR_NOTOPEN ;

	rs = u_shutdown(op->fd,cmd) ;

	return rs ;
}


#ifdef	COMMENT

/* check if any files have changed */
int fdesc_check(op,daytime)
FDESC		*op ;
time_t		daytime ;
{
	struct fdesc_file	*fep ;

	struct stat	sb ;

	int	rs, i, c = 0 ;
	int	f ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != FDESC_MAGIC)
	    return SR_NOTOPEN ;

	if (daytime <= 0)
	    daytime = time(NULL) ;

/* should we even check ? */

	if ((daytime - op->checktime) <= TI_FILECHECK)
	    return SR_OK ;

	op->checktime = daytime ;

/* loop through files looking for a change */

	for (i = 0 ; vecobj_get(&op->files,i,&fep) >= 0 ; i += 1) {

	    if (fep == NULL) continue ;

	    rs = u_stat(fep->fname,&sb) ;

	    f = ((rs < 0) || (sb.st_mtime > fep->mtime) || 
	        (sb.st_size != fep->len)) ;

	    if (f) {

	        c += 1 ;
	        fdesc_delfes(op,i) ;

	        if (rs >= 0)
	            fdesc_fileparse(op,i,fep) ;

	    } /* end if (file changed) */

	} /* end for */

#if	F_DEBUGS
	eprintf("fdesc_check: ret rs=%d\n",rs) ;
#endif

	return c ;
}
/* end subroutine (fdesc_check) */

#endif /* COMMENT */


/* close the connection */
int fdesc_close(op)
FDESC		*op ;
{
	int	rs ;
	int	cs ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != FDESC_MAGIC)
	    return SR_NOTOPEN ;

	rs = u_close(op->fd) ;

	if (op->pid > 0) {

		u_waitpid(op->pid,&cs,0) ;

	} /* end if (waiting for program) */

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (fdesc_close) */



/* INTERNAL SUBROUTINES */



