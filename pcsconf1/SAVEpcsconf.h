/* pcsconf */


#ifndef	PCSCONF_INCLUDE
#define	PCSCONF_INCLUDE


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>

#include	<vecstr.h>
#include	<localmisc.h>


#ifndef	PCSCONF_PCS
#define	PCSCONF_PCS		"/usr/add-on/pcs"
#endif

#define	PCSCONF_FILE1		"etc/conf"
#define	PCSCONF_FILE2		"etc/pcs.conf"

#define	PCSCONF_DEFRELAY	"emsr.lucent.com"
#define	PCSCONF_DEFGATEWAY	"emsr.lucent.com"
#define	PCSCONF_POSTMASTER	"pcs"
#define	PCSCONF_ORGANIZATION	"Lucent Technologies, Inc."
#define	PCSCONF_ORGDOMAIN	"lucent.com"

#define	PCSCONF_USERNAME	"pcs"

#define	PCSCONF_VERSION		"PCS package 3.0"

#define	PCSCONF_LEN		(3 * 1024)




struct pcsconf {
	char	*pcs ;			/* program root (for PCS) */
	char	*nodename ;
	char	*domainname ;
	char	*orgdomain ;		/* company/organization domain */
	char	*pcslogin ;		/* PCS username */
	char	*mailhost ;		/* site MAIL host */
	char	*mailnode ;		/* mailhost's nodename */
	char	*maildomain ;		/* local mail cluster domainname */
	char	*fromhost ;		/* return address host name */
	char	*fromnode ;		/* return address host name */
	char	*uucphost ;		/* site UUCP host (fully qualified) */
	char	*uucpnode ;		/* site UUCP host (a nodename) */
	char	*userhost ;		/* host with user access */
	char	*mailrelay ;		/* company mail relay */
	char	*mailgateway ;		/* company mail gateway */
	char	*postmaster ;		/* sent error mail to here */
	char	*organization ;		/* company/organization string */
	char	*version ;		/* PCS distribution package version */
	uid_t	uid_pcs ;
	gid_t	gid_pcs ;
} ;



#if	(! defined(PCSCONF_MASTER)) || (PCSCONF_MASTER == 0)

extern int pcsconf(char *,char *,struct pcsconf *,VECSTR *,VECSTR *,
	char *,int) ;

#endif /* PCSCONF_MASTER */


/* other PCS defines */

#ifndef	PCS_MSGIDLEN
#define	PCS_MSGIDLEN	(3 * MAXHOSTNAMELEN)
#endif


#endif /* PCSCONF_INCLUDE */



