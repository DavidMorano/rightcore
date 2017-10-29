/* main */

/* test the 'str' object */



int main()
{
	str	a = obj_str("here",-1) ;
	str	b = obj_str(NULL,-1) ;


	str_assign(&b,&a) ;


	free_str(&a) ;

	free_str(&b) ;

	return 0 ;
}


