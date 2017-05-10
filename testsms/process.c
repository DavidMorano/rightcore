/* process */

/* find the SSHs in this executable ! */


#define	CF_DEBUGS	0
#define	CF_DEBUG	1


/* revision history:

	= 01/09/01, David A­D­ Morano

	This subroutine was originally written.


*/




#include	<sys/types.h>
#include	<sys/mman.h>
#include	<elf.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"

#include	"lmapprog.h"
#include	"mipsdis.h"

#include	"recorder.h"
#include	"ssh.h"

#include	"lflowgroup.h"
#include	"lmipsregs.h"
#include	"opclass.h"
#include	"lexecop.h"
#include	"ldecode.h"
#include	"lexec.h"



/* local defines */

#define	INSTRDISLEN	40
#define	NINSTR_GUESS	10000		/* number branch instructions guess */

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif

#define	SSHFILE_MAGIC	"HAMMOCKS"
#define	SSHFILE_VERSION	0
#define	SSHFILE_TYPE	0

#ifndef	ENDIAN
#if	defined(SOLARIS) && defined(__sparc)
#define	ENDIAN		1
#else
#ifdef	_BIG_ENDIAN
#define	ENDIAN		1
#endif
#ifdef	_LITTLE_ENDIAN
#define	ENDIAN		0
#endif
#ifndef	ENDIAN
#error	"could not determine endianness of this machine"
#endif
#endif
#endif

#ifndef	LINELEN
#define	LINELEN		100
#endif



/* external subroutines */

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	cfhexi(const char *,int,uint *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */

struct counts {
	uint	ninstr ;		/* instructions */
	uint	cf ;			/* control-flow change */
	uint	br_rel ;		/* relative branches */
	uint	br_con ;		/* conditional */
	uint	br_fwd ;		/* conditional forward */
	uint	br_ssh ;		/* conditional SSH */
} ;


/* forward references */

static int	readem(struct proginfo *,LMAPPROG *,MIPSDIS *,
			RECORDER *,struct counts *,uint,uint) ;
static int	writecache(struct proginfo *,RECORDER *,char *) ;


/* local (module-scope static) data */







int process(pip,pmp,dp,sshfname)
struct proginfo	*pip ;
LMAPPROG	*pmp ;
MIPSDIS		*dp ;
char		sshfname[] ;
{
	LMAPPROG_SEGINFO	si ;

	LMAPPROG_SEGCURSOR	cur ;

	RECORDER		rec ;

	Elf32_Shdr		*shp ;

	bfile			cfile ;

	struct counts		c ;

	int	rs ;
	int	i, j ;
	int	n = NINSTR_GUESS ;


#if	CF_DEBUG
	if (pip->debuglevel >= 4)
	    debugprintf("process: entered pip=%p\n",pip) ;
#endif

	if (sshfname == NULL)
		return SR_FAULT ;

	(void) memset(&c,0,sizeof(struct counts)) ;

/* initialize the recorder */

	rs = recorder_init(&rec,n) ;

#if	CF_DEBUG
	if (pip->debuglevel >= 4)
	    debugprintf("process: recorder_init() rs=%d pip=%p\n",rs,pip) ;
#endif

	if (rs < 0)
	    goto badrec ;


/* loop, reading all instructions and figuring things out */

#ifdef	COMMENT
	lmapprog_segcurbegin(pmp,&cur) ;

	while ((rs = lmapprog_enumseg(pmp,&cur,&si)) >= 0) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(4)) {
	        debugprintf("process: lmapprog_enumseg() rs=%d\n",rs) ;
	        debugprintf("process: start=%08x len=%08x prot=%08x\n",
	            si.start,si.len,si.prot) ;
	    }
#endif

	    if (si.prot & PROT_EXEC) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process: got an executable one; start=%08x\n",
	                si.start) ;
#endif

	        rs = readem(pip,pmp,dp, &rec,&c,si.start,si.len) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process: readem() rs=%d\n",rs) ;
#endif

	    }

	} /* end while (looping through program segments) */

	lmapprog_segcurend(pmp,&cur) ;
