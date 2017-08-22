/* keytracker */

/* object to track used keys */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1994-03-24, David A­D­ Morano
	This object module was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

        This object tracks keys in a key-value pair-like structure (not
        uncommonly used with file DB queries, in the response to such queries).
        It tracks whether the keys are "done" or not as determined by whether
        the "done" object method is called with the index of the key in the
        key-value array as an argument.

        Yes, we could have used a hash table of the key names to indicate
        whether they were "done" or not, but for no good reason we used a bit
        array (indexed by the key-value index) to track the "done" status.


*****************************************************************************/


#define	KEYTRACKER_MASTER	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<localmisc.h>

#include	"keytracker.h"


/* local defines */


/* external subroutines */

extern int	nextfield(const char *,int,const char **) ;


/* forward references */

static int	keytracker_checkmore(KEYTRACKER *,const char *,int) ;

static int	matkey(const char *(*)[2],const char *,int) ;


/* exported subroutines */


int keytracker_start(op,keyvals)
KEYTRACKER	*op ;
const char	*(*keyvals)[2] ;
{
	int		rs ;
	int		n = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (keyvals == NULL) return SR_FAULT ;

	memset(op,0,sizeof(KEYTRACKER)) ;

	for (n = 0 ; keyvals[n][0] != NULL ; n += 1) ;

	if ((rs = bits_start(&op->dones,n)) >= 0) {
	    op->keyvals = keyvals ;
	}

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (keytracker_start) */


/* free up this keytracker object */
int keytracker_finish(op)
KEYTRACKER	*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->keyvals == NULL) return SR_NOTOPEN ;

	rs1 = bits_finish(&op->dones) ;
	if (rs >= 0) rs = rs1 ;

	op->keyvals = NULL ;
	return rs ;
}
/* end subroutine (keytracker_finish) */


int keytracker_done(op,n)
KEYTRACKER	*op ;
int		n ;
{
	int		rs ;

	if (op == NULL) return SR_FAULT ;

	if (op->keyvals == NULL) return SR_NOTOPEN ;

	if (n > op->n) return SR_INVALID ;

	rs = bits_set(&op->dones,n) ;

	return rs ;
}
/* end subroutine (keytracker_done) */


/* are there more keys that have not been completed? */
int keytracker_more(op,morestr)
KEYTRACKER	*op ;
const char	morestr[] ;
{
	int		rs = SR_OK ;
	int		f = FALSE ;
	const char	*tp, *sp ;

	if (op == NULL) return SR_FAULT ;

	if (op->keyvals == NULL) return SR_NOTOPEN ;

	sp = morestr ;
	while ((tp = strchr(sp,',')) != NULL) {

	    f = keytracker_checkmore(op,sp,(tp - sp)) ;
	    if (f) break ;

	    sp = (tp + 1) ;
	} /* end while */

	if ((! f) && (sp[0] != '\0')) {
	    f = keytracker_checkmore(op,sp,-1) ;
	}

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (keytracker_more) */


/* private subroutines */


static int keytracker_checkmore(op,sp,sl)
KEYTRACKER	*op ;
const char	*sp ;
int		sl ;
{
	int		cl ;
	int		f = FALSE ;
	const char	*cp ;

	if (sl < 0)
	    sl = strlen(sp) ;

	if ((cl = nextfield(sp,sl,&cp)) >= 0) {
	    int	ki ;
	    if ((ki = matkey(op->keyvals,cp,cl)) >= 0) {
	        int	rc = bits_test(&op->dones,ki) ;
	        f = (rc == 0) ;
	    }
	}

	return f ;
}
/* end subroutine (keytracker_checkmore) */


static int matkey(keyvals,kp,kl)
const char	*(*keyvals)[2] ;
const char	kp[] ;
int		kl ;
{
	int		i ;
	int		f = FALSE ;

	if (kl < 0)
	    kl = strlen(kp) ;

	for (i = 0 ; keyvals[i][0] != NULL ; i += 1) {

	    f = (strncmp(keyvals[i][0],kp,kl) == 0) ;
	    f = f && (keyvals[i][0][kl] == '\0') ;
	    f = f && (keyvals[i][1] != NULL) ;
	    if (f) break ;

	} /* end for */

	return (f) ? i : -1 ;
}
/* end subroutine (matkey) */


