/* main */


#include	<stdio.h>


/* forward references */

static int output_head(FILE *) ;
static int output_trans(FILE *,double,double,double,double,double,double) ;
static int output_tail(FILE *,double,double,double,double) ;

static char	s7[] = "       " ;




int main()
{
	FILE	*fp = stdout ;

	double	rate = 0.1 ;
	double	deposit = 100.0, loan, reserve ;
	double	t_deposit = 0.0 ;
	double	t_reserve = 0.0 ;
	double	t_loan = 0.0 ;
	double	t_demand = 0.0 ;


	output_head(fp) ;

	while (deposit > 0.1) {

		loan = deposit * (1.0 - rate) ;
		reserve = deposit * rate ;

		t_deposit += deposit ;
		t_reserve += reserve ;
		t_loan += loan ;
		t_demand = t_deposit + t_loan ;

		output_trans(fp,t_deposit,t_reserve,t_loan,t_demand,
			deposit,loan) ;

		deposit = loan ;

	} /* end while */

	output_head(fp) ;

	output_tail(fp,t_deposit,t_reserve,t_loan,t_demand) ;

	fclose(fp) ;

	return 0 ;
}
/* end subroutine */



/* LOCAL SUBROUTINES */



static int output_head(fp)
FILE	*fp ;
{
	int	rs ;


	rs = fprintf(fp," deposit reserve    loan  demand deposit    loan\n") ;

	return rs ;
}

static int output_trans(fp,t_deposit,t_reserve,t_loan,t_demand,deposit,loan)
FILE	*fp ;
double	t_deposit, t_reserve, t_loan, t_demand ;
double	deposit, loan ;
{
	int	rs ;


	rs = fprintf(fp," %7.2f %7.2f %7.2f %7.2f %7.2f %7.2f\n",
		t_deposit,t_reserve,t_loan,t_demand,deposit,loan) ;

	return rs ;
}

static int output_tail(fp,t_deposit,t_reserve,t_loan,t_demand)
FILE	*fp ;
double	t_deposit, t_reserve, t_loan, t_demand ;
{
	int	rs ;


	rs = fprintf(fp," %7.2f %7.2f %7.2f %7.2f\n",
		t_deposit,t_reserve,t_loan,t_demand) ;

	return rs ;
}



