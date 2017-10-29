/*	static char h_ctype[] = "@(#) ctype.h:  4.2 12/12/82";	*/
/* ctype.h
**
**	TECO character properties header
**
**	David Kristol, June, 1982
*/

/*	Declare character properties.  The bits are deliberately
**	designed to fit in a byte.
*/

#define	Cf_let		(1<<0)	/* character is a letter */
#define	Cf_digit	(1<<1)	/* character is a digit */
#define	Cf_sym		(1<<2)	/* character is a symbol constituent */
#define	Cf_lc		(1<<3)	/* character is lower case */
#define	Cf_uc		(1<<4)	/* character is upper case */
#define	Cf_lt		(1<<5)	/* character is a line terminator */
#define	Cf_ctrl		(1<<6)	/* character may follow '^' as part of
				** control character construction
				*/
/* Note:  a character is a Q-register if it has a non-zero entry in
** Ct_qreg.
*/
#define	Cf_sqreg	(1<<7)	/* character is special Q-register name */


/* Declare character property predicates. */

#define	ISLET(c)	( (Ct_flags[c] & Cf_let) != 0)
#define	ISDIG(c)	( (Ct_flags[c] & Cf_digit) != 0)
#define	ISSYM(c)	( (Ct_flags[c] & (Cf_let | Cf_digit | Cf_sym)) != 0 )
#define	ISQREG(c,special) \
			(    (Ct_qreg[c] != 0) \
			 &&  (special || (Ct_flags[c] & Cf_sqreg) == 0) \
			 )
#define	ISLT(c)		( (Ct_flags[c] & Cf_lt) != 0 )
#define	QNUM(c)		( Ct_qreg[c] != 0 ? Ct_qreg[c] : -1 )
#define	TRCTRL(c)	( (Ct_flags[c] & Cf_ctrl) != 0 ? Ct_ctrl[c] : -1 )
#define	ISLC(c)		( (Ct_flags[c] & Cf_lc) != 0 )
#define	ISUC(c)		( (Ct_flags[c] & Cf_uc) != 0 )
#define	LOWER(c)	( (Ct_flags[c] & Cf_uc) != 0 ? Ct_case[c] : c )
#define	UPPER(c)	( (Ct_flags[c] & Cf_lc) != 0 ? Ct_case[c] : c )

/* Declare character tables. */

extern unsigned char Ct_ctrl[];	/* control character table */
extern unsigned char Ct_qreg[];	/* Q-register table */
extern unsigned char Ct_case[];	/* case table */
extern unsigned char Ct_flags[];/* character flags */
