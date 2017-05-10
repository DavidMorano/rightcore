/* last modified %G% version %I% */

/*
	Rick Meis
	April 1990
	- originally written

	David A.D. Morano
	November 1991
	- adapted from the original
*/


/****************************************************************************
 *
 *Program Name: conv_to_gen.c
 *
 * This program converts extension and circuit pack locations in SATSIM     
 * scripts (.ap files) to variables outlined in the model file.
 * It reads the model file (dr06_CP.s for instance) to 
 * determine what extension perfix variable and carrier-slot location variable
 * to replace in the SATSIM file.

	Synopsis:
		sat_fu [-f filter_file] [input_file]


*************************************************************************** */


#define		VERSION		"0"


#include	<fcntl.h>

#include	<ctype.h>

#include	<bfile.h>

#include	"sat_f.h"

#include	"localmisc.h"


#define		NPARG		2
#define		NULL		((char *) 0)


/* external functions */

extern char	*strcpy() ;
extern char	*strcat() ;
extern char	*getenv() ;


/* external variables */

extern int	errno ;


char	buf[LINELEN] ;
char	name[LINELEN] ;
char	loc[LINELEN] ;
char	pattern[MAXLINES][LINELEN] ;

char    *largv[(MAXLINES * 2) + 2] ;

char 	*loc_pt ;
char 	*carr_pt ;


int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile	errfile, *efp = &errfile ;
	bfile	filfile, *ffp = &filfile ;

	int	argl, aol ;
	int	pan, i, j ;
	int	argn = 1, fd ;
	int	len ;
	int	f_cp ;
	int	f_debug = FALSE ;
	int	f_usage = FALSE ;

	char	*argp, *aop ;
	char	*progname ;

	char    *ffname = NULL ;
	char	*infname = NULL ;
	char	*outfname = NULL ;


	if (bopen(efp,BERR,"wca",0666) < 0) return BAD ;

	progname = argv[0] ;

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
	                switch ((int) *aop) {

	                case 'f':
	                    if (argc <= 0) goto badargnum ;

	                    argp = argv[i++] ;
	                    argc -= 1 ;
	                    argl = strlen(argp) ;

	                    ffname = argp ;
	                    break ;

	                case 'V':
	                    bprintf(efp,"%s: SAT Filter Uncompile - ",
	                        progname) ;

	                    bprintf(efp,"version %s\n",
	                        VERSION) ;

	                    break ;

			case 'D':
				f_debug = TRUE ;
				break ;

	                default:
	                    bprintf(efp,"%s: unknown option - %c\n",
	                        progname,*aop) ;

	                case '?':
	                    f_usage = TRUE ;

	                } /* end switch */

	            }

	        } else {

	            pan += 1 ;	/* increment position count */

	        } /* end if */

	    } else {

	        if (pan < NPARG) {

	            switch (pan) {

	            case 0:
	                if (argl > 0) infname = argp ;

	                break ;

	            case 1:
	                if (argl > 0) outfname = argp ;

	            default:
	                break ;
	            }

	            pan += 1 ;

	        } else {

	            bprintf(efp,
	                "%s: extra arguments ignored\n",progname) ;

	        }

	    } /* end if */

	}  /* end while */

	if (f_usage) goto usage ;


/* check the arguments */

	if (ffname == NULL) ffname = getenv("A_FILTER") ;

	if (ffname == NULL) ffname = getenv("CPLOC") ;

	if (ffname == NULL) {

	    bprintf(efp,
	        "%s: no filter file is specified\n",progname) ;

	    goto badret ;
	}


/******************************************************************
	 * Check if the circuit pack file is readable
 ****************************************************************** */

	if (f_debug)
	    bprintf(efp,"%s: using filter file \"%s\"\n",
	    progname,ffname) ;

	if (bopen(ffp,ffname,"r") < 0) {

	    bprintf(efp,
	        "%s: filter file is not readable\n",progname) ;

	    goto badret ;
	}

	i = 0 ;
	f_cp = FALSE ;
	largv[0] = "sed" ;


/******************************************************************
         * Read CP file to determine what values are to be set for the
         * extension and circuit pack variables. f_cp is reset before
	 * getting next line from input file.
****************************************************************** */