#endif /* COMMENT */


	for (i = 0 ; (rs = lmapprog_getsec(pmp,i,&shp)) >= 0 ; i += 1) {

	    uint	secstart, seclen ;


	    if ((shp->sh_type & SHT_PROGBITS) &&
	        (shp->sh_flags & SHF_ALLOC) && 
	        (shp->sh_flags & SHF_EXECINSTR)) {


	        secstart = (uint) shp->sh_addr ;
	        seclen = (uint) shp->sh_size ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process: got i=%d start=%08x len=%08x\n",
	                i,secstart,seclen) ;
#endif

	        rs = readem(pip,pmp,dp, &rec,&c,secstart,seclen) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(4))
	            debugprintf("process: readem() rs=%d\n",rs) ;
#endif

	    }

	} /* end for (looping through program sections) */



/* where are we ? */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("process: ninstr=%u\n",c.ninstr) ;
	    debugprintf("process: cf=%u\n",c.cf) ;
	    debugprintf("process: br_rel=%u\n",c.br_rel) ;
	    debugprintf("process: br_con=%u\n",c.br_con) ;
	    debugprintf("process: br_fwd=%u\n",c.br_fwd) ;
	    debugprintf("process: br_ssh=%u\n",c.br_ssh) ;
	}
#endif

	if (pip->verboselevel > 0) {

	    bprintf(pip->ofp,"ninstr=%u\n",c.ninstr) ;

	    bprintf(pip->ofp,"cf=%u\n",c.cf) ;

	    bprintf(pip->ofp,"br_rel=%u\n",c.br_rel) ;

	    bprintf(pip->ofp,"br_con=%u\n",c.br_con) ;

	    bprintf(pip->ofp,"br_fwd=%u\n",c.br_fwd) ;

	    bprintf(pip->ofp,"br_ssh=%u\n",c.br_ssh) ;

	} /* end if (printing results) */


	rs = recorder_count(&rec) ;

	n = rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("process: count=%d\n",n) ;
#endif

/* OK, write out the cache file */

	rs = writecache(pip,&rec,sshfname) ;

	if (rs < 0)
	    goto badcache ;


/* optionally verify (only works if there is ONLY one text segment) */

#if	CF_DEBUGS
	{
	    bfile	source ;

	    SSH	hammocks ;

	    SSH_ENT	*ep ;

	    uint	ia ;

	    int	len, line ;

	    char	tmpfname[MAXPATHLEN + 1] ;
	    char	linebuf[LINELEN + 1] ;


	    rs = ssh_init(&hammocks,sshfname) ;

	    debugprintf("process: ssh_init() rs=%d\n",rs) ;

	    sncpy2(tmpfname,MAXPATHLEN,pip->basename,".ia") ;

	    rs = bopen(&source,tmpfname,"r",0666) ;

	    debugprintf("process: reading IA=%s rs=%d\n",tmpfname,rs) ;

	    line = 0 ;
	    while ((len = breadline(&source,linebuf,LINELEN)) > 0) {

	        line += 1 ;
	        debugprintf("process: i=%d source=>%t<\n",
	            line,linebuf,(len - 1)) ;

	        rs = cfhexui(linebuf,len,&ia) ;

	        if (rs < 0)
	            debugprintf("process: cfhexui() rs=%d\n", rs) ;

	        rs = ssh_check(&hammocks,ia,&ep) ;

	        if ((rs < 0) || (ep->ia != ia) || 
	            ((ep->type & SSH_BTSSH) == 0))
	            debugprintf("process: bad verify rs=%d ia=%08x\n",
	                rs,ia) ;

	    } /* end while */

	    debugprintf("process: free lance \n") ;

	    ia = 0 ;
	    rs = ssh_check(&hammocks,ia,&ep) ;

	    debugprintf("process: ia=%08x rs=%d\n",ia,rs) ;

	    ia = 27 ;
	    rs = ssh_check(&hammocks,ia,&ep) ;

	    debugprintf("process: ia=%08x rs=%d\n",ia,rs) ;

	    ia = 0x004420ec ;
	    rs = ssh_check(&hammocks,ia,&ep) ;

	    debugprintf("process: ia=%08x rs=%d\n",ia,rs) ;


	    rs = 0 ;

	    bclose(&source) ;

	    ssh_free(&hammocks) ;
	}
#endif /* CF_DEBUGS */


/* we're out of here */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("process: exiting rs=%d\n",rs) ;
#endif

