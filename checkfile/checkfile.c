/* File Checksum Program */

/*
	David A.D. Morano
	October 1988
*/


#define	PRINT		0



/************************************************************************

	Arguments:

	- file name of file to be checked
	- optional file name of file to be used as database
	- optional mail address


************************************************************************/


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/path.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<pwd.h>

#include	<bfile.h>

#include	"stddef.h"



/* local defines */

#ifndef	MAXNAMELEN
#define	MAXNAMELEN	256
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	NROW		32
#define	NCOL		32
#define	HOLDSIZE	(NROW * NCOL * 4)



/* external subroutines */


/* local structures */

struct sumentry {
	char	name[MAXPATHLEN + 1] ;
	int	namelen ;
	long	sumrow, sumcol ;
	long	lasttime ;
} ;





int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct sumentry		entry ;

	struct ustat	ss, *sp = &ss ;

	struct passwd	*pwsp ;

	bfile	errfile, *efp = &errfile ;
	bfile	infile, *ifp = &infile ;
	bfile	sumfile, *sfp = &sumfile ;
	bfile	tempfile, *tfp = &tempfile ;

	offset_t	sumoff, emptyoff ;

	long	hold[NROW][NCOL] ;
	long	xoracc, sumrow, sumcol ;
	long	clock ;

	int	fildes[2] ;

	int	i, j, rs, len ;
	int	fill_len ;
	int	namelen ;
	int	empty, newfile ;			/* flag */

	int	uid ;

	char	*cp, namebuf[MAXPATHLEN + 1], name[MAXPATHLEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1], dbuf[100], buf[100] ;
	char	*mailaddress ;


	rs = bopen(efp,BERR,"w",0666) ;
	if (rs < 0) return (rs) ;


	uid = (int) getuid() ;

	pwsp = (struct passwd *) getpwuid(uid) ;

	mailaddress = pwsp->pw_name ;


	switch (argc) {

	case 1:
	    bprintf(efp,"no file given as argument\n") ;

	    bclose(efp) ;

	    return BAD ;

	case 2:
	    stat(argv[1],sp) ;

	    if (! (sp->st_mode & S_IFREG)) goto exit2 ;

	    rs = bopen(ifp,argv[1],"r",0666) ;
	    if (rs < 0) goto nofile ;

	    rs = bopen(sfp,"sumfile","rwc",0666) ;
	    if (rs < 0) goto nobase ;

	    break ;

	case 3:
	case 4:
	    stat(argv[1],sp) ;

	    if (! (sp->st_mode & S_IFREG)) goto exit2 ;

	    rs = bopen(ifp,argv[1],"r",0666) ;
	    if (rs < 0) goto nofile ;

	    rs = bopen(sfp,argv[2],"rw",0666) ;
	    if (rs < 0) goto nobase ;

	    if ((argc == 4) && (argv[3] != "")) mailaddress = argv[3] ;

	    break ;

	default:
	    bprintf(efp,"can't handle this yet\n") ;

	    bclose(efp) ;

	    return BAD ;

	} /* end switch */



/* calculate the full path name to this file */

	if (argv[1][0] != '/') {

	    pipe(fildes) ;

	    bflush(efp) ;

	    if (fork() == 0) {

	        close(1) ;		/* close standard output */

	        dup(fildes[1]) ;

	        execl("/bin/pwd","pwd",0) ;

	    }

	    bopen(tfp,fildes[0],"r") ;

	    namelen = breadline(tfp,namebuf,MAXPATHLEN) - 1 ;

	    bclose(tfp) ;

	    close(fildes[1]) ;


	    if ((namelen + strlen(argv[1])) >= (MAXPATHLEN - 1)) 
		goto toolong ;

	    namebuf[namelen++] = '/' ;

	} else 
		namelen = 0 ;


	for (i = 0 ; argv[1][i] != '\0' ; i += 1)
	    namebuf[namelen++] = argv[1][i] ;

	namebuf[namelen] = 0 ;


	name[0] = 0 ;

#if	PRINT
	bprintf(efp,"path of file %d : %s\n",namelen,namebuf) ;
#endif

	pathcondense(namebuf,name) ;

	namelen = strlen(name) ;

	movc(namelen,name,namebuf) ;

	namebuf[namelen] = 0 ;

#if	PRINT
	bprintf(efp,"after condense %d : %s\n",namelen,namebuf) ;
#endif

/* do we still have a regular file with this path ? */

	stat(namebuf,sp) ;

	if (! (sp->st_mode & S_IFREG)) 
		goto exit ;



/* find file in the sum file, if no entry - make one */

	empty = FALSE ;
	while ((len = bread(sfp,&entry,sizeof(struct sumentry))) > 0) {

	    if (namelen == 0 && (! empty)) {

	        empty = TRUE ;
		btell(sfp,&emptyoff) ;

		emptyoff -= sizeof(struct sumentry) ;

	    }

	    if (namelen != entry.namelen) 
		continue ;

	    if (cmpc(namelen,entry.name,namebuf) != 0) 
		continue ;

		btell(sfp,&sumoff) ;

		sumoff -= sizeof(struct sumentry) ;
	    newfile = FALSE ;
	    goto sumit ;

	} /* end while */

/* filename not found, establish an entry for this file */

	newfile = TRUE ;

	if (empty) 
		sumoff = emptyoff ;

	else 
		btell(sfp,&sumoff) ;

	movc(namelen,namebuf,entry.name) ;

	entry.namelen = namelen ;


