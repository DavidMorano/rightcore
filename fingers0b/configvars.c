/* configvars */


#define	CF_DEBUGS		0	/* compile-time debugging */
#define	CF_DEBUGSFIELD		0
#define	CF_ALLOCFILENAME	1


/* revision history:

	= 1998-06-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is an object that reads configuration files and organizes the
        content into the object for structured access.


*******************************************************************************/


#define	CONFIGVARS_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<char.h>
#include	<field.h>
#include	<mallocstuff.h>
#include	<localmisc.h>

#include	"configvars.h"


/* local defines */

#define	CONFIGVARS_WSETS	0
#define	CONFIGVARS_WVARS	1

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#undef	BUFLEN
#define	BUFLEN		(LINEBUFLEN * 2)

#define	KEYBUFLEN	20

#define	ISCONT(b,bl)	\
	(((bl) >= 2) && ((b)[(bl) - 1] == '\n') && ((b)[(bl) - 2] == '\\'))


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

int		configvars_addfile(CONFIGVARS *,const char *,VECITEM *) ;

static int	configvars_parsefile(CONFIGVARS *,int,VECITEM *) ;
static int	configvars_finishallvars(CONFIGVARS *) ;
static int	configvars_finishfiles(CONFIGVARS *) ;
static int	configvars_addvar(CONFIGVARS *,int,int,char *,int,char *,int) ;

static int	file_start(CONFIGVARS_FILE *,const char *) ;
static int	file_addvar(CONFIGVARS_FILE *,int,int,char *,int,char *,int) ;
static int	file_finish(CONFIGVARS_FILE *) ;

#ifdef	COMMENT
static int	bgetcline(bfile *,char *,int,int *) ;
#endif

static int	var_start(CONFIGVARS_VAR *,int,char *,int,char *,int) ;
static int	var_finish(CONFIGVARS_VAR *) ;

static void	badline(vecitem *,char *,int) ;
static void	freeit() ;


/* local variables */

