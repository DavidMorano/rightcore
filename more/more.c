/* main */

/* custom more program */
/* last modified %G% version %I% */


#define	F_DEBUG		0


/* revision history :

	= July 1986, David A­D­ Morano


*/


/**************************************************************************

	Provide the 'more' program function, only do it correctly
	and probably much more simply.



***************************************************************************/


#define		VERSION		"1"



#include	<fcntl.h>

#include	<bfile.h>
#include	<termstr.h>
#include	<vsystem.h>

#include	"misc.h"



#define		LINELEN		100
#define		DEFMAXLINES	21
#define		NPARG		1



extern int	cfdeci(const char *,int,int *) ;




int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		errfile, *efp = &errfile ;
	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;

	int	argl, aol ;
	int	temp ;

	int	pan, i ;
	int	len, line, rs ;
	int	tfd ;
	int	maxlines = DEFMAXLINES ;
	int	nblanks ;
	int	f_dash = FALSE ;
	int	f_ttyout = FALSE ;
	int	f_squeeze = FALSE ;
	int	f_usage = FALSE ;

	char	*argp, *aop ;
	char	*progname ;
	char	*infname ;
	char	*bp, buf[LINELEN] ;


	progname = argv[0] ;
	if (bopen(efp,2L,"wca",0666) < 0) return BAD ;

	infname = ((char *) 0) ;

	pan = 0 ;			/* number of positional so far */
	i = 1 ;
	argc -= 1 ;

	while (argc > 0) {

	    argp = argv[i++] ;
	    argc -= 1 ;
	    argl = strlen(argp) ;

	    if ((argl > 0) && (*argp == '-')) {

	        if (argl > 1) {

	            aop = argp ;
	            aol = argl ;

	            while (--aol) {

	                aop += 1 ;
	                switch (tolower(*aop)) {

	                case 'p':
	                    if (argc <= 0) goto notenough ;

	                    argp = argv[i++] ;
	                    argc -= 1 ;
	                    argl = strlen(argp) ;

	                    if (cfdec(argl,argp,&temp) != OK) goto badparam ;

	                    maxlines = (int) temp ;
	                    break ;

			case 's':
			    f_squeeze = TRUE ;
			    break ;

	                case 'V':
	                    bprintf(efp,"%s: version %s\n",progname,VERSION) ;

	                    break ;

			case '?':
			    f_usage = TRUE ;
			    break ;

	                default:
	                    printf("%s: unknown option - %c\n",progname,
				*aop) ;

	                    goto usage ;

	                } /* end switch */

	            }

	        } else {

	            f_dash = TRUE ;
	            pan += 1 ;		/* increment position count */

	        } /* end if */

	    } else {

	        if (pan < NPARG) {

	            switch (pan) {

	            case 0:
	                infname = argp ;

	                break ;

	            default:
	                break ;

	            }

	            pan += 1 ;

	        } else {

	            bprintf(efp,"%s: extra arguments ignored\n",progname) ;

	        }

	    } /* end if */

	} /* end while */


	if (f_usage) goto usage ;


/* check the arguments */

	if (f_dash) infname = (char *) 0L ;

	if ((rs = bopen(ifp,infname,"r",0000)) < 0) goto badin ;

	if (maxlines < 2) maxlines = 2 ;

/* do program */

	tfd = 1 ;
	if (isatty(tfd)) {

	    if ((rs = tty_open(tfd)) < 0) goto badterm ;

	    f_ttyout = TRUE ;
	    tty_control(tfd,fc_setmode,(fm_nofilter | fm_noecho)) ;

	} else {

	    if ((rs = bopen(ofp,tfd,"wct",0666)) < 0) goto badout ;

	}

	line = 0 ;
	nblanks = 0 ;
	while ((len = bgetline(ifp,buf,LINELEN)) > 0) {

	    if (f_ttyout) {

		if (f_squeeze) {

 		    if ((len == 1) && (buf[0] == '\n')) nblanks += 1 ;

		    else nblanks = 0 ;

		    if (nblanks < 2)
	            	tty_write(tfd,buf,len) ;

		} else
	            tty_write(tfd,buf,len) ;

#if	F_DEBUG
	        bwrite(efp,buf,len) ; 
	        bflush(efp) ;
#endif

	        line += 1 ;
	        if (line >= maxlines) {

	            bp = buf ;
	            bp += sprintf(bp,"%smore : %s",
			TERMSTR_REVERSE,TERMSTR_NORM) ;

	            tty_write(tfd,buf,(int) (bp - buf)) ;

	            len = tty_read(tfd,buf,1) ;

	            if (len < 0) goto badret ;

	            else if (len == 0) {

	                tty_write(tfd,"\r\033[K",4) ;

	                break ;

	            } else if ((len > 0) && (buf[0] == '\r')) {

	                line = maxlines ;
	                tty_write(tfd,"\r\033[K",4) ;

	            } else if ((len > 0) && (buf[0] == 'q')) {

	                tty_write(tfd,"\r\033[K",4) ;

	                break ;

	            } else {

	                line = 0 ;
	                tty_write(tfd,"\r\033[K",4) ;

	            }

	        }

	    } else bwrite(ofp,buf,len) ;

	}

	tty_close(tfd) ;

done:
	bclose(ifp) ;

	if (f_ttyout) tty_close(tfd) ;

	else bclose(ofp) ;

	bclose(efp) ;

	return OK ;

badin:
	bprintf(efp,"can't open infile (%d)\n",rs) ;

	goto badret ;

badout:
	bprintf(efp,"can't open outfile (%d)\n",rs) ;

	goto badret ;

badterm:
	bprintf(efp,"can't open terminal file (%d)\n",rs) ;

	goto badret ;

notenough:
	printf(efp,"%s: not enough arguments given\n",progname) ;

	goto badret ;

badparam:
	bprintf(efp,"%s: bad parameter specified\n",progname) ;

	goto badret ;

usage:
	bprintf(efp,"usage: %s [-|file] [-s]\n",
		progname) ;

badret:
	if (f_ttyout) tty_close(tfd) ;

	fclose(efp) ;

	return BAD ;
}