/* do the summing */
sumit:
	sumrow = 0 ;
	sumcol = 0 ;
	while ((len = bread(ifp,hold,HOLDSIZE)) > 0) {

	    if (len < HOLDSIZE) {

	        fill_len = HOLDSIZE - len ;
	        cp = ((char *) hold) + len ;

	        for ( ; fill_len > 0 ; fill_len -= 1) 
			*cp++ = 0 ;

	    } /* end if */


/* accumulate the row sums */

	    for (i = 0 ; i < NROW ; i += 1) {

	        xoracc = 0 ;
	        for (j = 0 ; j < NCOL ; j += 1) {

	            xoracc ^= hold[i][j] ;

	        }

	        sumrow += xoracc ;

	    }


/* accumulate the column sums */

	    for (j = 0 ; j < NCOL ; j += 1) {

	        xoracc = 0 ;
	        for (i = 0 ; i < NROW ; i += 1) {

	            xoracc ^= hold[i][j] ;

	        }

	        sumcol += xoracc ;

	    }

	} /* end while */

/* end of check summing */

	u_time(&clock) ;


	if (! newfile) {

	    if ((entry.lasttime >= sp->st_mtime) &&
	        ((sumrow != entry.sumrow) || (sumcol != entry.sumcol))) {

	        entry.sumrow = sumrow ;
	        entry.sumcol = sumcol ;

	        u_ctime(&clock) ;

	        timebuf[19] = '\0' ;

/* start procedure to send mail */

	        bopen(tfp,"sum_mail_file","wc",0666) ;

	        bprintf(tfp,"FROM:       file-checker\n") ;

	        bprintf(tfp,"DATE:       %s\n",timebuf) ;

	        bprintf(tfp,"SUBJECT:    corrupted file\n\n") ;

	        bprintf(tfp,"Your file \"%s\"\n",namebuf) ;

	        bprintf(tfp,"has been corrupted.  This was detected\n") ;

	        bprintf(tfp,"at the date shown at the top of\n") ;

	        bprintf(tfp,"this letter.  The corruption occurred since\n") ;

	        bprintf(tfp,"the last check on this file which was\n") ;

	        timebuf = ctime(&entry.lasttime) ;
	        timebuf[19] = '\0' ;

	        bprintf(tfp,"conducted at %s.\n",timebuf) ;

	        bprintf(tfp,"\n") ;

	        bclose(tfp) ;

	        sprintf(buf,"/bin/mail %s < sumtempfile",mailaddress) ;

	        system(buf) ;

	        unlink("sumtempfile") ;

	    }

	} /* end if */

/* make final update to the entry and write it back to database */

	entry.sumrow = sumrow ;
	entry.sumcol = sumcol ;

	entry.lasttime = clock ;

	bseek(sfp,sumoff,0) ;

	bwrite(sfp,&entry,sizeof(struct sumentry)) ;


/* finished */
exit:
	bclose(sfp) ;

exit1:
	bclose(ifp) ;

exit2:
	bclose(efp) ;

	fflush(stdout) ;

	fflush(stderr) ;

	return OK ;

nofile:
	bprintf(efp,"file to be checked \"%s\" can not be opened\n",argv[1]) ;

	goto exit2 ;

nobase:
	bprintf(efp,"could not open the database file\n") ;

	goto exit1 ;

toolong:
	bprintf(efp,"file path name is too long\n") ;

	goto exit ;
}
/* end subroutine (main) */



/* LOCAL SUBROUTINES */



struct nsb {
	char	*sp ;
	int	lenr ;
	char	*fp ;
	int	flen ;
	int	term ;
} ;


int pathcondense(pi,po)
char	*pi, *po ;
{
	struct nsb	s ;

	int	ilen ;
	int	c ;

	char	*ip = pi, *op = po ;
	char	*fp ;


	ilen = strlen(pi) ;

	if (ilen == 0) 
		return BAD ;

	s.sp = pi ;
	s.lenr = ilen ;

	while (s.lenr > 0) {

	    nextname(&s) ;

	    if ((s.flen == 1) && (*s.fp == '.')) {

	        if ((op != po) && (op[-1] == '/') && (s.lenr == 0))
	            op -= 1 ;

	    } else if ((s.flen == 2) && (cmpc(2,s.fp,"..") == 0)) {

	        if (op != po) {

	            if (op[-1] == '/') 
			op -= 1 ;

	            op -= 1 ;
	            while (op != po && *op != '/') 
			op -= 1 ;

	            if (s.lenr > 0 && s.term) 
			*op++ = '/' ;

	        } else {

	            op = (char *) movc(s.flen,s.fp,op) ;

	            if (s.lenr > 0 && s.term) 
			*op++ = '/' ;

	        }

	    } else {

	        op = (char *) movc(s.flen,s.fp,op) ;

	        if (s.lenr > 0 && s.term) 
			*op++ = '/' ;

	    }

	}

	*op++ = '\0' ;

	return OK ;
}
/* end subroutine (pathcondense) */


int nextname(s)
struct nsb	*s ;
{
	int	flen, lenr = s->lenr ;

	char	*cp ;


	s->fp = s->sp ;
	cp = s->sp ;
	flen = 0 ;

top:
	if (lenr == 0) 
		goto end ;

	if (*cp == '/') 
		goto term ;

	cp += 1 ;
	flen += 1 ;
	lenr -= 1 ;
	goto	top ;

term:
	s->lenr = lenr - 1 ;
	s->sp = cp + 1 ;
	s->flen = flen ;
	s->term = TRUE ;
	return OK ;

end:
	s->sp = cp ;
	s->lenr = lenr ;
	s->flen = flen ;
	s->term = FALSE ;
	return OK ;
}
/* end subroutine (nextname) */



