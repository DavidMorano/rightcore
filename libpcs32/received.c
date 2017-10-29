/* received */

/* manage a "received" object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SAFE		0		/* run in "safe" mode */
#define	CF_EMPTYVALUE	0		/* allow for empty values */
#define	CF_FIELDWORD	1		/* used 'field_word(3dam)' */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This object module processes and stores the information in a "received"
	header field for mail messages.

	Note that some (stupid) mailers put characters like quote characters
	and other weirdo characters into some of the value fields of the
	RECEIVED header.  I don't know how many of these weirdo characters are
	legal in an RFC-822 value (if any) but we are currently using
	'field_word(3dam)' with only the semi-colon character as a terminator
	to try to handle these wirdo cases.  If quote characters are not
	allowed to represent a regular legal character (without any escape
	quoting), then maybe 'field_get(3dam)' should be used.

	Note also that all RFC-822 comments are removed from the RECEIVED
	header value before trying to parse it out into components.  Doing full
	tokenization on these strings while preserving comments would make this
	crap as hard as it is for mail addresses (where we preserve comments
	associated with each address)!


*******************************************************************************/


#define	RECEIVED_MASTER		0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<strings.h>

#include	<vsystem.h>
#include	<field.h>
#include	<sbuf.h>
#include	<localmisc.h>

#include	"mhcom.h"
#include	"received.h"


/* local defines */


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	field_word(FIELD *,const uchar *,const char **) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* global variables */

const char	*received_keys[] = {
	"from",
	"by",
	"with",
	"id",
	"for",
	"date",
	"via",
	NULL
} ;


/* local structures */


/* forward references */

static int received_bake(RECEIVED *,int,const char *,int) ;


/* local variables */

/*

	  9	(tab)
	 10	(new line)
	 11	(vertical tab)
	 12	(form feed)
	 13	(carriage return)
	 32	(space)
	 59	;

*/

