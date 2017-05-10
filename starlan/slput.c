/* last modified %G% version %I% */

/* program to copy files to the STARLAN server */

/*
	David A.D. Morano
	November 1990
*/


#define		TITLE		"scp"
#define		VERSION		"0"


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>


#define		STARDIR		"mtsvb!/sv10/slan/rje"
#define		NPARG		20
#define		LINESIZE	4000


extern int	bopen(), bclose() ;
extern int	bread(), bwrite() ;
extern int	breadline(), bprintf() ;


int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile	in, *ifp = &in ;
	bfile	out, *ofp = &out ;
	bfile	err, *efp = &err ;

	struct ustat	ss ;

	long	argl, optl ;

	int	pan, i, n, f_dash = FALSE ;
	int	rs, len ;
	int	f_binary = FALSE, f_chmod ;
	int	oldmode ;
	int	rs_child, pfda[2] ;
	int	efd ;

	char	*argp, *optp ;
	char	*filename[NPARG] ;
	char	tmpname[40] ;
	char	dst[100] ;
	char	buf[LINESIZE] ;


	efd = dup(BERR) ;

	rs = bopen(&err,efd,"w",0666) ;

	if (rs < 0) return (rs) ;


	pan = 0 ;			/* number of positional so far */
	i = 1 ;
	argc -= 1 ;

	while (argc > 0) {

	    argp = argv[i++] ;
	    argc -= 1 ;
	    argl = strlen(argp) ;

	    if ((argl > 0) && (*argp == '-')) {

	        if (argl > 1) {

	            optp = argp ;
	            optl = argl ;
	            while (--optl) {

	                optp += 1 ;
	                switch (tolower(*optp)) {

	                case 'b':
	                    f_binary = TRUE ;
	                    break ;

	                case 'v':
	                    bprintf(efp,"%s: version %s\n",TITLE,VERSION) ;

	                    break ;

	                default:
	                    printf("%s: unknown option - %c\n",TITLE,*argp) ;

	                    goto usage ;

	                } /* end switch */

	            } /* end while */

	        } else {

	            f_dash = TRUE ;

	        } /* end if */

	    } else {

	        if (pan < NPARG) {

	            if (argl > 0) filename[pan] = argp ;

	            pan += 1 ;

	        } else {

	            bprintf(efp,"%s: extra arguments ignored\n",TITLE) ;

	        }

	    } /* end if */

	} /* end while */


/* check arguments */

	if (f_dash) bprintf(efp,"%s: dash argument ignored\n",TITLE) ;

	if (pan < 1) goto not_enough ;

	for (n = 0 ; n < pan ; n += 1) {

	    if (! f_binary) {

	        rs = bopen(ifp,filename[n],"r",0666) ;

	        if (rs < 0) goto bad_iopen ;

	        sprintf(tmpname,"/tmp/scp_%d",getpid()) ;

	        rs = bopen(ofp,tmpname,"wt",0666) ;

	        if (rs < 0) goto bad_topen ;

/* copy to the temporary file */

	        while ((len = breadline(ifp,buf,LINESIZE)) >= 0) {

	            if (buf[len - 1] == '\n') {

	                rs = bwrite(ofp,buf,len - 1) ;

	                if (rs < 0) goto bad_fout ;

	                rs = bwrite(ofp,"\r\n",2) ;

	                if (rs < 0) goto bad_fout ;

	            } else {

	                rs = bwrite(ofp,buf,len) ;

	                if (rs < 0) goto bad_fout ;

	            }

	        }

/* put the control Z at the end */

	        bputc(ofp,26) ;

	        bclose(ifp) ;

	        bclose(ofp) ;

	        f_chmod = FALSE ;

	    } else {

	        strcpy(tmpname,filename[n]) ;

/* make file readable by all */

	        stat(filename[n],&ss) ;

	        oldmode = (int) ss.st_mode ;

/* spawn in order to 'chmod' the file because we may be running 'suid' */

	        if (fork() == 0) {

	            setuid(getuid()) ;

	            setgid(getgid()) ;

	            chmod(filename[n],oldmode | 0444) ;

	            exit(0) ;
	        }

	        wait((char *) 0) ;

	        f_chmod = TRUE ;

	    }

/* prepare to execute the 'uucp' */

	    sprintf(dst,"%s/%s",STARDIR,filename[n]) ;

	    pipe(pfda) ;

	    if (fork() == 0) {

	        close(pfda[0]) ;

	        close(0) ;

	        open("/dev/null",O_RDONLY,0666) ;

	        close(1) ;

	        dup(pfda[1]) ;

	        close(2) ;

	        dup(pfda[1]) ;

	        close(pfda[1]) ;

	        execlp("uucp","uucp",tmpname,dst,0) ;

	        exit(BAD) ;
	    }

	    close(pfda[1]) ;

	    wait(&rs_child) ;

	    if (rs_child & 0xFF) {

	        if ((rs_child & 0xFF) == 0177) {

	            bprintf(efp,"%s: problem with 'uucp' program - %d\n",
	                TITLE,(rs_child >> 8) & 0xFF) ;

	        } else {

	            bprintf(efp,"%s: problem with 'uucp' program - %d\n",
	                TITLE,rs_child & 0xFF) ;

	        }

	    } else if ((rs_child >> 8) & 0xFF) {

	        bprintf(efp,"%s: error in transferring file %s - %d\n",
	            TITLE,filename[n],(rs_child >> 8) & 0xFF) ;

	    }

	    if (f_chmod) {

	        if (fork() == 0) {

	            setuid(getuid()) ;

	            setgid(getgid()) ;

	            chmod(filename[n],oldmode) ;

	            exit(0) ;
	        }

	        wait((char *) 0) ;

	    }

	    while ((len = read(pfda[0],buf,LINESIZE)) > 0) {

	        bwrite(efp,buf,len) ;

	    }

	    close(pfda[0]) ;

	    bflush(efp) ;

	} /* end of for loop */

	if (f_binary) unlink(tmpname) ;


/* finish up and get out */

	bclose(efp) ;

	return OK ;

not_enough:
	bprintf(efp,"%s: not enough arguments given\n",TITLE) ;

	goto bad_ret ;

bad_fout:
	bprintf(efp,"bad return after write to out file - %d\n",
	    rs) ;

	goto bad_fret ;

bad_iopen:
	bprintf(efp,"could not open file %s - %d\n",filename[n],
	    rs) ;

	goto bad_fret ;

bad_topen:
	bprintf(efp,"could not open tmp %s - %d\n",tmpname,
	    rs) ;

bad_fret:
	bclose(ifp) ;

	bclose(ofp) ;

	goto bad_ret ;

usage:
	bprintf(efp,"%s usage: scp file1 [file2] [-b]\n",TITLE) ;

bad_ret:
	bclose(efp) ;

	return BAD ;
}


