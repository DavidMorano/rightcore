/* eprintf */


#define	F_DEBUGS	0
#define	F_CLOSE		0		/* do not turn this on */



#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<string.h>
#include	<varargs.h>
#include	<errno.h>

#include	<vsystem.h>

#include	"misc.h"




/* local defines */

#define	FD_STDERR	3
#define	FD_BADERR	4
#define	BUFLEN		512
#define	O_FLAGS		(O_WRONLY | O_CREAT | O_APPEND)
#define	EWRITE_TRIES	10



/* external subroutines */

extern int	format() ;


/* external variables */


/* forward references */

static int	ewrite(int,char *,int) ;


/* local static data */

static int	err_fd = FD_STDERR ;




int eprintf(va_alist)
va_dcl
{
	va_list	ap ;

	int	len, rs ;

	char	buf[BUFLEN + 1] ;
	char	*fmt ;


	if (err_fd < 0)
	    return SR_NOTOPEN ;

	va_begin(ap) ;

	fmt = (char *) va_arg(ap,char *) ;

	len = format(buf,BUFLEN,NULL,NULL,fmt,ap) ;

	va_end(ap) ;

	rs = ewrite(err_fd,buf,len) ;

	if (rs == SR_BADF)
	    err_fd = -1 ;

	return rs ;
}
/* end subroutine (eprintf) */


int eprint(s)
char	*s ;
{
	int	rs ;


#if	F_DEBUGS
	{

	    char	*errstr = "eprint: entered\n" ;

	    ewrite(FD_BADERR,errstr,strlen(errstr)) ;

	}
#endif /* F_DEBUGS */

	if (err_fd < 0)
	    return SR_NOTOPEN ;

	rs = ewrite(err_fd,s,strlen(s)) ;

	if (rs == SR_BADF)
	    err_fd = -1 ;

	return rs ;
}
/* end subroutine (eprint) */


int esetfd(fd)
int	fd ;
{
	struct ustat	sb ;


	err_fd = -1 ;
	if (fd >= 0) {

	    if ((fd < 256) && (u_fstat(fd,&sb) >= 0))
	        err_fd = fd ;

	}

	if (err_fd >= 0)
	    u_fchmod(err_fd,0666) ;

	return err_fd ;
}
/* end subroutine (esetfd) */


int eopen(filename)
char	filename[] ;
{
	struct ustat	sb ;


#if	F_CLOSE
	if (err_fd >= 0)
	    u_close(err_fd) ;
#endif

	if ((err_fd = u_open(filename,O_FLAGS,0666)) >= 0)
	    u_fchmod(err_fd,0666) ;

	return err_fd ;
}
/* end subroutine (eopen) */


int eclose()
{


#if	F_CLOSE
	if (err_fd >= 0)
	    u_close(err_fd) ;
#endif

	err_fd = -1 ;
	return 0 ;
}
/* end subroutine (eclose) */


int egetfd()
{
	int	rs = err_fd ;

#if	F_DEBUGS
	char	*errstr ;
#endif /* F_DEBUGS */


#if	F_DEBUGS
	errstr = "egetfd: entering\n" ;
	ewrite(FD_BADERR,errstr,strlen(errstr)) ;
#endif /* F_DEBUGS */

#ifdef	COMMENT
	if ((err_fd >= 0) && (fcntl(err_fd,F_GETFL,0) != 0))
	    err_fd = -1 ;
#endif /* COMMENT */

#if	F_DEBUGS
	errstr = "egetfd: exiting\n" ;
	ewrite(FD_BADERR,errstr,strlen(errstr)) ;
#endif /* F_DEBUGS */

	return err_fd ;
}
/* end subroutine (egetfd) */



/* INTERNAL SUBROUTINES */



/* error (low level) write function */
static int ewrite(fd,buf,len)
int	fd, len ;
char	buf[] ;
{
	int	tries = 0 ;
	int	rs ;


again:
	if ((rs = write(err_fd,buf,len)) < 0) rs = (- errno) ;

	tries += 1 ;
	if (tries >= EWRITE_TRIES)
	    return rs ;

	if (rs == SR_INTR) goto again ;

	return rs ;
}
/* end subroutine (ewrite) */



