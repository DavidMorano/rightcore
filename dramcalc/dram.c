

#define	F_SCANF		0
#define	F_DDR400	0


#include	<math.h>
#include	<stdio.h>


int main()
{
	double	rcd, cl, rp ;
	double	clkf ;
	double	clkp ;
	double	cycle ;
	double	sbw, bbw ;
	double	read ;


#if	F_SCANF
	scanf("%f %f %f %f",&clkf,&rcd,&cl,&rp) ;
#else
#if	F_DDR400
	clkf = 200.0e6 ;
	cl = 3.0 ;
#else
	clkf = 167.0e6 ;
	cl = 2.5 ;
#endif
	rcd = 3.0 ;
	rp = 3.0 ;
#endif

	printf("clk=%8.2f rcd=%8.2f cl=%8.2f rp=%8.2f\n",
		clkf,rcd,cl,rp) ;

	clkp = 1.0 / clkf ;

	read = clkp * (rcd + cl) ;

	cycle = clkp * (rcd + floor(cl + 0.1) + rp) ;

	sbw = 1.0 / cycle ;

	printf("read=%12.2e cycle=%12.2e sbw=%12.2e\n",
		read,cycle,sbw) ;

	bbw = 32.0 / (22.0 * clkp) ;
	printf("bbw=%12.2e \n",bbw) ;


#if	F_DDR400
#else
	cycle = 60.0e-9 ;
	sbw = 1.0/cycle ;
	printf("DDR333 BW=%12.2e ck=%4.2f\n",sbw,(cycle/clkp)) ;
#endif

	fclose(stdout) ;

	return 0 ;
}

