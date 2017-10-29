static char SCCSID[] = "@(#) ctype.c:  4.1 12/12/82";
/* ctype.c
**
**	TECO character type tables
**
**	David Kristol, June, 1982
**
**	This module contains tables which describe TECO's notion
**	of various character properties.  Those properties are:
**
**		is lower case character
**		is upper case character
**		is symbol constituent
**		is Q-register name
**		is special Q-register name
**		is letter
**		is digit
**		is character c, such that ^c is a valid control
**			character construct
**
**	The astute observer will note a close correspondence between
**	some of these properties and the properties that various
**	conditional constructs test.
**
**	For many of these properties there is a separate table.  The
**	separate table has an entry if the property is true (in general).
**	For example, if a character is a Q-register name, the corresponding
**	entry in the Q-register table is the text block number related
**	to that character.
**
**	The various predicates and property symbols are defined by
**	ctype.h .
*/


#include "ctype.h"			/* define property types */
#include "mdconfig.h"			/* for "ENVVAR" */
#include "qname.h"			/* Q-register names */
/* control character table
**
** If a character c has the Cf_ctrl flag bit set in Ct_flags, Ct_ctrl
** contains the character to replace "^c" by.
*/

unsigned char Ct_ctrl[] =
{
/* 000 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 010 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 020 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 030 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 040 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 050 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 060 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 070 */	0,	0,	0,	0,	0,	0,	0,	0177,
/* 100 */	00,	01,	02,	03,	04,	05,	06,	07,
/* 110 */	010,	011,	012,	013,	014,	015,	016,	017,
/* 120 */	020,	021,	022,	023,	024,	025,	026,	027,
/* 130 */	030,	031,	032,	033,	034,	035,	036,	037,
/* 140 */	0,	01,	02,	03,	04,	05,	06,	07,
/* 150 */	010,	011,	012,	013,	014,	015,	016,	017,
/* 160 */	020,	021,	022,	023,	024,	025,	026,	027,
/* 170 */	030,	031,	032,	0,	0,	0,	0,	0
};
/* Q register table
**
** A byte in this table contains the text block number for the
** corresponding Q-register if the character is a regular or special
** Q-register name.  The Cf_sqreg flag bit is set in the Ct_flags
** byte corresponding to the character if the Q-register is a "special
** one".
*/

#ifdef	ENVVAR				/* if support environment variable
					** feature
					*/
#define	Q_ENV	Q_env			/* use real register */
#else
#define	Q_ENV	0			/* register doesn't exist */
#endif


unsigned char Ct_qreg[] =
{
/* 000 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 010 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 020 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 030 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 040 */	0,	0,	0,	0,	Q_ENV,	0,	0,	0,
/* 050 */	0,	0,	Q_file,	0,	0,	0,	0,	0,
/* 060 */	Q_0,	Q_1,	Q_2,	Q_3,	Q_4,	Q_5,	Q_6,	Q_7,
/* 070 */	Q_8,	Q_9,	0,	0,	0,	0,	0,	0,
/* 100 */	0,	Q_A,	Q_B,	Q_C,	Q_D,	Q_E,	Q_F,	Q_G,
/* 110 */	Q_H,	Q_I,	Q_J,	Q_K,	Q_L,	Q_M,	Q_N,	Q_O,
/* 120 */	Q_P,	Q_Q,	Q_R,	Q_S,	Q_T,	Q_U,	Q_V,	Q_W,
/* 130 */	Q_X,	Q_Y,	Q_Z,	0,	0,	0,	0,	Q_search,
/* 140 */	0,	Q_A,	Q_B,	Q_C,	Q_D,	Q_E,	Q_F,	Q_G,
/* 150 */	Q_H,	Q_I,	Q_J,	Q_K,	Q_L,	Q_M,	Q_N,	Q_O,
/* 160 */	Q_P,	Q_Q,	Q_R,	Q_S,	Q_T,	Q_U,	Q_V,	Q_W,
/* 170 */	Q_X,	Q_Y,	Q_Z,	0,	0,	0,	0,	0
};
/* Case table
**
** Entries in this table are for case conversion.  For a character c,
** if either the Cf_lc or Cf_uc flag is set in Ct_flags, this table
** contains the character corresponding to the OTHER case.  For example,
** Cf_lc is set for 'h', and this table contains 'H' in the corresponding
** table entry.
*/

