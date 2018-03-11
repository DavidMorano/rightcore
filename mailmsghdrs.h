/* mailmsghdrs */


/* revision history:

	= 2002-07-21, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2002 David A­D­ Morano.  All rights reserved. */

#ifndef	MAILMSGHDRS_INCLUDE
#define	MAILMSGHDRS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<mailmsg.h>


/* object defines */

#define	MAILMSGHDRS_MAGIC	0x87987598
#define	MAILMSGHDRS		struct mailmsghdrs_head

#ifndef	NULL
#define	NULL		0
#endif

#define	HI_FROM		0
#define	HN_FROM		mailmsghdrs_names[HI_FROM]
#define	HL_FROM		4

#define	HI_TO		1
#define	HN_TO		mailmsghdrs_names[HI_TO]
#define	HL_TO		2

#define	HI_DATE		2
#define	HN_DATE		mailmsghdrs_names[HI_DATE]
#define	HL_DATE		4

#define	HI_SUBJECT	3
#define	HN_SUBJECT	mailmsghdrs_names[HI_SUBJECT]
#define	HL_SUBJECT	7

#define	HI_TITLE	4
#define	HN_TITLE	mailmsghdrs_names[HI_TITLE]
#define	HL_TITLE	5

#define	HI_MESSAGEID	5
#define	HN_MESSAGEID	mailmsghdrs_names[HI_MESSAGEID]
#define	HL_MESSAGEID	10

#define	HI_ARTICLEID	6
#define	HN_ARTICLEID	mailmsghdrs_names[HI_ARTICLEID]
#define	HL_ARTICLEID	10

#define	HI_CLEN		7
#define	HN_CLEN		mailmsghdrs_names[HI_CLEN]
#define	HL_CLEN		14

#define	HI_NEWSGROUPS	8
#define	HN_NEWSGROUPS	mailmsghdrs_names[HI_NEWSGROUPS]
#define	HL_NEWSGROUPS	10

#define	HI_INREPLYTO	9
#define	HN_INREPLYTO	mailmsghdrs_names[HI_INREPLYTO]
#define	HL_INREPLYTO	11

#define	HI_BOARD	10
#define	HN_BOARD	mailmsghdrs_names[HI_BOARD]
#define	HL_BOARD	5

#define	HI_LINES	11
#define	HN_LINES	mailmsghdrs_names[HI_LINES]
#define	HL_LINES	5

#define	HI_REPLYTO	12
#define	HN_REPLYTO	mailmsghdrs_names[HI_REPLYTO]
#define	HL_REPLYTO	8

#define	HI_REFERENCES	13
#define	HN_REFERENCES	mailmsghdrs_names[HI_REFERENCES]
#define	HL_REFERENCES	10

#define	HI_CTYPE	14
#define	HN_CTYPE	mailmsghdrs_names[HI_CTYPE]
#define	HL_CTYPE	12

#define	HI_EXPIRES	15
#define	HN_EXPIRES	mailmsghdrs_names[HI_EXPIRES]
#define	HL_EXPIRES	sizeof(HN_EXPIRES)

#define	HI_KEYWORDS	16
#define	HN_KEYWORDS	mailmsghdrs_names[HI_KEYWORDS]
#define	HL_KEYWORDS	8

#define	HI_CONTROL	17
#define	HN_CONTROL	mailmsghdrs_names[HI_CONTROL]
#define	HL_CONTROL	7

#define	HI_XLINES	18
#define	HN_XLINES	mailmsghdrs_names[HI_XLINES]
#define	HL_XLINES	7

#define	HI_PATH		19
#define	HN_PATH		mailmsghdrs_names[HI_PATH]
#define	HL_PATH		4

#define	HI_ERRORSTO	20
#define	HN_ERRORSTO	mailmsghdrs_names[HI_ERRORSTO]
#define	HL_ERRORSTO	9

#define	HI_RETURNPATH	21
#define	HN_RETURNPATH	mailmsghdrs_names[HI_RETURNPATH]
#define	HL_RETURNPATH	11

#define	HI_RECEIVED	22
#define	HN_RECEIVED	mailmsghdrs_names[HI_RECEIVED]
#define	HL_RECEIVED	8

#define	HI_XQUEUESPEC	23
#define	HN_XQUEUESPEC	mailmsghdrs_names[HI_XQUEUESPEC]
#define	HL_XQUEUESPEC	11

#define	HI_XSERVICE	24
#define	HN_XSERVICE	mailmsghdrs_names[HI_XSERVICE]
#define	HL_XSERVICE	9

#define	HI_XJOBID	25
#define	HN_XJOBID	mailmsghdrs_names[HI_XJOBID]
#define	HL_XJOBID	7

