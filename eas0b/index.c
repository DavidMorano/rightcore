/* index */

/* create the index of the library file */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0
#define	CF_DEBUG	1


/* revision history:

	= David A.D. Morano, October 1994
	Program was originally written.

	= David A.D. Morano, March 1996
	The program was slightly modified to use TMPDIR as
	the directory for temporary files.

*/


/****************************************************************************

	Create an index of subcircuits on the library file.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/utsname.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<time.h>
#include	<pwd.h>
#include	<grp.h>
#include	<signal.h>
#include	<strings.h>		/* for |strcasecmp(3c)| */
#include	<errno.h>

#include	<bfile.h>
#include	<baops.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external functions */

extern char	*malloc_str() ;
extern char	*strbasename() ;


/* external variables */

extern struct global		g ;


/* forward references */

struct circuit		*mkcircuit() ;

offset_t		gotcircuit() ;

int			addblock() ;
int			writecir() ;
int			loadtypes(), loadtype() ;
int			cirtypematch() ;

#ifdef	COMMENT
void			deleteblock() ;
#endif


/* local structures */


/* local globals */


/* local variables */

static char	*keywords[] = {
	NULL,
	".main",
	".subckt",
	".finis",
	".end",
	NULL,
} ;

/* circuit types */
char	*cirtypes[] = {
	"envelope",
	"main",
	"subckt",
	NULL,
} ;


#define	CIR_ENVELOPE	0
#define	CIR_MAIN	1
#define	CIR_SUB		2


/* exported subroutines */


int mkindex(int argc,cchar **argv,cchar **envv)
{
	bfile	infile, *ifp = &infile ;
	bfile	tmpfile, *tfp = &tmpfile ;
	bfile	outfile ;
	bfile	errfile, *efp = &errfile ;

	struct global	*gdp = &g ;

	struct circuit	*e_cirp, *cirp ;

	struct type	*tp, *headp = NULL ;

	offset_t	blockstart, offset ;

	int	argr, argl, aol, akl, avl, kwi ;
	int	npa, i, ai ;
	int	len, rs ;
	int	maxai ;
	int	l, pn ;
	int	blen ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_extra = FALSE ;
	int	f_optminus, f_optplus, f_optequal ;
	int	f_bol, f_eol ;
	int	f_cktmain, f_cktsub ;
	int	f_contents = FALSE ;
	int	type ;
	int	line ;

	char	*argp, *aop, *akp, *avp ;
	char	argpresent[MAXARGGROUPS] ;
	char	buf[BUFLEN + 1], *bp ;
	char	cirtype[4] ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	subfname[MAXPATHLEN + 1] ;
	char	*infname = NULL ;
	char	*outfname = NULL ;
	char	*cp, *subname ;


	cirtype[0] = 0 ;

/* start processing the (possibly null) envelope circuit */

	if ((e_cirp = mkcircuit("envelope",-1,CIR_ENVELOPE,0)) == NULL)
	    goto badalloc ;

	gdp->top = e_cirp ;
	gdp->bottom = e_cirp ;

#if	CF_DEBUG
	if (gdp->debuglevel > 1)
	    debugprintf("main: made envelope circuit %08lX\n",e_cirp) ;
#endif

/* do it -- go through all lines looking for SUBCIRCUITs (or MAIN) */

	line = 0 ;
	f_bol = TRUE ;
	offset = 0 ;
	blen = 0 ;
	blockstart = 0 ;
	while ((len = breadline(gdp->ifp,gdp->buf,BUFLEN)) > 0) {

	    l = len ;
	    f_eol = FALSE ;
	    if (gdp->buf[l - 1] == '\n')
	        ((l -= 1), (f_eol = TRUE)) ;

	    gdp->buf[l] = '\0' ;

	    if (f_bol) {

#if	CF_DEBUG
	        if (gdp->debuglevel > 1) {

	            debugprintf("main: beginning of line\n") ;

	            debugprintf("main: >%s<\n", gdp->buf) ;

	        }
#endif

/* skip over white space */

	        cp = gdp->buf ;
	        while (ISWHITE(*cp)) cp += 1 ;

/* scan for an ADVICE circuit (main or sub) keyword */

	        f_cktsub = FALSE ;
	        f_cktmain = (strncasecmp(cp,".main",5) == 0) ;

	        if (! f_cktmain)
	            f_cktsub = (strncasecmp(cp,".subckt",7) == 0) ;

	        if (f_cktmain || f_cktsub) {

	            if (gdp->debuglevel > 0) bprintf(gdp->efp,
	                "%s: got a new subcircuit\n",
	                gdp->progname) ;

	            type = CIR_MAIN ;
	            if (f_cktsub) type = CIR_SUB ;

/* we got a new subcircuit, finish off the block of this circuit */

	            if (addblock(e_cirp,blockstart,blen) < 0)
	                goto badalloc ;

#if	CF_DEBUG
	            if (gdp->debuglevel > 1)
	                debugprintf("main: closed off block s=%ld bl=%d\n",
	                    blockstart,blen) ;
#endif

/* start the new circuit */

#if	CF_DEBUG
	if (gdp->debuglevel > 1) debugprintf(
	"main: calling 'gotcircuit' linelen=%d\n",len) ;
#endif

	            l = cp - gdp->buf ;
	            if ((offset = gotcircuit(gdp,offset,
	                gdp->buf,len,l,type,1)) < 0) goto badgotcir ;

/* think about it, it is pretty neat that we do NOT have to do this seek */

#ifdef	COMMENT
	            bseek(gdp->ifp,offset,SEEK_SET) ;
#endif

	            blockstart = offset ;
	            blen = 0 ;
	            len = 0 ;

	        } /* end if (a new subcircuit) */

#if	CF_DEBUG
	        if (gdp->debuglevel > 1)
	            debugprintf("main: bottom of BOL o=%ld\n",offset) ;
#endif

	    } /* end if (BOL) */

#if	CF_DEBUG
	    if (gdp->debuglevel > 1)
	        debugprintf("main: bottom of while loop o=%ld\n",
	            offset) ;
#endif

	    offset += len ;
	    blen += len ;
	    if ((len > 0) && f_eol) line += 1 ;

	    f_bol = f_eol ;

	} /* end while (scanning circuits) */

/* finish the last block */

	if (addblock(e_cirp,blockstart,blen) < 0)
	    goto badalloc ;

	e_cirp->lines = line ;

/* end of looping through (cataloging) the stuff */

#if	CF_DEBUG
	if (gdp->debuglevel > 0) bprintf(gdp->efp,
	    "%s: finished scanning circuits, input file len=%ld\n",
	    gdp->progname,offset) ;
#endif


}
/* end subroutine (mkindex) */


