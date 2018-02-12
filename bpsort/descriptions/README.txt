BPSORT

This program is a part of the Levo research stuff. This program reads a file
that has branch prediction results in it and sorts them on the number of bits
used for the predictor. It calculates the number of bits for the predictor based
on the parameters in the file and by calling the '_stats()' method of the
associated predictor object.

Synopsis:
$ bpsort <dbfile> ...

