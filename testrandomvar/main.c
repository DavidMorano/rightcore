/* main (testrandom) */

/* test the random number generator */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	F_PRINT		1		/* print out results ? */
#define	F_ANALYZE	1		/* analyze results ? */
#define	F_RESULTS	1		/* do results ? */
#define	F_OURS		1		/* use our RV generator ? */


/* revision history:

	= 98/02/05, David A­D­ Morano

	I put this together to see if this old object still works
	as intended !


*/


/******************************************************************************

	This subroutine is the main part of a program to test the
	random variable generator object.



******************************************************************************/



#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<time.h>
#include	<dirent.h>
#include	<string.h>
#include	<stdlib.h>

#include	<bfile.h>
#include	<mallocstuff.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"randomvar.h"




/* local defines */

#define	LINELEN		100
#define	NRANDS_PRINT	100
#define	NRANDS_ANALYSE	10000000



/* external subroutines */


/* forward references */






int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		outfile, *ofp = &outfile ;

	RANDOMVAR	rng ;

	int	rs, i, j, k, len ;
	int	ex = EX_DATAERR ;
	int	fd_debug ;
	int	seed ;
	int	hits_bits[2][64] ;
	int	hits_nibbles[16][16] ;
	int	hits_bytes[256][8] ;

	double	mean_bits[64] ;
	double	mean_nibbles[16] ;
	double	mean_bytes[8] ;

	double	mean_allbits ;
	double	mean_allnibbles ;
	double	mean_allbytes ;

	char	*cp ;
	char	*rsfname ;
	char	buf[MAXPATHLEN + 1] ;
	char	*progname ;



	progname = argv[0] ;

	if (bopen(ofp,BFILE_STDOUT,"dwct",0666) >= 0)
	    bcontrol(ofp,BC_LINEBUF,0) ;


#if	CF_DEBUGS
	debugprintf("main: entered\n") ;
#endif


	seed = 0 ;
	if ((argc >= 2) && (argv[1] != NULL)) {

	    cfdeci(argv[1],-1,&seed) ;

#if	CF_DEBUGS
	    debugprintf("main: seed=%d\n",seed) ;
#endif

	}


#if	CF_DEBUGS
	debugprintf("main: zeroing\n") ;
#endif

	for (i = 0 ; i < 2 ; i += 1) {

	    for (j = 0 ; j < 64 ; j += 1)
	        hits_bits[i][j] = 0 ;

		mean_bits[i] = 0.0 ;
	}

	mean_allbits = 0.0 ;

	for (i = 0 ; i < 16 ; i += 1) {

	    for (j = 0 ; j < 16 ; j += 1)
	        hits_nibbles[i][j] = 0 ;

		mean_nibbles[i] = 0.0 ;
	}

	mean_allnibbles = 0.0 ;

	for (i = 0 ; i < 256 ; i += 1) {

	    for (j = 0 ; j < 8 ; j += 1)
	        hits_bytes[i][j] = 0 ;

		mean_bytes[i] = 0.0 ;
	}

	mean_allbytes = 0.0 ;

#if	CF_DEBUGS
	debugprintf("main: starting\n") ;
#endif

	srandom(seed) ;

	randomvar_start(&rng,FALSE,seed) ;



#if	F_PRINT

#if	CF_DEBUGS
	debugprintf("main: sampling\n") ;
#endif

	j = 0 ;
	for (i = 0 ; i < NRANDS_PRINT ; i += 1) {

	    LONG	rv ;


#if	F_OURS
	    randomvar_getlong(&rng,&rv) ;
#else
		rv = random() ;

		rv = rv << 32 ;
		rv = rv | random() ;
#endif

	    bprintf(ofp," %08llx %08llx",rv,(rv >> 32)) ;

		j += 2 ;
		if (j >= 8) {

			j = 0 ;
			bprintf(ofp,"\n") ;

		}

	} /* end for */

#endif /* F_PRINT */


#if	F_ANALYZE

#if	CF_DEBUGS
	debugprintf("main: analysing\n") ;
