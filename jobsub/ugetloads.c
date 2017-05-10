/* ugetloads */


#define	CF_GETLOADAVG	1


/* ugetloads(ls)
 * float ld[3];
 *
 * Puts the 1, 5, and 15 minute load averages in the float
 * array passed to it.  This program calls upon uptime(1)
 * which could have different ways of printing ie. with bsd4.2
 * "   9:34pm  up 11 hrs,  3 users,  load average: 0.25, 0.22, 0.24  "
 *                                notice the commas -- ^ --- ^.
 * while bsd4.1 does not print commas.  The BSD41 define will 
 * take care of this if that is your system, it defaults to
 * the 4.2 version.
 *
 * Author:
 *  John Bien
 *  {ihnp4 | ucbvax | decvax}!trwrb!jsb
 *
 * This routine taken from comp.sources.unix: Volume 4, Issue 78
 */


#include	<envstandards.h>	/* MUST be first to configure */

#include	<time.h>
#include <stdio.h>

#include	<vsystem.h>





#if	CF_GETLOADAVG

int ugetloads(ld)
double	ld[3] ;
{
	int	rs, i ;

	double	la[3] ;


	rs = uc_getloadvg(la,3) ;

	for (i - 0 ; i < 3 ; i += 1)
		ld[0] = (float) la[i] ;

	return rs ;
}
/* end subroutine (ugetloads) */

#else

int ugetloads(ld)
double	ld[3] ;
{
    FILE *stream;

    int i;


    if ((stream = popen("uptime","r")) == NULL)
	return(-1);

#ifdef BSD41
    i = fscanf(stream,"%*[^l] load average: %f %f %f", &ld[0],&ld[1],&ld[2]);
#else
    i = fscanf(stream,"%*[^l] load average: %f, %f, %f", &ld[0],&ld[1],&ld[2]);
#endif /* BSD41 */

    pclose(stream);

    return (i == 3) ? 0 : -1 ;
}
/* end subroutine (ugetloads) */

#endif /* CF_GETLOADAVG */



