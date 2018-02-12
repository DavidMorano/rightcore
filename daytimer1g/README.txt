DAYTIMER

This program displays the time on a terminal screen.  It can optionally
use a status area of the screen.

Synopsis:
$ daytimer [<mailfname>|-] [<offint>] [-t <timeout>] [-o <opt(s)>] 
		[-s] [-r <refresh>] [-V]

Arguments:
<mailfname>	mail file for new-mail check
<offint>	time offset
-t <timeout>	screen blanking timeout
-s		have the display go to a status area
-r <refresh>	set the display refresh interval
-p <piddir>	PID lock directory
-o <options>	where an option is one of:
			offint		time offset

