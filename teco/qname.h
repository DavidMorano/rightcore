/*	static char h_qname[] = "@(#) qname.h:  4.2 12/12/82";	*/
/* define Q register and pseudo Q register names */

#define Q_text		1	/* main text buffer */
#define Q_search	2	/* search string buffer */
#define Q_file		3	/* filename string buffer */
#define Q_cmd		4	/* command string buffer */
#define Q_stack		5	/* Q register stack */
#define Q_goto		6	/* target label of goto */
#define	Q_env		7	/* name of environment variable */
#define Q_syscmd	8	/* temporary for EG command */
#define Q_btemp		9	/* temporary for Buildstring */
#define Q_entemp	10	/* temporary for CMencmd */

/* vanilla Q registers */
/* these Q registers are assumed to be in lexical order following Q_A */

#define Q_A		11	/* QA */
#define Q_B		12	/* etc. */
#define Q_C		13
#define Q_D		14
#define Q_E		15
#define Q_F		16
#define Q_G		17
#define Q_H		18
#define Q_I		19
#define Q_J		20
#define Q_K		21
#define Q_L		22
#define Q_M		23
#define Q_N		24
#define Q_O		25
#define Q_P		26
#define Q_Q		27
#define Q_R		28
#define Q_S		29
#define Q_T		30
#define Q_U		31
#define Q_V		32
#define Q_W		33
#define Q_X		34
#define Q_Y		35
#define Q_Z		36

/* these Q registers are assumed to be in order following Q_0 */

#define Q_0		37
#define Q_1		38
#define Q_2		39
#define Q_3		40
#define Q_4		41
#define Q_5		42
#define Q_6		43
#define Q_7		44
#define Q_8		45
#define Q_9		46


/* minimum and maximum Q register numbers */

#define Q_MIN	1
#define Q_MAX	46
