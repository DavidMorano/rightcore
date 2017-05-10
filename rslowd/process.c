/* process */


#define	CF_DEBUG	1



#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<dirent.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<time.h>
#include	<ftw.h>

#include	<bfile.h>
#include	<field.h>
#include	<logfile.h>
#include	<vecelem.h>
#include	<vechand.h>
#include	<vecstr.h>
#include	<pcsconf.h>
#include	<msg.h>
#include	<msgheaders.h>
#include	<ema.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#undef	BUFLEN
#define	BUFLEN	100



/* external subroutines */

extern int	msg_init() ;
extern int	msg_matenv(), msg_mathead() ;
extern int	msgheaders() ;
extern int	ng_parse() ;
extern int	unlinkd() ;
extern int	path_init(), path_parse() ;

extern char	*malloc_str() ;
extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_edate(), *timestr_hdate() ;

extern void	list_articles() ;


/* external variables */

extern struct global	g ;

extern struct pcsconf	p ;

extern struct userinfo	u ;


/* forward references */

static int	ph_write() ;
static int	ext_id() ;
static int	didheader() ;






int process(ifp,aqp)
bfile		*ifp ;
struct vecelem	*aqp ;
{
	bfile			afile, *afp = &afile ;

	struct ustat		sb ;

	struct msg		amsg, *msgp = &amsg ;

	struct msg_env		*mep ;

	struct msg_header	*mhp ;

	struct msg_instance	*mip ;

	struct msg_line		*mlp ;

	vecstr		m ;

	struct newsgroup	*ngp ;

	struct article		a ;

	time_t	subtime ;

	offset_t	offset, off_clen, off_start, off_finish ;

	int	i, j, len, l ;
	int	narticles = 0 ;
	int	clen ;
	int	rs, rs2 ;
	int	f_bol, f_eol ;

	char	linebuf[LINELEN + 1] ;
	char	**msghvalues ;
	char	tmpfname[MAXPATHLEN + 1] ;
	char	afname[MAXPATHLEN + 1] ;
	char	hname[LINELEN + 1] ;
	char	buf[BUFLEN + 1] ;
	char	*cp ;


#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("process: entered\n") ;
#endif

	afname[0] = '\0' ;

/* find the start of a message */

	offset = 0L ;
	len = breadline(ifp,linebuf,LINELEN) ;

	offset += len ;

/* top of loop */

