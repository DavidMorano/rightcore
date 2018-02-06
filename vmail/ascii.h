/* ascii */

/* ASCII character defines */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	Originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	ASCII_INCLUDE
#define	ASCII_INCLUDE	1


#define	CH_NUL		0x00
#define	CH_SOH		0x01
#define	CH_STX		0x02
#define	CH_ETX		0x03		/* end of text (control-C) */
#define	CH_EOT		0x04		/* end of transmission (control-D) */
#define	CH_ENQ		0x05
#define	CH_ACK		0x06
#define	CH_BEL		0x07
#define	CH_BS		0x08
#define	CH_TAB		0x09
#define	CH_HT		0x09
#define	CH_LF		0x0A		/* control-J */
#define	CH_VT		0x0B		/* control-K */
#define	CH_FF		0x0C		/* control-L */
#define	CH_CR		0x0D		/* control-M */
#define	CH_SO		0x0E		/* contrll-N */
#define	CH_SI		0x0F		/* control-O */
#define	CH_DLE		0x10		/* control-P */
#define	CH_DC1		0x11		/* control-Q */
#define	CH_DC2		0x12		/* control-R */
#define	CH_DC3		0x13		/* control-S */
#define	CH_DC4		0x14		/* control-T */
#define	CH_NAK		0x15		/* control-U */
#define	CH_SYN		0x16		/* control-V */
#define	CH_ETB		0x17		/* control-W */
#define	CH_CAN		0x18		/* control-X */
#define	CH_EM		0x19		/* control-Y */
#define	CH_SUB		0x1A		/* control-Z (substitute) */
#define	CH_ESC		0x1B
#define	CH_FS		0x1C		/* field separator */
#define	CH_GS		0x1D		/* group separator */
#define	CH_RS		0x1E		/* record separator */
#define	CH_US		0x1F		/* unit separator */

#define	CH_SP		0x20		/* space */
#define	CH_PLUS		0x2B		/* plus */
#define	CH_MINUS	0x2D		/* minus */
#define	CH_SQUOTE	0x27		/* quote single (') */
#define	CH_BQUOTE	0x60		/* quote back (`) */
#define	CH_DQUOTE	0x22		/* quote double (") */
#define	CH_FSLASH	0x2F		/* slash forward */
#define	CH_BSLASH	0x5C		/* slash backward */
#define	CH_LPAREN	0x28		/* parenthesis left */
#define	CH_RPAREN	0x29		/* parenthesis right */
#define	CH_COMMA	0x2C		/* comma */
#define	CH_LANGLE	0x3C		/* angle left */
#define	CH_RANGLE	0x3E		/* angle right */
#define	CH_LBRACK	0x5B		/* bracket left */
#define	CH_RBRACK	0x5D		/* bracket right */
#define	CH_LBRACE	0x7B		/* brace left */
#define	CH_RBRACE	0x7D		/* brace right */
#define	CH_DEL		0x7F		/* delete */

#define	CH_B0		0x80
#define	CH_B1		0x81
#define	CH_B2		0x82
#define	CH_B3		0x83
#define	CH_IND		0x84		/* index */
#define	CH_NEL		0x85		/* next line */
#define	CH_SSA		0x86
#define	CH_ESA		0x87
#define	CH_HTS		0x88		/* horizontal tab set */
#define	CH_HTJ		0x89
#define	CH_VTS		0x8A
#define	CH_PLD		0x8B
#define	CH_PLU		0x8C
#define	CH_RI		0x8D		/* reverse index */
#define	CH_SS2		0x8E		/* single shift 2 */
#define	CH_SS3		0x8F		/* single shift 3 */

#define	CH_DCS		0x90		/* device control string */
#define	CH_PU1		0x91
#define	CH_PU2		0x92
#define	CH_STS		0x93
#define	CH_CRH		0x94
#define	CH_MW		0x95
#define	CH_SPA		0x96
#define	CH_EPA		0x97
#define	CH_SOS		0x98		/* start of string */
/* code-0x99 is not specifically defined */
#define	CH_DECID	0x9A		/* DEC private identification */
#define	CH_CSI		0x9B		/* control sequence introducer */
#define	CH_ST		0x9C		/* string terminator */
#define	CH_OSC		0x9D		/* operating system command */
#define	CH_PM		0x9E		/* privacy message */
#define	CH_APC		0x9F		/* application program command */

#define	CH_NBSP		0xA0		/* non-breaking space */


/* aliases: some extras because they appear now and then */

#define	CH_NL		CH_LF		/* new-line */
#define	CH_EOL		CH_LF		/* end-of-line */
#define	CH_XON		0x11
#define	CH_XOFF		0x13
#define	CH_RIND		CH_RI		/* reverse index */
#define	CH_SPACE	CH_SP		/* space */
#define	CH_SYNC		CH_SYN		/* sync */
#define	CH_APOSTROPHE	CH_SQUOTE	/* apostrophe */
#define	CH_QUOTE	CH_DQUOTE	/* quote double */
#define	CH_BELL		CH_BEL		/* bell */


#endif /* ASCII_INCLUDE */