/* process a subcircuit */

offset_t gotcircuit(gdp,blockstart,linebuf,linelen,index,type,sl)
struct global	*gdp ;
char		linebuf[] ;
int	linelen ;
int	index ;
offset_t	blockstart ;
int	type ;
int	sl ;
{
	bfile		outfile, *ofp = &outfile ;

	struct circuit	*cirp ;

	offset_t		offset = blockstart ;

	int	l, len, blen = linelen ;
	int	f_bol, f_eol ;
	int	f_finis = FALSE ;
	int	f_exit ;
	int	f_cktmain, f_cktsub ;
	int	line = 1 ;

	char	subname[MAXPATHLEN + 1] ;
	char	subfname[MAXPATHLEN + 1] ;
	char	*cp ;


#if	CF_DEBUG
	if (gdp->debuglevel > 1)
	    debugprintf("gotcircuit: entered sl=%d o=%ld linelen=%d\n",
	        sl,offset,linelen) ;
#endif

	linebuf[linelen] = '\0' ;

#if	CF_DEBUG
	if (gdp->debuglevel > 1)
	    debugprintf("gotcircuit: line end zapped\n") ;
#endif

/* try to get a subcricuit "name" */

	cp = linebuf + index ;

#if	CF_DEBUG
	if (gdp->debuglevel > 1)
	    debugprintf("gotcircuit: starting point calculated\n") ;
#endif

#if	CF_DEBUG
	if (gdp->debuglevel > 1) {

	    debugprintf("gotcircuit: cp=%W\n",cp,10) ;

		l = strlen(cp) ;

		debugprintf("gotcircuit: len=%d\n",l) ;

	}
#endif

/* skip over white space, if any (there should not be any) */

	while (ISWHITE(*cp)) cp += 1 ;

#if	CF_DEBUG
	if (gdp->debuglevel > 1)
	    debugprintf("gotcircuit: white space\n") ;
#endif

/* skip over the keyword */

	while (*cp && (! ISWHITE(*cp))) cp += 1 ;

#if	CF_DEBUG
	if (gdp->debuglevel > 1)
	    debugprintf("gotcircuit: non-white space\n") ;
#endif

/* skip over possibly more white space */

	while (ISWHITE(*cp)) cp += 1 ;

/* copy the line buffer to the subname buffer */

	strcpy(subname,cp) ;

#if	CF_DEBUG
	if (gdp->debuglevel > 1)
	    debugprintf("gotcircuit: copied circuit name\n") ;
#endif

	if ((cp = strpbrk(subname,". \t(")) != NULL)
	    *cp = '\0' ;

#if	CF_DEBUG
	if (gdp->debuglevel > 1)
	    debugprintf("gotcircuit: subname=%s\n",subname) ;
#endif

	if (gdp->debuglevel > 0) bprintf(gdp->efp,
		"%s: subcircuit name \"%s\"\n",
		gdp->progname,subname) ;

#if	CF_DEBUG
	if (strcasecmp(subname,"skewx5") == 0)
	bprintf(gdp->efp,
		"%s: subcircuit name \"%s\"\n",
		gdp->progname,subname) ;
#endif

/* make a new circuit descriptor, 'blockstart' is set */

	if ((cirp = mkcircuit(subname,-1,type,sl)) == NULL)
	    goto bad ;

#if	CF_DEBUG
	if (gdp->debuglevel > 1)
	    debugprintf("gotcircuit: made a circuit sl=%d %08lX %08lX\n",
	        sl,cirp,gdp->bottom) ;
#endif

/* link this circuit description block into the list of them */

	(gdp->bottom)->next = cirp ;
	gdp->bottom = cirp ;

/* increment the offset in the file */

	offset += linelen ;

/* scan lines until we see another subcircuit or the end of this one ! */

	f_bol = TRUE ;
	f_eol = FALSE ;
	f_exit = FALSE ;
	while ((! (f_exit && f_eol)) &&
	    ((len = breadline(gdp->ifp,gdp->buf,BUFLEN)) > 0)) {

	    l = len ;
	    f_eol = FALSE ;
	    if (gdp->buf[l - 1] == '\n')
	        ((l -= 1), (f_eol = TRUE)) ;

	    gdp->buf[l] = '\0' ;

#if	CF_DEBUG
	    if (gdp->debuglevel > 1) {

	        debugprintf("gotcircuit: looping\n") ;

	        debugprintf("gotcircuit: >%s<\n",gdp->buf) ;

	    }
#endif

	    if (f_bol) {

#if	CF_DEBUG
	        if (gdp->debuglevel > 1)
	            debugprintf("gotcircuit: top of BOL sl=%d\n",sl) ;
#endif

	        cp = gdp->buf ;
	        while (ISWHITE(*cp)) cp += 1 ;

	        f_cktsub = FALSE ;
	        f_cktmain = (strncasecmp(cp,".main",5) == 0) ;

	        if (! f_cktmain)
	            f_cktsub = (strncasecmp(cp,".subckt",7) == 0) ;

#if	CF_DEBUG
	        if (gdp->debuglevel > 1)
	            debugprintf("gotcircuit: about to decide new circuit sl=%d\n",
	                sl) ;
#endif

/* do we have a subcircuit ? */

	        if (f_cktmain || f_cktsub) {

#if	CF_DEBUG
	            if (gdp->debuglevel > 1) {

	                debugprintf("gotcircuit: new circuit again\n") ;

	                debugprintf("gotcircuit: >%s<\n",gdp->buf) ;

	            }
#endif

	            type = CIR_MAIN ;
	            if (f_cktsub) type = CIR_SUB ;

/* we got a new subcircuit, finish off the block of this circuit */

	            if (addblock(cirp,blockstart,blen) < 0)
	                goto bad ;

#if	CF_DEBUG
	            if (gdp->debuglevel > 1) debugprintf(
	                "gotcircuit: closed off this block o=%ld bl=%d\n",
	                blockstart,blen) ;
#endif

/* start the new circuit */

#if	CF_DEBUG
	if (gdp->debuglevel > 1) debugprintf(
	"gotcircuit: calling 'gotcircuit' linelen=%d\n",len) ;
#endif

	            l = cp - gdp->buf ;
	            if ((offset = gotcircuit(gdp,offset,gdp->buf,len,
	                l,type,sl + 1)) < 0)
	                return BAD ;

#if	CF_DEBUG
	            if (gdp->debuglevel > 1) debugprintf(
	                "gotcircuit: returned from 'gotcircuit' o=%ld\n",
	                offset) ;
#endif

/* think about it, it is pretty neat that we do NOT have to do this seek */

#ifdef	COMMENT
	            bseek(gdp->ifp,offset,SEEK_SET) ;
#endif

	            blockstart = offset ;
	            blen = 0 ;
	            len = 0 ;

	        } else if ((strncasecmp(cp,".finis",6) == 0) ||
	            (strncasecmp(cp,".end",4) == 0)) {

#if	CF_DEBUG
	            if (gdp->debuglevel > 1)
	                debugprintf("gotcircuit: end of circuit sl=%d\n",
	                    sl) ;
#endif

	            f_finis = TRUE ;
	            f_exit = TRUE ;

	        } /* end if (what type of action) */

#if	CF_DEBUG
	        if (gdp->debuglevel > 1)
	            debugprintf("gotcircuit: end of BOL sl=%d\n",
	                sl) ;
#endif

	    } /* end if (BOL) */

#if	CF_DEBUG
	    if (gdp->debuglevel > 1)
	        debugprintf("gotcircuit: bottom of while sl=%d\n",sl) ;
#endif

	    offset += len ;
	    blen += len ;
	    if ((len > 0) && f_eol) line += 1 ;

	    f_bol = f_eol ;

	} /* end while */

/* store the last block */

	if (addblock(cirp,blockstart,blen) < 0)
	    goto bad ;

	cirp->lines = line ;
	return offset ;

bad:
	if (gdp->debuglevel > 2) 
		debugprintf( "gotcircuit: got a bad circuit discovered\n") ;

	return BAD ;

badwrite:
	return BAD ;
}
/* end subroutine (gotcircuit) */