unsigned char Ct_case[] =
{
/* 000 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 010 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 020 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 030 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 040 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 050 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 060 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 070 */	0,	0,	0,	0,	0,	0,	0,	0,
/* 100 */	'`',	'a',	'b',	'c',	'd',	'e',	'f',	'g',
/* 110 */	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',
/* 120 */	'p',	'q',	'r',	's',	't',	'u',	'v',	'w',
/* 130 */	'x',	'y',	'z',	'{',	'|',	'}',	'~',	0,
/* 140 */	'@',	'A',	'B',	'C',	'D',	'E',	'F',	'G',
/* 150 */	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',
/* 160 */	'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',
/* 170 */	'X',	'Y',	'Z',	'[',	'\\',	']',	'^',	0
};
/* character flags
**
** This table contains the various flag bits describing each character.
*/

unsigned char Ct_flags[] =
{
/* 000 ^@ */	0,
/* 001 ^A */	0,
/* 002 ^B */	0,
/* 003 ^C */	0,
/* 004 ^D */	0,
/* 005 ^E */	0,
/* 006 ^F */	0,
/* 007 ^G */	0,
/* 010 BS */	0,
/* 011 TAB */	0,
/* 012 LF */	Cf_lt,
/* 013 VT */	Cf_lt,
/* 014 FF */	Cf_lt,
/* 015 CR */	0,
/* 016 ^N */	0,
/* 017 ^O */	0,
/* 020 ^P */	0,
/* 021 ^Q */	0,
/* 022 ^R */	0,
/* 023 ^S */	0,
/* 024 ^T */	0,
/* 025 ^U */	0,
/* 026 ^V */	0,
/* 027 ^W */	0,
/* 030 ^X */	0,
/* 031 ^Y */	0,
/* 032 ^Z */	0,
/* 033 ESC */	0,
/* 034 ^\ */	0,
/* 035 ^] */	0,
/* 036 ^^ */	0,
/* 037 ^_ */	0,
/* 040 SPACE */	0,
/* 041 ! */	0,
/* 042 " */	0,
/* 043 # */	0,
/* 044 $ */	Cf_sqreg,
/* 045 % */	0,
/* 046 & */	0,
/* 047 ' */	0,
/* 050 ( */	0,
/* 051 ) */	0,
/* 052 * */	Cf_sqreg,
/* 053 + */	0,
/* 054 , */	0,
/* 055 - */	0,
/* 056 . */	0,
/* 057 / */	0,
/* 060 0 */	Cf_digit,
/* 061 1 */	Cf_digit,
/* 062 2 */	Cf_digit,
/* 063 3 */	Cf_digit,
/* 064 4 */	Cf_digit,
/* 065 5 */	Cf_digit,
/* 066 6 */	Cf_digit,
/* 067 7 */	Cf_digit,
/* 070 8 */	Cf_digit,
/* 071 9 */	Cf_digit,
/* 072 : */	0,
/* 073 ; */	0,
/* 074 < */	0,
/* 075 = */	0,
/* 076 > */	0,
/* 077 ? */	Cf_ctrl,
/* 100 @ */	Cf_ctrl | Cf_uc,
/* 101 A */	Cf_ctrl | Cf_uc | Cf_let,
/* 102 B */	Cf_ctrl | Cf_uc | Cf_let,
/* 103 C */	Cf_ctrl | Cf_uc | Cf_let,
/* 104 D */	Cf_ctrl | Cf_uc | Cf_let,
/* 105 E */	Cf_ctrl | Cf_uc | Cf_let,
/* 106 F */	Cf_ctrl | Cf_uc | Cf_let,
/* 107 G */	Cf_ctrl | Cf_uc | Cf_let,
/* 110 H */	Cf_ctrl | Cf_uc | Cf_let,
/* 111 I */	Cf_ctrl | Cf_uc | Cf_let,
/* 112 J */	Cf_ctrl | Cf_uc | Cf_let,
/* 113 K */	Cf_ctrl | Cf_uc | Cf_let,
/* 114 L */	Cf_ctrl | Cf_uc | Cf_let,
/* 115 M */	Cf_ctrl | Cf_uc | Cf_let,
/* 116 N */	Cf_ctrl | Cf_uc | Cf_let,
/* 117 O */	Cf_ctrl | Cf_uc | Cf_let,
/* 120 P */	Cf_ctrl | Cf_uc | Cf_let,
/* 121 Q */	Cf_ctrl | Cf_uc | Cf_let,
/* 122 R */	Cf_ctrl | Cf_uc | Cf_let,
/* 123 S */	Cf_ctrl | Cf_uc | Cf_let,
/* 124 T */	Cf_ctrl | Cf_uc | Cf_let,
/* 125 U */	Cf_ctrl | Cf_uc | Cf_let,
/* 126 V */	Cf_ctrl | Cf_uc | Cf_let,
/* 127 W */	Cf_ctrl | Cf_uc | Cf_let,
/* 130 X */	Cf_ctrl | Cf_uc | Cf_let,
/* 131 Y */	Cf_ctrl | Cf_uc | Cf_let,
/* 132 Z */	Cf_ctrl | Cf_uc | Cf_let,
/* 133 [ */	Cf_ctrl | Cf_uc,
/* 134 \ */	Cf_ctrl | Cf_uc,
/* 135 ] */	Cf_ctrl | Cf_uc,
/* 136 ^ */	Cf_ctrl | Cf_uc,
/* 137 _ */	Cf_ctrl | Cf_sqreg | Cf_sym,
/* 140 ` */	Cf_lc,
/* 141 a */	Cf_ctrl | Cf_lc | Cf_let,
/* 142 b */	Cf_ctrl | Cf_lc | Cf_let,
/* 143 c */	Cf_ctrl | Cf_lc | Cf_let,
/* 144 d */	Cf_ctrl | Cf_lc | Cf_let,
/* 145 e */	Cf_ctrl | Cf_lc | Cf_let,
/* 146 f */	Cf_ctrl | Cf_lc | Cf_let,
/* 147 g */	Cf_ctrl | Cf_lc | Cf_let,
/* 150 h */	Cf_ctrl | Cf_lc | Cf_let,
/* 151 i */	Cf_ctrl | Cf_lc | Cf_let,
/* 152 j */	Cf_ctrl | Cf_lc | Cf_let,
/* 153 k */	Cf_ctrl | Cf_lc | Cf_let,
/* 154 l */	Cf_ctrl | Cf_lc | Cf_let,
/* 155 m */	Cf_ctrl | Cf_lc | Cf_let,
/* 156 n */	Cf_ctrl | Cf_lc | Cf_let,
/* 157 o */	Cf_ctrl | Cf_lc | Cf_let,
/* 160 p */	Cf_ctrl | Cf_lc | Cf_let,
/* 161 q */	Cf_ctrl | Cf_lc | Cf_let,
/* 162 r */	Cf_ctrl | Cf_lc | Cf_let,
/* 163 s */	Cf_ctrl | Cf_lc | Cf_let,
/* 164 t */	Cf_ctrl | Cf_lc | Cf_let,
/* 165 u */	Cf_ctrl | Cf_lc | Cf_let,
/* 166 v */	Cf_ctrl | Cf_lc | Cf_let,
/* 167 w */	Cf_ctrl | Cf_lc | Cf_let,
/* 170 x */	Cf_ctrl | Cf_lc | Cf_let,
/* 171 y */	Cf_ctrl | Cf_lc | Cf_let,
/* 172 z */	Cf_ctrl | Cf_lc | Cf_let,
/* 173 { */	Cf_lc,
/* 174 | */	Cf_lc,
/* 175 } */	Cf_lc,
/* 176 ~ */	Cf_lc,
/* 177 DEL */	0
};