	while (len > 0) {

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("process: top of loop\n") ;
#endif

	    while ((len > 0) && (! msg_matenv(linebuf,len)) &&
	        (! msg_mathead(linebuf,len,hname)) &&
	        ((len > 1) || (linebuf[0] != '\n'))) {

	        len = breadline(ifp,linebuf,LINELEN) ;

	        offset += len ;

	    } /* end while */

	    if (len <= 0) break ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("process: got a message\n") ;
#endif

/* process the headers of this message */

	    l = msg_init(msgp,ifp,offset,linebuf,len) ;

	    offset += l ;

	    msgheaders_init(&msghvalues,msgp) ;


/* OK, process what we can from this article */

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("process: got stuff from it, let's process\n") ;
#endif


/* get the article ID */

	    a.f_jobfile = FALSE ;
	    a.f_nopath = TRUE ;
	    a.af[0] = '\0' ;
	    a.articleid[0] = '\0' ;
	    if (g.arg_articleid != NULL)
	        strcpy(a.articleid,g.arg_articleid) ;

	    else if (msghvalues[HI_ARTICLEID] != NULL)
	        ext_id(a.articleid,msghvalues[HI_ARTICLEID]) ;

	    if ((a.articleid[0] == '\0') || (isdigit(a.articleid[0]))) {

	        a.f_jobfile = TRUE ;
	        mkjobfile("/tmp",0644,tmpfname) ;

	        l = sfbasename(tmpfname,-1,&cp) ;

	        unlinkd(tmpfname,-1) ;

	        strwcpy(a.af,cp,l) ;

	        if (a.articleid[0] == '\0')
	            strcpy(a.articleid,a.af) ;

	    } else
	        strcpy(a.af,a.articleid) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("process: articleid=%s af=%s\n",
	            a.articleid,a.af) ;
#endif


/* message ID */

	    a.messageid[0] = '\0' ;
	    if (msghvalues[HI_MESSAGEID] != NULL)
	        ext_id(a.messageid,msghvalues[HI_MESSAGEID]) ;

	    if (a.messageid[0] == '\0')
	        sprintf(a.messageid,"%s@%s.%s",
	            a.af,
	            u.nodename,
	            u.domainname) ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("process: messageid=%s\n",a.messageid) ;
#endif


/* do we have this article already in our database ? */




/* get the newsgroups that it has */

	    ng_init(&a.ngs) ;

	    if (ng_count(&g.arg_ngs) > 0)
	        ng_copy(&a.ngs,&g.arg_ngs) ;

	    else if (msghvalues[HI_NEWSGROUPS] != NULL)
	        ng_parse(&a.ngs,msghvalues[HI_NEWSGROUPS]) ;

	    else
	        ng_parse(&a.ngs,"misc") ;

#if	CF_DEBUG
	    if (g.debuglevel > 1) {

	        debugprintf("process: newsgroups so far\n") ;

	        for (i = 0 ; ng_get(&a.ngs,i,&ngp) >= 0 ; i += 1) {

	            if (ngp == NULL) continue ;

	            debugprintf("process: ng=%s\n",ngp->name) ;

	        }

	    }
#endif

/* is it a control message ? */

	    a.control[0] = '\0' ;
	    if (msghvalues[HI_CONTROL] != NULL) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("process: it's a control message\n") ;
#endif

	        strcpy(a.control,msghvalues[HI_CONTROL]) ;

	        sprintf(afname,"%s/%s",g.bbcontroldir,a.af) ;

	        rs = OK ;

	    } else {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("process: its a regular message\n") ;
#endif


/* do we have at least one valid newsgroup for us ? */

	        a.ngdir[0] = '\0' ;
	        for (i = 0 ; (rs = vecstrget(&a.ngs,i,&ngp)) >= 0 ; i += 1) {

	            if (ngp == NULL) continue ;

	            if (ngp->dir == NULL) {

	                if (pcs_ngdir(g.pcs,g.bbnewsdir,ngp->name,a.ngdir) >= 0)
	                    ngp->dir = malloc_str(a.ngdir) ;

	                else
	                    ngp->dir = malloc_str("") ;

	            }

	            if (ngp->dir[0] != '\0') break ;

	        } /* end for */

	        if (rs >= 0) {

	            strcpy(a.ngdir,ngp->dir) ;

	            sprintf(afname,"%s/%s/%s",g.bbnewsdir,ngp->dir,a.af) ;

#if	CF_DEBUG
	            if (g.debuglevel > 1) {

	                debugprintf("process: have directory for newsgroup\n") ;

	                debugprintf("process: ngdir=%s\n",a.ngdir) ;

	            }
#endif

	        }


	    } /* end if (where to create the file) */


#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("process: continuing\n") ;
#endif


/* get the content length if this article has one */

	    clen = -1 ;
	    if (msghvalues[HI_CLEN] != NULL) {

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("process: we have a 'content-length' header\n") ;
#endif

	        if (cfdec(msghvalues[HI_CLEN],-1,&clen) < 0)
	            clen = -1 ;

	    }

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("process: clen=%d\n",clen) ;
#endif

/* do we already have this article ? */

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("process: afname=%s\n",afname) ;
#endif