#define	HI_XORIGHOST	26
#define	HN_XORIGHOST	mailmsghdrs_names[HI_XORIGHOST]
#define	HL_XORIGHOST	10

#define	HI_XORIGUSER	27
#define	HN_XORIGUSER	mailmsghdrs_names[HI_XORIGUSER]
#define	HL_XORIGUSER	10

#define	HI_XUSERNAME	28
#define	HN_XUSERNAME	mailmsghdrs_names[HI_XUSERNAME]
#define	HL_XUSERNAME	10

#define	HI_SENDER	29
#define	HN_SENDER	mailmsghdrs_names[HI_SENDER]
#define	HL_SENDER	6

#define	HI_CC		30
#define	HN_CC		mailmsghdrs_names[HI_CC]
#define	HL_CC		2

#define	HI_BCC		31
#define	HN_BCC		mailmsghdrs_names[HI_BCC]
#define	HL_BCC		3

#define	HI_STATUS	32
#define	HN_STATUS	mailmsghdrs_names[HI_STATUS]
#define	HL_STATUS	6

#define	HI_CLINES	33
#define	HN_CLINES	mailmsghdrs_names[HI_CLINES]
#define	HL_CLINES	13		/* content-lines */

#define	HI_CENCODING	34
#define	HN_CENCODING	mailmsghdrs_names[HI_CENCODING]
#define	HL_CENCODING	16		/* content-encoding */

#define	HI_ORGANIZATION	35
#define	HN_ORGANIZATION	mailmsghdrs_names[HI_ORGANIZATION]
#define	HL_ORGANIZATION	12		/* organization */

#define	HI_DELIVEREDTO	36
#define	HN_DELIVEREDTO	mailmsghdrs_names[HI_DELIVEREDTO]
#define	HL_DELIVEREDTO	sizeof(HN_DELIVEREDTO)

#define	HI_XORIGINALTO	37
#define	HN_XORIGINALTO	mailmsghdrs_names[HI_XORIGINALTO]
#define	HL_XORIGINALTO	sizeof(HN_XORIGINALTO)

#define	HI_XPRIORITY	38
#define	HN_XPRIORITY	mailmsghdrs_names[HI_XPRIORITY]
#define	HL_XPRIORITY	sizeof(HN_XPRIORITY)

#define	HI_PRIORITY	39
#define	HN_PRIORITY	mailmsghdrs_names[HI_PRIORITY]
#define	HL_PRIORITY	sizeof(HN_PRIORITY)

#define	HI_XFACE	40
#define	HN_XFACE	mailmsghdrs_names[HI_XFACE]
#define	HL_XFACE	sizeof(HN_XFACE)

#define	HI_XBBNEWS	41
#define	HN_XBBNEWS	mailmsghdrs_names[HI_XBBNEWS]
#define	HL_XBBNEWS	sizeof(HN_XBBNEWS)

#define	HI_XUUID	42
#define	HN_XUUID	mailmsghdrs_names[HI_XUUID]
#define	HL_XUUID	sizeof(HN_XUUID)

#define	HI_XUTI		43
#define	HN_XUTI		mailmsghdrs_names[HI_XUTI]
#define	HL_XUTI		sizeof(HN_XUTI)

#define	HI_XMCDATE	44
#define	HN_XMCDATE	mailmsghdrs_names[HI_XMCDATE]
#define	HL_XMCDATE	sizeof(HN_XMXDATE)

#define	HI_XMAILER	45
#define	HN_XMAILER	mailmsghdrs_names[HI_XMAILER]
#define	HL_XMAILER	8

#define	HI_XFORWARDEDTO	46
#define	HN_XFORWARDEDTO	mailmsghdrs_names[HI_XFORWARDEDTO]
#define	HL_XFORWARDEDTO	14

#define	HI_SUBJ		47
#define	HN_SUBJ		mailmsghdrs_names[HI_SUBJ]
#define	HL_SUBJ		4

/* put all new entries before this last (fake) one */

#define	HI_NULL		48
#define	HN_NULL		NULL
#define	HL_NULL		-1


struct mailmsghdrs_head {
	uint		magic ;
	const char	**v ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern const char	*mailmsghdrs_names[] ;

#ifdef	__cplusplus
}
#endif


#if	(! defined(MAILMSGHDRS_MASTER)) || (MAILMSGHDRS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int mailmsghdrss_start(MAILMSGHDRS *,MAILMSG *) ;
extern int mailmsghdrss_finish(MAILMSGHDRS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(MAILMSGHDRS_MASTER)) || (MAILMSGHDRS_MASTER == 0) */

#endif /* MAILMSGHDRS_INCLUDE */


