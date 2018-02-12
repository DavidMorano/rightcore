LIBTD

This is a terminal display library. This library contains routines (not unlike
in the UNIX 'curses' library) to display information in "windows" on a terminal
display.

Routines:

/* local variables */

struct termdisplay	win, *wp = &win ;
struct td_win		*wp ;


int td_init(wp,tfd,termtype,lines)
struct td_window	*wp ;
int	tfd ;			/* terminal file descriptor */
int	lines ;
char	*termtype ;
{
}


int td_free(wp)


int td_subwindow(wp,row,length)


int td_subwindows(wp)


int td_control(wp,cmd,arg1,arg2)
int	cmd ;

	cmd		description
	-------------------------------------------------------
	TD_CLINES	change the number of lines
	TD_CSCROLL	scroll the window up or down
	TD_CSUBWIN	create a subwindow
	TD_CCURSOR	position the cursor
	TD_CEOL		enter or exit "erase to EOL" mode



int td_printf(wp,win,format,args) ;


int td_pprintf(wp,win,r,c,format,args) ;


EXAMPLES:

	td_printf(wp,win,0,0,"hello to you\v\n") ;

	td_printf(wp,win,-1,-1,"continuation of hello to you\v\n") ;

	td_printf(wp,win,-1,-1,"continuation %{ri} of hello to you\v\n") ;

