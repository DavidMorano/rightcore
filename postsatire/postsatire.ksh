#!/usr/bin/ksh
# POSTSATIRE


# intercept Postscript output from the troff post-processor 
# and insert a gray "SATIRE" diagonally on every page
# 
sed 's/^%%EndProlog/\/SATIRE { \
	save \/Times-Roman findfont 200 scalefont setfont\
	10 10 scale\
	0 -11 72 mul translate\
	72 3 mul 72 1 mul moveto\
	60 rotate 0.95 setgray (SATIRE) show restore } def\
%%EndProlog/' | sed '/^%%Page:/a\
SATIRE'