/* get circuit descriptor */

struct circuit *mkcircuit(name,nlen,type,sl)
char	*name ;
int	nlen, type ;
int	sl ;
{
	struct circuit	*cirp ;

	char		*np ;


	if (nlen > 0)
	    name[nlen] = '\0' ;

	else
	    nlen = (int) strlen(name) ;

	if ((np = malloc_str(name)) == NULL)
	    return NULL ;

	if ((cirp = (struct circuit *) malloc(sizeof(struct circuit))) == NULL)
	    return NULL ;

	cirp->next = NULL ;
	cirp->bp = NULL ;
	cirp->name = np ;
	cirp->nlen = nlen ;
	cirp->type = type ;
	cirp->sl = sl ;
	return cirp ;
}
/* end subroutine (mkcircuit) */


/* add a block to the circuit */

int addblock(cirp,offset,len)
struct circuit	*cirp ;
offset_t	offset ;
long	len ;
{
	struct block	*hbp, *bp ;

	char		*np ;


	if (len <= 0) return OK ;

	if ((bp = (struct block *) malloc(sizeof(struct block))) == NULL)
	    return BAD ;

	bp->next = NULL ;
	bp->start = offset ;
	bp->len = len ;
	if (cirp->bp == NULL) {

	    cirp->bp = bp ;

	} else {

	    hbp = cirp->bp ;
	    while (hbp->next != NULL) hbp = hbp->next ;

	    hbp->next = bp ;
	}

	return OK ;
}
/* end subroutine (addblock) */