static const unsigned char 	fterms[32] = {
	0x00, 0x1B, 0x00, 0x00,
	0x01, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

static const char	*configkeys[] = {
	"define",
	"export",
	"set",
	"unset",
	NULL
} ;

enum configkeys {
	configkey_define,
	configkey_export,
	configkey_set,
	configkey_unset,
	configkey_overlast
} ;

enum vartypes {
	vartype_set,
	vartype_var,
	vartype_export,
	vartype_define,
	vartype_unset,
	vartype_overlast
} ;


/* exported subroutines */


int configvars_open(cvp,cfname,eep)
CONFIGVARS	*cvp ;
const char	cfname[] ;
VECITEM		*eep ;
{
	int	rs = SR_OK ;


	if (cvp == NULL)
	    return SR_FAULT ;

	memset(cvp,0,sizeof(CONFIGVARS)) ;
	cvp->magic = 0 ;
	cvp->checktime = time(NULL) ;

/* initialize */

	if ((rs = vecitem_start(&cvp->fes,10,0)) < 0)
	    goto bad1 ;

	if ((rs = vecitem_start(&cvp->defines,10,0)) < 0)
	    goto bad2 ;

	if ((rs = vecitem_start(&cvp->exports,10,0)) < 0)
	    goto bad3 ;

	if ((rs = vecitem_start(&cvp->unsets,10,0)) < 0)
	    goto bad4 ;

	if ((rs = vecitem_start(&cvp->vars,10,VECITEM_PSORTED)) < 0)
	    goto bad5 ;

	if ((rs = vecitem_start(&cvp->sets,10,VECITEM_PSORTED)) < 0)
	    goto bad6 ;

	cvp->magic = CONFIGVARS_MAGIC ;
	if ((cfname != NULL) && (cfname[0] != '\0')) {

	    rs = configvars_addfile(cvp,cfname,eep) ;
		if (rs < 0)
			goto bad7 ;

	}

ret0:
	return rs ;

/* handle bad things */
bad7:
	vecitem_finish(&cvp->sets) ;

bad6:
	vecitem_finish(&cvp->vars) ;

bad5:
	vecitem_finish(&cvp->unsets) ;

bad4:
	vecitem_finish(&cvp->exports) ;

bad3:
	vecitem_finish(&cvp->defines) ;

bad2:
	vecitem_finish(&cvp->fes) ;

bad1:
	cvp->magic = 0 ;
	goto ret0 ;
}
/* end subroutine (configvars_open) */


/* add a file to the list of files */
int configvars_addfile(cvp,cfname,eep)
CONFIGVARS	*cvp ;
const char	cfname[] ;
VECITEM		*eep ;
{
	CONFIGVARS_FILE	fe ;

	int	rs ;
	int	fi ;


	if (cvp == NULL)
	    return SR_FAULT ;

	if (cvp->magic != CONFIGVARS_MAGIC)
	    return SR_NOTOPEN ;

#if	CF_DEBUGS
	debugprintf("configvars_addfile: ent, file=%s\n",cfname) ;
#endif

	if ((cfname == NULL) || (cfname[0] == '\0'))
		return SR_FAULT ;

	if ((rs = vecitem_count(&cvp->fes)) < 0)
	    return rs ;

	if (rs >= sizeof(int))
	    return SR_TOOBIG ;

	rs = file_start(&fe,cfname) ;

	if (rs < 0)
		return rs ;

	if ((rs = vecitem_add(&cvp->fes,&fe,sizeof(CONFIGVARS_FILE))) < 0)
	    goto bad5 ;

	fi = rs ;

/* parse the file for the first time */

#if	CF_DEBUGS
	debugprintf("configvars_addfile: calling configvars_parsefile\n") ;
#endif

	rs = configvars_parsefile(cvp,fi,eep) ;

	if (rs < 0)
		goto bad6 ;

#if	CF_DEBUGS
	debugprintf("configvars_addfile: ret rs=%d\n",rs) ;
#endif

ret:
	return rs ;

/* handle bad things */
bad6:
	vecitem_del(&cvp->fes,fi) ;

bad5:
	file_finish(&fe) ;

bad0:
	return rs ;
}
/* end subroutine (configvars_addfile) */


int configvars_close(cvp)
CONFIGVARS	*cvp ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (cvp == NULL)
	    return SR_FAULT ;

	if (cvp->magic != CONFIGVARS_MAGIC)
	    return SR_NOTOPEN ;

/* free up the complex data types */

	rs1 = configvars_finishallvars(cvp) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = configvars_finishfiles(cvp) ;
	if (rs >= 0) rs = rs1 ;

	cvp->magic = 0 ;
	return rs ;
}
/* end subroutine (configvars_close) */


/* private subroutines */


