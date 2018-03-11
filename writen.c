/* writen */



extern int	uc_writen(int,void *,int) ;







int writen(fd,buf,len)
int	fd ;
char	buf[] ;
int	len ;
{


	return uc_writen(fd,buf,len) ;
}



