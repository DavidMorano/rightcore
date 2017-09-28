COOKIESET

This little program (built-in command) typesets cookie files (like FORTUNE
files) into TROFF input language so that the output can be further processed for
printing. See this procedure (here) for use.

The TROFF output language (which uses the MM macros) is created with:
$ cookieset -p 14 -f ZI <cookiefile> > <cookie.troff>

This can then be printed in the usual way with something like:
$ troff -Tpost -mm <cookie.troff> | dpost -x 0.25 -y -0.1 | lp -T postscript

Arguments:
<cookiefile>	soruce cookie file
<cookie.troff>	TROFF output from program

