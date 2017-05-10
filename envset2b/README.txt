ENVSET

This program runs another program with a precise environment.

Symopsis:

$ envset [-df <def>] [-xf <xe>] [<other(s)>] <prog> {<arg0>|-} [<argn> ...]]

where:

-df <def>	definition file
-xf <xe>	eXportable Environment file
<other(s)>	any of:
			-nice <n>
<prog>		program executable to execute
<arg0>		first argument to the program

Some standard environment variables are created or retained under
normal circumstances.