#ifdef	COMMENT

/* delete a block sub-list from a point in a block list */

void deleteblock(be)
struct block	*be ;
{


	if (be == NULL) return ;

	if (be->next != NULL) deleteblolk(be->next) ;

	free(be) ;

}
/* end subroutine (deleteblock) */

#endif


/* write out a circuit to the output file */

int writecir(gdp,cirp)
struct global	*gdp ;
struct circuit	*cirp ;
{
	bfile		subfile, *sfp ;

	struct block	*bp ;

	int		i, len, sl = cirp->sl, rs ;

	char		subfname[MAXPATHLEN + 1] ;
	char		namebuf[MAXPATHLEN + 1] ;


#if	CF_DEBUG
	if (gdp->debuglevel > 1)
	    debugprintf("writecir: entered cir=%s\n",cirp->name) ;
#endif

/* if we are in "separate" mode, then create the new file */

	if (gdp->f.separate) {

#if	CF_DEBUG
	    if (gdp->debuglevel > 1)
	        debugprintf("writecir: separated\n") ;
#endif

/* convert the name to something a little nicer */

	    for (i = 0 ; cirp->name[i] != '\0' ; i += 1) {

	        if (cirp->name[i] == '/')
	            subfname[i] = '_' ;

	        else
	            subfname[i] = tolower(cirp->name[i]) ;

	    } /* end for */

	    subfname[i] = '\0' ;
	    if ((gdp->suffix != NULL) && (gdp->suffix[0] != '\0')) {

	        subfname[i++] = '.' ;
	        strcpy(subfname + i,gdp->suffix) ;

	    }

	    sfp = &subfile ;
	    if ((rs = bopen(sfp,subfname,"wct",0666)) < 0)
	        return rs ;

	} else {

#if	CF_DEBUG
	    if (gdp->debuglevel > 1)
	        debugprintf("writecir: not separated\n") ;
#endif

	    sfp = gdp->ofp ;

	} /* end if (determining output mode) */

/* write out the header commnets */

