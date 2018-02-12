MKDICTORDER

This set of programs makes a C-language fragment that is an array of short
integers.

This array serves as a sort of translation table for use in finding the
dictionary-collating-ordinal number of a latin character. The array is indexed
by the latin character (an 8-bit clean character) and returns the short integer
representing its dictionary-collating-ordinal number.

If the file 'dictorder.txt' does not already exist, either create it by hand,
or compile and execute the program found in 'dictorder.c' to create a starting
point for further editing.  This file should contain a continuous list of
characters that are ordered in their correct dictionary-order-sequence.  This
file will be read as standard-input of the program found in 'main.c' and the
resulting C-landuage array of shorts will be printed to standard-output.

The program in 'main.c' is "made" by executing 'make'.  This produces
the program named 'mkdictorder'.

