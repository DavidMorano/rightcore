THESES (software program)

This little program reads the input file(s) and puts each contiguous
group of lines to the output in what will become a separate text block
when TROFF-formatted.

The original TROFF source (used the 'mm' macros) was created with:

$ theses.x -p 14 -f ZI split.txt > split.troff

This was then edited to create 'split.mm'.  Then that was processed as:

$ troff -Tpost -mm -rL11.5i -rW6.5i split.mm | dpost > split.ps

