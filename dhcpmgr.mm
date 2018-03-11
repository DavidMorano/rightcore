'\" te
.TH dhcpmgr 1M "13 Mar 2001" "SunOS 5.8" "Maintenance Commands"
.SH "NAME"
dhcpmgr \- graphical interface for managing DHCP service
.SH "SYNOPSIS"
.PP
\fB/usr/sadm/admin/bin/dhcpmgr\fR
.SH "DESCRIPTION"
.PP
\fBdhcpmgr\fR is a graphical user interface which enables you to manage the Dynamic Host Configuration Protocol (\fBDHCP\fR) service on the local system\&. It performs the functions of the \fBdhcpconfig\fR, \fBdhtadm\fR, and \fBpntadm\fR
command line utilities\&. You must be \fBroot\fR to use \fBdhcpmgr\fR\&. The \fBdhcpmgr\fR Help, available from the \fBHelp\fR menu, contains detailed information about using the tool\&.
.SH "USAGE"
.PP
You can perform the following tasks using \fBdhcpmgr\fR: 
.IP "Configure DHCP service" 6
Use \fBdhcpmgr\fR to configure the DHCP daemon as a \fBDHCP\fR server, and select the data store to use for storing network configuration tables\&.\&.
.IP "Configure BOOTP relay service" 6
Use \fBdhcpmgr\fR to configure the DHCP daemon as a \fBBOOTP\fR relay\&.
.IP "Manage DHCP or BOOTP relay service" 6
Use \fBdhcpmgr\fR to start, stop, enable, disable or unconfigure the \fBDHCP\fR service or \fBBOOTP\fR relay service, or change DHCP server parameters\&.
.IP "Manage DHCP addresses" 6
Use \fBdhcpmgr\fR to add, modify, or delete \fBIP\fR addresses leased by the \fBDHCP\fR service\&.
.IP "Manage DHCP macros" 6
Use \fBdhcpmgr\fR to add, modify or delete macros used to supply configuration parameters to \fBDHCP\fR clients\&.
.IP "Manage DHCP options" 6
Use \fBdhcpmgr\fR to add, modify or delete options used to define parameters deliverable through \fBDHCP\fR\&.
.IP "Convert to a new DHCP data store" 6
Use \fBdhcpmgr\fR to configure the DHCP server to use a different data store, and convert the DHCP data to the format used by the new data store\&.
.IP "Move DHCP data to another server" 6
Use \fBdhcpmgr\fR to export data from one Solaris DHCP server and import data onto another Solaris DHCP server\&.
.SH "EXIT STATUS"
.PP
The following exit values are returned:
.IP "\fB0\fR " 6
Successful completion\&. 
.IP "\fBnon-zero\fR " 6
An error occurred\&.
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
.nr 38 \wSUNWdhcm
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
.if t .if \n(TW>\n(.li .tm Table at line 49 file Input is too wide - \n(TW units
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
\&\h'|\n(40u'Availability\h'|\n(41u'SUNWdhcm
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
\fBdhcpconfig\fR(1M), \fBdhtadm\fR(1M), \fBpntadm\fR(1M), \fBin\&.dhcpd\fR(1M), \fBdhcpsvc\&.conf\fR(4), \fBdhcp_network\fR(4), \fBdhcptab\fR(4), \fBattributes\fR(5), \fBdhcp\fR(5), \fBdhcp_modules\fR(5)
.PP
\fISolaris DHCP Service Developer\&'s Guide\fR
.PP
\fISystem Administration Guide, Volume 3\fR
...\" created by instant / solbook-to-man, Thu 08 Mar 2018, 13:54
