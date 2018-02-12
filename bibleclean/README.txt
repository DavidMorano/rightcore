BIBLECLEAN

This program reads the format of the KJV bible as shipped and processes it into
a format suitable for indexing.

Synopsis:
$ fgrep -v 'subbook=' bible.txt | bibleclean > bibledb.txt

For Spanish:
$ bibleclean biblespanish_dam1.txt > bibledb.txt

