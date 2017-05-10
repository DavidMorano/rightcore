/* last modified %G% version %I% */

/* Multiple Index Data Base (MIDB) */

/*
	David A.D. Morano
	July 1986
*/


#include	<sys/types.h>
#include	<fcntl.h>

#include	"localmisc.h"
#include	"midb.h"



/* external subroutines */

extern long	lseek() ;

extern int	open(), close(), read(), write() ;


/* external variables */

extern int	errno ;


/* forward reference */



/* defines for 'midb_open' */

#define		MIDB_READ	1
#define		MIDB_WRITE	2
#define		MIDB_APPEND	4
#define		MIDB_CREAT	8
#define		MIDB_TRUNC	16



int midb_open(dbp,name,os,mode)
bfile	*dbp ;
char	*name, *os ;
int	mode ;
{
	int	fd, 
	int	boflag = 0, oflag = 0 ;


	if (dbp == NULL)
	    return MIDB_FAULT ;

	if (*os == '\0') 
	    return MIDB_BADNAME ;

	oflag = 0 ;
	while (*os) {

	    switch (*os++) {

	    case 'r':
	        boflag |= MIDB_READ ;
	        break ;

	    case 'w':
	        boflag |= MIDB_WRITE ;
	        break ;

	    case 'm':
	    case '+':
	        boflag |= (MIDB_READ | MIDB_WRITE) ;
	        break ;

	    case 'a':
	        oflag |= (O_APPEND | O_CREAT) ;
	        break ;

	    case 'c':
	        oflag |= (O_CREAT) ;
	        break ;

	    case 'e':
		oflag |= O_EXCL ;
		break ;

	    case 't':
	        oflag |= (O_CREAT | O_TRUNC) ;
	        break ;

	    case 'n':
		oflag |= O_NDELAY ;
		break ;

/* POSIX "binary" mode */
	    case 'b':
		break ;

	    default:
	        break ;
	    }

	}

/* we don't want "exclusive" if we do not have "create" */

	if (oflag & O_EXCL) {

	    if (! (oflag & O_CREAT)) oflag &= (~ O_EXCL) ;

	}

	if ((boflag & MIDB_READ) && (boflag & MIDB_WRITE)) {

	    oflag |= O_RDWR ;

	} else if (boflag & MIDB_READ) {

	    oflag |= O_RDONLY ;

	} else if (boflag & MIDB_WRITE) oflag |= O_WRONLY ;


	if ((oflag & O_WRONLY) && (! (oflag & O_APPEND)))
	    oflag |= (O_CREAT | O_TRUNC) ;

	if (name < ((char *) 20)) {

	    fd = (int) name ;
	    oflag = fcntl(fd,F_GETFL) ;

	    if (oflag < 0) return (- errno) ;

	} else {

	    if ((fd = open(name,oflag,mode)) < 0) return (- errno) ;

	}

	dbp->bufsize = MIDB_BUFSIZE ;
	if (dbp->magic != MIDB_MAGIC) dbp->buf = (char *) malloc(MIDB_BUFSIZE) ;

	dbp->fd = fd ;
	dbp->oflag = oflag ;
	dbp->stat = 0 ;
	if ((dbp->offset = lseek(dbp->fd,0L,1)) < 0) {

		dbp->stat |= MIDBSM_NOTSEEK ;
		dbp->offset = 0 ;
	}

	dbp->len = 0 ;
	dbp->bp = dbp->buf ;

	if ((oflag & O_APPEND) && (! (dbp->stat & MIDBSM_NOTSEEK)))
	    dbp->offset = lseek(dbp->fd,0L,2) ;

	if (isatty(dbp->fd)) dbp->stat |= MIDBSM_LINEBUF ;

	dbp->magic = MIDB_MAGIC ;		/* set magic number */
	return OK ;
}
/* end subroutine (midb_open) */



