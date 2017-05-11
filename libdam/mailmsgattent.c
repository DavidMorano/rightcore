/* mailmsgattent */

/* extra subroutines for the 'mailmsgattent' object */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-03-01, David A­D­ Morano
        The subroutine was written from scratch to do what the previous program
        by the same name did.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These subroutines are extra methods to the MAILMSGATTENT object. See the
        code for details!


*******************************************************************************/


#define	MAILMSGATTENT_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"mailmsgattent.h"
#include	"contentencodings.h"
#include	"ismmclass.h"


/* local defines */

#define	MAILMSGATTENT_TMPCNAME	"mkmsgXXXXXXXXX"

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	mktmpfile(char *,mode_t,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */

int		mailmsgattent_analyze(MAILMSGATTENT *,const char *) ;

static int	mailmsgattent_analyzer(MAILMSGATTENT *,bfile *,bfile *) ;


/* local variables */


/* exported subroutines */


int mailmsgattent_code(MAILMSGATTENT *ep,cchar *tmpdname)
{
	int		rs = SR_OK ;
	int		code = 0 ;

#if	CF_DEBUGS
	debugprintf("mailmsgattent_code: ent file=%s type=%s\n",
	    ep->attfname,ep->type) ;
#endif

	if (ep->type != NULL) {
	    rs = mailmsgattent_isplaintext(ep) ;
	} else {
	    ep->f_plaintext = TRUE ;
	}

	if (! ep->f_plaintext) code = CE_BASE64 ;

#if	CF_DEBUGS
	debugprintf("mailmsgattent_code: default_code=%d\n",code) ;
	debugprintf("mailmsgattent_code: encoding=%s\n",ep->encoding) ;
	debugprintf("mailmsgattent_code: f_plaintext=%d\n",ep->f_plaintext) ;
#endif

	if ((rs >= 0) && (ep->encoding == NULL)) {

#if	CF_DEBUGS
	debugprintf("mailmsgattent_code: need coding\n") ;
#endif

	    if (ep->f_plaintext) {

#if	CF_DEBUGS
	debugprintf("mailmsgattent_code: plaintext\n") ;
#endif

	        if ((rs = mailmsgattent_analyze(ep,tmpdname)) >= 0) {
	            code = rs ;
#if	CF_DEBUGS
	debugprintf("mailmsgattent_code: _analyze() rs=%d\n",rs) ;
#endif
	            if ((code >= CE_7BIT) && (code < CE_OVERLAST)) {
			cchar	*enc = contentencodings[code] ;
			cchar	*cp ;
			if ((rs = uc_mallocstrw(enc,-1,&cp)) >= 0) {
	                    ep->encoding = cp ;
			}
		    }
		}

	    } else {
	        cchar	*enc = contentencodings[contentencoding_base64] ;
		cchar	*cp ;
	        code = CE_BASE64 ;
		if ((rs = uc_mallocstrw(enc,-1,&cp)) >= 0) {
	            ep->encoding = cp ;
		}
	    } /* end if */

	} else {
	    int		si ;
	    cchar	*enc = ep->encoding ;
	    if ((si = matstr(contentencodings,enc,-1)) >= 0) {
	        code = si ;
	    }
	}

#ifdef	COMMENT
	if ((rs < 0) || (code == 0)) {

	    if (ep->encoding != NULL) {
	        uc_free(ep->encoding) ;
	        ep->encoding = NULL ;
	    }

	    if (rs >= 0) {
	        cchar	*enc = contentencodings[contentencoding_base64] ;
		cchar	*cp ;
	        code = CE_BASE64 ;
		if ((rs = uc_mallocstrw(enc,-1,&cp)) >= 0) {
	            ep->encoding = cp ;
		}
	    }

	} /* end if (error) */
#else /* COMMENT */
	if (rs < 0) {
	    if (ep->encoding != NULL) {
	        uc_free(ep->encoding) ;
	        ep->encoding = NULL ;
	    }
	}
#endif /* COMMENT */

#if	CF_DEBUGS
	debugprintf("mailmsgattent_code: ret rs=%d code=%d encoding=%s\n",
	    rs,code,ep->encoding) ;
#endif

	if (rs >= 0)  {
	    ep->cte = code ;
	}

	return (rs >= 0) ? code : rs ;
}
/* end subroutine (mailmsgattent_code) */


int mailmsgattent_setcode(MAILMSGATTENT *ep,int code)
{
	int		rs = SR_OK ;
	if ((code >= 0) && (code < CE_OVERLAST)) {
	    cchar	*enc ;
	    ep->cte = code ;
	    if (ep->encoding == NULL) {
		cchar	*cp ;
	        enc = contentencodings[code] ;
		if ((rs = uc_mallocstrw(enc,-1,&cp)) >= 0) {
	            ep->encoding = cp ;
		}
	    }
	} else {
	    rs = SR_INVALID ;
	}
	return rs ;
}
/* end subroutine (mailmsgattent_setcode) */


/* analyze (as best as we can) an attachment */
int mailmsgattent_analyze(MAILMSGATTENT *ep,cchar *tmpdname)
{
	bfile		infile, *ifp = &infile ;
	int		rs ;
	int		rs1 ;
	int		code = 0 ;
	const char	*atf = ep->attfname ;

	if ((atf == NULL) || (atf[0] == '\0') || (atf[0] == '-'))
	    atf = BFILE_STDIN ;

	if ((rs = bopen(ifp,atf,"r",0666)) >= 0) {
	    bfile_stat	sb ;
	    if ((rs = bcontrol(ifp,BC_STAT,&sb)) >= 0) {
		bfile	auxfile, *afp = &auxfile ;
		int	f_needaux = TRUE ;
		char	auxfname[MAXPATHLEN+1] ;

		auxfname[0] = '\0' ;
	        if (S_ISREG(sb.st_mode)) {
#if	CF_DEBUGS
		{
	        debugprintf("mailmsgattent_analyze: needaux=%d\n",f_needaux) ;
		}
#endif
	            ep->clen = (int) sb.st_size ;
	            f_needaux = FALSE ;
	        }

#if	CF_DEBUGS
	        debugprintf("mailmsgattent_analyze: needaux=%d\n",f_needaux) ;
#endif

	        if (f_needaux) {
	            const char	*tmpcname = MAILMSGATTENT_TMPCNAME ;
	            char	tmpfname[MAXPATHLEN + 1] ;

	            if (tmpdname == NULL)
	                rs = SR_FAULT ;

	            if (rs >= 0) {
	                if (tmpdname[0] != '\0') {
	                    rs = mkpath2(tmpfname,tmpdname,tmpcname) ;
	                } else {
	                    rs = mkpath1(tmpfname,tmpcname) ;
			}
	            }

	            if (rs >= 0) {
	                if ((rs = mktmpfile(auxfname,0660,tmpfname)) >= 0) {
			    cchar	*afn = auxfname ;
			    cchar	*cp ;
			    if ((rs = uc_mallocstrw(afn,-1,&cp)) >= 0) {
				ep->auxfname = cp ;
	                        rs = bopen(afp,ep->auxfname,"wct",0666) ;
	                        if (rs < 0) {
	                            uc_free(ep->auxfname) ;
	                            ep->auxfname = NULL ;
	                        }
			    } /* end if (m-a) */
	                    if (rs < 0) {
	                        uc_unlink(auxfname) ;
	                        auxfname[0] = '\0' ;
	                    }
	                } /* end if (mktmpfile) */
	            } /* end if (ok) */

	        } /* end if (needed an auxillary file) */

/* finally! perform the analysis */

	        if (rs >= 0) {

	            if (! f_needaux) afp = NULL ;
	            rs = mailmsgattent_analyzer(ep,afp,ifp) ;
		    code = rs ;

#if	CF_DEBUGS
	            debugprintf("mailmsgattent_analyze: _analyzer() rs=%d\n",
			rs) ;
#endif
	            if (rs < 0) {
	                if (ep->auxfname != NULL) {
	                    uc_free(ep->auxfname) ;
	                    ep->auxfname = NULL ;
	                }
	                if (auxfname[0] != '\0')
	                    u_unlink(auxfname) ;
	            }
	        }

	        if (f_needaux) {
	            if (afp != NULL) bclose(afp) ;
	        } else {
	            bseek(ifp,0L,SEEK_SET) ;
		}

	    } /* end if (stat) */
	    rs1 = bclose(ifp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (file-open) */

#if	CF_DEBUGS
	debugprintf("mailmsgattent_analyze: ret rs=%d ce=%u\n",rs,code) ;
#endif

	return (rs >= 0) ? code : rs ;
}
/* end subroutine (mailmsgattent_analyze) */


static int mailmsgattent_analyzer(MAILMSGATTENT *ep,bfile *afp,bfile *ifp)
{
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		lines = 0 ;
	int		len ;
	int		ch ;
	int		i ;
	int		clen = 0 ;
	int		code = 0 ;
	char		lbuf[LINEBUFLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("mailmsgattent_analyzer: ent\n") ;
#endif

	while ((rs = breadline(ifp,lbuf,llen)) > 0) {
	    len = rs ;

	    if (code < CE_BINARY) {
	        for (i = 0 ; i < len ; i += 1) {
	            ch = MKCHAR(lbuf[i]) ;

	            if (ismmclass_binary(ch)) {
	                code = CE_BINARY ;
	            } else if ((code < CE_8BIT) && ismmclass_8bit(ch)) {
	                code = CE_8BIT ;
	            }

#if	CF_DEBUGS
		    debugprintf("mailmsgattent_analyzer: ch=%02x code=%d\n",
			ch,code) ;
#endif

		    if (code >= CE_BINARY) break ;
	        } /* end for (class characterization) */
	    } /* end if (continue) */

	    if (afp != NULL) {
	        rs = bwrite(afp,lbuf,len) ;
	    } else {
	        if (code >= CE_BINARY) break ;
	    }

	    clen += len ;
	    if (lbuf[len - 1] == '\n') lines += 1 ;

	    if (rs < 0) break ;
	} /* end while */

	if (rs >= 0) {
	    if (ep->clen < 0) ep->clen = clen ;
	    ep->clines = (code < CE_BINARY) ? lines : -1 ;
	}

#if	CF_DEBUGS
	debugprintf("mailmsgattent_analyzer: ret rs=%d code=%u\n",rs,code) ;
#endif

	return (rs >= 0) ? code : rs ;
}
/* end subroutine (mailmsgattent_analyzer) */


