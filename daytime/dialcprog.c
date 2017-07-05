/* dialcprog */

/* dial to a program on the local cluster */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUGRNODE	0		/* special debug */
#define	CF_DEBUGFSYNC	1		/* special debug */
#define	CF_REVENT	0		/* received poll events */
#define	CF_WAITTIMEOUT	1		/* really only for testing! */
#define	CF_RECENT	1		/* use only recently updated nodes */
#define	CF_BESTNODE	1		/* use BESTNODE object */


/* revision history:

	= 2003-07-21, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is used to dial out to a program that resides on the
	local cluster.  The program will be executed on whatever node in the
	cluster is specified.  This subroutine has the typical "dial" type
	sematics in that a single bi-directional pipe is returned with the
	input and output from the program on it.  An optional additional pipe
	with the standard error from the program is possible also.

	Also, note that Solaris 7 appears to be broken, along with maybe
	Solaris 8 and all that follow.  The stupid rules for how stupid sockets
	work keeps changing.  There are several semantics for stupid sockets on
	Solaris now and even within one of those, its stupid semantics seems to
	subtly change from time to time.

	What is up with the 'timeout' argument?

	<0              use the system default timeout (xx minutes --
			whatever)
	==0             asynchronously spawn the connect and do not wait
			for it
	>0		use the timeout as it is

	Synopsis:

	int dialcprog(pr,node,progfname,av,ev,to,opts,fd2p)
	const char	pr[] ;
	const char	node[] ;
	const char	progfname[] ;
	const char	*av[] ;
	const char	*ev[] ;
	int		to ;
	int		opts ;
	int		*fd2p ;

	Arguments:

	pr		program root
	node		remote node to dial
	progfname	program file-name
	av		argument vector
	ev		environment vector 
	to		timeout ('>0' means use one, '-1' means don't)
	options		any dial options
	fd2p		pointer to integer for standard-error file-descriptor

	Returns:

	>=0		file descriptor
	<0		error in dialing


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<estrings.h>
#include	<ascii.h>
#include	<stdorder.h>
#include	<vecstr.h>
#include	<msfile.h>
#include	<hostinfo.h>
#include	<hostent.h>
#include	<sockaddress.h>
#include	<inetaddr.h>
#include	<ids.h>
#include	<getxusername.h>
#include	<filebuf.h>
#include	<localmisc.h>

#include	"streamsync.h"
#include	"dialopts.h"
#include	"dialcprogmsg.h"
#include	"nodesearch.h"
#include	"envlist.h"
#include	"msflag.h"
#include	"nodedb.h"
#include	"clusterdb.h"
#include	"mkcexsync.h"


/* local defines */

#define	PROGRAMROOTVAR1	"CPROG_PROGRAMROOT"
#define	PROGRAMROOTVAR2	"LOCAL"

#define	PROGRAMROOT	"/usr/add-on/local"

#ifndef	NODESFNAME
#define	NODESFNAME	"etc/cluster/nodes"
#endif

#ifndef	NODEFNAME
#define	NODEFNAME	"etc/node"
#endif

#ifndef	CLUSTERFNAME1
#define	CLUSTERFNAME1	"etc/cluster"
#endif

#ifndef	CLUSTERFNAME2
#define	CLUSTERFNAME2	"etc/clusters"
#endif

#define	MSFNAME		"var/ms"
#define	CEXECERFNAME	"lib/cluster/cexecer"

#ifndef	VARNODE
#define	VARNODE		"NODE"
#endif

#ifndef	VARPWD
#define	VARPWD		"PWD"
#endif

#ifndef	VARDISPLAY
#define	VARDISPLAY	"DISPLAY"
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		(8 * 1024)	/* maximum variable length */
#endif

#define	BUFLEN		(4 * VBUFLEN)
#define	SYNCLEN		6		/* length of the SYNC string */
#define	DATABUFLEN	30
#define	HOSTBUFLEN	(8 * MAXHOSTNAMELEN)
#define	ENVBUFLEN	(MAXHOSTNAMELEN + 40)
#define	ENODELEN	(NODENAMELEN+sizeof(VARNODE))

#define	TO_SYNC		40
#define	TO_READ		40
#define	TO_ACCEPT	40
#define	TO_UP		(12 * 60 * 60)	/* 12 hours! (fairly generous) */
#define	TO_VALID	(24 * 3600)

#define	DEFNODES	200		/* default notes in cluster */
#define	NENVS		150		/* default environment variables */
#define	DEFFILESIZE	(2 * 1024 * 1024)

#define	PROTONAME	"tcp"
#define	LOCALNODE	"local"
#define	LOCALHOST	"localhost"

#define	AVAILTHRESH	0.2		/* availability threshold */

#define	DIALINFO	struct dialinfo
#define	DIALINFO_FL	struct dialinfo_flags
#define	DIALINFO_AI	struct dialinfo_ai
#define	DIALINFO_SI	struct dialinfo_si

#define	BESTNODE	struct bestnode

