'\" te
.TH dhcpinfo 1 "29 Jul 1999" "SunOS 5.8" "User Commands"
.SH "NAME"
dhcpinfo \- display values of parameters received through DHCP
.SH "SYNOPSIS"
.PP
\fBdhcpinfo\fR [  -\fBc\fR  ]  [ -\fBi\fR\ \fIinterface\fR ]  [ -\fBn\fR \fIlimit\fR  ]  \fIcode\fR 
.PP
\fBdhcpinfo\fR [  -\fBc\fR  ]  [ -\fBi\fR\ \fIinterface\fR ]  [ -\fBn\fR \fIlimit\fR  ]  \fIidentifier\fR 
.SH "DESCRIPTION"
.PP
The \fBdhcpinfo\fR utility prints the \fBDHCP\fR-supplied value(s) of the parameter requested on the command line\&. The parameter may be identified either by its numeric code in the \fBDHCP\fR specification, or by its mnemonic
identifier, as listed in \fBdhcp_inittab\fR(4)\&. This command is intended to be used in command substitutions in the shell scripts invoked by \fBinit\fR(1M) at system boot\&. It first contacts the \fBDHCP\fR client daemon \fBdhcpagent\fR(1M) to verify that \fBDHCP\fR has successfully completed on the requested interface\&. If \fBDHCP\fR has successfully completed on the requested interface, \fBdhcpinfo\fR retrieves
the values for the requested parameter\&. Parameter values echoed by \fBdhcpinfo\fR should not be used without checking its exit status\&. See \fIEXIT STATUS\fR\&.
.PP
See \fBdhcp_inittab\fR(4) for the list of mnemonic identifier codes for all \fBDHCP\fR parameters\&. See \fIRFC 2132, DHCP
Options and BOOTP Vendor Extensions\fR for more detail\&.
.SS "Output Format"
.PP
The output from \fBdhcpinfo\fR consists of one or more lines of \fBASCII\fR text; the format of the output depends upon the requested parameter\&. The number of values returned per line and the total number of lines output for a given parameter are determined by the parameter\&'s \fIgranularity\fR and \fImaximum\fR values, respectively, as defined by \fBdhcp_inittab\fR(4)\&.
.PP
The format of each individual value is determined by the data type of the option, as determined by \fBdhcp_inittab\fR(4)\&. The possible data types and their
formats are listed below: 
.sp
.TS
.if \n+(b.=1 .nr d. \n(.c-\n(c.-1
.de 35
.ps \n(.s
.vs \n(.vu
.in \n(.iu
.if \n(.u .fi
.if \n(.j .ad
.if \n(.j=0 .na
..
.nf
.nr #~ 0
.if n .nr #~ 0.6n
.ds #d .d
.if \(ts\n(.z\(ts\(ts .ds #d nl
.fc
.nr 33 \n(.s
.rm 80 81 82
.nr 34 \n(.lu
.eo
.am 82
.br
.di a+
.35
.ft \n(.f
.ll 2.000000in
.if \n(.l<\n(82 .ll \n(82u
.in 0
\fBUNUMBER8\fR, \fBUNUMBER16\fR, \fBUNUMBER32\fR, \fBUNUMBER64\fR
.br
.di
.nr a| \n(dn
.nr a- \n(dl
..
.ec \
.eo
.am 81
.br
.di b+
.35
.ft \n(.f
.ll 2.097222in
.if \n(.l<\n(81 .ll \n(81u
.in 0
One or more decimal digits, optionally preceded by a minus sign
.br
.di
.nr b| \n(dn
.nr b- \n(dl
..
.ec \
.eo
.am 82
.br
.di c+
.35
.ft \n(.f
.ll 2.000000in
.if \n(.l<\n(82 .ll \n(82u
.in 0
\fBSNUMBER8\fR, \fBSNUMBER16\fR, \fBSNUMBER32\fR, \fBSNUMBER64\fR
.br
.di
.nr c| \n(dn
.nr c- \n(dl
..
.ec \
.eo
.am 81
.br
.di d+
.35
.ft \n(.f
.ll 2.097222in
.if \n(.l<\n(81 .ll \n(81u
.in 0
The string "0x" followed by a two-digit hexadecimal value
.br
.di
.nr d| \n(dn
.nr d- \n(dl
..
.ec \
.35
.nf
.ll \n(34u
.nr 80 0
.nr 38 \wData Type
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \wUnsigned Number
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \wSigned Number
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \w\fBIP\fR Address
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \wOctet
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \wString
.if \n(80<\n(38 .nr 80 \n(38
.80
.rm 80
.nr 38 1.402778in
.if \n(80<\n(38 .nr 80 \n(38
.nr 81 0
.nr 38 \wFormat
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \wOne or more decimal digits
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \wDotted-decimal notation
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \wZero or more \fBASCII\fR characters
.if \n(81<\n(38 .nr 81 \n(38
.81
.rm 81
.nr 38 \n(b-
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \n(d-
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 2.097222in
.if \n(81<\n(38 .nr 81 \n(38
.nr 82 0
.nr 38 \w\fBdhcp_inittab\fR(4) type
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \w\fBIP\fR
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \w\fBOCTET\fR
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \w\fBASCII\fR
.if \n(82<\n(38 .nr 82 \n(38
.82
.rm 82
.nr 38 \n(a-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 \n(c-
.if \n(82<\n(38 .nr 82 \n(38
.nr 38 2.000000in
.if \n(82<\n(38 .nr 82 \n(38
.35
.nf
.ll \n(34u
.nr 38 1n
.nr 79 0
.nr 40 \n(79+(0*\n(38)
.nr 80 +\n(40
.nr 41 \n(80+(3*\n(38)
.nr 81 +\n(41
.nr 42 \n(81+(3*\n(38)
.nr 82 +\n(42
.nr TW \n(82
.if t .if \n(TW>\n(.li .tm Table at line 42 file Input is too wide - \n(TW units
.fc  
.nr #T 0-1
.nr #a 0-1
.eo
.de T#
.ds #d .d
.if \(ts\n(.z\(ts\(ts .ds #d nl
.mk ##
.nr ## -1v
.ls 1
.ls
..
.ec
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'Data Type\h'|\n(41u'Format\h'|\n(42u'\fBdhcp_inittab\fR(4) type
.ne \n(a|u+\n(.Vu
.if (\n(a|+\n(#^-1v)>\n(#- .nr #- +(\n(a|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'Unsigned Number\h'|\n(41u'One or more decimal digits\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.a+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ne \n(b|u+\n(.Vu
.ne \n(c|u+\n(.Vu
.if (\n(b|+\n(#^-1v)>\n(#- .nr #- +(\n(b|+\n(#^-\n(#--1v)
.if (\n(c|+\n(#^-1v)>\n(#- .nr #- +(\n(c|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'Signed Number\h'|\n(41u'\h'|\n(42u'
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(41u
.in +\n(37u
.b+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(##u-1v
.nr 37 \n(42u
.in +\n(37u
.c+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'\fBIP\fR Address\h'|\n(41u'Dotted-decimal notation\h'|\n(42u'\fBIP\fR
.ne \n(d|u+\n(.Vu
.if (\n(d|+\n(#^-1v)>\n(#- .nr #- +(\n(d|+\n(#^-\n(#--1v)
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'Octet\h'|\n(41u'\h'|\n(42u'\fBOCTET\fR
.mk ##
.nr 31 \n(##
.sp |\n(##u-1v
.nr 37 \n(41u
.in +\n(37u
.d+
.in -\n(37u
.mk 32
.if \n(32>\n(31 .nr 31 \n(32
.sp |\n(31u
.ta \n(80u \n(81u \n(82u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'String\h'|\n(41u'Zero or more \fBASCII\fR characters\h'|\n(42u'\fBASCII\fR
.fc
.nr T. 1
.T# 1
.35
.rm a+
.rm b+
.rm c+
.rm d+
.TE
.if \n-(b.=0 .nr c. \n(.c-\n(d.-17
.sp
.SH "OPTIONS"
.PP
The following options are supported:
.IP "-\fBc\fR" 6
Displays the output in a canonical format\&. This format is identical to the \fBOCTET\fR format with a granularity of \fB1\fR\&.
.IP "-\fBi\fR \fIinterface\fR" 6
Specifies the interface to retrieve values for \fBDHCP\fR parameters from\&. If this option is not specified, the primary interface is used\&.
.IP "-\fBn\fR \fIlimit\fR" 6
Limits the list of values displayed to \fIlimit\fR lines\&.
.SH "OPERANDS"
.PP
The following operands are supported:
.RS
.IP "\fIcode\fR" 6
Numeric code for the requested \fBDHCP\fR parameter, as defined by the \fBDHCP\fR specification\&.
Vendor options are  specified by adding 256 to the actual vendor code\&.
.IP "\fIidentifier\fR" 6
Mnemonic symbol for the requested \fBDHCP\fR parameter, as listed in \fBdhcp_inittab\fR(4)\&.
.sp
.RE
.PP
 
.SH "EXIT STATUS"
.PP
The following exit values are returned:
.RS
.IP "\fB0\fR" 6
Successful operation\&.
.IP "\fB2\fR" 6
The operation was not successful\&. The \fBDHCP\fR client daemon may not be running, the interface might have failed to configure, or no satisfactory \fBDHCP\fR responses were received\&.
.IP "\fB3\fR" 6
Bad arguments\&.
.IP "\fB4\fR" 6
The operation timed out\&.
.IP "\fB6\fR" 6
Some system error (should never occur)\&.
.sp
.RE
.SH "ATTRIBUTES"
.PP
See \fBattributes\fR(5) for descriptions of the following attributes:
.sp
.TS
.if \n+(b.=1 .nr d. \n(.c-\n(c.-1
.de 35
.ps \n(.s
.vs \n(.vu
.in \n(.iu
.if \n(.u .fi
.if \n(.j .ad
.if \n(.j=0 .na
..
.nf
.nr #~ 0
.if n .nr #~ 0.6n
.ds #d .d
.if \(ts\n(.z\(ts\(ts .ds #d nl
.fc
.nr 33 \n(.s
.rm 80 81
.nr 80 0
.nr 38 \wATTRIBUTE TYPE
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \wAvailability
.if \n(80<\n(38 .nr 80 \n(38
.nr 38 \wInterface Stability
.if \n(80<\n(38 .nr 80 \n(38
.80
.rm 80
.nr 38 2.750000in
.if \n(80<\n(38 .nr 80 \n(38
.nr 81 0
.nr 38 \wATTRIBUTE VALUE
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \wSUNWcsr
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 \wEvolving
.if \n(81<\n(38 .nr 81 \n(38
.81
.rm 81
.nr 38 2.750000in
.if \n(81<\n(38 .nr 81 \n(38
.nr 38 1n
.nr 79 0
.nr 40 \n(79+(1*\n(38)
.nr 80 +\n(40
.nr 41 \n(80+(3*\n(38)
.nr 81 +\n(41
.nr TW \n(81
.nr TW +1*\n(38
.if t .if \n(TW>\n(.li .tm Table at line 93 file Input is too wide - \n(TW units
.ne 3v+0p
.if n .ne 8v
.fc  
.nr #T 0-1
.nr #a 0-1
.nr #a 0-1
.eo
.de T#
.ds #d .d
.if \(ts\n(.z\(ts\(ts .ds #d nl
.mk ##
.nr ## -1v
.ls 1
.if \n(#T>=0 .nr #a \n(#T
.if \n(T. .vs \n(.vu-\n(.sp
.if \n(T. \h'|0'\s\n(33\l'|\n(TWu\(ul'\s0
.if \n(T. .vs
.if \n(#a>=0 .sp -1
.if \n(#a>=0 \h'|0'\s\n(33\h'-\n(#~u'\L'|\n(#au-1v'\s0\v'\n(\*(#du-\n(#au+1v'\h'|\n(TWu'
.if \n(#a>=0 .sp -1
.if \n(#a>=0 \h'(|\n(41u+|\n(80u)/2u'\s\n(33\h'-\n(#~u'\L'|\n(#au-1v'\s0\v'\n(\*(#du-\n(#au+1v'\h'|\n(TWu'
.if \n(#a>=0 .sp -1
.if \n(#a>=0 \h'|\n(TWu'\s\n(33\h'-\n(#~u'\L'|\n(#au-1v'\s0\v'\n(\*(#du-\n(#au+1v'
.ls
..
.ec
.nr 36 \n(.v
.vs \n(.vu-\n(.sp
\h'|0'\s\n(33\l'|\n(TWu\(ul'\s0
.vs \n(36u
.mk #a
.ta \n(80u \n(81u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'ATTRIBUTE TYPE\h'|\n(41u'ATTRIBUTE VALUE
.nr 36 \n(.v
.vs \n(.vu-\n(.sp
\h'|0'\s\n(33\l'|\n(TWu\(ul'\s0
.vs \n(36u
.ta \n(80u \n(81u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'Availability\h'|\n(41u'SUNWcsr
.nr 36 \n(.v
.vs \n(.vu-\n(.sp
\h'|0'\s\n(33\l'|\n(TWu\(ul'\s0
.vs \n(36u
.ta \n(80u \n(81u 
.nr 31 \n(.f
.nr 35 1m
\&\h'|\n(40u'Interface Stability\h'|\n(41u'Evolving
.fc
.nr T. 1
.T# 1
.35
.nr #a 0
.TE
.if \n-(b.=0 .nr c. \n(.c-\n(d.-7
.sp
.SH "SEE ALSO"
.PP
\fBdhcpagent\fR(1M), \fBifconfig\fR(1M), \fBinit\fR(1M),\fBdhcp_inittab\fR(4),\fBattributes\fR(5)
.PP
Alexander, S\&., and R\&. Droms, \fIRFC 2132, DHCP Options and BOOTP Vendor Extensions\fR,  Silicon Graphics, Inc\&., Bucknell University, March 1997\&.
...\" created by instant / solbook-to-man, Thu 08 Mar 2018, 14:30