static const uchar	fterms[] = {
	0x00, 0x3E, 0x00, 0x00,
	0x01, 0x00, 0x00, 0x08,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int received_start(RECEIVED *op,cchar hbuf[],int hlen)
{
	MHCOM		com ;
	int		rs ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("received_start: ent\n") ;
#endif

	if (op == NULL) return SR_FAULT ;
	if (hbuf == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("received_start: got object pointer\n") ;
#endif

	memset(op,0,sizeof(RECEIVED)) ;

	if (hlen < 0)
	    hlen = strlen(hbuf) ;

#if	CF_DEBUGS
	debugprintf("received_start: slen=%d s=>%t<\n",slen,s,slen) ;
#endif

/* prepare a MHCOM object for comment parsing */

	if ((rs = mhcom_start(&com,hbuf,hlen)) >= 0) {
	    const char	*sp ;
	    int		sl ;
	    if ((sl = mhcom_getval(&com,&sp)) > 0) {
	        const int	size = (sl + 1) ;
	        void		*p ;
	        if ((rs = uc_malloc(size,&p)) >= 0) {
	            op->a = p ;
	            if ((rs = received_bake(op,size,sp,sl)) >= 0) {
	                c = rs ;
	                op->magic = RECEIVED_MAGIC ;
	            }
	            if (rs < 0) {
	                uc_free(op->a) ;
	                op->a = NULL ;
	            }
	        } /* end if (memory-allocation) */
	    } /* end if (non-zero content) */
	    mhcom_finish(&com) ;
	} /* end if (mhcom) */

#if	CF_DEBUGS
	{
	    int	i ;
	    for (i = 0 ; i < received_keyoverlast ; i += 1) {
	        if (op->key[i] != NULL)
	            debugprintf("received_start: k=%s v=>%s<\n",
	                received_keys[i],op->key[i]) ;
	    }
	}
#endif /* CF_DEBUGS */

#if	CF_DEBUGS
	debugprintf("received_start: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (received_start) */


/* free up a this object */
int received_finish(RECEIVED *op)
{
	int		rs = SR_OK ;
	int		rs1 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != RECEIVED_MAGIC) return SR_NOTOPEN ;
#endif

	if (op->a != NULL) {
	    rs1 = uc_free(op->a) ;
	    if (rs >= 0) rs = rs1 ;
	    op->a = NULL ;
	}

#if	CF_DEBUGS
	debugprintf("received_finish: ret rs=%d\n",rs) ;
#endif

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (received_finish) */


int received_getkey(RECEIVED *op,int ki,cchar **rpp)
{
	int		cl = 0 ;

#if	CF_SAFE
	if (op == NULL) return SR_FAULT ;

	if (op->magic != RECEIVED_MAGIC) return SR_NOTOPEN ;
#endif

	if (ki < 0) return SR_INVALID ;

	if (ki >= received_keyoverlast) return SR_NOENT ;

	cl = (op->key[ki] != NULL) ? strlen(op->key[ki]) : 0 ;

	if (rpp != NULL) {
	    *rpp = op->key[ki] ;
	}

	return cl ;
}
/* end subroutine (received_getkey) */


int received_getitem(RECEIVED *op,int ki,cchar **rpp)
{

	return received_getkey(op,ki,rpp) ;
}
/* end subroutine (received_getitem) */


/* private subroutines */


static int received_bake(RECEIVED *op,int size,const char *sp,int sl)
{
	SBUF		sb ;
	int		rs ;
	int		rs1 ;
	int		ki = -1 ;
	int		wi = 0 ;
	int		c = 0 ;
	int		f_prevmatch = FALSE ;

	if ((rs = sbuf_start(&sb,op->a,size)) >= 0) {
	    FIELD	fsb ;

	    if ((rs = field_start(&fsb,sp,sl)) >= 0) {
	        const char	*fp ;
	        int		fl ;

	        while (rs >= 0) {

#if	CF_FIELDWORD
	            fl = field_word(&fsb,fterms,&fp) ;
#else
	            fl = field_get(&fsb,fterms,&fp) ;
#endif

	            if (fl <= 0)
	                break ;

#if	CF_DEBUGS
	            debugprintf("received_start: "
			"c=%u ki=%d wi=%u w=%t term=>%c<\n",
	                c,ki,wi,fp,fl,fsb.term) ;
#endif

	            rs1 = -1 ;
	            if (! f_prevmatch)
	                rs1 = matcasestr(received_keys,fp,fl) ;

#if	CF_DEBUGS
	            debugprintf("received_start: match=%d\n",rs1) ;
#endif

#if	(! CF_EMPTYVALUE)
	            f_prevmatch = (rs1 >= 0) ;
#endif

	            if (rs1 >= 0) {

	                if (c > 0)
	                    sbuf_char(&sb,'\0') ;

	                ki = rs1 ;
	                wi = 0 ;

	            } else if (ki >= 0) {

	                if ((wi == 0) && (ki >= 0)) {
	                    const char	*cp ;

	                    sbuf_getpoint(&sb,&cp) ;

	                    op->key[ki] = cp ;
	                    c += 1 ;
	                }

	                if (wi > 0)
	                    sbuf_char(&sb,' ') ;

	                rs = sbuf_strw(&sb,fp,fl) ;
	                wi += 1 ;

	            } /* end if */

	            if (fsb.term == ';') break ;
	        } /* end while */

	        if ((rs >= 0) && (fsb.term == ';')) {
	            const char	*cp ;
	            int		cl ;
	            if (c > 0)
	                sbuf_char(&sb,'\0') ;

	            sbuf_getpoint(&sb,&cp) ;

	            op->key[received_keydate] = cp ;
	            c += 1 ;

	            cp = (const char *) fsb.lp ;
	            cl = fsb.ll ;
	            sbuf_strw(&sb,cp,cl) ;

	        } /* end if (had a date) */

	        field_finish(&fsb) ;
	    } /* end if (field) */

	    rs1 = sbuf_finish(&sb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sbuf) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (received_bake) */


