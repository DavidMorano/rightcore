/* strerrabbr */

/* return an abbreviation string given a system-error return number */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        We take a system-error return number and we return a corresponding
        abbreviation string.

	Synopsis:
	cchar *strerrabbr(uint n)

	Arguments:
	n		system-error return number to lookup

	Returns:
	-		character-string representation of system-error return


*******************************************************************************/


#include	<envstandards.h>
#include	<sys/types.h>
#include	<vsystem.h>
#include	<localmisc.h>


/* local structures */

struct sysret {
	int		n ;
	const char	*s ;
} ;


/* local variables */

static const struct sysret	cvts[] = {
	{ SR_PERM, "PERM" },
	{ SR_NOENT, "NOENT" },
	{ SR_SRCH, "SRCH" },
	{ SR_INTR, "INTR" },
	{ SR_IO, "IO" },
	{ SR_NXIO, "NXIO" },
	{ SR_2BIG, "2BIG" },
	{ SR_NOEXEC, "NOEXEC" },
	{ SR_BADF, "BADF" },
	{ SR_CHILD, "CHILD" },
	{ SR_AGAIN, "AGAIN" },
	{ SR_NOMEM, "NOMEM" },
	{ SR_ACCES, "ACCES" },
	{ SR_FAULT, "FAULT" },
	{ SR_NOTBLK, "NOTBLK" },
	{ SR_BUSY, "BUSY" },
	{ SR_EXIST, "EXIST" },
	{ SR_XDEV, "XDEV" },
	{ SR_NODEV, "NODEV" },
	{ SR_NOTDIR, "NOTDIR" },
	{ SR_ISDIR, "ISDIR" },
	{ SR_INVAL, "INVAL" },
	{ SR_NFILE, "NFILE" },
	{ SR_MFILE, "MFILE" },
	{ SR_NOTTY, "NOTTY" },
	{ SR_TXTBSY, "TXTBSY" },
	{ SR_FBIG, "FBIG" },
	{ SR_NOSPC, "NOSPC" },
	{ SR_SPIPE, "NOTSEEK" },
	{ SR_ROFS, "ROFS" },
	{ SR_MLINK, "MLINK" },
	{ SR_PIPE, "PIPE" },
	{ SR_DOM, "DOM" },
	{ SR_RANGE, "RANGE" },
	{ SR_NOMSG, "NOMSG" },
	{ SR_IDRM, "IDRM" },
	{ SR_CHRNG, "CHRNG" },
	{ SR_L2NSYNC, "L2NSYNC" },
	{ SR_L3HLT, "L3HLT" },
	{ SR_L3RST, "L3RST" },
	{ SR_LNRNG, "LNRNG" },
	{ SR_UNATCH, "UNATCH" },
	{ SR_NOCSI, "NOCSI" },
	{ SR_L2HLT, "L2HLT" },
	{ SR_DEADLK, "DEADLK" },
	{ SR_NOLCK, "NOLCK" },
	{ SR_CANCELED, "CANCELED" },
	{ SR_NOTSUP, "NOTSUP" },
	{ SR_DQUOT, "DQUOT" },
	{ SR_BADE, "BADE" },
	{ SR_BADR, "BADR" },
	{ SR_XFULL, "XFULL" },
	{ SR_NOANO, "NOANO" },
	{ SR_BADRQC, "BADRQC" },
	{ SR_BADSLT, "BADSLT" },
	{ SR_DEADLOCK, "DEADLOCK" },
	{ SR_BFONT, "BFONT" },
	{ SR_OWNERDEAD, "OWNERDEAD" },
	{ SR_NOTRECOVERABLE, "NOTRECOVERABLE" },
	{ SR_NOSTR, "NOSTR" },
	{ SR_NODATA, "NODATA" },
	{ SR_TIME, "TIME" },
	{ SR_NOSR, "NOSR" },
	{ SR_NONET, "NONET" },
	{ SR_NOPKG, "NOPKG" },
	{ SR_REMOTE, "REMOTE" },
	{ SR_NOLINK, "NOLINK" },
	{ SR_ADV, "ADV" },
	{ SR_SRMNT, "SRMNT" },
	{ SR_COMM, "COMM" },
	{ SR_PROTO, "PROTO" },
	{ SR_MULTIHOP, "MULTIHOP" },
	{ SR_BADMSG, "BADMSG" },
	{ SR_NAMETOOLONG, "NAMETOOLONG" },
	{ SR_OVERFLOW, "OVERFLOW" },
	{ SR_NOTUNIQ, "NOTUNIQ" },
	{ SR_BADFD, "BADFD" },
	{ SR_REMCHG, "REMCHG" },
	{ SR_LIBACC, "LIBACC" },
	{ SR_LIBBAD, "LIBBAD" },
	{ SR_LIBSCN, "LIBSCN" },
	{ SR_LIBMAX, "LIBMAX" },
	{ SR_LIBEXEC, "LIBEXEC" },
	{ SR_ILSEQ, "ILSEQ" },
	{ SR_NOSYS, "NOSYS" },
	{ SR_LOOP, "LOOP" },
	{ SR_RESTART, "RESTART" },
	{ SR_STRPIPE, "STRPIPE" },
	{ SR_NOTEMPTY, "NOTEMPTY" },
	{ SR_USERS, "USERS" },
	{ SR_NOTSOCK, "NOTSOCK" },
	{ SR_DESTADDRREQ, "DESTADDRREQ" },
	{ SR_MSGSIZE, "MSGSIZE" },
	{ SR_PROTOTYPE, "PROTOTYPE" },
	{ SR_NOPROTOOPT, "NOPROTOOPT" },
	{ SR_PROTONOSUPPORT, "PROTONOSUPPORT" },
	{ SR_SOCKTNOSUPPORT, "SOCKTNOSUPPORT" },
	{ SR_OPNOTSUPP, "OPNOTSUPP" },
	{ SR_PFNOSUPPORT, "PFNOSUPPORT" },
	{ SR_AFNOSUPPORT, "AFNOSUPPORT" },
	{ SR_ADDRINUSE, "ADDRINUSE" },
	{ SR_ADDRNOTAVAIL, "ADDRNOTAVAIL" },
	{ SR_NETDOWN, "NETDOWN" },
	{ SR_NETUNREACH, "NETUNREACH" },
	{ SR_NETRESET, "NETRESET" },
	{ SR_CONNABORTED, "CONNABORTED" },
	{ SR_CONNRESET, "CONNRESET" },
	{ SR_NOBUFS, "NOBUFS" },
	{ SR_ISCONN, "ISCONN" },
	{ SR_NOTCONN, "NOTCONN" },
	{ SR_SHUTDOWN, "SHUTDOWN" },
	{ SR_TOOMANYREFS, "TOOMANYREFS" },
	{ SR_TIMEDOUT, "TIMEDOUT" },
	{ SR_CONNREFUSED, "CONNREFUSED" },
	{ SR_HOSTDOWN, "HOSTDOWN" },
	{ SR_HOSTUNREACH, "HOSTUNREACH" },
	{ SR_WOULDBLOCK, "WOULDBLOCK" },
	{ SR_ALREADY, "ALREADY" },
	{ SR_INPROGRESS, "INPROGRESS" },
	{ SR_STALE, "STALE" },
	{ SR_BAD, "BAD" },
	{ SR_EXIT, "EXIT" },
	{ SR_NOENTRY, "NOENTRY" },
	{ SR_NOTOPEN, "NOTOPEN" },
	{ SR_WRONLY, "WRONLY" },
	{ SR_RDONLY, "RDONLY" },
	{ SR_NOTSEEK, "NOTSEEK" },
	{ SR_ACCESS, "ACCESS" },
	{ SR_INVALID, "INVALID" },
	{ SR_EXISTS, "EXISTS" },
	{ SR_LOCKED, "LOCKED" },
	{ SR_INUSE, "INUSE" },
	{ SR_LOCKLOST, "LOCKLOST" },
	{ SR_HANGUP, "HANGUP" },
	{ SR_POLLERR, "POLLERR" },
	{ SR_TOOBIG, "TOOBIG" },
	{ SR_BADFMT, "BADFMT" },
	{ SR_FULL, "FULL" },
	{ SR_EMPTY, "EMPTY" },
	{ SR_EOF, "EOF" },
	{ SR_NOEXIST, "NOEXIST" },
	{ SR_NOTFOUND, "NOTFOUND" },
	{ SR_BADREQUEST, "BADREQUEST" },
	{ SR_NOTCONNECTED, "NOTCONNECTED" },
	{ SR_OPEN, "OPEN" },
	{ SR_OUT, "OUT" },
	{ SR_NOTAVAIL, "NOTAVAIL" },
	{ SR_BADSLOT, "BADSLOT" },
	{ SR_SEARCH, "SEARCH" },
	{ SR_NOANODE, "NOANODE" },
	{ SR_BUGCHECK, "BUGCHECK" },
	{ SR_LOOK, "LOOK" },
	{ SR_DOWN, "DOWN" },
	{ SR_UNAVAIL, "UNAVAIL" },
	{ SR_TIMEOUT, "TIMEOUT" },
	{ SR_CREATED, "CREATED" },
	{ SR_OK, "OK" }
} ;


/* exported subroutines */


const char *strerrabbr(int rs)
{
	int		i ;
	int		f = FALSE ;
	const char	*s ;

	for (i = 0 ; cvts[i].s != 0 ; i += 1) {
	    f = (cvts[i].n == rs) ;
	    if (f) break ;
	} /* end for */

	s = (f) ? cvts[i].s : "*UNK*" ;
	return s ;
}
/* end subroutine (strerrabbr) */