	    rs2 = OK ;
	    if ((afname[0] != '\0') && (stat(afname,&sb) < 0) &&
	        ((rs2 = bopen(afp,afname,"wct",0666)) >= 0)) {

/* process this article */

	        path_init(&a.path) ;

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("process: we do not have so write it out\n") ;
#endif

	        vecstrinit(&m,HI_NULL,0) ;

/* OK, we now write out the headers as we have them */

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("process: checking> %s\n",HN_RETURNPATH) ;
#endif

	        if ((msghvalues[HI_RETURNPATH] != NULL) &&
	            ((i = msg_headersearch(msgp,HN_RETURNPATH,&mhp)) >= 0)) {

#if	CF_DEBUG
	            if (g.debuglevel > 1)
	                debugprintf("process: writing> %s\n",HN_RETURNPATH) ;
#endif

	            ph_write(afp,mhp,
	                HN_RETURNPATH,HL_RETURNPATH,-1) ;

	            msghvalues[HI_RETURNPATH] = NULL ;
	            vecstradd(&m,mailmsghdrs_names[HI_RETURNPATH],HL_RETURNPATH) ;

	        }


#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("process: checking> %s\n",HN_PATH) ;
#endif

	        if ((msghvalues[HI_PATH] != NULL) &&
	            ((i = msg_headersearch(msgp,HN_PATH,&mhp)) >= 0)) {

	            a.f_nopath = FALSE ;

#if	CF_DEBUG
	            if (g.debuglevel > 1)
	                debugprintf("process: writing> %s\n",HN_PATH) ;
#endif

	            path_parse(&a.path,mhp->value) ;

	            ph_write(afp,mhp,
	                HN_PATH,HL_PATH,-1) ;

	            msghvalues[HI_PATH] = NULL ;
	            vecstradd(&m,mailmsghdrs_names[HI_PATH],HL_PATH) ;

	        }

/* OK, now we put the "Received" lines back */

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("process: checking> %s\n",HN_RECEIVED) ;
#endif

	        if ((msghvalues[HI_RECEIVED] != NULL) &&
	            ((i = msg_headersearch(msgp,HN_RECEIVED,&mhp)) >= 0)) {

#if	CF_DEBUG
	            if (g.debuglevel > 1)
	                debugprintf("process: writing> %s\n",HN_RECEIVED) ;
#endif

	            ph_write(afp,mhp,
	                HN_RECEIVED,HL_RECEIVED,1) ;

	            msghvalues[HI_RECEIVED] = NULL ;
	            vecstradd(&m,mailmsghdrs_names[HI_RECEIVED],HL_RECEIVED) ;

	        }


/* add header lines that may not have been there ! */

/* Content-Length */

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("process: writing> %s %d\n",
	                HN_CLEN,clen) ;
#endif

	        if (clen < 0) {

	            bprintf(afp,"%s: ",HN_CLEN) ;

	            off_clen = (offset_t) bseek(afp,0L,SEEK_CUR) ;

	            bprintf(afp,"                \n") ;

	        } else
	            bprintf(afp,"%s: %d\n",HN_CLEN,clen) ;

	        msghvalues[HI_CLEN] = NULL ;
	        vecstradd(&m,mailmsghdrs_names[HI_CLEN],HL_CLEN) ;


/* Message-Id */

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("process: writing> %s\n",HN_MESSAGEID) ;
#endif

	        bprintf(afp,"%s: <%s>\n",HN_MESSAGEID,a.messageid) ;

	        msghvalues[HI_MESSAGEID] = NULL ;
	        vecstradd(&m,mailmsghdrs_names[HI_MESSAGEID],HL_MESSAGEID) ;



/* put all remaining headers out */

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("process: put all remaining\n") ;
#endif

	        vecstradd(&m,mailmsghdrs_names[HI_REFERENCES],HL_REFERENCES) ;

	        vecstradd(&m,mailmsghdrs_names[HI_NEWSGROUPS],HL_NEWSGROUPS) ;

	        vecstradd(&m,mailmsghdrs_names[HI_ARTICLEID],HL_ARTICLEID) ;

	        vecstradd(&m,mailmsghdrs_names[HI_DATE],HL_DATE) ;

	        vecstradd(&m,mailmsghdrs_names[HI_FROM],HL_FROM) ;

	        vecstradd(&m,mailmsghdrs_names[HI_TO],HL_TO) ;

	        vecstradd(&m,mailmsghdrs_names[HI_SUBJECT],HL_SUBJECT) ;

	        vecstradd(&m,mailmsghdrs_names[HI_KEYWORDS],HL_KEYWORDS) ;


#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("process: put remaining looping\n") ;
#endif

	        for (i = 0 ; msg_headget(msgp,i,&mhp) >= 0 ; i += 1) {

	            if (mhp == NULL) continue ;

	            if (! didheader(&m,mhp)) {

#if	CF_DEBUG
	                if (g.debuglevel > 1)
	                    debugprintf("process: remaining writing> %\n",
	                        mhp->name) ;
#endif

	                ph_write(afp,mhp,
	                    mhp->name,mhp->nlen,0) ;

	            }

	        } /* end for (putting all other headers out) */


/* references */

	        if ((msghvalues[HI_REFERENCES] != NULL) &&
	            ((i = msg_headersearch(msgp,HN_REFERENCES,&mhp)) >= 0)) {

	            ph_write(afp,mhp,
	                HN_REFERENCES,HL_REFERENCES,1) ;

	        }


/* which newsgroups */

#if	CF_DEBUG
	        if (g.debuglevel > 1) {

	            debugprintf("process: newsgroups so far\n") ;

	            for (i = 0 ; ng_get(&a.ngs,i,&ngp) >= 0 ; i += 1) {

	                if (ngp == NULL) continue ;

	                debugprintf("process: ng=%s\n",ngp->name) ;

	            }

	        }
#endif

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("process: writing the newsgroups header\n") ;
#endif

	        len = bprintf(afp,"%s: ",HN_NEWSGROUPS) ;

	        j = 0 ;
	        for (i = 0 ; ng_get(&a.ngs,i,&ngp) >= 0 ; i += 1) {

	            if (ngp == NULL) continue ;

#if	CF_DEBUG
	            if (g.debuglevel > 1)
	                debugprintf("process: %d newsgroup> %s\n",i,ngp->name) ;
#endif

	            if (j == 0) {

	                len += bwrite(afp,ngp->name,ngp->len) ;

	            } else {

	                if ((len + ngp->len + 2) > MSG_MAXLINELEN) {

	                    len = 8 ;
	                    bprintf(afp,",\n\t") ;

	                } else {

	                    len += 2 ;
	                    bprintf(afp,", ") ;

	                }

	                len += bwrite(afp,ngp->name,ngp->len) ;

	            }

	            j += 1 ;

	        } /* end for */

	        bprintf(afp,"\n") ;

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("process: finished writing the newsgroups header\n") ;
#endif


/* put the article ID */

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("process: writing> %s\n",HN_ARTICLEID) ;
#endif

	        bprintf(afp,"%s: <%s>\n",HN_ARTICLEID,a.articleid) ;


/* date of posting */

	        if ((msghvalues[HI_REFERENCES] != NULL) &&
	            ((i = msg_headersearch(msgp,HN_REFERENCES,&mhp)) >= 0)) {

	            ph_write(afp,mhp,
	                HN_REFERENCES,HL_REFERENCES,1) ;

	        } else {

	            if (bcontrol(ifp,BC_STAT,&sb) >= 0)
	                subtime = sb.st_mtime ;

	            else
	                subtime = g.daytime ;

	            bprintf(afp,"%s:       %s\n",HN_DATE,
	                timestr_hdate(subtime,buf)) ;

	        }


/* "from" person */

	        if ((msghvalues[HI_FROM] != NULL) &&
	            ((i = msg_headersearch(msgp,HN_FROM,&mhp)) >= 0)) {

	            ph_write(afp,mhp,
	                HN_FROM,HL_FROM,1) ;

	        } else {

	            if (g.arg_from != NULL)
	                cp = g.arg_from ;

	            else if (g.uu_user != NULL)
	                cp = g.uu_user ;

	            else {

	                cp = buf ;
	                if (u.mailname == NULL)
	                    sprintf(buf,"%s!%s",p.fromnode,u.username) ;

	                else
	                    sprintf(buf,"%s <%s!%s>",
	                        u.mailname,p.fromnode,u.username) ;

	            }

	            bprintf(afp,"%s:       %s\n",HN_FROM,cp) ;

	        }


/* to whom ! (not needed) */

#ifdef	COMMENT
	        bprintf(afp,"%s:       %s\n",HN_TO,u.username) ;
#endif


/* OK, now we put the "Subject" lines back */

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("process: checking> %s\n",HN_SUBJECT) ;
#endif

	        if ((msghvalues[HI_SUBJECT] != NULL) &&
	            ((i = msg_headersearch(msgp,HN_SUBJECT,&mhp)) >= 0)) {

#if	CF_DEBUG
	            if (g.debuglevel > 1) {

	                debugprintf("process: writing> %s %d\n",
	                    mhp->name,mhp->nlen) ;

	                debugprintf("process: value=%s vlen=%d\n",
	                    mhp->value,mhp->vlen) ;

	            }
#endif

	            ph_write(afp,mhp,
	                HN_SUBJECT,HL_SUBJECT,1) ;

#if	CF_DEBUG
	            if (g.debuglevel > 1)
	                debugprintf("process: done writing> %s\n",HN_SUBJECT) ;
#endif

	            msghvalues[HI_SUBJECT] = NULL ;

	        } else if (g.arg_subject != NULL) {

	            bprintf(afp,"%s: %s\n",HN_SUBJECT,g.arg_subject) ;

	        }


/* keywords */

	        if ((msghvalues[HI_KEYWORDS] != NULL) &&
	            ((i = msg_headersearch(msgp,HN_KEYWORDS,&mhp)) >= 0)) {

	            ph_write(afp,mhp,
	                HN_KEYWORDS,HL_KEYWORDS,1) ;

	        }


/* write out the end-of-header marker (a blank line) */

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("process: writing> EOH\n") ;
#endif

	        bprintf(afp,"\n") ;

/* copy the article body to the newly spooled article file */

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("process: copying body, clen=%d\n",clen) ;
#endif

	        if (clen >= 0) {

#if	CF_DEBUG
	            if (g.debuglevel > 1)
	                debugprintf("process: we are writing w/ CLEN\n") ;
#endif

	            l = clen ;
	            while ((l > 0) &&
	                ((len = bread(ifp,linebuf,MIN(l,LINELEN))) > 0)) {

	                offset += len ;
	                bwrite(afp,linebuf,len) ;

	                l -= len ;

	            }

	        } else {

#if	CF_DEBUG
	            if (g.debuglevel > 1)
	                debugprintf("process: we are writing w/o CLEN\n") ;
#endif

	            off_start = offset ;
	            off_finish = offset ;
	            f_bol = TRUE ;
	            while ((len = breadline(ifp,linebuf,LINELEN)) > 0) {

	                offset += len ;
	                f_eol = FALSE ;
	                if (linebuf[len - 1] == '\n') f_eol = TRUE ;

#if	CF_DEBUG
	                if (g.debuglevel > 1)
	                    debugprintf("process: body line> %W",linebuf,len) ;
#endif

	                if (f_bol && (linebuf[0] != '>') && 
	                    msg_matenv(linebuf,len))
	                    break ;

	                off_finish = offset ;
	                bwrite(afp,linebuf,len) ;

	                f_bol = f_eol ;

	            } /* end while */

	        } /* end if */

/* write out the content length if this article didn't have one already */

	        if (clen < 0) {

#if	CF_DEBUG
	            if (g.debuglevel > 1)
	                debugprintf("process: writing post CLEN\n") ;
#endif

	            bseek(afp,off_clen,SEEK_SET) ;

	            bprintf(afp,"%ld",off_finish - off_start) ;

	        }

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("process: closing article file\n") ;
#endif

	        bclose(afp) ;

/* queue this article up for forwarding */

#if	CF_DEBUG
	        if (g.debuglevel > 1) {

	            debugprintf("process: berfore article add, na=%d\n",
	                narticles) ;

	            list_articles(aqp,"0a") ;

	        }
#endif

	        vecelemadd(aqp,&a,sizeof(struct article)) ;

#if	CF_DEBUG
	        if (g.debuglevel > 1) {

	            debugprintf("process: after article add, na=%d\n",
	                narticles) ;

	            list_articles(aqp,"0b") ;

	        }
#endif

	    } else {

/* dump this article */

	        ng_free(&a.ngs) ;

#if	CF_DEBUG
	        if (g.debuglevel > 1)
	            debugprintf("process: we have it already, rs2=%d\n",rs2) ;
#endif

	        if (rs2 < 0) {

	            logfile_printf(&g.lh,
	                "could not create article file (rs %d)\n",
	                rs2) ;

	            if ((! g.f.quiet) || g.f.verbose)
	                bprintf(g.efp,
	                    "%s: could not create article file (rs %d)\n",
	                    g.progname,rs) ;

	        }

/* dump the rest of this article */

	        if (clen >= 0) {

	            l = clen ;
	            while ((l > 0) &&
	                ((len = bread(ifp,linebuf,MIN(l,LINELEN))) > 0)) {

	                offset += len ;
	                l -= len ;

	            }

	        } else {

	            f_bol = TRUE ;
	            while ((len = breadline(ifp,linebuf,LINELEN)) > 0) {

	                offset += len ;
	                f_eol = FALSE ;
	                if (linebuf[len - 1] == '\n') f_eol = TRUE ;

	                if (f_bol && (linebuf[0] != '>') && 
	                    msg_matenv(linebuf,len))
	                    break ;

	                f_bol = f_eol ;

	            } /* end while */

	        } /* end if (dumping the article) */

	    } /* end if (have or not the article) */

/* we are done with this article, let's clean up a bit */