ret1:
	recorder_free(&rec) ;

ret0:
	return ((rs >= 0) ? c.br_ssh : rs) ;

/* bad things */
badrec:
	fprintf(pip->efp,
	    "%s: could not initialize the recorder (%d)\n",
	    pip->progname,rs) ;

	goto ret0 ;

badcache:
	fprintf(pip->efp,
	    "%s: could not create the cache file (%d)\n",
	    pip->progname,rs) ;

	goto ret1 ;

}
/* end subroutine (process) */



/* LOCAL SUBROUTINES */



static int readem(pip,pmp,dp,rp,cp,start,len)
struct proginfo	*pip ;
LMAPPROG	*pmp ;
MIPSDIS		*dp ;
RECORDER	*rp ;
struct counts	*cp ;
uint		start, len ;
{
	LDECODE di ;

#if	CF_DEBUGS
	bfile	source ;
#endif

	uint	end = start + len ;
	uint	ia ;
	uint	ia_lastbranch = 0 ;
	uint	ia_target = 0 ;
	uint	instr ;
	uint	instr_lastbranch ;
	uint	ninstr ;

	int	rs = SR_OK ;
	int	rs1, rs2 ;
	int	n ;
	int	f_cf ;
	int	f_forward ;

	char	instrdis[40] ;

#if	CF_DEBUGS
	char	tmpfname[MAXPATHLEN + 1] ;
#endif


#if	CF_DEBUGS
	sncpy2(tmpfname,MAXPATHLEN,pip->basename,".ia") ;

	rs = bopen(&source,tmpfname,"wca",0666) ;

	debugprintf("readem: making IA=%s rs=%d\n",tmpfname,rs) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("readem: start=%08x\n",start) ;
#endif

	ninstr = 0 ;
	ia = start ;
	while ((ia < end) &&
	    ((rs = lmapprog_readinstr(pmp,ia,&instr)) >= 0)) {

	    int	f_forward ;


	    rs = ldecode_decode(&di,ia,instr) ;

	    f_cf = (di.opclass == OPCLASS_BREL) || 
	        (di.opclass == OPCLASS_JREL) ||
	        (di.opclass == OPCLASS_JIND) ;

	    f_cf = f_cf && (rs >= 0) ;

/* OK, do some stuff */

#ifdef	COMMENT
	    if (c_cf && (di.opclass == OPCLASS_BREL)) {

	        recorder_add(rp,ia_lastbranch,ia_target,-1,0) ;

	    }
#endif /* COMMENT */

/* SS hammock conditional branches */

	    if (ia == ia_target) {

	        cp->br_ssh += 1 ;
	        n = (ia_target - ia_lastbranch) / 4 ;

	        recorder_add(rp,ia_lastbranch,ia_target,n,
	            RECORDER_BTFWD | RECORDER_BTSSH) ;

/* some verbosity */

	        if (pip->verboselevel >= 2) {

	            bprintf(pip->ofp,"%08x",ia_lastbranch) ;

	            rs1 = SR_NOTSUP ;
	            if (pip->f.instrdis) {

	                rs1 = mipsdis_getlevo(dp,
	                    ia_lastbranch,instr_lastbranch,
	                    instrdis,INSTRDISLEN) ;

	                if (rs1 < 0)
	                    strcpy(instrdis,"unknown") ;

	                bprintf(pip->ofp," %s <%d>",instrdis,n) ;

	            } /* end if */

	            bprintf(pip->ofp,"\n") ;

	        } /* end if (verbose) */

#if	CF_DEBUGS
	        bprintf(&source,"%08x\n",ia_lastbranch) ;
#endif

	    } /* end if (got a SS-hammock) */

/* maintenance */

	    if (f_cf) {

	        cp->cf += 1 ;
	        ia_target = 0 ;
	        if (di.opclass == OPCLASS_BREL) {

	            cp->br_rel += 1 ;
	            if ((instr & 0xffff0000) != 0x10000000) {

	                cp->br_con += 1 ;
	                if (di.ta > ia) {

	                    cp->br_fwd += 1 ;
	                    ia_lastbranch = ia ;
	                    ia_target = di.ta ;
	                    instr_lastbranch = instr ;

	                } /* end if (forward conditional) */

	            } /* end if (conditional) */

	        } /* end if (relative) */

	    } /* end if (control flow) */

	    ia += 4 ;
	    ninstr += 1 ;

	} /* end while (looping through instructions) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("readem: out-of-loop rs=%d\n",rs) ;
#endif


	cp->ninstr += ninstr ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("readem: ninstr=%u\n",cp->ninstr) ;
	    debugprintf("process: cf=%u\n",cp->cf) ;
	    debugprintf("process: br_rel=%u\n",cp->br_rel) ;
	    debugprintf("process: br_con=%u\n",cp->br_con) ;
	    debugprintf("process: br_fwd=%u\n",cp->br_fwd) ;
	    debugprintf("process: br_ssh=%u\n",cp->br_ssh) ;
	}
#endif

#if	CF_DEBUGS
	bclose(&source) ;
#endif

	return ninstr ;
}
/* end subroutine (readem) */


/* write out the cache file */
static int writecache(pip,rp,fname)
struct proginfo	*pip ;
RECORDER	*rp ;
char		fname[] ;
{
	bfile	cfile ;

	offset_t	off ;

	uint	*rectab ;
	uint	(*recind)[2] ;
	uint	header[4] ;

	int	rs, size ;
	int	rtsize , risize ;
	int	rtlen, rilen ;

	char	magicbuf[16] ;
	char	*cp ;


#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("writecache: entered fname=%s\n",fname) ;
#endif

	if (fname == NULL)
		return SR_FAULT ;

	u_unlink(fname) ;

	rs = bopen(&cfile,fname,"wct",0666) ;

	if (rs < 0)
	    goto ret0 ;

/* prepare and write the file magic */

	cp = strwcpy(magicbuf,SSHFILE_MAGIC,14) ;

	*cp++ = '\n' ;
	(void) memset(cp,0,(magicbuf + 16) - cp) ;

	rs = bwrite(&cfile,magicbuf,16) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("writecache: wrote magic rs=%d\n",rs) ;
#endif

/* prepare and write the version and encoding */

	magicbuf[0] = SSHFILE_VERSION ;
	magicbuf[1] = ENDIAN ;
	magicbuf[2] = SSHFILE_TYPE ;
	magicbuf[3] = 0 ;

	rs = bwrite(&cfile,magicbuf,4) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("writecache: wrote version-encoding rs=%d\n",rs) ;
#endif

/* get the tables and write them out (along with table headers) */

	rs = recorder_gettab(rp,&rectab) ;

	if (rs < 0)
	    goto badtab ;

	rtsize = rs ;
	rs = recorder_sizeindex(rp) ;

	if (rs < 0)
	    goto badtab ;

	risize = rs ;
	rs = uc_malloc(risize,&recind) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("writecache: rtsize=%d\n",rtsize) ;
	    debugprintf("writecache: risize=%d\n",risize) ;
	    debugprintf("writecache: uc_malloc() rs=%d\n",rs) ;
	}
#endif

	if (rs < 0)
	    goto ret1 ;

	rs = recorder_mkindex(rp,recind,risize) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("writecache: recorder_mkindex() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret2 ;

	rs = btell(&cfile,&off) ;

	if (rs < 0)
	    goto ret2 ;

/* get some other stuff (!) */

	rtlen = recorder_rtlen(rp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("writecache: rtlen=%d\n",rtlen) ;
#endif

	rilen = recorder_countindex(rp) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("writecache: rilen=%d\n",rilen) ;
#endif

	header[0] = off + 16 ;
	header[1] = rtlen ;
	header[2] = off + 16 + rtsize ;
	header[3] = rilen ;

	size = 4 * sizeof(int) ;
	rs = bwrite(&cfile,header,size) ;

	if (rs < 0)
	    goto ret2 ;

	rs = bwrite(&cfile,rectab,rtsize) ;

	if (rs < 0)
	    goto ret2 ;

	rs = bwrite(&cfile,recind,risize) ;

	if (rs < 0)
	    goto ret2 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("writecache: wrote everything !\n") ;
#endif


/* we're out of here */
ret2:
	free(recind) ;

ret1:
	bclose(&cfile) ;

ret0:
	return rs ;

badtab:
	goto ret1 ;
}
/* end subroutine (writecache) */



