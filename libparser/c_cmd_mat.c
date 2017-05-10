/* cm_cmd_mat */



#include	<string.h>

#include	"localmisc.h"




#define MAXMCH  100        	/* maximum number of matchable objects */


int	imat ;       	        /* number of keyword matches */
int	imatch[MAXMCH] ;	/* array of matches indexes */

/*
 *      c_cmd_match  -  match object to command in list
 *
 *		Finds the best match to the object, using partial
 *		matches of the object to the command
 *
 *		If more than one partial match, takes the best one;
 *		if multiple "best" scores, returns all of them.
 *		Returns the number of matches as the function value.
 */


int c_cmd_match(object,lptr)
char	*object;
char	*lptr[];
{
	int iscore;
	int j;
	int n;


	imat = 0;
	iscore = -1;

	for (j = 0 ; 
	    (lptr[j] != NULL) && (strcmp(lptr[j],"END") != 0) ; j += 1) {

	    n = c_mat_score(lptr[j],object);

	    if (n == 0 || n < iscore)
	        continue;

	    if (n == iscore) {

	        imatch[imat++] = j;
	        continue;
	    }

	    if (n > iscore) {

	        imat = 1;
	        imatch[0] = j;
	        iscore = n;
	        continue;
	    }
	}

	return(imat);
}
/* end subroutine (c_cmd_mat) */


/*
 *	c_mat_score - compute the match score of object to key
 *
 *		Returns the number of characters of obj
 *		which match the keyword.
 *		Ignores case differences for the match.
 */


int c_mat_score(key,obj)
char	*key, *obj ;
{
	int n;


	n = 0;
	while (*obj != '\0') {

	    if (tolower(*key) != tolower(*obj)) {

	        return(0);

	    } else {

	        key++;
	        obj++;
	        n++;
	    }

	} /* end while */

	return n ;
}
/* end subroutine (cm_mat_scor) */