#endif

	for (i = 0 ; i < NRANDS_ANALYSE ; i += 1) {

	    ULONG	rv ;

		int	riv ;


#if	F_OURS
	    randomvar_getulong(&rng,&rv) ;
#else
		riv = random() ;
		rv = (ULONG) riv ; rv = rv << 32 ;

		riv = random() ;
		rv ^= ((ULONG) riv) ;
#endif /* F_OURS */


	    for (j = 0 ; j < 64 ; j += 1)
	        hits_bits[(rv >> j) & 1][j] += 1 ;

	    for (j = 0 ; j < 16 ; j += 1)
	        hits_nibbles[(rv >> (j * 4)) & 15][j] += 1 ;

	    for (j = 0 ; j < 8 ; j += 1)
	        hits_bytes[(rv >> (j * 8)) & 255][j] += 1 ;

	} /* end for */


/* bit means */

	for (j = 0 ; j < 64 ; j += 1) {

		for (i = 0 ; i < 2 ; i += 1)
		mean_bits[j] += (((double) hits_bits[i][j]) * ((double) i)) ;

		mean_bits[j] /= ((double) NRANDS_ANALYSE) ;

		bprintf(ofp,"mean_bits[%3d]=%12.10f\n",j,mean_bits[j]) ;

#if	F_OURS
		mean_allbits += mean_bits[j] ;
#else
		if ((j != 31) && (j != 63))
		mean_allbits += mean_bits[j] ;
#endif /* F_OURS */

	} /* end for */

#if	F_OURS
	mean_allbits /= 64.0 ;
#else
	mean_allbits /= 62.0 ;
#endif

	bprintf(ofp,"mean_allbits=%12.10f\n",mean_allbits) ;


/* nibble means */

	for (j = 0 ; j < 16 ; j += 1) {

		for (i = 0 ; i < 2 ; i += 1)
		mean_nibbles[j] += 
			(((double) hits_nibbles[i][j]) * ((double) i)) ;

		mean_nibbles[j] /= ((double) NRANDS_ANALYSE) ;

		bprintf(ofp,"mean_bibbles[%3d]=%12.10f\n",j,mean_nibbles[j]) ;

#if	F_OURS
		mean_allnibbles += mean_nibbles[j] ;
#else
		if ((j != 7) && (j != 15))
		mean_allnibbles += mean_nibbles[j] ;
#endif /* F_OURS */

	} /* end for */

#if	F_OURS
	mean_allnibbles /= 16.0 ;
#else
	mean_allnibbles /= 14.0 ;
#endif

	bprintf(ofp,"mean_allnibbles=%12.10f\n",mean_allnibbles) ;


#endif /* F_ANALYZE */


	randomvar_finish(&rng) ;



/* print the results */

#if	F_RESULTS

#if	CF_DEBUGS
	debugprintf("main: results\n") ;
#endif

	{
	    double	n, p ;


	    for (j = 0 ; j < 64 ; j += 1) {

	        bprintf(ofp,"bit    %2d",j) ;

	        for (k = 0 ; k < 2 ; k += 1) {

	            n = (double) hits_bits[k][j] ;
	            p = n * 100.0 / ((double) NRANDS_ANALYSE) ;
	                bprintf(ofp," %14.10f",p) ;

	        }

	        bprintf(ofp,"\n") ;

	    }

	    for (j = 0 ; j < 16 ; j += 1) {

	        for (i = 0 ; i < 2 ; i += 1) {

	            bprintf(ofp,"nibble %2d",
	                j) ;

	            for (k = 0 ; k < 8 ; k += 1) {

	                n = (double) hits_nibbles[(i * 8) + k][j] ;
	                p = n * 100.0 / ((double) NRANDS_ANALYSE) ;
	                bprintf(ofp," %14.10f",p) ;

	            }

	            bprintf(ofp,"\n") ;

	        }

	    }

	    for (j = 0 ; j < 8 ; j += 1) {

	        for (i = 0 ; i < 32 ; i += 1) {

	            bprintf(ofp,"byte   %2u",
	                j) ;

	            for (k = 0 ; k < 8 ; k += 1) {

	                n = (double) hits_bytes[(i * 8) + k][j] ;
	                p = n * 100.0 / ((double) NRANDS_ANALYSE) ;
	                bprintf(ofp," %14.10f",p) ;

	            }

	            bprintf(ofp,"\n") ;

	        } /* end for (inner) */

	    } /* end for (outer) */


	} /* end block */

#endif /* F_RESULTS */


#if	CF_DEBUGS
	debugprintf("main: done\n") ;
#endif


	ex = EX_OK ;

done:
	bclose(ofp) ;

	return ex ;
}
/* end subroutine (main) */



