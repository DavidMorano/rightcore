void int_winchange()
{
	int	olderrno = errno ;
	if_winchange = TRUE ;
	errno = olderrno ;
}
/* end subroutine (int_winchange) */