/* parse a file */
static int configvars_parsefile(cvp,fi,eep)
CONFIGVARS	*cvp ;
int		fi ;
VECITEM		*eep ;
{
	struct ustat	sb ;

	CONFIGVARS_FILE	*fep ;

	CONFIGVARS_VAR	ve ;

	FIELD		fsb ;

	bfile		file, *fp = &file ;

	int	rs = SR_OK ;
	int	i ;
	int	c, type ;
	int	sl, cl, len, line = 0 ;
	int	klen, vlen ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	keybuf[KEYBUFLEN + 1], *bp ;
	char	*key, *value ;
	char	*cp ;


#if	CF_DEBUGS
	debugprintf("configvars_parsefile: ent\n") ;
#endif

/* get the pointer to our own file structure */

	if ((rs = vecitem_get(&cvp->fes,fi,&fep)) < 0)
	    return rs ;

	if (fep == NULL)
	    return SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("configvars_parsefile: 2\n") ;
#endif

	if ((rs = bopen(fp,fep->filename,"r",0664)) < 0)
	    return rs ;

#if	CF_DEBUGS
	debugprintf("configvars_parsefile: bopen() rs=%d\n",
	    rs) ;
#endif

	if ((rs = bcontrol(fp,BC_STAT,&sb)) < 0)
	    goto done ;

/* have we already parsed this one? */

#if	CF_DEBUGS
	debugprintf("configvars_parsefile: 4\n") ;
#endif

	rs = SR_OK ;
	if (fep->mtime >= sb.st_mtime)
	    goto done ;

#if	CF_DEBUGS
	debugprintf("configvars_parsefile: 5\n") ;
#endif

	fep->mtime = sb.st_mtime ;

/* start processing the configuration file */

#if	CF_DEBUGS
	debugprintf("configvars_parsefile: start processing\n") ;
#endif

	while ((rs = breadlines(fp,linebuf,LINEBUFLEN,&line)) > 0) {
	    len = rs ;

	    if (len == 1) continue ;	/* blank line */

	    if (linebuf[len - 1] != '\n') {

#ifdef	COMMENT
	        f_trunc = TRUE ;
#endif
	        while ((c = bgetc(fp)) >= 0) {

	            if (c == '\n') 
			break ;

		}

	        continue ;

	    } else
		len -= 1 ;

/* pre-process this a little bit */

	    cp = linebuf ;
	    cl = len ;
	    while ((cl > 0) && CHAR_ISWHITE(*cp)) {
		cp += 1 ;
		cl -= 1 ;
	    }

	    if ((cl == 0) || (cp[0] == '#'))
		continue ;

/* do this line */

	    fsb.lp = cp ;
	    fsb.rlen = cl ;

#if	CF_DEBUGS && CF_DEBUGSFIELD
	        debugprintf("configvars_parsefile: line> %t\n",
			fsb.lp,fsb.rlen) ;
#endif

	    field_get(&fsb,fterms) ;

#if	CF_DEBUGS && CF_DEBUGSFIELD
	    {
	        if (fsb.flen >= 0) {
	            debugprintf("configvars_parsefile: field> %t\n",
			fsb.fp,fsb.flen) ;
	        } else
	            debugprintf("configvars_parsefile: field> *none*\n") ;

	    }
#endif /* CF_DEBUGSFIELD */

/* empty or comment only line */

	    if (fsb.flen <= 0) continue ;

/* convert key to lower case */

	    cl = MIN(fsb.flen,KEYBUFLEN) ;
	    strwcpylc(keybuf,fsb.fp,cl) ;

/* check if key is a built-in one */

	    i = matostr(configkeys,2,keybuf,cl) ;

	    if (i >= 0) {

#if	CF_DEBUGS
	            debugprintf("configvars_parsefile: keyword=%s\n",
	                configkeys[i]) ;
#endif

	        type = -1 ;
	        switch (i) {

/* unsets */
	        case configkey_unset:
		    type = vartype_unset ;
	            field_get(&fsb,fterms) ;

	            if (fsb.flen > 0) {

	                type = 3 ;
	                rs = file_addvar(fep,type,fi,fsb.fp,fsb.flen,NULL,0) ;

	                if (rs < 0)
	                    goto badalloc ;

	            }

	            break ;

/* export environment */
	        case configkey_set:
			if (type < 0)
				type = vartype_set ;

/* FALLTHROUGH */

	        case configkey_define:
			if (type < 0)
				type = vartype_define ;

/* FALLTHROUGH */

	        case configkey_export:
			if (type < 0)
				type = vartype_export ;

/* get first part */

#if	CF_DEBUGS
	                debugprintf("configvars_parsefile: D/E FP\n") ;
#endif

	            field_get(&fsb,fterms) ;

#if	CF_DEBUGS
	                debugprintf("configvars_parsefile: D/E FP flen=%d\n",
	                    fsb.flen) ;
#endif

	            if (fsb.flen <= 0) {

	                rs = SR_INVALID ;
	                if (eep != NULL)
	                    badline(eep,fep->filename,line) ;

	                break ;
	            }

	            key = fsb.fp ;
	            klen = fsb.flen ;

/* get second part */

	            field_get(&fsb,fterms) ;

	            value = (fsb.flen >= 0) ? fsb.fp : NULL ;
	            vlen = fsb.flen ;

	            if (i == configkey_set) {
	                rs = configvars_addvar(cvp,fi,CONFIGVARS_WSETS,
	                    key,klen,value,vlen) ;

	            } else
	                rs = file_addvar(fep,type,fi,key,klen,value,vlen) ;

	            if (rs < 0)
	                goto badalloc ;

	            break ;

	        default:
	            rs = SR_NOTSUP ;
	            goto badprogram ;

	        } /* end switch */

	    } else {

		int	alen ;

		char	abuf[LINEBUFLEN + 1] ;


/* unknown keyword, it is just another variable ! */

	        key = fsb.fp ;
	        klen = fsb.flen ;

/* store the key along with the remainder of this "line" */

	        rs = configvars_addvar(cvp,fi,CONFIGVARS_WVARS,
	            key,klen,fsb.lp,fsb.rlen) ;

	        if (rs < 0)
	            goto badalloc ;

	    } /* end if */

	} /* end while (reading lines) */

/* we're out of here one way or another! */
badprogram:
badalloc:
badconfig:

/* done with configuration file processing */
done:

#if	CF_DEBUGS
	debugprintf("configvars_parsefile: ret rs=%d\n",
	    rs) ;
#endif

	bclose(fp) ;

	return rs ;
}
/* end subroutine (configvars_parsefile) */


