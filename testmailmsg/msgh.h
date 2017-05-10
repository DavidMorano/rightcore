/* msgh */


#ifndef	MSGH_INCLUDE
#define	MSGH_INCLUDE


#ifndef	NULL
#define	NULL	((void *) 0)
#endif


struct msgh_flags {
	uint	present : 1 ;
	uint	processed : 1 ;
} ;

struct msgh {
	struct msgh_flags	f ;
	char	*name ;
	char	*value ;
	int	nlen ;
	int	vlen ;
} ;



extern char	*mailmsghdrs_names[] ;



#define		HI_FROM		0
#define		HN_FROM		mailmsghdrs_names[HI_FROM]
#define		HL_FROM		4

#define		HI_TO		1
#define		HN_TO		mailmsghdrs_names[HI_TO]
#define		HL_TO		2

#define		HI_DATE		2
#define		HN_DATE		mailmsghdrs_names[HI_DATE]
#define		HL_DATE		4

#define		HI_SUBJECT	3
#define		HN_SUBJECT	mailmsghdrs_names[HI_SUBJECT]
#define		HL_SUBJECT	7

#define		HI_TITLE	4
#define		HN_TITLE	mailmsghdrs_names[HI_TITLE]
#define		HL_TITLE	5

#define		HI_MESSAGEID	5
#define		HN_MESSAGEID	mailmsghdrs_names[HI_MESSAGEID]
#define		HL_MESSAGEID	10

#define		HI_ARTICLEID	6
#define		HN_ARTICLEID	mailmsghdrs_names[HI_ARTICLEID]
#define		HL_ARTICLEID	10

#define		HI_CLEN		7
#define		HN_CLEN		mailmsghdrs_names[HI_CLEN]
#define		HL_CLEN		14

#define		HI_NEWSGROUPS	8
#define		HN_NEWSGROUPS	mailmsghdrs_names[HI_NEWSGROUPS]
#define		HL_NEWSGROUPS	10

#define		HI_XMAILER	9
#define		HN_XMAILER	mailmsghdrs_names[HI_XAMILER]
#define		HL_XMAILER	8

#define		HI_BOARD	10
#define		HN_BOARD	mailmsghdrs_names[HI_BOARD]
#define		HL_BOARD	5

#define		HI_LINES	11
#define		HN_LINES	mailmsghdrs_names[HI_LINES]
#define		HL_LINES	5

#define		HI_REPLYTO	12
#define		HN_REPLYTO	mailmsghdrs_names[HI_REPLYTO]
#define		HL_REPLYTO	8

#define		HI_REFERENCES	13
#define		HN_REFERENCES	mailmsghdrs_names[HI_REFERENCES]
#define		HL_REFERENCES	10

#define		HI_CTYPE	14
#define		HN_CTYPE	mailmsghdrs_names[HI_CTYPE]
#define		HL_CTYPE	12

#define		HI_SUBJ		15
#define		HN_SUBJ		mailmsghdrs_names[HI_SUBJ]
#define		HL_SUBJ		4

#define		HI_KEYWORDS	16
#define		HN_KEYWORDS	mailmsghdrs_names[HI_KEYWORDS]
#define		HL_KEYWORDS	8

#define		HI_CONTROL	17
#define		HN_CONTROL	mailmsghdrs_names[HI_CONTROL]
#define		HL_CONTROL	7

#define		HI_XSERVICE	18
#define		HN_XSERVICE	mailmsghdrs_names[HI_XSERVICE]
#define		HL_XSERVICE	9

#define		HI_PATH		19
#define		HN_PATH		mailmsghdrs_names[HI_PATH]
#define		HL_PATH		4

#define		HI_ERRORTO	20
#define		HN_ERRORTO	mailmsghdrs_names[HI_ERRORTO]
#define		HL_ERRORTO	8

#define		HI_RETURNPATH	21
#define		HN_RETURNPATH	mailmsghdrs_names[HI_RETURNPATH]
#define		HL_RETURNPATH	11

#define		HI_RECEIVED	21
#define		HN_RECEIVED	mailmsghdrs_names[HI_RECEIVED]
#define		HL_RECEIVED	8


/* put all new entries before this last (fake) one */

#define		HI_NULL		22
#define		HN_NULL		NULL
#define		HL_NULL		-1


#endif /* MSGH_INCLUDE */



