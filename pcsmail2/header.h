/* header */

/************************************************************************
 *									*
 * The information contained herein is for the use of AT&T Information	*
 * Systems Laboratories and is not for publications.  (See GEI 13.9-3)	*
 *									*
 *	(c) 1984 AT&T Information Systems				*
 *									*
 * Authors of the contents of this file:				*
 *									*
 *		J.Mukerji						*
 *									*

 ************************************************************************/



#ifdef	MASTER

char *hvalue;

char	*headers[] = {	
	"Bcc:",			/* 0 */
	"Board:",		/* 1 */
	"Cc:",			/* 2 */
	"Confirming-Id:",
	"Date:",
	"From:",
	"Keywords:",
	"Message-Id:",
	"Options:",
	"Original-Date:",
	"Original-Recipient:",
	"Original-Subject:",
	"Reason:",
	"References:",
	"Received-By:",
	"Sender:",
	"Status:",
	"Subject:",
	"Title:",
	"To:",
	"Full-Name:",
	"content-type",
	"content-length",
	"content-lines",
	"lines",
	"unknown",
	(char *) 0,
} ;

#else

extern char	*hvalue ;

extern char	*headers[] ;

#endif


#define	MH_BCC			0
#define	MH_BOARD		1
#define	MH_CC			2
#define	MH_CONFIRMING_ID	3
#define	MH_DATE			4
#define	MH_FROM			5
#define	MH_KEYWORDS		6
#define	MH_MESSAGE_ID		7
#define	MH_OPTIONS		8
#define	MH_ODATE		9
#define	MH_ORECIPIENT		10
#define	MH_OSUBJECT		11
#define	MH_REASON		12
#define	MH_REFERENCE		13
#define	MH_RECEIVED_BY		14
#define	MH_SENDER		15
#define	MH_STATUS		16
#define	MH_SUBJECT		17
#define	MH_TITLE		18
#define	MH_TO			19
#define	MH_FULLNAME		20
#define	MH_CTYPE		21
#define	MH_CLEN			22
#define	MH_CLINES		23
#define	MH_LINES		24
#define	MH_UNKNOWN		25


#define	HS_BCC			headers[0]
#define	HS_BOARD		headers[1]
#define	HS_CC			headers[2]
#define	HS_CONFID		headers[3]
#define	HS_DATE			headers[4]
#define	HS_FROM			headers[5]
#define	HS_KEYWORD		headers[6]
#define	HS_MESSID		headers[7]
#define	HS_OPTIONS		headers[8]
#define	HS_ODATE		headers[9]
#define	HS_ORECIP		headers[10]
#define	HS_OSUBJ		headers[11]
#define	HS_REASON		headers[12]
#define	HS_REF			headers[13]
#define	HS_RECBY		headers[14]
#define	HS_SENDER		headers[15]
#define	HS_STATUS		headers[16]
#define	HS_SUBJECT		headers[17]
#define	HS_TITLE		headers[18]
#define	HS_TO			headers[19]
#define	HS_FULLNAME		headers[20]
#define	HS_CTYPE		headers[MH_CTYPE]
#define	MS_CLEN			headers[MH_CLEN]
#define	MS_CLINES		headers[MH_CLINES]
#define	MS_LINES		headers[MH_LINES]
#define	MS_UNKNOWN		headers[MH_UNKNOWN]






#define BCC		0
#define BOARD		1
#define CC		2
#define CONFIRMING_ID	3
#define DATE		4
#define FROM		5
#define KEYWORDS	6
#define MESSAGE_ID	7
#define OPTIONS		8
#define ODATE		9
#define ORECIPIENT	10
#define OSUBJECT	11
#define REASON		12
#define REFERENCE	13
#define RECEIVED_BY	14
#define SENDER		15
#define STATUS		16
#define SUBJECT		17
#define TITLE		18
#define TO		19
#define	FULLNAME	20


#define Bcc			headers[0]
#define Board			headers[1]
#define Cc			headers[2]
#define Confid			headers[3]
#define Date			headers[4]
#define From			headers[5]
#define Keyword			headers[6]
#define Messid			headers[7]
#define Options			headers[8]
#define Odate			headers[9]
#define Orecip			headers[10]
#define Osubj			headers[11]
#define Reason			headers[12]
#define Ref			headers[13]
#define Recby			headers[14]
#define Sender			headers[15]
#define Status			headers[16]
#define Subject			headers[17]
#define Title			headers[18]
#define To			headers[19]
#define	Fullname		headers[20]



