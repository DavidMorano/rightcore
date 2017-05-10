/* bld_links */



/************************************************************************
 *									
 * The information contained herein is for the use of AT&T Information	
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	
 *									
 *	(c) 1984 AT&T Information Systems				
 *									
 * Authors of the contents of this file:				
 *									
 *		A.M.Toto						
 *									

 ************************************************************************/



#include	<sys/types.h>
#include	<stdio.h>

#include	<logfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* external subroutines */

extern struct sys_tbl	sname[] ;



bld_link(trans, hname)
struct table *trans ;
char *hname ;
{
	struct table *tbl_tmp ;
	struct sys_tbl *sys_tmp ;

	int scount ;
	int match ;


/* Now go through the sys_tbl looking for a match
	 * for cur_ent.user. If we run out of systems to
	 * look at, the system we're looking for is
	 * obviously not there.  Make an entry in the
	 * sys_tbl.
	 */

/* The syslink field is set to NULL when the table */
/* entry is allocated by nmalloc. If syslink is not*/
/* NULL that implies that this entry is already in */
/* a mailbag and need not be put in it again	   */

	if (trans->syslink != NULL) {	/* it is already in a mailbag */

	    return ;
	}
	scount = tot_sys ;
	sys_tmp = &sname[0] ;
	match = NO ;

	while ( scount != 0 ) {

	    if ( strcmp ( sys_tmp->sysname, hname ) == 0 ) {

/* Follow the links in the translation
			 * table until we reach the end of the
			 * list.  At that point, put the address
			 * of the cur_ent in the link field
			 * and a NULL in cur_ent's link
			 * field.
			 */

	        tbl_tmp = sys_tmp->trans_link ;
	        while(tbl_tmp->syslink != (struct table *)NULL) {

	            tbl_tmp = tbl_tmp->syslink ;
	        }

/* here's where we have to do some * entering */

	        match = YES ;

/* This entry needs to be added only if it is not  */
/* already in the mailbag			   */

	        if ( trans != tbl_tmp ) {

	            tbl_tmp->syslink = trans ;
/*trans->syslink = (struct table *) NULL;*/
	        }
	        break ;

	    } else {

	        scount-- ;
	        sys_tmp++ ;
	    }
	}

/* Check to see if a match has been found.  If not,
	 * we have to enter the new system in the sys_tbl,
	 * assign the trans field and put a NULL in 
	 * trans->syslink.
	 */

	if (! match) {

	    strcpy ( sys_tmp->sysname, hname ) ;
	    sys_tmp->trans_link = trans ;
	    trans->syslink = (struct table *) NULL ;
	    tot_sys++ ;
	}

}
/* end subroutine (bld_link) */


