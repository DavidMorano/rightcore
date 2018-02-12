PRCAT

This is a hack to allow for the pagination of the CATMAN files used in the
EXPTOOLS package that pass as manual pages! The manual pages in the EXPTOOLS
package are already formatted and only exist in the stupid formatted form. In
order to print these out they need to be properly paginated with embedded
form-feed characters. The PRCAT program reads these files and adds the form-feed
characters as needed. Details on where the form-feed characters are added is on
the PRCAT manual page.

I wish this program wasn't necessary but the stupid EXPTOOLS people decided that
we do not deserve to have the unformatted source manual pages in the
distribution!

Synopsis:
$ toolan <program> | prcat > <program>.txt

