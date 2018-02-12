SPAMSPLICE

This small program conditionally splices in the SPAMASSASSIN program. If
SPAMASSASSIN has not already been run on the message data, we splice it in to
recieve as standard input the output from our filtering. The output from
SPAMASSASSIN goes to our output. If the SPAMASSASSIN program has already been
run on the message, we just output to standard output ourselves.

