/* hname */

/* address type headers */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	HNAME_INCLUDE
#define	HNAME_INCLUDE	1


enum hnames {
	hname_errorsto,
	hname_replyto,
	hname_sender,
	hname_from,
	hname_to,
	hname_cc,
	hname_bcc,
	hname_org,
	hname_msgid,
	hname_subject,
	hname_overlast
} ;


extern const char	*hname[] ;


#endif /* HNAME_INCLUDE */


