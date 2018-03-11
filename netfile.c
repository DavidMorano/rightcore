/* netfile */

/* read a NETRC file and make its contents available */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-09-22, David A­D­ Morano
	This subroutine module was adopted for use from a previous NETRC
	reading program.  This version currently ignores all 'macdef' entries.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	OK, here is the deal.  We ignore the 'macdef' key totally except to
	process the fact that it has a value (which is supposed to be the name
	of the newly defined macro) and the actual definition (or body of the
	macro) on the next line.

	If there is no 'machine' key associated with some 'login' key, then we
	make a fake NULL machine grouping, but only one of these within the
	whole 'netrc' file!

	If there are more than one 'login', 'account', or what have you, key
	associated with a single 'machine' key, then we ignore all but the
	first.

	Finally, this may be more (except for the 'macdef' key) than what the
	standard FTP and 'rexec(3)' NETRC file parsers do!


*******************************************************************************/


#define	CF_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecitem.h>
#include	<bfile.h>
#include	<field.h>
#include	<localmisc.h>

#include	"netfile.h"


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	KEYBUFLEN	10

#define	NETSTATE	struct netstate


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	snwcpylc(char *,int,const char *,int) ;
extern int	snwcpyuc(char *,int,const char *,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;


/* external variables */


/* local structures */

enum netitems {
	netitem_machine,
	netitem_login,
	netitem_password,
	netitem_account,
	netitem_overlast
} ;

struct netstate {
	int		c ;
	const char	*item[netitem_overlast] ;
} ;


/* forward references */

int		netfile_close(NETFILE *) ;

static int	netfile_parse(NETFILE *,NETSTATE *,const char *) ;
static int	netfile_item(NETFILE *,NETSTATE *,int,cchar *,int) ;

static int	netstate_start(NETSTATE *) ;
static int	netstate_reset(NETSTATE *) ;
static int	netstate_item(NETSTATE *,int,const char *,int) ;
static int	netstate_ready(NETSTATE *) ;
static int	netstate_finish(NETSTATE *) ;

static int	entry_start(NETFILE_ENT *,NETSTATE *) ;
static int	entry_finish(NETFILE_ENT *) ;

static int	getnii(int) ;


/* local variables */

static const unsigned char 	fterms[32] = {
	0x7F, 0xFE, 0xC0, 0xFE,
	0x8B, 0x00, 0x00, 0x24, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x80,
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 
} ;

static const char *netkeys[] = {
	"machine",
	"login",
	"username",
	"password",
	"account",
	"macdef",
	"default",
	NULL
} ;

enum netkeys {
	netkey_machine,
	netkey_login,
	netkey_username,
	netkey_password,
	netkey_account,
	netkey_macdef,
	netkey_default,
	netkey_overlast
} ;

static const int	readies[] = {
	netitem_machine,
	netitem_login,
	-1
} ;


/* exported subroutines */


int netfile_open(NETFILE *vep,cchar netfname[])
{
	int		rs ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUGS
	debugprintf("netfile_open: ent fname=%s\n",netfname) ;
#endif

	if (vep == NULL)
	    return SR_FAULT ;

/* initialize */

	if ((rs = vecitem_start(vep,10,0)) >= 0) {
	    struct ustat	sb ;
	    if ((rs = u_stat(netfname,&sb)) >= 0) {
	        if (! S_ISDIR(sb.st_mode)) {
	            NETSTATE	ns ;
	            if ((rs = netstate_start(&ns)) >= 0) {
	                rs = netfile_parse(vep,&ns,netfname) ;
	                c = rs ;
	                rs1 = netstate_finish(&ns) ;
	                if (rs >= 0) rs = rs1 ;
	            } /* end if (netstate) */
	        } else
	            rs = SR_ISDIR ;
	    } /* end if (stat) */
	    if (rs < 0)
	        vecitem_finish(vep) ;
	} /* end if (vecitem) */

#if	CF_DEBUGS
	debugprintf("netfile_open: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (netfile_open) */


/* close this netfile data structure */
int netfile_close(NETFILE *vep)
{
	NETFILE_ENT	*ep ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; vecitem_get(vep,i,&ep) >= 0 ; i += 1) {
	    if (ep != NULL) {
	        rs1 = entry_finish(ep) ;
	        if (rs >= 0) rs = rs1 ;
	    }
	} /* end while */

	rs1 = vecitem_finish(vep) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (netfile_close) */


/* get an entry */
int netfile_get(NETFILE *vep,int i,NETFILE_ENT **epp)
{
	int		rs ;

	rs = vecitem_get(vep,i,epp) ;

	return rs ;
}
/* end subroutine (netfile_get) */


/* private subroutines */


static int netfile_parse(NETFILE *vep,NETSTATE *nsp,cchar netfname[])
{
	bfile		ourfile, *nfp = &ourfile ;
	int		rs ;
	int		rs1 ;
	int		c = 0 ;
	if ((rs = bopen(nfp,netfname,"r",0664)) >= 0) {
	    FIELD	fsb ;
	    const int	llen = LINEBUFLEN ;
	    int		len ;
	    int		ch ;
	    int		f_macdef = FALSE ;
	    char	lbuf[LINEBUFLEN + 1] ;
	    char	keybuf[KEYBUFLEN+1] ;
	    while ((rs = breadline(nfp,lbuf,llen)) > 0) {
	        len = rs ;
	        if (lbuf[len - 1] != '\n') {
	            while ((ch = bgetc(nfp)) >= 0) {
	                if (ch == '\n') break ;
	            }
	            continue ;
	        }
	        len -= 1 ;
#if	CF_DEBUGS
	        debugprintf("netfile_open: line> %t",lbuf,len) ;
#endif
	        if (f_macdef) {
	            if (len == 0) f_macdef = FALSE ;
	            continue ;
	        }
	        if ((rs = field_start(&fsb,lbuf,len)) >= 0) {
	            int		f_default = FALSE ;
	            int		fl ;
	            const char	*fp ;
	            while ((fl = field_get(&fsb,fterms,&fp)) > 0) {
	                int	ml = MIN(KEYBUFLEN,fl) ;
	                int	nki ;
	                strwcpylc(keybuf,fp,ml) ;
	                if ((nki = matpstr(netkeys,2,keybuf,ml)) >= 0) {
	                    int		cl = 0 ;
	                    cchar	*cp = NULL ;
	                    switch (nki) {
	                    case netkey_machine:
	                    case netkey_login:
	                    case netkey_username:
	                    case netkey_password:
	                    case netkey_account:
	                        if (fsb.term == '#') break ;
	                        if ((cl = field_get(&fsb,fterms,&fp)) >= 0) {
	                            cp = fp ;
	                        }
	                        break ;
	                    case netkey_macdef:
	                        f_macdef = TRUE ;
	                        break ;
	                    case netkey_default:
	                        f_default = TRUE ;
	                        break ;
	                    } /* end switch */
	                    if ((nki >= 0) && (! f_macdef) && (! f_default)) {
	                        rs = netfile_item(vep,nsp,nki,cp,cl) ;
	                    }
	                } /* end if (keyword) */
	                if (f_macdef || f_default) break ;
	                if (fsb.term == '#') break ;
	                if (rs < 0) break ;
	            } /* end while (processing keys) */
	            field_finish(&fsb) ;
	        } /* end if (field) */
		if (rs < 0) break ;
	    } /* end while (reading lines) */
	    if (rs >= 0) {
	        if ((rs = netfile_item(vep,nsp,-1,NULL,0)) >= 0) {
	            c = vecitem_count(vep) ;
		}
	    }
	    rs1 = bclose(nfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (file) */
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (netfile_parse) */


static int netfile_item(NETFILE *vep,NETSTATE *nsp,int nki,cchar *sp,int sl)
{
	int		rs = SR_OK ;
	int		nii = getnii(nki) ;

#if	CF_DEBUGS
	debugprintf("netfile_item: ent nii=%d s=>%t<\n",nii,sp,sl) ;
#endif

	if ((nii == netitem_machine) || (nii < 0)) {
#if	CF_DEBUGS
	debugprintf("netfile_item: machine\n") ;
#endif
	    if (netstate_ready(nsp) > 0) {
		NETFILE_ENT	e ;
#if	CF_DEBUGS
	debugprintf("netfile_item: ready m=%s\n",nsp->item[0]) ;
	debugprintf("netfile_item: ready l=%s\n",nsp->item[1]) ;
#endif
	        if ((rs = entry_start(&e,nsp)) >= 0) {
		    const int	esize = sizeof(NETFILE_ENT) ;
	            if ((rs = vecitem_add(vep,&e,esize)) >= 0) {
	                int	ei = rs ;
	                rs = netstate_reset(nsp) ;
	                if (rs < 0)
	                    vecitem_del(vep,ei) ;
	            } /* end if (vecitem_add) */
	            if (rs < 0)
	                entry_finish(&e) ;
	        } /* end if (entry_start) */
	    } /* end if (netstate_ready) */
	} /* end if */

	if ((rs >= 0) && (nii >= 0) && (sp != NULL)) {
#if	CF_DEBUGS
	debugprintf("netfile_item: insert\n") ;
#endif
	    rs = netstate_item(nsp,nii,sp,sl) ;
	}

	return rs ;
}
/* end subroutine (netfile_item) */


static int netstate_start(NETSTATE *nsp)
{

	memset(nsp,0,sizeof(NETSTATE)) ;

	return SR_OK ;
}
/* end subroutine (netstate_start) */


static int netstate_reset(NETSTATE *nsp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; i < netitem_overlast ; i += 1) {
	    if (nsp->item[i] != NULL) {
	        rs1 = uc_free(nsp->item[i]) ;
	        if (rs >= 0) rs = rs1 ;
	        nsp->item[i] = NULL ;
	    }
	} /* end for */

	nsp->c = 0 ;
	return rs ;
}
/* end subroutine (netstate_reset) */


static int netstate_item(NETSTATE *nsp,int ki,cchar *sp,int sl)
{
	int		rs = SR_OK ;

	if ((ki < 0) || (ki >= netitem_overlast))
	    return SR_INVALID ;

	if (sp != NULL) {
	    const char	*cp ;

	    if (nsp->item[ki] != NULL) {
	        uc_free(nsp->item[ki]) ;
	        nsp->item[ki] = NULL ;
	    }

	    if ((rs = uc_mallocstrw(sp,sl,&cp)) >= 0) {
	        nsp->item[ki] = cp ;
	        nsp->c += 1 ;
	    }

	} /* end if (non-null) */

	return rs ;
}
/* end subroutine (netstate_item) */


static int netstate_ready(NETSTATE *nsp)
{
	int		i ;
	const char	*sp ;

	for (i = 0 ; readies[i] >= 0 ; i += 1) {
	    sp = nsp->item[i] ;
	    if ((sp == NULL) || (sp[0] == '\0')) break ;
	} /* end for */

#if	CF_DEBUGS
	debugprintf("netstate_ready: i=%u readies=%d\n",i,readies[i]) ;
#endif

	return (readies[i] >= 0) ? FALSE : TRUE ;
}
/* end subroutine (netstate_ready) */


static int netstate_finish(NETSTATE *nsp)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		i ;

	for (i = 0 ; i < netitem_overlast ; i += 1) {
	    if (nsp->item[i] != NULL) {
	        rs1 = uc_free(nsp->item[i]) ;
	        if (rs >= 0) rs = rs1 ;
	        nsp->item[i] = NULL ;
	    }
	} /* end for */

	nsp->c = 0 ;
	return rs ;
}
/* end subroutine (netstate_finish) */


static int entry_start(NETFILE_ENT *ep,NETSTATE *nsp)
{
	const int	size = sizeof(NETFILE_ENT) ;
	int		i ;

	memset(ep,0,size) ;

	for (i = 0 ; i < netitem_overlast ; i += 1) {
	    switch (i) {
	    case netitem_machine:
	        ep->machine = nsp->item[i] ;	/* transfer */
	        break ;
	    case netitem_login:
		ep->login = nsp->item[i] ;	/* transfer */
	        break ;
	    case netitem_password:
	        ep->password = nsp->item[i] ;	/* transfer */
	        break ;
	    case netitem_account:
	        ep->account = nsp->item[i] ;	/* transfer */
	        break ;
	    } /* end switch */
	    nsp->item[i] = NULL ;
	} /* end for */

	return SR_OK ;
}
/* end subroutine (entry_start) */


static int entry_finish(NETFILE_ENT *mep)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (mep->machine != NULL) {
	    rs1 = uc_free(mep->machine) ;
	    if (rs >= 0) rs = rs1 ;
	    mep->machine = NULL ;
	}

	if (mep->login != NULL) {
	    rs1 = uc_free(mep->login) ;
	    if (rs >= 0) rs = rs1 ;
	    mep->login = NULL ;
	}

	if (mep->password != NULL) {
	    rs1 = uc_free(mep->password) ;
	    if (rs >= 0) rs = rs1 ;
	    mep->password = NULL ;
	}

	if (mep->account != NULL) {
	    rs1 = uc_free(mep->account) ;
	    if (rs >= 0) rs = rs1 ;
	    mep->account = NULL ;
	}

	return rs ;
}
/* end subroutine (entry_finish) */


static int getnii(int nki)
{
	int		nii = -1 ;

	switch (nki) {
	case netkey_machine:
	    nii = netitem_machine ;
	    break ;
	case netkey_login:
	case netkey_username:
	    nii = netitem_login ;
	    break ;
	case netkey_password:
	    nii = netitem_password ;
	    break ;
	case netkey_account:
	    nii = netitem_account ;
	    break ;
	} /* end switch */

	return nii ;
}
/* end subroutine (getnii) */


