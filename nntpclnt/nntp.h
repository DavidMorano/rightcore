/*
 * Response codes for NNTP server
 *
 * @(#)Header: nntp.h,v 1.8 90/07/05 02:08:31 sob Exp $
 *
 * First digit:
 *
 *	1xx	Informative message
 *	2xx	Command ok
 *	3xx	Command ok so far, continue
 *	4xx	Command was correct, but couldn't be performed
 *		for some specified reason.
 *	5xx	Command unimplemented, incorrect, or a
 *		program error has occurred.
 *
 * Second digit:
 *
 *	x0x	Connection, setup, miscellaneous
 *	x1x	Newsgroup selection
 *	x2x	Article selection
 *	x3x	Distribution
 *	x4x	Posting
 *	x8x	Authorization
 */

#define	CHAR_DEBUG	'0'
#define	CHAR_INF	'1'
#define	CHAR_OK		'2'
#define	CHAR_CONT	'3'
#define	CHAR_ERR	'4'
#define	CHAR_FATAL	'5'

#define	INF_HELP	100	/* Help text on way */
#define	INF_DATE	111	/* Date */
#define	INF_AUTH	180	/* Authorization capabilities */
#define	INF_DEBUG	199	/* Debug output */

#define	OK_CANPOST	200	/* Hello; you can post */
#define	OK_NOPOST	201	/* Hello; you can't post */
#define	OK_SLAVE	202	/* Slave status noted */
#define OK_OPTIONS      204     /* Options acknowledged (V2) */
#define	OK_GOODBYE	205	/* Closing connection */
#define OK_COOKIE       206     /* Mark output (V2) */
#define OK_VERSION      207     /* Version output (V2) */
#define	OK_GROUP	211	/* Group selected */
#define	OK_GROUPS	215	/* Newsgroups follow */
#define OK_BYTES        216     /* List by bytecount (V2) */
#define	OK_XINDEX	218	/* Tin style group index file follows */
#define	OK_ARTICLE	220	/* Article (head & body) follows */
#define	OK_HEAD		221	/* Head follows */
#define	OK_BODY		222	/* Body follows */
#define	OK_NOTEXT	223	/* No text sent -- stat, next, last */
#define	OK_OVER		224	/* Overview data follows */
#define OK_BHEAD        226     /* Head by bytecount (V2) */
#define OK_BBODY        228     /* Body by bytecount (V2) */
#define	OK_NEWNEWS	230	/* New articles by message-id follow */
#define	OK_NEWGROUPS	231	/* New newsgroups follow */
#define	OK_XFERED	235	/* Article transferred successfully */
#define OK_IDXFERED     236     /* Message-id transferred successfully (V2) */
#define OK_IHAVE        237     /* IHAVE mode on control channel (V2) */
#define	OK_POSTED	240	/* Article posted successfully */
#define OK_AUTH         250     /* Authorization ok (V2) */
#define OK_BATCHRCVD    263     /* Batch Data Received (V2) */
#define OK_IDLIST       265     /* Message Id list follows (V2) */
#define OK_BATCH        267     /* Batch follows (V2) */
#define OK_BBATCH       268     /* Article batch follows by bytecount (V2) */
#define	OK_OLDAUTHSYS	280	/* Authorization system ok */
#define	OK_OLDAUTH	281	/* Authorization (user/pass) ok */
#define	OK_BIN		282	/* binary data follows */

#define CONT_ID         331     /* Send Message-ids here (V2) */
#define CONT_BYTE       332     /* Send bytecount bytes (V2) */
#define	CONT_XFER	335	/* Continue to send article */
#define	CONT_POST	340	/* Continue to post article */
#define CONT_GO         341     /* Start sending (V2) */
#define CONT_AUTH       350     /* Continue with authorization (V2) */
#define CONT_IDLIST     361     /* Send a list of Message-ids (V2) */
#define CONT_BATCH      363     /* Start sending BATCH data (V2) */
#define	CONT_OLDAUTHI	380	/* authorization is required */
#define	CONT_OLDAUTHD	381	/* <type> authorization data required */

#define	ERR_GOODBYE	400	/* Have to hang up for some reason */
#define	ERR_NOGROUP	411	/* No such newsgroup */
#define	ERR_NCING	412	/* Not currently in newsgroup */
#define	ERR_XINDEX	418	/* No tin style index file for newsgroup */
#define	ERR_NOCRNT	420	/* No current article selected */
#define	ERR_NONEXT	421	/* No next article in this group */
#define	ERR_NOPREV	422	/* No previous article in this group */
#define	ERR_NOARTIG	423	/* No such article in this group */
#define	ERR_NOART	430	/* No such article at all */
#define ERR_NOPORT      431     /* Problem with DATA port (V2) */
#define ERR_NOLIST      432     /* No list of newsgroups available (V2)*/
#define	ERR_GOTIT	435	/* Already got that article, don't send */
#define	ERR_XFERFAIL	436	/* Transfer failed */
#define	ERR_XFERRJCT	437	/* Article rejected, don't resend */
#define ERR_ASKAGN      438     /* Ask again later (V2) */
#define ERR_TEMP        439     /* Service temporarily unavailable (V2) */
#define	ERR_NOPOST	440	/* Posting not allowed */
#define	ERR_POSTFAIL	441	/* Posting failed */
#define ERR_NOAUTH      450     /* authorization required for command (V2) */
#define ERR_AUTHSYS     451     /* <type> authorization system wanted (V2) */
#define ERR_AUTHREJ     452     /* Authorization rejected (V2) */
#define	ERR_AUTHEXP	453	/* Authorization info has expired on server */
#define ERR_GARBLED     463     /* Inbound batch garbled (V2) */
#define ERR_NOXFER      467     /* No articles follow (V2) */
#define	ERR_OLDNOAUTH	480	/* authorization required for command */
#define	ERR_OLDAUTHSYS	481	/* Authorization system invalid */
#define	ERR_OLDAUTHREJ	482	/* Authorization data rejected */

#define	ERR_COMMAND	500	/* Command not recognized */
#define	ERR_CMDSYN	501	/* Command syntax error */
#define	ERR_ACCESS	502	/* Access to server denied */
#define	ERR_FAULT	503	/* Program fault, command not performed */
#define ERR_PATH        531     /* DATA-PATCH option not set (V2) */
#define ERR_BATCH       567     /* BATCH option not set (V2) */
#define	ERR_OLDAUTHBAD	580	/* Authorization Failed */

/* RFC 977 defines this; don't change it. */

#define	NNTP_STRLEN	512
