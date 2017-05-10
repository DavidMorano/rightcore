/* main */

/* test the DSTR object */



int main()
{
	DSTR	a ;
	DSTR	b ;


	dstr_start(&a,"here",-1) ;

	dstr_start(&a,NULL,0) ;


	dstr_assign(&b,&a) ;


	dstr_finish(&a) ;

	dstr_finish(&b) ;

	return 0 ;
}



