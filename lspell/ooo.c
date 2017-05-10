
static int strlistmks_nfbegin(STRLISTMKS *op,const char *dbname)
{
	int	rs = SR_OK ;
	int	dnl ;

	const char	*dnp ;

	memset(op,0,sizeof(STRLISTMKS)) ;
	op->fd = -1 ;
	if ((dnl = sfdirname(dbname,-1,&dnp)) >= 0) {
	    int		bnl ;
	    const char	*bnp ;
	    if ((bnl = sfbasename(dbname,-1,&bnp)) > 0) {
		int	size = 0 ;
		char	*bp ;
		size += (dnl+1) ;
		size += (bnl+1) ;
		if ((rs = uc_malloc(size,&bp)) >= 0) {
		    op->ai = bp ;
		    op->idname = bp ;
		    bp = (strwcpy(bp,dnp,dnl) + 1) ;
		    op->ibname = bp ;
		    bp = (strwcpy(bp,bnp,bnl) + 1) ;
		    if ((rs = strlistmks_dirwritable(op)) >= 0) {
			const char	*fsuf = STRLISTMKS_FSUF ;
		        rs = strlistmks_create(op,fsuf) ;
		    }
		    if (rs < 0) {
			uc_free(idx->ai) ;
			idx->ai = NULL ;
			idx->idname = NULL ;
			idx->ibname = NULL ;
		    }
		} /* end if (memory-allocation) */
	    } else
		rs = SR_INVALID ;
	} else
	    rs = SR_BADFMT ;

	return rs ;
}
/* end subroutine (strlistmks_nfbegin) */


static int strlistmks_nfend(STRLISTMKS *op)
{
	int	rs = SR_OK ;
	int	rs1 ;

	if (op->nfname != NULL) {
	    rs1 = uc_free(op->nfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nfname = NULL ;
	}

	if (op->ai != NULL) {
	    rs1 = uc_free(op->ai) ;
	    if (rs >= 0) rs = rs1 ;
	    op->ai = NULL ;
	    idx->idname = NULL ;
	    idx->ibname = NULL ;
	}

	return rs ;
}
/* end subroutine (strlistmks_nfend) */


static int strlistmks_dirwritable(STRLISTMKS *op)
{
	const int	am = (X_OK|W_OK) ;
	int	rs ;
	const char	*dname = op->idname ;

	if (dname[0] == '\0') dname = "." ;

	rs = perm(dname,-1,-1,NULL,am) ;

	return rs ;
}
/* end subroutine (strlistmks_dirwritable) */


static int strlistmks_nfcreate(STRLISTMKS *op,const char *fsuf)
{
	time_t		dt = time(NULL) ;
	const int	clen = MAXNAMELEN ;
	int		rs ;
	const char	*ibname = op->ibname ;
	const char	*end = ENDIANSTR ;
	char		cbuf[MAXNAMELEN+1] ;

	if ((rs = sncpy5(cbuf,clen,ibname,".",fsuf,end,"n")) >= 0) {
	    const char	*nfname = cbuf ;
	    char	tbuf[MAXPATHLEN+1] ;
	    if (op->idname[0] != '\0') {
		rs = mkpath2(tbuf,op->idname,cbuf) ;
		nfname = tbuf ;
	    }
	    if ((rs = strlistmks_nfopen(op,nfname)) == SR_EXISTS) {
		if ((rs = strlistmks_nfold(op,dt,nfname)) > 0) {
	    	    rs = strlistmks_nfopen(op,nfname) ;
	 	} else (rs >= 0) {
	    	    op->f.inprogress = TRUE ;
		    rs = strlistmks_nfopentmp(op,fsuf) ;
		}
	    }
	} /* end if (making component name) */

	return rs ;
}
/* end subroutine (strlistmks_nfcreate) */


static int strlistmks_nfdestroy(STRLISTMKS *op)
{
	int	rs = SR_OK ;
	int	rs1 ;

	rs1 = strlistmks_nfclose(op) ;
	if (rs >= 0) rs = rs1 ;

	if (op->nfname != NULL) {
	    if (op->nfname[0] != '\0') u_unlink(op->nfname) ;
	    rs1 = uc_free(op->nfname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nfname = NULL ;
	}

	return rs ;
}
/* end subroutine (strlistmks_nfdestroy) */


static int strlistmks_nfopen(STRLISTMKS *op,const char *nfname)
{
	const int	of = (O_CREAT | O_EXCL | O_WRONLY) ;
	int	rs ;

	if ((rs = u_open(nfname,of,op->om)) >= 0) {
	    op->nfd = rs ;
	}

	return rs ;
}
/* end subroutine (strlistmks_nfopen) */


static int strlistmks_nfclose(STRLISTMKS *op)
{
	int	rs = SR_OK ;
	int	rs1 ;
	if (op->fd >= 0) {
	    rs1 = u_close(op->fd) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fd = -1 ;
	}
	return rs ;
}
/* end subroutine (strlistmks_nfclose) */


static int strlistmks_nfold(STRLISTMKS *op,time_t dt,const char *nfname)
{
	struct ustat	sb ;
	int	rs ;
	int	f = FALSE ;
	if ((rs = u_stat(nfname,&sb)) >= 0) {
	    if ((dt-sb.st_mtime) >= TO_OLD) {
		if (u_unlink(nfname) >= 0) f = TRUE ;
	    }
	} else if (isNotPresent(rs)) {
	    rs = SR_OK ;
	    f = TRUE ;
	}
	return (rs >= 0) ? f : rs ;
}
/* end subroutine (strlistmks_nfold) */


static int strlistmks_nfopentmp(STRLISTMKS *op,const char *fsuf)
{
	const int	of = (O_WRONLY | O_CREAT) ;
	const int	clen = MAXNAMELEN ;
	int		rs = SR_OK ;
	const char	*fpre = "sm" ;
	const char	*xxx = "XXXXXXXX" ;
	const char	*end = ENDIANSTR ;
	char	cbuf[MAXNAMELEN + 1] ;
	if ((rs = sncpy6(cbuf,clen,fpre,xxx,".",fsuf,end,"n")) >= 0) {
	    char	infname[MAXPATHLEN + 1] ;
	    if (op->idname[0] != '\0') {
		rs = mkpath2(infname,op->idname,cbuf) ;
	    } else
		rs = mkpath1(infname,cbuf) ;
	    if (rs >= 0) {
	        char	obuf[MAXPATHLEN + 1] = { 0 } ;
		if ((rs = opentmpfile(infname,of,op->om,obuf)) >= 0) {
	                op->nfd = rs ;
		        rs = strlistmks_nfstore(op,obuf) ;
		        if (rs < 0) {
		            if (obuf[0] != '\0') u_unlink(obuf) ;
			    u_close(op->nfd) ;
			    op->fd = -1 ;
		        }
		} /* end if (opentmpfile) */
	    } /* end if (ok) */
	} /* end if (making file-name) */
	return rs ;
}
/* end subroutine (strlistmks_nfopentmp) */


static int strlistmks_nfstore(STRLISTMKS *op,const char *nf)
{
	int	rs ;

	const char	*cp ;

	if (op->nfname != NULL) {
	    uc_free(op->nfname) ;
	    op->nfname = NULL ;
	}

	rs = uc_mallocstrw(nf,-1,&cp) ;
	if (rs >= 0) op->nfname = cp ;

	return rs ;
}
/* end subroutine (strlistmks_nfstore) */

