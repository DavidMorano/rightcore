/* keysym */

/* KeySym character defines */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	Originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	Notes:

	= Gaps

        There are gaps in the KEYSYM record here. Not all key-codes have
        associated KEYSYMs, at least not KEYSYMs that are listed here. It does
        not mean that those missing key-codes do not exist. They exist but just
        have not had a KEYSYM name assigned here. A KEYSYM may be assigned
        someplace else.

	= Schema

        KEYSYMs that have values between 0 and 255 represent the ISO-Latin-1
        characters that are associated with those values. For other KEYSYMs,
        they have relatively pseudo-random values.

	= Aliases

        Some KEYSYM names are aliases of others. So it is not a one-to-one
        mapping. Several KEYSYMs may map to the same values.


******************************************************************************/


#ifndef	KEYSYM_INCLUDE
#define	KEYSYM_INCLUDE	1


#define	KEYSYM_SOH		0x01
#define	KEYSYM_STX		0x02
#define	KEYSYM_ETX		0x03
#define	KEYSYM_EOT		0x04
#define	KEYSYM_ENQ		0x05
#define	KEYSYM_ACK		0x06
#define	KEYSYM_BEL		0x07
#define	KEYSYM_BS		0x08
#define	KEYSYM_TAB		0x09
#define	KEYSYM_HT		0x09
#define	KEYSYM_LF		0x0A
#define	KEYSYM_VT		0x0B
#define	KEYSYM_FF		0x0C
#define	KEYSYM_CR		0x0D
#define	KEYSYM_SO		0x0E
#define	KEYSYM_SI		0x0F
#define	KEYSYM_DLE		0x10
#define	KEYSYM_DC1		0x11
#define	KEYSYM_DC2		0x12
#define	KEYSYM_DC3		0x13
#define	KEYSYM_DC4		0x14
#define	KEYSYM_NAK		0x15
#define	KEYSYM_SYN		0x16
#define	KEYSYM_ETB		0x17
#define	KEYSYM_CAN		0x18
#define	KEYSYM_EM		0x19
#define	KEYSYM_CY		0x19
#define	KEYSYM_SUB		0x1A
#define	KEYSYM_ESC		0x1B
#define	KEYSYM_FS		0x1C		/* field separator */
#define	KEYSYM_GS		0x1D		/* group separator */
#define	KEYSYM_RS		0x1E		/* record separator */
#define	KEYSYM_US		0x1F		/* unit separator */
#define	KEYSYM_SP		0x20		/* space */
#define	KEYSYM_PLUS		0x2B		/* plus */
#define	KEYSYM_MINUS		0x2D		/* minus */
#define	KEYSYM_SQUOTE		0x27		/* quote single */
#define	KEYSYM_DQUOTE		0x22		/* quote double */
#define	KEYSYM_FSLASH		0x2F		/* slash forward */
#define	KEYSYM_BSLASH		0x5C		/* slash backward */
#define	KEYSYM_LPAREN		0x28		/* parenthesis left */
#define	KEYSYM_RPAREN		0x29		/* parenthesis right */
#define	KEYSYM_LBRACE		0x7B		/* brace left */
#define	KEYSYM_RBRACE		0x7D		/* brace right */
#define	KEYSYM_LBRACK		0x5B		/* bracket left */
#define	KEYSYM_RBRACK		0x5D		/* bracket right */
#define	KEYSYM_DEL		0x7F		/* delete */

#define	KEYSYM_IND		0x84		/* index */
#define	KEYSYM_NEL		0x85		/* next line */
#define	KEYSYM_HTS		0x88		/* horizontal tab set */
#define	KEYSYM_RI		0x8D		/* reverse index */
#define	KEYSYM_SS2		0x8E		/* single shift 2 */
#define	KEYSYM_SS3		0x8F		/* single shift 3 */
#define	KEYSYM_DCS		0x90		/* device control string */
#define	KEYSYM_SOS		0x98		/* start of string */
#define	KEYSYM_DECID		0x9A		/* DEC private identification */
#define	KEYSYM_CSI		0x9B		/* cntrl-seq-intro */
#define	KEYSYM_ST		0x9C		/* string terminator */
#define	KEYSYM_OSC		0x9D		/* operating system command */
#define	KEYSYM_PM		0x9E		/* privacy message */
#define	KEYSYM_APC		0x9F		/* app-program-command */
#define	KEYSYM_NBSP		0xA0		/* non-breaking space */


/* aliases: some extras because they appear now and then */

#define	KEYSYM_NL		KEYSYM_LF	/* new-line */
#define	KEYSYM_EOL		KEYSYM_LF	/* end-of-line */
#define	KEYSYM_XON		0x11
#define	KEYSYM_XOFF		0x13
#define	KEYSYM_RIND		KEYSYM_RI	/* reverse index */
#define	KEYSYM_SPACE		KEYSYM_SP	/* space */
#define	KEYSYM_SYNC		KEYSYM_SYN	/* sync */
#define	KEYSYM_APOSTROPHE	KEYSYM_SQUOTE	/* apostrophe */
#define	KEYSYM_QUOTE		KEYSYM_DQUOTE	/* quote double */
#define	KEYSYM_BELL		KEYSYM_BEL	/* bell */

#define	KEYSYM_LineFeed		KEYSYM_LF
#define	KEYSYM_FormFeed		KEYSYM_FF
#define	KEYSYM_Plus		'+'
#define	KEYSYM_Minus		'-'