	    msgheaders_free(&msghvalues) ;

	    msg_free(msgp) ;

	    narticles += 1 ;

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("process: bottom loop\n") ;
#endif

	    if (clen >= 0) {

	        f_bol = TRUE ;
	        while ((len = breadline(ifp,linebuf,LINELEN)) > 0) {

	            f_eol = FALSE ;
	            if (linebuf[len - 1] == '\n') f_eol = TRUE ;

	            offset += len ;
	            if (f_bol && 
			(((linebuf[0] != '>') && msg_matenv(linebuf,len)) ||
	                msg_mathead(linebuf,len,hname))) break ;

	            f_bol = f_eol ;

	        } /* end while */

	    } /* end if (post pocessing for a message w/ CLEN) */

	} /* end while (processing article messages) */


#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("process: exiting narticles=%d\n",narticles) ;
#endif

	return narticles ;
}
/* end subroutine (process) */


static int ph_write(afp,mhp,name,nlen,mode)
bfile			*afp ;
struct msg_header	*mhp ;
char			name[] ;
int			nlen, mode ;
{
	struct msg_instance	*mip ;

	struct msg_line		*mlp ;

	int		line ;


#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("ph_write: entered, name=%s, mode=%d\n",
	        name,mode) ;
#endif

	if (mhp->vlen < 0) return BAD ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("ph_write: vlen=%d\n",mhp->vlen) ;
