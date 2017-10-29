/*	static char h_tflags[] = "@(#) tflags.h:  5.1 2/22/83";	*/
/* tflags.h
 *
 *	Define TECO flag word bits
 */

/* Define bits in ED word */

#define ED_upar		(1<<0)	/* allow ^ in search strings */
#define ED_yank		(1<<1)	/* unconditionally allow Y */
#define ED_expand	(1<<2)	/* allow memory expansion */
/* 1<<3 reserved */
#define ED_predot	(1<<4)	/* preserve . on failing searches */
/* 1<<5 reserved */
#define ED_sstep	(1<<6)	/* step . by 1 on search match */


/* Define bits for EH flag */

#define EH_short	(1<<0)	/* output short form of error only */
#define EH_type		3	/* bits providing mask for type of
				** error message to produce
				*/
#define EH_trace	(1<<2)	/* do error trace on error */



/* Define bits for ET flag */

#define ET_image	(1<<0)	/* type-out is in image mode */
#define ET_crt		(1<<1)	/* terminal is a crt */
#define ET_rlc		(1<<2)	/* terminal can read lower case */
#define ET_rnoe		(1<<3)	/* read with no echo */
#define ET_ctO		(1<<4)	/* cancel ^O */
#define ET_rnow		(1<<5)	/* read with no wait (^T) */
#define	ET_ttydet	(1<<6)	/* detach terminal */
#define ET_mung		(1<<7)	/* abort on error */
#define ET_trunc	(1<<8)	/* truncate output to terminal width */
#define ET_crtW		(1<<9)	/* terminal is VT52 and VT52 support is
				 * available (can do W cmd)
				 */
#define ET_refW		(1<<10)	/* VT11 is present and VT11 support is
				 * available (can do W cmd)
				 */
#define ET_ctC		(1<<15)	/* trap ^C */

#define ET_get	(ET_crt|ET_rlc|ET_crtW|ET_refW)
				/* bits that can be retrieved when looking
				 * at ET flag
				 */