	if (strcasecmp(cirp->name,"envelope") == 0)
	    bprintf(sfp,
	        "* start circuit \"envelope\" (lines=%d)\n\n",
	        cirp->lines + 6) ;

	else
	    bprintf(sfp,
	        "* start circuit \"%s\" (level=%d, lines=%d)\n\n",
	        cirp->name,sl,cirp->lines + 6) ;

/* loop through the blocks of this circuit, writing them out */

	bp = cirp->bp ;
	while (bp != NULL) {

#if	CF_DEBUG
	    if (gdp->debuglevel > 1)
	        debugprintf("writecir: got a block start=%ld len=%d\n",
	            bp->start,bp->len) ;
#endif

#if	CF_DEBUG
	    if (gdp->debuglevel > 2) {

	        debugprintf(
	            "writecir: writing circuit \"%s\" block s=%ld l=%d\n",
	            cirp->name,bp->start,bp->len) ;

	        if ((rs = bseek(gdp->ifp,bp->start,SEEK_SET)) < 0)
	            debugprintf("writecir: error from seek (rs %d)\n",
	                rs) ;

	        while ((len = breadline(gdp->ifp,namebuf,MAXPATHLEN)) > 0) {

	            if (namebuf[len - 1] == '\n') len -= 1 ;

	            namebuf[len] = '\0' ;
	            debugprintf("writecir: >%s<\n",namebuf) ;

	        }
	    }
#endif

	    bseek(gdp->ifp,bp->start,SEEK_SET) ;

	    if ((rs = bcopyblock(gdp->ifp,sfp,(int) bp->len)) < 0)
	        goto badwrite ;

	    bp = bp->next ;

	} /* end while */

/* write out the trailer commnets */

	if (strcasecmp(cirp->name,"envelope") == 0)
	    bprintf(sfp,
	        "\n* end circuit \"envelope\"\n\n\n") ;

	else
	    bprintf(sfp,
	        "\n* end circuit \"%s\" (level=%d)\n\n\n",
	        cirp->name,sl) ;

	rs = OK ;

#if	CF_DEBUG
	if (gdp->debuglevel > 1)
	    debugprintf("writecir: exiting normally\n") ;
#endif

done:
	if (gdp->f.separate)
	    bclose(sfp) ;

	else
	    bflush(sfp) ;

	return rs ;

badwrite:
	rs = BAD ;
	goto done ;
}
/* end subroutine (writecir) */


/* set if we have a circuit match */
int cirtypematch(cirtype,cirp)
char		cirtype[] ;
struct circuit	*cirp ;
{


	if (cirtype[0] == 0) return TRUE ;

	if (BATST(cirtype,cirp->type)) return TRUE ;

	return FALSE ;
}
/* end subroutine (cirtypematch) */