#endif

	if (mode == 0) {

	    line = 0 ;
	    bprintf(afp,"%W: ",
	        name,nlen) ;

	    mip = mhp->i ;
	    while (mip != NULL) {

	        mlp = mip->line ;
	        while (mlp != NULL) {

	            if (mlp->llen > 0) {

	                if (line == 0)
	                    bprintf(afp,"%W\n",
	                        mlp->line,mlp->llen) ;

	                else
	                    bprintf(afp,"%s%W%s",
	                        (mlp->f_bol ? "\t" : ""),
	                        mlp->line,mlp->llen,
	                        (mlp->f_eol ? "\n" : "")) ;

	                line += 1 ;

	            }

	            mlp = mlp->next ;

	        }

	        mip = mip->next ;

	    }

	} else if ((mode == 1) && 
	    ((mhp->nlen + mhp->vlen + 2) <= MSG_MAXLINELEN)) {

	    bprintf(afp,"%W: %W\n",
	        name,nlen,
	        mhp->value,mhp->vlen) ;

	} else {

#if	CF_DEBUG
	    if (g.debuglevel > 1)
	        debugprintf("ph_write: default\n") ;
#endif

	    mip = mhp->i ;
	    if (mip != NULL) {

	        bprintf(afp,"%W: %W",
	            name,nlen,
	            mip->value,mip->vlen) ;

	        mip = mip->next ;
	    }

	    while (mip != NULL) {

	        bprintf(afp,",\n\t%W",
	            mip->value,mip->vlen) ;

	        mip = mip->next ;

	    }

	    bprintf(afp,"\n") ;

	} /* end if */

