/* config.h
 * This file was produced by running the config.h.SH script, which
 * gets its values from config.sh, which is generally produced by
 * running Configure.
 *
 * Feel free to modify any of this as the need arises.  Note, however,
 * that running config.h.SH again will wipe out any changes you've made.
 * For a more permanent change edit config.sh and rerun config.h.SH.
 * $Id: config.h.SH,v 1.6 1992/08/03 04:51:45 sob Exp sob $
 *
 * $Log: config.h.SH,v $
 * Revision 1.6  1992/08/03  04:51:45  sob
 * Release version 1.6
 *
 *
 * 
 */
/* This software is Copyright 1992 by Stan Barber. 
 *
 * Permission is hereby granted to copy, reproduce, redistribute or otherwise
 * use this software as long as: there is no monetary profit gained
 * specifically from the use or reproduction of this software, it is not
 * sold, rented, traded or otherwise marketed, and this copyright notice is
 * included prominently in any copy made. 
 *
 * The author make no claims as to the fitness or correctness of this software
 * for any use whatsoever, and it is provided as is. Any use of this software
 * is at the user's own risk. 
 */

/* name of the site.  May be overridden by HOSTFILE, gethostname, uname, etc. */
#define SITENAME "big.att.com"
/*#undef HOSTFILE "" /* defined if we read the hostname from a file */

/* domain suffix */
#define DOMAIN "info.att.com"

/* login name of news administrator, if any. */
#define NEWSADMIN "news"	/**/

/* news library */
#define LIB "/usr/add-on/netnews/lib/news"

/* root uid */
#define ROOTID 0


/*#undef NONETDB	/* gethostbyname available? */
#define index  strchr		/* cultural */
#define rindex strrchr		/*  differences? */
#define bcopy(s,d,n) memcpy((char*)d,(char*)s,(int)n)	/* Different */
#define bzero(d,n)   memset((char*)d,0,(int)n)		/*  flavors. */
/*#undef void int	/* is void to be avoided? */
/*#undef SUNOS4		/* running SunOS 4.X? */
/*#undef EUNICE		/* no linking? */
/*#undef VMS		/* not currently used, here just in case */
			/*  (undef to take name from ~/.fullname) */
/*#undef BERKNAMES	/* if so, are they Berkeley format? */
			/* (that is, ":name,stuff:") */
#define USGNAMES	/* or are they USG format? */
			/* (that is, ":stuff-name(stuff):") */
#define DO_DOTDIR	/* use DOT_DIR to figure out where .signature is */
#define STRCASECMP	/* is strcasecmp is available? */
#define STRNCASECMP	/* is strncasecmp is available? */
#define SETEUID	/* is seteuid available? */
/*#undef WHOAMI		/* should we include whoami.h? */
#define GETPWENT	/* should we include getpwent? */
/*#undef GHNAME	/* do we have a gethostname function? */
#define DOUNAME 	/* do we have a uname function? */
/*#undef PHOSTNAME ""	/* how to get host name with popen */
#define SERVER_FILE "/usr/add-on/netnews/lib/news/server"	/* news server file */
