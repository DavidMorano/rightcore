

int main()
{
	VSSEM	sem ;

	VSCS	vsb ;




	vs_seminit(&sem) ;


	while (vs_semread(&sem) != SR_OUT) ;



	vs_reader(ifd,opts,&sem,&csb,NULL,NULL,buf,buflen,0,0,0,0) ;



	while (vsb.rs == SR_OUT) ;


	vs_wait(&sem) ;

	vs_waitany(&sem,&sem2,NULL) ;



}