#if	CF_DEBUG
	if (g.debuglevel > 1)
	    debugprintf("ph_write: exiting\n") ;
#endif

	return OK ;
}
/* end subroutine (ph_write) */


/* extract an ID type header value */
static int ext_id(articleid,s)
char	s[], articleid[] ;
{
	struct ema		aid ;

	struct ema_ent	*ep ;

	int	i ;


	articleid[0] = '\0' ;
	ema_start(&aid) ;

	if (ema_parse(&aid,s) > 0) {

	    for (i = 0 ; ema_get(&aid,i,&ep) >= 0 ; i += 1) {

	        if (ep == NULL) continue ;

	        if ((! ep->f.error) && (ep->rlen > 0)) {

	            strcpy(articleid,ep->route) ;

	            break ;
	        }

	    }
	}

	ema_finish(&aid) ;

	return (articleid[0] == '\0') ? BAD : OK ;
}
/* end subroutine (ext_id) */


/* did we already "do" this header ? */
static int didheader(hp,mhp)
vecstr		*hp ;
struct msg_header	*mhp ;
{
	int	i ;

	char	*cp ;


	for (i = 0 ; vecstrget(hp,i,&cp) >= 0 ; i += 1) {

	    if (cp == NULL) continue ;

	    if (strcasecmp(cp,mhp->name) == 0)
	        return TRUE ;

	}

	return FALSE ;
}
/* end subroutine (didheader) */