static int configvars_addvar(cvp,fi,w,key,klen,value,vlen)
CONFIGVARS	*cvp ;
int		fi, w ;
char		key[], value[] ;
int		klen, vlen ;
{
	CONFIGVARS_VAR	v ;

	VECITEM		*slp ;

	int	rs ;


	if ((rs = var_start(&v,fi,key,klen,value,vlen)) < 0)
	    return rs ;

	slp = (w) ? &cvp->sets : &cvp->vars ;
	if ((rs = vecitem_add(slp,&v,sizeof(CONFIGVARS_VAR))) < 0)
	    var_finish(&v) ;

	return rs ;
}
/* end subroutine (configvars_addvar) */


/* free up all of the varaiable entries in this CONFIGVARS object */
static int configvars_finishallvars(cvp)
CONFIGVARS	*cvp ;
{
	CONFIGVARS_VAR	*cep ;

	VECITEM		*slp ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i, j ;


	for (j = 0 ; j < 5 ; j += 1) {

	    switch (j) {

	    case vartype_set:
	        slp = &cvp->sets ;
	        break ;

	    case vartype_var:
	        slp = &cvp->vars ;
	        break ;

	    case vartype_export:
	        slp = &cvp->exports ;
	        break ;

	    case vartype_define:
	        slp = &cvp->defines ;
	        break ;

	    case vartype_unset:
	        slp = &cvp->unsets ;
	        break ;

	    } /* end switch */

	    for (i = 0 ; vecitem_get(slp,i,&cep) >= 0 ; i += 1) {
	        if (cep == NULL) continue ;

	        rs1 = var_finish(cep) ;
		if (rs >= 0) rs = rs1 ;

	    } /* end for */

	    rs1 = vecitem_finish(slp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end for */

	return rs ;
}
/* end subroutine (configvars_finishallvars) */


/* free up all of the entries in this CONFIGVARS list associated w/ a file */
static int configvars_finishfes(cvp,fi)
CONFIGVARS	*cvp ;
int		fi ;
{
	CONFIGVARS_VAR	*cep ;

	VECITEM		*slp ;

	int	rs = SR_OK ;
	int	i, j ;


#if	CF_DEBUGS
	debugprintf("configvars_finishfes: want to delete all fi=%d\n",fi) ;
#endif

	for (j = 0 ; j < 2 ; j += 1) {

	    slp = (j == 0) ? &cvp->sets : &cvp->vars ;
	    i = 0 ;
	    while (vecitem_get(slp,i,&cep) >= 0) {

	        if ((cep != NULL) &&
	            ((cep->fi == fi) || (fi < 0))) {

#if	CF_DEBUGS
	            debugprintf("configvars_finishfes: got one\n") ;
#endif

	            var_finish(cep) ;

	            vecitem_del(slp,i) ;

	        } else
	            i += 1 ;

	    } /* end while */

#if	CF_DEBUGS
	    debugprintf("configvars_finishfes: popped up to, rs=%d i=%d\n",
		rs,i) ;
#endif

	} /* end for */

#if	CF_DEBUGS
	debugprintf("configvars_finishfes: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (configvars_finishfes) */


/* free up all of the files in this CONFIGVARS object */
static int configvars_finishfiles(cvp)
CONFIGVARS		*cvp ;
{
	CONFIGVARS_FILE	*cfp ;

	VECITEM		*slp ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;


	slp = &cvp->fes ;
	for (i = 0 ; vecitem_get(slp,i,&cfp) >= 0 ; i += 1) {
	    if (cfp == NULL) continue ;

	    rs1 = file_finish(cfp) ;
	    if (rs >= 0) rs = rs1 ;

	} /* end for */

	rs1 = vecitem_finish(slp) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (configvars_finishfiles) */


/* initialize a file entry */
static int file_start(cfp,filename)
CONFIGVARS_FILE	*cfp ;
const char	filename[] ;
{
	int	rs ;


	memset(cfp,0,sizeof(CONFIGVARS_FILE)) ;

	cfp->mtime = 0 ;
	if ((cfp->filename = mallocstr(filename)) == NULL)
	    return SR_NOMEM ;

	if ((rs = vecitem_start(&cfp->defines,0,0)) < 0)
	    goto bad2 ;

	if ((rs = vecitem_start(&cfp->exports,0,0)) < 0)
	    goto bad3 ;

	if ((rs = vecitem_start(&cfp->unsets,0,0)) < 0)
	    goto bad4 ;

ret0:
	return rs ;

/* bad stuff */
bad4:
	vecitem_finish(&cfp->exports) ;

bad3:
	vecitem_finish(&cfp->defines) ;

bad2:
	uc_free(cfp->filename) ;

bad1:
	goto ret0 ;
}
/* end subroutine (file_start) */


/* free up all of the stuff within a file entry */
static int file_finish(cfp)
CONFIGVARS_FILE	*cfp ;
{
	VECITEM		*slp ;

	CONFIGVARS_VAR	*vep ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	i ;


	slp = &cfp->defines ;
	for (i = 0 ; vecitem_get(slp,i,&vep) >= 0 ; i += 1) {
	    if (vep == NULL) continue ;

	    rs1 = var_finish(vep) ;
	    if (rs >= 0) rs = rs1 ;

	} /* end for */

	rs1 = vecitem_finish(slp) ;
	if (rs >= 0) rs = rs1 ;

	slp = &cfp->exports ;
	for (i = 0 ; vecitem_get(slp,i,&vep) >= 0 ; i += 1) {
	    if (vep == NULL) continue ;

	    rs1 = var_finish(vep) ;
	    if (rs >= 0) rs1 ;

	} /* end for */

	rs1 = vecitem_finish(slp) ;
	if (rs >= 0) rs = rs1 ;

	slp = &cfp->unsets ;
	for (i = 0 ; vecitem_get(slp,i,&vep) >= 0 ; i += 1) {
	    if (vep == NULL) continue ;

	    rs1 = var_finish(vep) ;
	    if (rs >= 0) rs = rs1 ;

	} /* end for */

	rs1 = vecitem_finish(slp) ;
	if (rs >= 0) rs = rs1 ;

	if (cfp->filename != NULL) {
	    uc_free(cfp->filename) ;
	    cfp->filename = NULL ;
	}

	return rs ;
}
/* end subroutine (file_finish) */


/* add a variable to this file */
static int file_addvar(cfp,type,fi,key,klen,value,vlen)
CONFIGVARS_FILE	*cfp ;
int		type ;
int		fi ;
char		key[], value[] ;
int		klen, vlen ;
{
	CONFIGVARS_VAR	ve ;

	VECITEM		*vlp ;

	int	rs = SR_NOMEM ;


	rs = var_start(&ve,0,key,klen,value,vlen) ;

	if (rs < 0)
	    goto bad1 ;

	switch (type) {

	case vartype_define:
	    vlp = &cfp->defines ;
	    break ;

	case vartype_export:
	    vlp = &cfp->exports ;
	    break ;

	case vartype_unset:
	    vlp = &cfp->unsets ;
	    break ;

	} /* end switch */

	if ((rs = vecitem_add(vlp,&ve,sizeof(CONFIGVARS_VAR))) < 0)
	    goto bad2 ;

ret0:
	return rs ;

bad2:
	var_finish(&ve) ;

bad1:
	goto ret0 ;
}
/* end subroutine (file_addvar) */


static int var_start(cep,fi,k,klen,v,vlen)
CONFIGVARS_VAR	*cep ;
int		fi ;
char		*k, *v ;
int		klen, vlen ;
{
	int	rs = SR_OK ;
	int	len ;

	char	*vb, *cp ;


	memset(cep,0,sizeof(CONFIGVARS_VAR)) ;

	if (k == NULL)
	    return SR_FAULT ;

	memset(&cep->f,0,sizeof(struct configvars_vflags)) ;

	if (klen < 0)
	    klen = strlen(k) ;

	if (v != NULL) {

	    if (vlen < 0)
	        vlen = strlen(v) ;

	} else
	    vlen = 0 ;

	len = klen + vlen + 2 ;
	if ((rs = uc_malloc(len,&vb)) < 0)
	    return rs ;

	cp = strwcpy(vb,k,klen) + 1 ;

	cep->fi = fi ;
	cep->key = vb ;
	cep->klen = klen ;
	cep->value = NULL ;
	cep->vlen = 0 ;
	if (v != NULL) {

	    strwcpy(cp,v,vlen) ;

	    cep->value = cp ;
	    cep->vlen = vlen ;

	} else
	    *cp = '\0' ;

	cep->fmask = 0 ;
	return SR_OK ;
}
/* end subroutine (var_start) */


/* free up an entry */
static int var_finish(cep)
CONFIGVARS_VAR	*cep ;
{


#if	CF_DEBUGS
	debugprintf("var_finish: ent\n") ;
#endif

	freeit(&cep->key) ;

/* there is no explicit "value" to free since it is part of the key string */

	return 0 ;
}
/* end subroutine (var_finish) */


/* free up the resources occupied by a CONFIG_STRUCTURE */
static void freeit(vp)
char		**vp ;
{

	if (*vp != NULL) {
	    uc_free(*vp) ;
	    *vp = NULL ;
	}

}
/* end subroutine (freeit) */


static void badline(eep,fname,line)
vecitem	*eep ;
char	fname[] ;
int	line ;
{
	struct configvars_errline	e ;


	if (eep == NULL)
	    return ;

	e.line = line ;
	strwcpy(e.filename,fname,MAXPATHLEN) ;

	if (eep != NULL)
	(void) vecitem_add(eep,&e,sizeof(struct configvars_errline)) ;

}
/* end subroutine (badline) */


#ifdef	COMMENT

/* get a line with possible continuation */
static int bgetcline(fp,linebuf,llen,lcp)
bfile	*fp ;
char	linebuf[] ;
int	llen ;
int	*lcp ;
{
	int	rs ;
	int	i ;
	int	alen, tlen = 0 ;
	int	f_first = TRUE ;
	int	f_cont ;


	while (f_first || (f_cont = ISCONT(linebuf,tlen))) {

	f_first = FALSE ;
	i = (f_cont) ? (tlen - 2) : tlen ;
	alen = llen - i ;
	rs = breadline(fp,linebuf + i,alen) ;

	if (rs < 0)
		return rs ;

	if (rs == 0)
		break ;

	tlen += rs ;
	*lcp += 1 ;

	} /* end while */

	if (linebuf[tlen - 1] == '\\')
		tlen -= 1 ;

	return tlen ;
}
/* end subroutine (bgetcline) */

#endif /* COMMENT */



