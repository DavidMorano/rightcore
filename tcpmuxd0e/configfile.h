/* configfile */


/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */


#ifndef	CONFIGFILE_INCLUDE
#define	CONFIGFILE_INCLUDE


#include	<envstandards.h>	/* MUST be first to configure */

#include	<vecstr.h>


/* object defines */

#define	CONFIGFILE		struct configfile


struct configfile {
	unsigned long	magic ;		/* magic number */
	vecstr	defines ;	/* defined variables */
	vecstr	unsets ;	/* unset ENV variables */
	vecstr	exports ;	/* environment variables */
	char	*root ;			/* program root */
	char	*tmpdir ;		/* environment variable */
	char	*logfname ;		/* log file name */
	char	*workdir ;		/* working directory */
	char	*directory ;		/* directory */
	char	*user ;			/* default username */
	char	*group ;		/* default groupname */
	char	*pidfname ;		/* traditionally hold PID */
	char	*lockfname ;		/* lock file */
	char	*interrupt ;
	char	*polltime ;
	char	*filetime ;
	char	*port ;			/* port to listen on */
	char	*userpass ;		/* user password file */
	char	*machpass ;		/* machine password file */
	char	*srvtab ;		/* SRVTAB */
	char	*sendmail ;		/* SENDMAIL program path */
	char	*envfname ;		/* environment file */
	char	*pathfname ;		/* PATH file */
	char	*devicefname ;		/* daemon device file path */
	char	*seedfname ;		/* seed file path */
	char	*logsize ;		/* default target log length */
	char	*organization ;
	char	*timeout ;
	char	*removemul ;		/* remove multiplier */
	char	*acctab ;		/* access table file */
	char	*paramfname ;		/* parameter file */
	char	*nrecips ;		/* number of recips at a time */
	char	*helpfname ;
	char	*statfname ;		/* status file name */
	char	*passfname ;		/* pass-FD file name */
	char	*eigenfname ;
	char	*options ;
	char	*interval ;		/* poll interval */
	char	*stampdir ;		/* timestamp directory */
	char	*maxjobs ;		/* maximum jobs */
	int	badline ;	/* line number of bad thing */
	int	srs ;		/* secondary return status */
	int	loglen ;	/* log file length */
	int	minwordlen ;
	int	maxwordlen ;
	int	keys ;
} ;



#ifndef	CONFIGFILE_MASTER

#ifdef	__cplusplus
extern "C" {
#endif

extern int configfile_start(CONFIGFILE *,const char *) ;
extern int configfile_finish(CONFIGFILE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* CONFIGFILE_MASTER */

#endif /* CONFIGFILE_INCLUDE */


