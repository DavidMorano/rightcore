/* main (testansi) */

/* test something to do with ANSI compliance */


#define	CF_MINUSONE	0		/* minus one */
#define	CF_ANSI		0		/* ANSI conformance */
#define	CF_STDC		1		/* __STDC__ */
#define	CF_INST		1		/* loop variable instance */
#define	CF_STDIO	1		/* stdio */
#define	CF_EXP1		0		/* exp-1 */
#define	CF_EXP2		0		/* exp-2 */
#define	CF_EXP3		0		/* exp-3 */
#define	CF_EXP4		0		/* exp-4 */
#define	CF_GENERIC	0		/* generic */


/* local defines */

#include	<envstandards.h>
#include	<sys/types.h>
#include	<stdio.h>
#include	<bfile.h>
#include	<localmisc.h>


/* external subroutines */

#if	CF_MINUSONE
extern int	minus_one(int) ;
#endif


/* forward references */

#if	CF_EXP2
static int	printsub(bfile *,uint) ;
#endif /* CF_EXP2 */

#if	CF_EXP4
static int	sub4(bfile *) ;
#endif /* CF_EXP4 */

#if	CF_GENERIC
static int	findi(int) ;
static int	findl(long) ;
#endif /* CF_GENERIC */

#define	ourfind(x)	_Generic((x),int:findi,long:findl)(x)


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	bfile		ofile, *ofp = &ofile ;
	int		rs ;
	int		ex = 0 ;
	cchar		*ifn = BFILE_STDOUT ;

	if ((rs = bopen(ofp,ifn,"wct",0666)) >= 0) {
	    cchar	*fmt ;

#if	CF_STDC
	    {
		off_t	fo = -1 ;
	        fmt = "STDC=%u\n" ;
	        bprintf(ofp,fmt,__STDC__) ;
	        fmt = "fo=%lld\n" ;
	        bprintf(ofp,fmt,fo) ;
	    }
#endif /* CF_STDC */

#if	CF_INST
	{
	    fmt = "&s: i=%d\n" ;
	    for (int i = 0 ; i < 2 ; i += 1) {
	        bprintf(ofp,fmt,i) ;
	    }
	}
#endif /* CF_INST */

#if	CF_STDIO
	    {
		FILE	*ifp = fopen("main.c","r") ;
		off_t	fo = -1 ;
	        fmt = "STDC=%u\n" ;
	        bprintf(ofp,fmt,__STDC__) ;
	        fmt = "fo=%lld\n" ;
	        bprintf(ofp,fmt,fo) ;
		fo = ftell(ifp) ;
	        bprintf(ofp,fmt,fo) ;
		fclose(ifp) ;
	    }
#endif /* CF_STDIO */

#if	CF_MINUSONE
	v = minus_one(1) ;
#endif

#if	CF_ANSI
	{
	    uint	uv ;
	    uint	ui = 26U ;
	    int		v ;
	    int 	i = us ;
	    ushort	us = 0xFFFF ;
	    uchar	uc = (uchar) v ;

	if (us < v) {
		bprintf(ofp,"less (K&R)\n") ;
	} else if (us > v) {
		bprintf(ofp,"greater (ANSI)\n") ;
	} else {
		bprintf(ofp,"equal\n") ;
	}

	bprintf(ofp,"uc=%08X us=%08X ui=%08X i=%08X\n",
		uc,us,ui,i) ;

	uv = v ;
	bprintf(ofp,"v=%08X uv=%08X\n", v,uv) ;
	}
#endif /* CF_ANSI */


/* second experiment */

#if	CF_EXP2
	{
	    int		v = uc ;
	    uchar	uc = 0xFF ;
	    char	ch ;

	bprintf(ofp,"uc=%04x v=%08x\n",uc,v) ;

	printsub(ofp,uc) ;

	ch = 0xff ;
	v = ch ;
	bprintf(ofp,"ch=%04x v=%08x\n",ch,v) ;

	printsub(ofp,ch) ;

	}
#endif /* CF_EXP2 */


/* third experiment */

#if	CF_EXP3
	{
	    uint	uv = ch ;
	    uchar	uc = 0xff ;
	    uv = uc ;
	    bprintf(ofp,"uc=%08x uv=%08x\n",ch,uv) ;
	}

#endif /* CF_EXP3 */

/* fourth */

#if	CF_EXP4
	sub4(ofp) ;
#endif /* CF_EXP4 */

#if	CF_GENERIC
	bprintf(ofp,"%u\n",ourfind(2)) ;
#endif

/* out of here */

	bclose(ofp) ;
	} /* end if (bfile) */

	if (rs < 0) ex = 1 ;
	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


#if	CF_EXP2
static int printsub(bfile *fp,uint v)
{
	return bprintf(fp,"%08x\n",v) ;
}
/* end subroutine (printsub) */
#endif /* CF_EXP2 */


#if	CF_EXP4
static int sub4(bfile *ofp) {
	off_t	pos = ((off_t) -1) ;
	bprintf(ofp,"off=%lld\n",pos) ;
	return SR_OK ;
}
#endif /* CF_EXP4 */


#if	CF_GENERIC
static int findi(int x) {
	return (x == 0) ;
}
static int findl(long x) {
	return (x == 0) ;
}
#endif /* CF_GENERIC */


