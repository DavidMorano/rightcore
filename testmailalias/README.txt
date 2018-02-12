TESTMAILALIAS

This program tests the MAILALIAS object.

Synopsis:
$ testmailalias.x -ROOT ${HOME} <profile>

Arguments:
<profile>	some indicator about conducting the database lookups

New objects in this test include:

MAILALIAS
KVSTAB
SVCTAB

The MAILALIAS object actually manages what could be one or more files (mail
alias files) put together. It manages a single MAILALIAS database file that can
be a conglomerate of several ASCII text mail alias files.

The MAILALIAS object will automatically rebuilt the MAILALIAS database as needed
from its constituent ASCII text mail alias files.

