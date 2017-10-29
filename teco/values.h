/*	static char h_values[] = "@(#) values.h:  4.2 12/12/82";	*/
/* values.h -- define value-related routines */
/* must be preceded by bool.h */

typedef int TNUMB;			/* this must agree with values.c */

extern short Pstackp;			/* current paren stack offset */
extern short Stackdepth;		/* current paren stack depth */

extern void Valinit();
extern void Set1val();
extern void Set2val();
extern void Set1dfl();
extern void Set012dfl();
extern void Setop();
extern void Nextval();
extern BOOL Get1val();
extern BOOL Get2val();
extern void Pushv();
extern void Popv();