#define	DEBUGFNAME	"/tmp/dialcprog.deb"


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	strkeycmp(const char *,const char *) ;
extern int	vstrkeycmp(const char **,const char **) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	findfilepath(const char *,char *,const char *,int) ;
extern int	getpwd(char *,int) ;
extern int	getnodename(char *,int) ;
extern int	dialprog(const char *,int,const char **,const char **,int *) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	vecstr_foilcmp(vecstr *,vecstr *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
extern int	mkhexstr(char *,int,void *,int) ;
#if	CF_REVENT
extern char	*d_reventstr(int,char *,int) ;
#endif
#endif /* CF_DEBUGS */

extern int	rcmdr(const char *,const char *,const char *,int *) ;
extern int	mkcexsync(char *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnrchr(const char *,int,int) ;


/* external variables */

extern char	**environ ;


/* local typedefs */

#if	defined(IRIX) && (! defined(TYPEDEF_INADDRT))
#define	TYPEDEF_INADDRT	1
typedef unsigned int	in_addr_t ;
#endif


/* local structures */

struct dbinfo {
	NODEDB		node ;
	CLUSTERDB	cluster ;
	uint		f_node:1 ;
	uint		f_cluster:1 ;
} ;

struct dialinfo_flags {
	uint		errchan:1 ;
	uint		empty:1 ;
} ;

struct dialinfo_ai {
	in_addr_t	addr ;
	int		port ;
} ;

struct dialinfo_si {
	SOCKADDRESS	sa ;
	int		salen ;
} ;

struct dialinfo {
	const char	*pr ;
	const char	*nodename ;
	const char	*username ;
	const char	*rhost ;
	DIALINFO_FL	f ;
	DIALINFO_AI	a ;
	DIALINFO_SI	sout, serr ;
	time_t		daytime ;
	int		opts ;
	ushort		flags ;
	char		pwd[MAXPATHLEN + 1] ;
	char		tmpfname[MAXPATHLEN + 1] ;
} ;

struct bestnode {
	DIALINFO	*dip ;
	MSFILE		ms ;
	uint		f_open:1 ;
} ;


/* forward references */

static int	getnodenames(const char *,vecstr *,const char *) ;
static int	dialremote(struct dialinfo *,cchar *,cchar **, cchar **,int *) ;
static int	findsync(FILEBUF *) ;
static int	getrnode(FILEBUF *,char *) ;

static int	sendvars(struct dialinfo *,int,cchar *,cchar **,cchar **) ;

static int	getclusters(struct dbinfo *,vecstr *,const char *) ;

static int	mklisten_start(struct dialinfo *, struct dialinfo_si *) ;
static int	mklisten_finish(struct dialinfo *, struct dialinfo_si *) ;
static int	mklisten_bind(struct dialinfo *, struct dialinfo_si *,int) ;

#if	CF_BESTNODE
static int	bestnode_open(BESTNODE *,DIALINFO *,cchar *,cchar *) ;
static int	bestnode_close(BESTNODE *) ;
static int	bestnode_get(BESTNODE *,struct dbinfo *,vecstr *,
			char *,double *) ;
#endif

static int	filebuf_sendrecord(FILEBUF *,int,const char *,int) ;

static int	loadlocalnames(const char *,struct dialinfo *,vecstr *) ;


/* local variables */

static const char	*envbads[] = {
	"_",
	"_A0",
	"_EF",
	"A__z",
	"SYSNAME",
	"NODE",
	"RELEASE",
	"VERSION",
	"MACHINE",
	"ARCHITECTURE",
	"HZ",
	"NCPU",
	"HOSTNAME",
	"TERMDEV",
	"TERM",
	"TERMCAP",
	"AUDIODEV",
	"DISPLAY",
	"RANDOM",
	"LOGNAME",
	NULL
} ;

static const char	*varnode = VARNODE ;

static const int	one = 1 ;


/* exported subroutines */


int dialcprog(pr,node,fname,av,ev,to,opts,fd2p)
const char	pr[] ;
const char	node[] ;
const char	fname[] ;
const char	*av[] ;
const char	*ev[] ;
int		to ;
int		opts ;
int		*fd2p ;
{
	struct ustat	sb ;
	DIALINFO	di ;
	IDS		id ;
	vecstr		localnames, nodenames ;
	int		rs, rs1 ;
	int		i ;
	int		fd = -1 ;
	int		f_local = FALSE ;
	int		f_localnames = FALSE ;
	int		f_nodenames = FALSE ;
	const char	*cp ;
	const char	*progfname ;
	char		lnodebuf[NODENAMELEN + 1] ;
	char		rnodebuf[NODENAMELEN + 1] ;
	char		usernamebuf[LOGNAME_MAX + 2] ;
	char		progbuf[MAXPATHLEN + 1] ;

#if	CF_DEBUGS
	debugprintf("dialcprog: ent pr=%s\n",pr) ;
	debugprintf("dialcprog: node=%s prog=%s to=%u opts=%08x\n",
	    node,fname,to,opts) ;
#endif

	if (node == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

/* get our program root */

	if ((pr == NULL) || (pr[0] == '\0'))
	    pr = getenv(PROGRAMROOTVAR1) ;

	if ((pr == NULL) || (pr[0] == '\0'))
	    pr = getenv(PROGRAMROOTVAR2) ;

	if ((pr == NULL) || (pr[0] == '\0'))
	    pr = PROGRAMROOT ;

#if	CF_DEBUGS
	debugprintf("dialcprog: ent pr=%s\n", pr) ;
#endif

	memset(&di,0,sizeof(struct dialinfo)) ;
	di.pr = pr ;
	di.rhost = node ;

	if (opts & DIALOPT_EMPTY)
	    di.f.empty = TRUE ;

	rs = ids_load(&id) ;
	if (rs < 0)
	    goto ret0 ;

/* get our local node name */

	di.nodename = lnodebuf ;
	rs = getnodename(lnodebuf,NODENAMELEN) ;
	if (rs < 0) goto ret1 ;

#if	CF_DEBUGS
	debugprintf("dialcprog: nodename=%s\n",di.nodename) ;
#endif

/* get our login name */

	di.username = usernamebuf ;
	rs = getusername(usernamebuf,USERNAMELEN,-1) ;
	if (rs < 0) goto ret1 ;

/* is the target node in the same cluster as we are? */

	{

#if	CF_DEBUGS
	    debugprintf("dialcprog: new cluster framework\n") ;
#endif

	    rs = vecstr_start(&localnames,5,0) ;

	    if (rs >= 0) {
	        f_localnames = TRUE ;

	        rs = loadlocalnames(pr,&di,&localnames) ;

	    } /* end if (local node names) */

	    if (rs >= 0) {
	        rs = vecstr_start(&nodenames,5,0) ;
	        f_nodenames = (rs >= 0) ;
	    }

/* check if the target is really local */

#if	CF_DEBUGS
	    debugprintf("dialcprog: check for remote node rs=%d\n",rs) ;
#endif

	    if ((rs >= 0) &&
	        ((node[0] != '+') || (node[1] != '\0'))) {

#if	CF_DEBUGS
	        debugprintf("dialcprog: remote specified node=%s\n",node) ;
#endif

	        rs = getnodenames(pr,&nodenames,node) ;

#if	CF_DEBUGS
	        debugprintf("dialcprog: getnodenames() rs=%d\n",rs) ;
#endif

	        if (rs == SR_NOENT)
	            rs = SR_NONET ;

	        if (rs >= 0) {
	            rs1 = vecstr_foilcmp(&localnames,&nodenames) ;
	            f_local = (rs1 > 0) ;
#if	CF_DEBUGS
	            debugprintf("dialcprog: vecstr_foilcmp() rs=%d\n",
	                rs1) ;
#endif
	        }

	    } /* end if (remote-specified node) */

#if	CF_DEBUGS
	    debugprintf("dialcprog: middle rs=%d f_local=%u\n",rs,f_local) ;
#endif

/* do we need to verify against cluster DBs? */

	    if ((rs >= 0) && 
	        (((! f_local) && ((opts & DIALOPT_NOCHECK) == 0)) ||
	        ((node[0] == '+') && (node[1] == '\0')))) {
	        struct dbinfo	dbi ;
	        vecstr	c_us, c_them ;

#if	CF_DEBUGS
	        debugprintf("dialcprog: checking or finding\n") ;
#endif

	        rs1 = vecstr_start(&c_us,10,0) ;

	        if (rs1 >= 0) {
	            rs1 = vecstr_start(&c_them,10,0) ;
	            if (rs1 < 0) vecstr_finish(&c_us) ;
	        }

#if	CF_DEBUGS
	        debugprintf("dialcprog: initialized lists (%d)\n",rs1) ;
#endif

	        if (rs1 >= 0) {
	            int	f_gotus ;

/* node DB */

	            rs1 = mkpath2(di.tmpfname,pr,NODEFNAME) ;

#if	CF_DEBUGS
	            debugprintf("dialcprog: nodefname=%s\n",di.tmpfname) ;
#endif

	            if (rs1 >= 0)
	                rs1 = perm(di.tmpfname,-1,-1,NULL,R_OK) ;

#if	CF_DEBUGS
	            debugprintf("dialcprog: node perm() rs=%d\n",rs1) ;
#endif

	            if (rs1 >= 0) {
	                rs1 = nodedb_open(&dbi.node,di.tmpfname) ;
	                dbi.f_node = (rs1 >= 0) ;
	            }

/* cluster DB */

	            rs1 = mkpath2(di.tmpfname,pr,CLUSTERFNAME1) ;

#if	CF_DEBUGS
	            debugprintf("dialcprog: cluster1fname=%s\n",di.tmpfname) ;
#endif

#ifdef	COMMENT
	            if (rs1 >= 0)
	                rs1 = perm(di.tmpfname,-1,-1,NULL,R_OK) ;

#if	CF_DEBUGS
	            debugprintf("dialcprog: cluster1 perm() rs=%d\n",rs1) ;
#endif

	            if (rs1 >= 0) {

	                rs1 = u_stat(di.tmpfname,&sb) ;

#if	CF_DEBUGS
	                debugprintf("dialcprog: cluster1 u_stat() rs=%d\n",
	                    rs1) ;
#endif

	                if ((rs1 >= 0) && S_ISDIR(sb.st_mode))
	                    rs1 = SR_ISDIR ;

	            }
#else /* COMMENT */
	            if (rs1 >= 0)
	                rs1 = u_stat(di.tmpfname,&sb) ;

	            if ((rs1 >= 0) && S_ISDIR(sb.st_mode))
	                rs1 = SR_ISDIR ;

	            if (rs1 >= 0)
	                rs1 = sperm(&id,&sb,R_OK) ;

#endif /* COMMENT */

	            if ((rs1 == SR_NOENT) || (rs1 == SR_ISDIR) ||
	                (rs1 == SR_ACCESS)) {

	                mkpath2(di.tmpfname,pr,CLUSTERFNAME2) ;

#if	CF_DEBUGS
	                debugprintf("dialcprog: cluster2fname=%s\n",
	                    di.tmpfname) ;
#endif

	                rs1 = perm(di.tmpfname,-1,-1,NULL,R_OK) ;

#if	CF_DEBUGS
	                debugprintf("dialcprog: cluster2 perm() rs=%d\n",rs1) ;
#endif

	            }

	            if (rs1 >= 0) {
	                rs1 = clusterdb_open(&dbi.cluster,di.tmpfname) ;
	                dbi.f_cluster = (rs1 >= 0) ;
	            }

#if	CF_DEBUGS
	            debugprintf("dialcprog: DB status f_node=%u f_cluster=%u\n",
	                dbi.f_node,dbi.f_cluster) ;
#endif

/* can we find anything out? */

	            if (dbi.f_node || dbi.f_cluster) {

	                rs1 = getclusters(&dbi,&c_us,di.nodename) ;
	                f_gotus = (rs1 >= 0) ;

#if	CF_DEBUGS
	                debugprintf("dialcprog: getclusters() rs=%d \n",rs1) ;
	                debugprintf("dialcprog: f_gotus=%u\n",f_gotus) ;
#endif

	            } /* end if */

/* different action depending on specified node */

	            if ((node[0] != '+') || (node[1] != '\0')) {

#if	CF_DEBUGS
	                debugprintf("dialcprog: cluster named node\n") ;
#endif

	                di.rhost = node ;

/* verify the node name given */

	                if (! f_local) {

#if	CF_DEBUGS
	                    debugprintf("dialcprog: remote node "
	                        "verification\n") ;
	                    debugprintf("dialcprog: db_node=%u "
	                        "db_cluster=%u\n",
	                        dbi.f_node,dbi.f_cluster) ;
#endif

	                    rs = SR_OK ;
	                    if (dbi.f_node || dbi.f_cluster) {

#if	CF_DEBUGS
	                        debugprintf("dialcprog: getclusters() "
	                            "node=%s\n",node) ;
#endif

	                        rs = getclusters(&dbi,&c_them,node) ;

#if	CF_DEBUGS
	                        debugprintf("dialcprog: getclusters() rs=%d\n",
	                            rs) ;
#endif

	                        if (rs >= 0) {

	                            if (f_gotus) {

#if	CF_DEBUGS
	                                {
	                                    int	i ;
	                                    const char	*cp ;
	                                    debugprintf("dialcprog: "
	                                        "cluster=us ¬\n") ;
	                                    for (i = 0 ; vecstr_get(&c_us,
	                                        i,&cp) >= 0 ; i += 1)
	                                        debugprintf("dialcprog: "
							"i=%u c=%s\n",i,cp) ;
	                                    debugprintf("dialcprog: "
						"cluster=them\n") ;
	                                    for (i = 0 ; vecstr_get(&c_them,
	                                        i,&cp) >= 0 ; i += 1)
	                                        debugprintf("dialcprog: "
							"i=%u c=%s\n",i,cp) ;
	                                }
#endif /* CF_DEBUGS */

	                                rs = vecstr_foilcmp(&c_us,&c_them) ;
	                                if (rs <= 0) rs = SR_HOSTUNREACH ;

	                            } else {

/* just check if the traget is in ANY cluster */

	                                rs = (rs > 0) ? 
						SR_OK : SR_HOSTUNREACH ;

	                            }
	                        }

	                    }

	                } /* end if (not local) */

	            } else {

#if	CF_DEBUGS
	                debugprintf("dialcprog: cluster unnamed node\n") ;
#endif

	                di.rhost = rnodebuf ;

/* find any suitable node */

#if	CF_BESTNODE
	                {
	                    BESTNODE	bn ;
	                    vecstr	cnodes ;
	                    int		f_cnodes = FALSE ;

	                    if ((rs = vecstr_start(&cnodes,30,0)) >= 0) {
	                        CLUSTERDB_CUR	cc ;
	                        int	cl ;
	                        char	cnode[NODENAMELEN + 1] ;


	                        for (i = 0 ; vecstr_get(&c_us,i,&cp) >= 0 ; 
	                            i += 1) {

	                            if (cp == NULL) continue ;

#if	CF_DEBUGS
	                            debugprintf("dialcprog: attached "
	                                "cluster=%s\n",cp) ;
#endif

	                            clusterdb_curbegin(&dbi.cluster,&cc) ;

	                            while (rs >= 0) {

	                                cl = clusterdb_fetch(&dbi.cluster,cp,
	                                    &cc,cnode,NODENAMELEN) ;

	                                if (cl == SR_NOTFOUND)
	                                    break ;

	                                rs = cl ;
	                                if (rs < 0) break ;

#if	CF_DEBUGS
	                                debugprintf("dialcprog: cluster "
	                                    "cl=%d node=%s\n",
	                                    cl,cnode) ;
#endif

	                                rs1 = vecstr_findn(&cnodes,cnode,cl) ;

	                                if (rs1 == SR_NOTFOUND) {
	                                    rs = vecstr_add(&cnodes,cnode,cl) ;
	                                }

	                            } /* end while */

	                            clusterdb_curend(&dbi.cluster,&cc) ;
	                        } /* end for */

	                    } /* end if */

#if	CF_DEBUGS
	                    debugprintf("dialcprog: unnamed node rs=%d\n",rs) ;
	                    debugprintf("dialcprog: attached nodes:\n") ;
	                    for (i = 0 ; vecstr_get(&cnodes,i,&cp) >= 0 ; 
	                        i += 1)
	                        debugprintf("dialcprog: node=%s\n",cp) ;
#endif

	                    if (rs >= 0)
	                        rs = bestnode_open(&bn,&di,di.pr,MSFNAME) ;

	                    if (rs >= 0)
	                        rs = bestnode_get(&bn,&dbi,&cnodes,
	                            rnodebuf,NULL) ;

	                    bestnode_close(&bn) ;
	                    if (f_cnodes) vecstr_finish(&cnodes) ;
	                }
#else /* CF_BESTNODE */

	                rs = SR_NOSYS ;

#endif /* CF_BESTNODE */

	            } /* end if (node name verification) */

#if	CF_DEBUGS
	            debugprintf("dialcprog: closing DBs\n") ;
#endif

	            if (dbi.f_cluster)
	                clusterdb_close(&dbi.cluster) ;

	            if (dbi.f_node)
	                nodedb_close(&dbi.node) ;

	            vecstr_finish(&c_them) ;

	            vecstr_finish(&c_us) ;

#if	CF_DEBUGS
	            debugprintf("dialcprog: half rs=%d\n",rs) ;
#endif

	        } /* end if (initialized) */

	    } /* end if (checking cluster membership) */

	    if (f_nodenames)
	        vecstr_finish(&nodenames) ;

	    if (f_localnames)
	        vecstr_finish(&localnames) ;

	} /* end block (use CLUSTERDB object) */

#ifdef	COMMENT
	if (f_local)
	    di.rhost = LOCALHOST ;
#endif

#if	CF_DEBUGS
	debugprintf("dialcprog: verification rs=%d\n",rs) ;
#endif

/* other initialization */

	di.pwd[0] = '\0' ;

/* find the program */

	progfname = fname ;
	if ((rs >= 0) && (fname[0] != '/')) {

	    if (di.pwd[0] == '\0') {
	        rs = getpwd(di.pwd,MAXPATHLEN) ;
	        if (rs < 0) goto badpwd ;
	    }

	    progfname = progbuf ;
	    mkpath2(progbuf,di.pwd,fname) ;

#if	CF_DEBUGS
	    debugprintf("dialcprog: progbuf=%s\n",progbuf) ;
#endif

	    if (strchr(fname,'/') == NULL) {

#if	CF_DEBUGS
	        debugprintf("dialcprog: no slash character\n") ;
#endif

	        rs = u_stat(progbuf,&sb) ;

#if	CF_DEBUGS
	        debugprintf("dialcprog: u_stat() rs=%d isreg=%u\n",
	            rs,S_ISREG(sb.st_mode)) ;
#endif

	        if ((rs < 0) || (! S_ISREG(sb.st_mode)) ||
	            (perm(progbuf,-1,-1,NULL,X_OK) < 0)) {

	            rs = findfilepath(NULL,progbuf,fname,X_OK) ;

#if	CF_DEBUGS
	            debugprintf("dialcprog: findfilepath() rs=%d\n",rs) ;
#endif

	            if (rs < 0)
	                goto badnoprog ;

	            if (rs > 0)
	                progfname = progbuf ;

	        } /* end if */

	    } /* end if (no slash character in original name) */

	} /* end if (finding the program) */

#if	CF_DEBUGS
	debugprintf("dialcprog: final rs=%d progfname=%s\n",rs,progfname) ;
	debugprintf("dialcprog: f_local=%u\n",f_local) ;
#endif

	if (rs >= 0) {
	    int		n = 0 ;
	    int		size ;
	    const char	**nev = NULL ;
	    if (ev == NULL) ev = (const char **) environ ;
	    if (ev != NULL) for (n = 0 ; ev[n] != NULL ; n += 1) ;
	    size = (n+2) * sizeof(const char *) ;
	    if ((rs = uc_malloc(size,&nev)) >= 0) {
	        int	i ;
	        int	j = 0 ;
	        char	enode[ENODELEN+1] ;
	        if (ev != NULL) {
	            int	f ;
	            for (i = 0 ; ev[i] != NULL ; i += 1) {
	                f = (ev[i][0] != varnode[0]) ;
	                f = f || (strkeycmp(ev[i],varnode) != 0) ;
	                if (f)
	                    nev[j++] = ev[i] ;
	            } /* end for */
	        }
	        if ((rs = snses(enode,ENODELEN,varnode,di.rhost)) >= 0) {
	            nev[j++] = enode ;
	            nev[j] = NULL ;

#if	CF_DEBUGS
	debugprintf("dialcprog: f_local=%u\n",f_local) ;
#endif

	            if (f_local) {
	                rs = dialprog(progfname,0,av,nev,fd2p) ;
	                fd = rs ;
	            } else {
	                di.pr = pr ;
	                di.opts = opts ;
	                di.f.errchan = (fd2p != NULL) ;
	                rs = dialremote(&di,progfname,av,nev,fd2p) ;
	                fd = rs ;
	            } /* end if (local or remote) */

	        } /* end if (snses) */
	        uc_free(nev) ;
	    } /* end if (memory allocation) */
	} /* end if (dialing out) */

/* we're done */
ret1:
	ids_release(&id) ;

ret0:

#if	CF_DEBUGS
	debugprintf("dialcprog: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;

/* bad stuff */
badpwd:
badnoprog:
	goto ret1 ;

}
/* end subroutine (dialcprog) */


/* local subroutines */


static int dialremote(dip,progfname,av,ev,fd2p)
struct dialinfo	*dip ;
const char	progfname[] ;
const char	**av ;
const char	**ev ;
int		*fd2p ;
{
	FILEBUF		rd ;
	int		rs = SR_OK ;
	int		cl ;
	int		fd ;
	int		afd, afd2 ;
	int		fbo ;
	int		opts ;
	int		to = TO_READ ;
	int		*ip = NULL ;
	const char	*un = dip->username ;
	char		rnode[MAXHOSTNAMELEN + 1] ;
	char		buf[BUFLEN + 1] ;

/* get the present working directory if we need it */

	if ((dip->opts & DIALOPT_PWD) && (dip->pwd[0] == '\0')) {
	    rs = getpwd(dip->pwd,MAXPATHLEN) ;
	    cl = rs ;
	    if (rs > 0) dip->pwd[cl] = '\0' ;
	}

/* dial the other end */

#if	CF_DEBUGS
	debugprintf("dialremote: dial rhost=%s\n",dip->rhost) ;
#endif

	if (rs >= 0)
	    rs = mkpath2(dip->tmpfname,dip->pr,CEXECERFNAME) ;

	if (rs >= 0)
	    rs = perm(dip->tmpfname,-1,-1,NULL,X_OK) ;

	if (rs < 0)
	    goto badnocexec ;

/* dial the designated node */

	ip = fd2p ;
	if (! (dip->opts & DIALOPT_NOLIGHT))
	    ip = NULL ;

#if	CF_DEBUGS
	debugprintf("dialremote: rcmdr() ip=%p\n",ip) ;
#endif

	if ((un != NULL) && (un[0] == '\0')) un = NULL ;
	rs = rcmdr(dip->rhost,un,dip->tmpfname,ip) ;
	fd = rs ;

#if	CF_DEBUGS
	debugprintf("dialremote: rcmdr() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto badrcmdr ;

/* indicate possible separate error channel on primary connection */

	if (ip != NULL)
	    dip->flags |= DIALCPROGMSG_FERRCHAN ;

/* initialize read buffer */

	fbo = FILEBUF_ONET ;
	rs = filebuf_start(&rd,fd,0L,BUFLEN,fbo) ;
	if (rs < 0)
	    goto badinit ;

/* get SYNC from the other end */

#if	CF_DEBUGS
	debugprintf("dialremote: findsync() \n") ;
#endif

	rs = findsync(&rd) ;

#if	CF_DEBUGS
	debugprintf("dialremote: findsync() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto badsync ;

	if (rs == 0) {

#if	CF_DEBUGS
	    debugprintf("dialremote: no SYNC!\n") ;
#endif

	    rs = SR_PROTO ;
	    goto badsync ;
	}

/* there should be more data left */

#if	CF_DEBUGS
	debugprintf("dialremote: getrnode() \n") ;
#endif

	rs = getrnode(&rd,rnode) ;

#if	CF_DEBUGS
	debugprintf("dialremote: getrnode() rs=%d rnode=%t\n",
	    rs,rnode,rs) ;
#endif

	if (rs < 0)
	    goto badproto ;

/* do we want to go into "light-weight" mode */

	if (! (dip->opts & DIALOPT_NOLIGHT)) {

#if	CF_DEBUGS
	    debugprintf("dialremote: LIGHT mode 1\n") ;
#endif

	    dip->a.addr = htonl(INADDR_ANY) ;
	    dip->a.port = (1 << 15) ;

	    rs = mklisten_start(dip,&dip->sout) ;
	    afd = rs ;

#if	CF_DEBUGS
	    debugprintf("dialremote: mklisten_start() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        goto badlisten ;

	    if (dip->f.errchan) {

	        rs = mklisten_start(dip,&dip->serr) ;
	        afd2 = rs ;

#if	CF_DEBUGS
	        debugprintf("dialremote: mklisten_start() rs=%d\n",rs) ;
#endif

	        if (rs < 0) {
	            u_close(afd) ;
	            mklisten_finish(dip,&dip->sout) ;
	            goto badlisten ;
	        }
	    }

	} /* end if (light-weight mode) */

/* write the "data" back to the other side */

#if	CF_DEBUGS
	debugprintf("dialremote: writing data\n") ;
#endif

	rs = sendvars(dip,fd,progfname,av,ev) ;

#if	CF_DEBUGS
	debugprintf("dialremote: sendvars() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto badsend ;

/* if we had an error channel, then find synchronization there also */

	if (ip != NULL) {
	    FILEBUF		rd2 ;

#if	CF_DEBUGS
	    debugprintf("dialremote: have an error channel FD=%d\n",*fd2p) ;
#endif

	    if ((rs = filebuf_start(&rd2,*fd2p,0L,BUFLEN,fbo)) >= 0) {

	        rs = findsync(&rd2) ;

#if	CF_DEBUGS
	        debugprintf("dialremote: findsync() rs=%d\n",rs) ;
#endif

	        if (rs == 0) rs = SR_PROTO ;

	        filebuf_finish(&rd2) ;
	    } /* end if (filebuf) */

	} /* end if */

	if (rs < 0)
	    goto badsync2 ;

/* read the final acknowledgement from the other side */

#if	CF_DEBUGS
	debugprintf("dialremote: final ACK uc_reade()\n") ;
#endif

	opts = (FM_EXACT | FM_TIMED) ;
	rs = uc_reade(fd,buf,4,to,opts) ;

#if	CF_DEBUGS
	debugprintf("dialremote: uc_reade() rs=%d\n", rs) ;
#endif

	if (rs < 0)
	    goto ret1 ;

	if (rs < 4) {
	    rs = SR_PROTO ;
	    goto ret1 ;
	}

	stdorder_rint(buf,&rs) ;

#if	CF_DEBUGS
	debugprintf("dialremote: CEXECER rs=%d\n", rs) ;
#endif

	if (rs < 0) {

#if	CF_DEBUGS
	    debugprintf("dialremote: bad return from CEXECER rs=%d\n",rs) ;
#endif

	    goto badack ;
	}

#if	CF_DEBUGS
	debugprintf("dialremote: got ACK\n") ;
#endif

/* are we in light-weight mode? */

	if (! (dip->opts & DIALOPT_NOLIGHT)) {
	    SOCKADDRESS	dummy ;
	    int		s, dummylen ;

#if	CF_DEBUGS
	    debugprintf("dialremote: LIGHT mode 2\n") ;
#endif

/* accept a connection on our primary listen socket */

	    dummylen = sizeof(SOCKADDRESS) ;
	    rs = uc_accepte(afd,&dummy,&dummylen,TO_ACCEPT) ;
	    s = rs ;

#if	CF_DEBUGS
	    debugprintf("dialremote: uc_accepte() rs=%d\n",rs) ;
#endif

	    if (rs < 0)
	        goto badaccept ;

/* close prior channel and make new connection on the primary channel */

	    u_close(fd) ;

	    fd = s ;
	    u_close(afd) ;

	    if (dip->f.errchan) {

	        dummylen = sizeof(SOCKADDRESS) ;
	        rs = uc_accepte(afd2,&dummy,&dummylen,TO_ACCEPT) ;
	        s = rs ;
	        if (rs < 0)
	            goto badaccept ;

	        *fd2p = s ;
	        u_close(afd2) ;

	    } /* end if */

	    mklisten_finish(dip,&dip->sout) ;

	    if (dip->f.errchan)
	        mklisten_finish(dip,&dip->serr) ;

	} /* end if (light-weight mode) */

	if (rs >= 0)
	    rs = u_setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,
	        (CONST char *) &one,sizeof(int)) ;

#if	CF_DEBUGS
	debugprintf("dialremote: u_setsockopt() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto badkeep ;

ret1:
	filebuf_finish(&rd) ;

badrcmdr:
badinit:
badnocexec:

#if	CF_DEBUGS
	debugprintf("dialremote: ret rs=%d fd=%u\n",rs,fd) ;
#endif

	return (rs >= 0) ? fd : rs ;

/* bad stuff */
badkeep:
badaccept:
badack:
badsync2:
badsend:
	if (! (dip->opts & DIALOPT_NOLIGHT)) {

	    u_close(afd) ;

	    mklisten_finish(dip,&dip->sout) ;

	    if (dip->f.errchan) {
	        u_close(afd2) ;
	        mklisten_finish(dip,&dip->serr) ;
	    }

	} /* end if (light-weight mode) */

badlisten:
badproto:
badsync:
	u_close(fd) ;

	if ((ip != NULL) && (fd2p != NULL))
	    u_close(*fd2p) ;

	goto ret1 ;

}
/* end subroutine (dialremote) */


/* find synchronization in the data stream */
static int findsync(bdp)
FILEBUF		*bdp ;
{
	STREAMSYNC	ps ;
	int		rs = SR_OK ;
	int		i ;
	int		len ;
	int		tlen = 0 ;
	int		f_sync = FALSE ;
	char		syncbuf[MKCEXSYNC_REQLEN + 1] ;
	char		databuf[DATABUFLEN + 1] ;

#if	CF_DEBUGS && CF_DEBUGFSYNC
	debugprintf("findsync: ent\n") ;
#endif

/* make the synchronization stream sequence */

	len = mkcexsync(syncbuf,MKCEXSYNC_REQLEN) ;

#if	CF_DEBUGS && CF_DEBUGFSYNC
	debugprintf("findsync: mkcexsync() len=%d\n",len) ;
#endif

/* initialize for finding synchronization */

	if ((rs = streamsync_start(&ps,syncbuf,len)) >= 0) {

	    while ((rs >= 0) && (! f_sync)) {

#if	CF_DEBUGS && CF_DEBUGFSYNC
	        debugprintf("findsync: filebuf_read() \n") ;
#endif

	        rs = filebuf_read(bdp,databuf,1,TO_SYNC) ;
	        len = rs ;
	        if (rs < 0)
	            break ;

	        if (len == 0)
	            break ;

	        for (i = 0 ; i < len ; i += 1) {

#if	CF_DEBUGS && CF_DEBUGFSYNC
	            debugprintf("findsync: ch=%02x(%c)\n",
	                databuf[i],
	                ((isprint(databuf[0])) ? databuf[0] : ' ')) ;
#endif

	            rs = streamsync_test(&ps,databuf[i]) ;

#if	CF_DEBUGS && CF_DEBUGFSYNC
	            debugprintf("findsync: streamsync_test() rs=%d\n", rs) ;
#endif

	            f_sync = (rs > 0) ;
	            if (f_sync) break ;
	            if (rs < 0) break ;
	        } /* end for */

	        tlen += i ;

	    } /* end while */

	    streamsync_finish(&ps) ;
	} /* end if (streamsync) */

#if	CF_DEBUGS && CF_DEBUGFSYNC
	debugprintf("findsync: ret rs=%d f_sync=%d\n",rs,f_sync) ;
#endif

	return (rs >= 0) ? f_sync : rs ;
}
/* end subroutine (findsync) */


/* get the remote node name */
static int getrnode(bdp,rnode)
FILEBUF		*bdp ;
char		rnode[] ;
{
	int		rs = SR_OK ;
	int		state = 0 ;
	int		i, j ;
	int		len ;
	int		tlen = 0 ;
	int		nlen = 0 ;
	int		f ;
	short		sw ;
	char		nlenbuf[5] ;
	char		databuf[DATABUFLEN + 1] ;

#if	CF_DEBUGS && CF_DEBUGRNODE
	debugprintf("getrnode: ent\n") ;
#endif

	j = 0 ;
	while ((rs >= 0) && (state < 4)) {

#if	CF_DEBUGS && CF_DEBUGRNODE
	    debugprintf("getrnode: while_loop rs=%d state=%d\n",rs,state) ;
#endif

	    rs = filebuf_read(bdp,databuf,1,TO_SYNC) ;
	    len = rs ;
	    if (rs < 0) break ;

#if	CF_DEBUGS && CF_DEBUGRNODE
	    {
	        char	hexbuf[100] ;
	        mkhexstr(hexbuf,20,databuf,MIN(20,len)) ;
	        debugprintf("getrnode: read= %s\n",hexbuf) ;
	    }
#endif

	    if (len == 0) break ;

	    tlen += len ;
	    for (i = 0 ; (i < len) && (state < 4) ; i += 1) {

#if	CF_DEBUGS && CF_DEBUGRNODE
	        debugprintf("getrnode: for_loop i=%d state=%d\n",i,state) ;
#endif

	        switch (state) {

	        case 0:
	        case 1:
	            nlenbuf[state] = (databuf[i] & 0xff) ;
	            if (state == 1) {
	                stdorder_rshort(nlenbuf,&sw) ;
	                nlen = sw ;
#if	CF_DEBUGS && CF_DEBUGRNODE
	                debugprintf("getrnode: sw=%d\n",sw) ;
	                debugprintf("getrnode: nlen=%d\n",nlen) ;
#endif
	                if (nlen > MAXHOSTNAMELEN) {
	                    rs = SR_INVALID ;
	                    break ;
	                }
	            }
	            state += 1 ;
	            break ;

	        case 2:
	            rnode[j++] = databuf[i] ;
	            if (j == nlen) state += 1 ;
	            break ;

	        case 3:
	            f = (databuf[i] == CH_SYNC) ;
	            rs = (f) ? SR_OK : SR_PROTO ;
	            if (rs < 0) break ;
	            state += 1 ;
	            break ;

	        } /* end switch */

#ifdef	COMMENT
	        if (state > 3) break ;
#endif

#if	CF_DEBUGS && CF_DEBUGRNODE
	        debugprintf("getrnode: bottom for_loop i=%d state=%d\n",
	            i,state) ;
#endif

	        if (rs < 0) break ;
	    } /* end for */

#if	CF_DEBUGS && CF_DEBUGRNODE
	    debugprintf("getrnode: out for_loop rs=%d i=%d \n",rs,i) ;
#endif

	    tlen += i ;

	} /* end while */

	rnode[nlen] = '\0' ;

#if	CF_DEBUGS && CF_DEBUGRNODE
	debugprintf("getrnode: ret rs=%d nlen=%d\n",rs,nlen) ;
#endif

	return (rs >= 0) ? nlen : rs ;
}
/* end subroutine (getrnode) */


static int mklisten_start(DIALINFO *dip,DIALINFO_SI *sip)
{
	const int	af = AF_INET4 ;
	const int	pf = PF_INET4 ;
	const int	st = SOCK_STREAM ;
	const int	pt = IPPROTO_TCP ;
	int		rs ;
	int		s = -1 ;

	if ((rs = sockaddress_start(&sip->sa,af,&dip->a.addr,0,0)) >= 0) {
	    sip->salen = rs ;
	    if ((rs = u_socket(pf,st,pt)) >= 0) {
	        s = rs ;
	        if ((rs = mklisten_bind(dip,sip,s)) >= 0) {
	            rs = u_listen(s,1) ;
	        }
	        if (rs < 0)
	            u_close(s) ;
	    } /* end if (socket) */
	    if (rs < 0)
	        sockaddress_finish(&sip->sa) ;
	} /* end if (sockaddress) */

	return (rs >= 0) ? s : rs ;
}
/* end subroutine (mklisten_start) */


static int mklisten_finish(DIALINFO *dip,DIALINFO_SI *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = sockaddress_finish(&sip->sa) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (mklisten_finish) */


static int mklisten_bind(DIALINFO *dip,DIALINFO_SI *sip,int s)
{
	struct sockaddr	*sap ;
	int		rs ;

#ifdef	COMMENT
	{
	    struct sockaddr_in	*sapin ;
	    int	i ;

	    for (i = dip->a.port ; i < (1 << 16) ; i += 1) {

	        sapin = (struct sockaddr_in *) &sip->sa ;
	        sapin->sin_port = htons(i) ;

	        sap = (struct sockaddr *) &sip->sa ;
	        rs = u_bind(s,sap,sip->salen) ;

	        if (rs >= 0) break ;
	    } /* end for */

	    dip->a.port = i ;
	}
#else /* COMMENT */
	{
	    sap = (struct sockaddr *) &sip->sa ;
	    if ((rs = u_bind(s,sap,sip->salen)) >= 0) {
	        sip->salen = sizeof(SOCKADDRESS) ;
	        rs = u_getsockname(s,sap,&sip->salen) ;
	    }
	}
#endif /* COMMENT */

#if	CF_DEBUGS
	{
	    ushort	usw ;
	    struct sockaddr_in	*sainp = (struct sockaddr_in *) &sip->sa ;
	    usw = ntohs(sainp->sin_port) ;
	    debugprintf("mklisten_bind: port=%u\n",(int) usw) ;
	}
#endif /* CF_DEBUGS */

	return rs ;
}
/* end subroutine (mklisten_bind) */


/* send the various variables over to the other side */
static int sendvars(dip,fd,progfname,av,ev)
DIALINFO	*dip ;
int		fd ;
const char	progfname[] ;
const char	**av ;
const char	**ev ;
{
	ENVLIST		vars ;
	FILEBUF		wr ;
	const int	envlen = ENVBUFLEN ;
	int		rs, rs1 ;
	int		i, cl ;
	int		fbo ;
	int		rtype ;
	const char	*cp ;
	char		buf[BUFLEN + 1] ;
	char		envbuf[ENVBUFLEN + 1] ;

	rs = envlist_start(&vars,NENVS) ;
	if (rs < 0)
	    goto ret0 ;

	fbo = FILEBUF_ONET ;
	rs = filebuf_start(&wr,fd,0L,BUFLEN,fbo) ;
	if (rs < 0)
	    goto ret1 ;

/* our (local) node name */

	rtype = dialcprogmsgtype_nodename ;
	rs = filebuf_sendrecord(&wr,rtype,dip->nodename,-1) ;

/* program file name */

	if (rs >= 0) {
	    rtype = dialcprogmsgtype_fname ;
	    rs = filebuf_sendrecord(&wr,rtype,progfname,-1) ;
	}

/* arguments */

	rtype = dialcprogmsgtype_arg ;
	for (i = 0 ; (rs >= 0) && (av[i] != NULL) ; i += 1) {
	    rs = filebuf_sendrecord(&wr,rtype,av[i],-1) ;
	} /* end for */

/* environment */

#if	CF_DEBUGS
	debugprintf("dialcprog/sendvars: environment vars\n") ;
#endif

	if ((rs >= 0) && (dip->rhost != NULL)) {
	    int		rl = -1 ;
	    int		el ;
	    const char	*rhost = dip->rhost ;
	    const char	*tp ;
	    if ((tp = strchr(rhost,'.')) != NULL) rl = (tp-rhost) ;
	    if ((rs = sncpy3w(envbuf,envlen,varnode,"=",rhost,rl)) >= 0) {
	        el = rs ;
	        if ((rs = envlist_add(&vars,envbuf,el)) >= 0) {
	            rtype = dialcprogmsgtype_env ;
	            rs = filebuf_sendrecord(&wr,rtype,envbuf,el) ;
	        }
	    }
	} /* end if (remote node as environment) */

	if ((rs >= 0) && (ev != NULL)) {

	    for (i = 0 ; (rs >= 0) && (ev[i] != NULL) ; i += 1) {

	        rs1 = envlist_present(&vars,ev[i],-1,NULL) ;

	        if (rs1 == SR_NOTFOUND) {
	            int	f = TRUE ;
	            f = f && (ev[i][0] != '-') ;
	            f = f && (matkeystr(envbads,ev[i],-1) < 0) ;
	            if (f) {

#if	CF_DEBUGS
	                debugprintf("dialcprog/sendvars: a e=>%t<\n",
	                    ev[i]) ;
#endif

	                rs = envlist_add(&vars,ev[i],-1) ;

	                if (rs >= 0) {
	                    rtype = dialcprogmsgtype_env ;
	                    rs = filebuf_sendrecord(&wr,rtype,ev[i],-1) ;
	                }

	            } else if (strkeycmp(VARDISPLAY,ev[i]) == 0) {

#if	CF_DEBUGS
	                debugprintf("dialcprog/sendvars: display=>%s<\n",
				ev[i]) ;
#endif

/* but let DISPLAY through, if it gets fixed properly */

	                cp = ev[i] ;
	                cl = -1 ;
	                if (ev[i][8] == ':') {
	                    const int	elen = ENVBUFLEN ;
	                    const char	*nn = dip->nodename ;
	                    const char	*var = VARDISPLAY ;
	                    const char	*rp = (cp+8) ;
	                    char	*bp = envbuf ;

	                    if ((cl = sncpy4(bp,elen,var,"=",nn,rp)) > 0) {
	                        cp = envbuf ;
	                    }

#if	CF_DEBUGS
	                    debugprintf("dialcprog/sendvars: n=>%s<\n",cp) ;
#endif

	                } /* end if */

	                if ((rs = envlist_add(&vars,cp,-1)) >= 0) {
	                    rtype = dialcprogmsgtype_env ;
	                    rs = filebuf_sendrecord(&wr,rtype,cp,cl) ;
	                }

	            } /* end if (good or bad environment variables) */

	        } /* end if (variable was not already present) */

	    } /* end for (looping through environment variables) */

	} /* end if (had some environment) */

/* present working directory and maybe more environment */

	if ((rs >= 0) && (dip->opts & DIALOPT_PWD) &&
	    (dip->pwd[0] != '\0')) {

	    rtype = dialcprogmsgtype_pwd ;
	    if ((rs = filebuf_sendrecord(&wr,rtype,dip->pwd,-1)) >= 0) {
	        cchar	*varpwd = VARPWD ;

	        rs1 = envlist_present(&vars,varpwd,-1,NULL) ;
	        if (rs1 == SR_NOTFOUND) {

#if	CF_DEBUGS
	            debugprintf("dialcprog/sendvars: adding PWD\n") ;
#endif

	            cp = envbuf ;
	            rs = snses(envbuf,ENVBUFLEN,varpwd,dip->pwd) ;
	            cl = rs ;
	            if (rs >= 0) {
	                rtype = dialcprogmsgtype_env ;
	                rs = filebuf_sendrecord(&wr,rtype,cp,cl) ;
	            }

	        } else
	            rs = rs1 ;
	    } /* end if */

	} /* end if (PWD) */

/* what about light-weight sockets? */

	if ((rs >= 0) && (! (dip->opts & DIALOPT_NOLIGHT))) {
	    struct dialcprogmsg_light	m5 ;

	    memset(&m5,0,sizeof(struct dialcprogmsg_light)) ;
	    m5.salen1 = dip->sout.salen ;
	    m5.salen2 = dip->serr.salen ;

	    memcpy(&m5.saout,&dip->sout.sa,dip->sout.salen) ;

	    if (dip->f.errchan) {
	        memcpy(&m5.saerr,&dip->serr.sa,dip->serr.salen) ;
	    } else {
	        memset(&m5.saerr,0,sizeof(SOCKADDRESS)) ;
	    }

	    cl = dialcprogmsg_light(buf,BUFLEN,0,&m5) ;

#if	CF_DEBUGS
	    debugprintf("dialcprog/sendvars: cl=%d\n",cl) ;
#endif

#if	CF_DEBUGS
	    {
	        char	hexbuf[100 + 1] ;
	        mkhexstr(hexbuf,100,buf,12) ;
	        debugprintf("dialcprog/sendvars: m5> %s\n",hexbuf) ;
	    }
#endif

	    rs = filebuf_write(&wr,buf,cl) ;

	} /* end if (light-weight sockets) */

/* indicate the end of all data records */

	if (rs >= 0) {
	    struct dialcprogmsg_end	m0 ;

	    memset(&m0,0,sizeof(struct dialcprogmsg_end)) ;
	    m0.flags = dip->flags ;
	    m0.opts = dip->opts ;

	    cl = dialcprogmsg_end(buf,BUFLEN,0,&m0) ;

	    rs = filebuf_write(&wr,buf,cl) ;

	} /* end if (end of data records) */

/* finish up */

#ifdef	COMMENT
	rs1 = filebuf_flush(&wr) ;
	if (rs >= 0) rs = rs1 ;
#endif

	rs1 = filebuf_finish(&wr) ;
	if (rs >= 0) rs = rs1 ;

/* we're out of here */
ret1:
	rs1 = envlist_finish(&vars) ;
	if (rs >= 0) rs = rs1 ;

ret0:

#if	CF_DEBUGS
	debugprintf("dialcprog/sendvars: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (sendvars) */


static int filebuf_sendrecord(FILEBUF *bp,int type,cchar *sp,int sl)
{
	int		rs ;
	int		tlen = 0 ;
	char		data[3] ;

	if (sl < 0)
	    sl = strlen(sp) ;

	if ((sl >= USHRT_MAX) || (sl > VBUFLEN))
	    return SR_TOOBIG ;

#if	CF_DEBUGS
	debugprintf("dialcprog/filebuf_sendrecord: t=%u s=>%t<\n",
	    type,sp,strlinelen(sp,sl,40)) ;
#endif

	data[0] = type ;
	if ((rs = stdorder_wushort((data+1),(sl+1))) >= 0) {
	    if ((rs = filebuf_write(bp,data,3)) >= 0) {
	        tlen += rs ;
	        if ((rs = filebuf_write(bp,sp,sl)) >= 0) {
	    	    tlen += rs ;
	    	    data[0] = '\0' ;
	    	    rs = filebuf_write(bp,data,1) ;
	    	    tlen += rs ;
		}
	    }
	}

#if	CF_DEBUGS && 0
	debugprintf("dialcprog/filebuf_sendrecord: ret rs=%d tlen=%u\n",
	    rs,tlen) ;
#endif

	return (rs >= 0) ? tlen : rs ;
}
/* end subroutine (filebuf_sendrecord) */


static int getclusters(dbip,sp,nodename)
struct dbinfo	*dbip ;
vecstr		*sp ;
const char	nodename[] ;
{
	const int	elen = NODEDB_ENTLEN ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		c = 0 ;

#if	CF_DEBUGS && 0
	debugprintf("getclusters: ent \n") ;
	debugprintf("getclusters: nodename=%s\n",nodename) ;
#endif

	if (dbip->f_node) {
	    NODEDB_CUR	cur ;
	    NODEDB_ENT	ste ;
	    char	ebuf[NODEDB_ENTLEN + 1] ;

	    if ((rs = nodedb_curbegin(&dbip->node,&cur)) >= 0) {

	        while (rs >= 0) {

#if	CF_DEBUGS && 0
	            debugprintf("getclusters: node whiletop\n") ;
#endif

	            rs1 = nodedb_fetch(&dbip->node,nodename,&cur,
	                &ste,ebuf,elen) ;

#if	CF_DEBUGS && 0
	            debugprintf("getclusters: nodedb_fetch() rs=%d\n",rs1) ;
#endif

	            if (rs1 == SR_NOTFOUND) break ;
	            rs = rs1 ;
	            if (rs < 0) break ;

	            if ((ste.clu != NULL) && (ste.clu[0] != '\0')) {

#if	CF_DEBUGS && 0
	                debugprintf("getclusters: cluster=%s\n",ste.clu) ;
#endif

	                if (vecstr_find(sp,ste.clu) == SR_NOTFOUND) {
	                    c += 1 ;
	                    rs = vecstr_add(sp,ste.clu,-1) ;
	                    if (rs < 0) break ;
	                }

	            } /* end if (got one) */

#if	CF_DEBUGS && 0
	            debugprintf("getclusters: node whilebot\n") ;
#endif

	        } /* end while */

#if	CF_DEBUGS && 0
	        debugprintf("getclusters: node whileout\n") ;
#endif

	        nodedb_curend(&dbip->node,&cur) ;
	    } /* end if (cursor) */

	} /* end if (DB lookup) */

#if	CF_DEBUGS && 0
	debugprintf("getclusters: middle\n") ;
#endif

/* try the CLUSTER table also */

	if ((rs >= 0) && dbip->f_cluster) {
	    CLUSTERDB		*cdb = &dbip->cluster ;
	    CLUSTERDB_CUR	cur ;
	    const int		clen = NODENAMELEN ;
	    const char		*nn = nodename ;
	    char		cname[NODENAMELEN + 1] ;


#if	CF_DEBUGS && 0
	    debugprintf("getclusters: cluster lookup\n") ;
#endif

	    if ((rs = clusterdb_curbegin(cdb,&cur)) >= 0) {

	        while (rs >= 0) {

#if	CF_DEBUGS && 0
	            debugprintf("getclusters: cluster whiletop\n") ;
#endif

	            rs1 = clusterdb_fetchrev(cdb,nn,&cur,cname,clen) ;
	            if (rs1 == SR_NOTFOUND) break ;

	            if (rs >= 0) {
	                if (vecstr_find(sp,cname) == SR_NOTFOUND) {
	                    c += 1 ;
	                    rs = vecstr_add(sp,cname,-1) ;
	                    if (rs < 0) break ;
	                }
	            }

#if	CF_DEBUGS && 0
	            debugprintf("getclusters: cluster whilebot\n") ;
#endif

	        } /* end while (fetchrev) */

#if	CF_DEBUGS && 0
	        debugprintf("getclusters: cluster whileout\n") ;
#endif

	        clusterdb_curend(cdb,&cur) ;
	    } /* end if (cursor) */

	} /* end if */

#if	CF_DEBUGS && 0
	debugprintf("getclusters: ret rs=%d c=%d\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (getclusters) */


#if	CF_BESTNODE

static int bestnode_open(op,dip,pr,fname)
BESTNODE	*op ;
struct dialinfo	*dip ;
const char	pr[] ;
const char	fname[] ;
{
	int		rs ;
	char		tmpfname[MAXPATHLEN + 1] ;

	memset(op,0,sizeof(BESTNODE)) ;

	mkpath2(tmpfname,pr,fname) ;

#if	CF_DEBUGS
	debugprintf("bestnode_open: msfname=%s\n",tmpfname) ;
#endif

	rs = msfile_open(&op->ms,tmpfname,O_RDWR,0666) ;

#if	CF_DEBUGS
	debugprintf("bestnode_open: msfile_open() 1 rs=%d\n",rs) ;
#endif

	if (rs == SR_ACCESS) {
	    rs = msfile_open(&op->ms,tmpfname,O_RDONLY,0666) ;
#if	CF_DEBUGS
	    debugprintf("bestnode_open: msfile_open() 2 rs=%d\n",rs) ;
#endif
	}

	if (rs >= 0) {
	    op->f_open = TRUE ;
	    op->dip = dip ;
	}

#if	CF_DEBUGS
	debugprintf("bestnode_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (bestnode_open) */

static int bestnode_close(op)
BESTNODE	*op ;
{
	int		rs = SR_NOTOPEN ;

	if (op == NULL) return SR_OK ;

	if (op->f_open) {
	    op->f_open = FALSE ;
	    rs = msfile_close(&op->ms) ;
	}

	return rs ;
}
/* end subroutine (bestnode_close) */

static int bestnode_get(op,dbip,cnp,nodename,ap)
BESTNODE	*op ;
struct dbinfo	*dbip ;
vecstr		*cnp ;
char		nodename[] ;
double		*ap ;
{
	MSFILE_ENT	se ;
	time_t		daytime ;
	double		bestavail ;
	uint		mintime, nowtime ;
	int		rs = SR_NOTFOUND ;
	int		rs1 ;
	int		i, nl, cl ;
	int		minspeed = INT_MAX ;
	int		c ;
	const char	*np = NULL ;

	if (op == NULL) return SR_FAULT ;

	if (! op->f_open) return SR_NOTOPEN ;

/* start in */

	daytime = time(NULL) ;

	nowtime = (uint) daytime ;
	mintime = 0 ;
	if (nowtime > TO_VALID)
	    mintime = (nowtime - TO_VALID) ;

	nodename[0] = '\0' ;

/* first find the minimum speed of all nodes */

	c = 0 ;
	for (i = 0 ; vecstr_get(cnp,i,&np) >= 0 ; i += 1) {
	    if (np == NULL) continue ;

#if	CF_DEBUGS
	    debugprintf("bestnode_get: cluster node=%s\n",
	        np) ;
#endif

	    rs1 = msfile_match(&op->ms,daytime,np,-1,&se) ;

#if	CF_DEBUGS
	    debugprintf("bestnode_get: msfile_match() rs=%d\n",rs1) ;
#endif

	    if ((rs1 >= 0) && (se.speed != 0) && 
	        ((daytime - se.utime) < TO_UP)) {

#if	CF_DEBUGS
	        debugprintf("bestnode_get: node=%s speed=%u\n",
	            se.nodename,se.speed) ;
#endif

	        c += 1 ;
	        if (se.speed < minspeed)
	            minspeed = se.speed ;

	    } /* end if (got one) */

	} /* end for (finding minimum speed) */

#if	CF_DEBUGS
	debugprintf("bestnode_get: minspeed=%u\n",minspeed) ;
#endif

/* then find the one with the most available computation */

	bestavail = -1000.0 ;
	rs = SR_NOTFOUND ;
	if (c > 0) {
	    double	capacity, used ;
	    double	avail, empty ;

#if	CF_DEBUGS
	    debugprintf("bestnode_get: finding most available computation\n") ;
#endif

	    c = 0 ;
	    for (i = 0 ; vecstr_get(cnp,i,&np) >= 0 ; i += 1) {
	        if (np == NULL) continue ;

#if	CF_DEBUGS
	        debugprintf("bestnode_get: node=%s\n", np) ;
#endif

	        rs = msfile_match(&op->ms,daytime,np,-1,&se) ;

#if	CF_DEBUGS
	        debugprintf("bestnode_get: msfile_match() rs=%d\n",rs) ;
#endif

	        if (rs < 0)
	            continue ;

	        if ((se.flags & MSFLAG_MDISABLED) || (se.dtime != 0))
	            continue ;

	        if (se.utime > mintime) {

	            capacity = (double) (se.speed * se.ncpu) ;
	            used = ((double) se.la[0]) / FSCALE ;
	            empty = ((double) se.ncpu) - used ;
	            used *= ((double) minspeed) ;

	            avail = capacity - used ;

#if	CF_DEBUGS
	            debugprintf("bestnode_get: node=%s avail=%8.3f\n",
	                se.nodename,avail) ;
#endif

	            if ((! op->dip->f.empty) || (empty > 0.0)) {

	                c += 1 ;
	                if (avail > bestavail) {

#if	CF_DEBUGS
	                    debugprintf("bestnode_get: best "
	                        "node=%s avail=%8.3f\n",
	                        se.nodename,avail) ;
#endif

	                    bestavail = avail ;
	                    cl = MSFILE_NODENAMELEN ;
	                    nl = strwcpy(nodename,se.nodename,cl) -
	                        nodename ;

#if	CF_DEBUGS
	                    debugprintf("bestnode_get: nodename=%s\n",
	                        nodename) ;
#endif

	                } /* end if (better) */

	            } /* end if (found one at all) */

	        } /* end if (got one) */

	    } /* end for */

#if	CF_DEBUGS
	    debugprintf("bestnode_get: most result rs=%d c=%u\n",rs,c) ;
#endif

	} /* end if (had at least one entry) */

/* done */

	msfile_close(&op->ms) ;

/* finish up */

	if (ap != NULL)
	    *ap = bestavail ;

	if ((rs == SR_NOTFOUND) && (c > 0))
	    rs = SR_OK ;

	if ((rs >= 0) && (nodename[0] == '\0'))
	    rs = SR_NOTFOUND ;

#if	CF_DEBUGS
	debugprintf("bestnode_get: ret rs=%d nodename=%s\n",
	    rs,nodename) ;
#endif

	return (rs >= 0) ? nl : rs ;
}
/* end subroutine (bestnode_get) */

#endif /* CF_BESTNODE */


static int loadlocalnames(pr,dip,lnp)
const char	*pr ;
DIALINFO	*dip ;
vecstr		*lnp ;
{
	const int	hlen = MAXHOSTNAMELEN ;
	int		rs = SR_OK ;

	if (rs >= 0)
	if ((rs = vecstr_add(lnp,dip->nodename,-1)) >= 0) {
	    char	hostname[MAXHOSTNAMELEN+1] ;
	    if ((rs = uc_gethostname(hostname,hlen)) >= 0) {
	        int	hl = rs ;
	        const char	*tp ;
	        if ((tp = strnrchr(hostname,hl,'.')) != NULL) {
	            hl = (tp-hostname) ;
	            hostname[hl] = '\0' ;
	        }
	        if ((rs = vecstr_adduniq(lnp,hostname,hl)) >= 0) {
	    	    if ((rs = vecstr_add(lnp,LOCALNODE,-1)) >= 0) {
	    		if ((rs = vecstr_add(lnp,LOCALHOST,-1)) >= 0) {
	    		    rs = getnodenames(pr,lnp,dip->nodename) ;
	    		    if (rs == SR_NOENT) rs = SR_NONET ;
			}
		    }
		} /* end if (vecstr_adduniq) */
	    } /* end if (uc_gethostname) */
	} /* end if (ok) */

	return rs ;
}
/* end subroutine (loadlocalnames) */


static int getnodenames(pr,nsp,node)
const char	pr[] ;
vecstr		*nsp ;
const char	node[] ;
{
	HOSTINFO	hi ;
	HOSTINFO_CUR	hc ;
	const int	af = AF_UNSPEC ;
	int		rs ;
	int		nl ;
	int		n = 0 ;
	const char	*np ;

	if ((rs = hostinfo_start(&hi,af,node)) >= 0) {

	    if ((rs = hostinfo_curbegin(&hi,&hc)) >= 0) {

	        while ((nl = hostinfo_enumname(&hi,&hc,&np)) >= 0) {

/* don't remove domain part since the node could be in another domain */

#ifdef	COMMENT
	            {
	                const char	*tp ;
	                if ((tp = strnchr(np,nl,'.')) != NULL)
	                    nl = tp - np ;
	            }
#endif

/* don't remove domain part since the node could be in another domain */

	            if ((nl > 0) && (vecstr_findn(nsp,np,nl) < 0)) {
	                rs = vecstr_add(nsp,np,nl) ;
	                if (rs < 0) break ;
	                n += 1 ;
	            } /* end if */

	        } /* end while */

	        hostinfo_curend(&hi,&hc) ;
	    } /* end if (cursor) */

	    hostinfo_finish(&hi) ;
	} /* end if (hostinfo) */

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (getnodenames) */


