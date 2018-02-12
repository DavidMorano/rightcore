PCSUUCPD

This program is a relatively standard UUCP login daemon (like the various other
'uucpd' programs floating around). Unfortunately, those other programs were not
coded to be flexible enough to be adapted for use by PCS (sigh)!

To run a stand-alone standing server:
$ pcsuucpd -d

To run it from something like INETD:
$ pcsuucpd

To explicitly specify a configuration file (stand-alone mode):
$ pcsuucpd -d -C ${PCS}/etc/pcsuucpd/engeering.conf

To change the listen port without changing the configutation file (if you even
have one) use:
$ pcsuucpd -d -p 6237