#define	KEYSYM_CurLeft		0x1002
#define	KEYSYM_CurRight		0x1003

#define	KEYSYM_CurUp		0x1004
#define	KEYSYM_CurDown		0x1005

#define	KEYSYM_Find		0x1008
#define	KEYSYM_Select		0x1009

#define	KEYSYM_Insert		0x100A
#define	KEYSYM_Remove		0x100B

#define	KEYSYM_Help		0x100C
#define	KEYSYM_Do		0x100D

#define	KEYSYM_Next		0x100E
#define	KEYSYM_Previous		0x100F

#define	KEYSYM_PF1		0x1011
#define	KEYSYM_PF2		0x1012
#define	KEYSYM_PF3		0x1013
#define	KEYSYM_PF4		0x1014

/* not all keyboards have these */
#define	KEYSYM_PageDown		0x1015
#define	KEYSYM_PageUp		0x1016

#define	KEYSYM_F1		0x2001
#define	KEYSYM_F2		0x2002
#define	KEYSYM_F3		0x2003
#define	KEYSYM_F4		0x2004
#define	KEYSYM_F5		0x2005
#define	KEYSYM_F6		0x2006
#define	KEYSYM_F7		0x2007
#define	KEYSYM_F8		0x2008
#define	KEYSYM_F9		0x2009
#define	KEYSYM_F10		0x200a
#define	KEYSYM_F11		0x200b
#define	KEYSYM_F12		0x200c
#define	KEYSYM_F13		0x200d
#define	KEYSYM_F14		0x200e
#define	KEYSYM_F15		0x200f
#define	KEYSYM_F16		0x2010
#define	KEYSYM_F17		0x2011
#define	KEYSYM_F18		0x2012
#define	KEYSYM_F19		0x2013
#define	KEYSYM_F20		0x2014
#define	KEYSYM_F21		0x2015
#define	KEYSYM_F22		0x2016
#define	KEYSYM_F23		0x2017
#define	KEYSYM_F24		0x2018
#define	KEYSYM_F25		0x2019
#define	KEYSYM_F26		0x201a
#define	KEYSYM_F27		0x201b
#define	KEYSYM_F28		0x201c
#define	KEYSYM_F29		0x201d
#define	KEYSYM_F30		0x201e
#define	KEYSYM_F31		0x201f
#define	KEYSYM_F32		0x2020
#define	KEYSYM_F33		0x2021
#define	KEYSYM_F34		0x2022
#define	KEYSYM_F35		0x2023
#define	KEYSYM_F36		0x2024
#define	KEYSYM_F37		0x2025
#define	KEYSYM_F38		0x2026
#define	KEYSYM_F39		0x2027
#define	KEYSYM_F40		0x2028
#define	KEYSYM_F41		0x2029
#define	KEYSYM_F42		0x202a
#define	KEYSYM_F43		0x202b
#define	KEYSYM_F44		0x202c
#define	KEYSYM_F45		0x202d
#define	KEYSYM_F46		0x202e
#define	KEYSYM_F47		0x202f
#define	KEYSYM_F48		0x2030
#define	KEYSYM_F49		0x2031
#define	KEYSYM_F50		0x2032
#define	KEYSYM_F51		0x2033
#define	KEYSYM_F52		0x2034
#define	KEYSYM_F53		0x2035
#define	KEYSYM_F54		0x2036
#define	KEYSYM_F55		0x2037
#define	KEYSYM_F56		0x2038
#define	KEYSYM_F57		0x2039
#define	KEYSYM_F58		0x203a
#define	KEYSYM_F59		0x203b
#define	KEYSYM_F60		0x203c
#define	KEYSYM_F61		0x203d
#define	KEYSYM_F62		0x203e
#define	KEYSYM_F63		0x203f
#define	KEYSYM_F64		0x2040
#define	KEYSYM_F65		0x2041
#define	KEYSYM_F66		0x2042
#define	KEYSYM_F67		0x2043
#define	KEYSYM_F68		0x2044
#define	KEYSYM_F69		0x2045
#define	KEYSYM_F70		0x2046
#define	KEYSYM_F71		0x2047
#define	KEYSYM_F72		0x2048
#define	KEYSYM_F73		0x2049
#define	KEYSYM_F74		0x204a
#define	KEYSYM_F75		0x204b
#define	KEYSYM_F76		0x204c
#define	KEYSYM_F77		0x204d
#define	KEYSYM_F78		0x204e
#define	KEYSYM_F79		0x204f
#define	KEYSYM_F80		0x2050
#define	KEYSYM_F81		0x2051
#define	KEYSYM_F82		0x2052
#define	KEYSYM_F83		0x2053
#define	KEYSYM_F84		0x2054
#define	KEYSYM_F85		0x2055
#define	KEYSYM_F86		0x2056
#define	KEYSYM_F87		0x2057
#define	KEYSYM_F88		0x2058
#define	KEYSYM_F89		0x2059
#define	KEYSYM_F90		0x205a
#define	KEYSYM_F91		0x205b
#define	KEYSYM_F92		0x205c
#define	KEYSYM_F93		0x205d
#define	KEYSYM_F94		0x205e
#define	KEYSYM_F95		0x205f
#define	KEYSYM_F96		0x2060
#define	KEYSYM_F97		0x2061
#define	KEYSYM_F98		0x2062
#define	KEYSYM_F99		0x2063


#endif /* KEYSYM_INCLUDE */