/* Reset flag before getting next line from file */

	f_cp = FALSE ;

	while ((len = breadline(ffp,buf, LINELEN)) > 0) {

/* check for blank line */

	    if (len <= 1) continue ;

	    buf[len - 1] = '\0' ;

/* check for a leading comment line */

	    for (j = 0 ; j < (len - 1) ; j += 1)
		if ((buf[j] != ' ') && (buf[j] != '\t')) break ;

	    if ((j < len) && (buf[j] == '#')) continue ;

/* OK, read the two fields */

	    if (sscanf(buf, "%s%s", name, loc) != 2) {

	        bprintf(efp, 
	            "%s: filter file format incorrect\n",
	            progname) ;

	        goto badret ;
	    }

	    strcpy(pattern[i], "s/") ;

	    carr_pt = pattern[i] + 2 ;


/************************************************************
		 * Check to determine if loc is a circuit pack location,
		 * if it is, add a 0* to remove any user inserted leading 
		 * zeros in a  circuit pack admin string. For example, remove 
		 * the first 0 from 01d1501.
 ************************************************************ */

	    if ((isalpha(loc[1])) || (isalpha(loc[2]))) {

	        *carr_pt++ = '0' ;
	        *carr_pt++ = '*' ;
	        f_cp = TRUE ;

	    } else f_cp = FALSE ;


/************************************************************
		 * If loc is a circuit pack, insert the following string
		 * "[lower-case, upper-case]" into the substitution string.
		 * This checks to determine if the character is either 
		 * lower-case or upper-case, for example, 1d0701 or 1D0701.
		 * Then the substitution in the .ap file is made correctly.
 ************************************************************ */

	    for (loc_pt = loc ; *loc_pt != '0' ; loc_pt += 1) {

	        if (isalpha(*loc_pt)) {

	            *carr_pt++ = '[' ;
	            *carr_pt++ = tolower(*loc_pt) ;
	            *carr_pt++ = toupper(*loc_pt) ;
	            *carr_pt++ = ']' ;

	        } else {

	            *carr_pt++ = *loc_pt ;
	        }
	    }


/************************************************************ 
		 * Check to see if the f_cp is set. If it is set, 
		 * do not send the extra characters required for the 
		 * the extension number check. This makes sure only extensions
		 * are checked and correct substitutions are make.
 ************************************************************ */

	    if (f_cp == TRUE)
	        strcat(pattern[i], "/${") ;

	    else
	        strcat(pattern[i], "\\([5-6][0-9][0-9]\\)/${") ;

	    strcat(pattern[i], name) ;


/************************************************************ 
		 * Check to see if the f_cp is set. If it is set, 
		 * do not send the first occurrence check for the extension 
		 * suffix ([5-6][0-9][0-9]).
 ************************************************************ */

	    if (f_cp == TRUE)
	        strcat(pattern[i], "}/") ;

	    else
	        strcat(pattern[i], "}\\1/") ;


/************************************************************ 
		 * Populate largv array with patterns for substitutions. 
 ************************************************************ */

	    largv[argn++] = "-e" ;
	    largv[argn++] = pattern[i] ;


/************************************************************ 
		 * Make sure input circuit pack file does have more than 
		 * MAXLINES of input.
 ************************************************************ */

	    if (i >= MAXLINES) {

	        bprintf(efp, 
	            "%s: filter file has too many entries\n",progname) ;

	        bprintf(efp, 
	            "%s: change MAXLINES to add more entries\n",progname) ;

	        goto badret ;

	    } else {

	        i += 1 ;
	    }
	}


/******************************************************************
	 * Set last location for largv array to NULL.
         * Exec sed with values to substitute as the ".ap" files are being
         * being processed.
****************************************************************** */

	largv[argn] = NULL ;

	if (infname != NULL) {

	    if ((fd = open(infname,O_RDONLY,0664)) < 0) {

	        bprintf(efp,
	            "%s: could not open input file - errno (%d)\n",
	            progname,errno) ;

	        goto badret ;
	    }

	    close(0) ;

	    dup(fd) ;

	}

	bflush(efp) ;

	return execvp("sed", largv) ;

badargnum:
	bprintf(efp,"%s: not enough arguments given\n",progname) ;

	goto badret ;

badparam:
	bprintf(efp,"%s: bad parameter specified\n",progname) ;

	goto badret ;

usage:
	bprintf(efp,
	    "usage: %s [-f filter] [input] [outfile] [-V] [-D] [-?]\n",
	    progname) ;

badret:
	bclose(efp) ;

	return BAD ;
}


