/* readn */



extern int	uc_readn(int,void *,int) ;





int readn(fd,buf,len)
int	fd ;
char	buf[] ;
int	len ;
{


	return uc_readn(fd,buf,len) ;
}



