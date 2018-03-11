'\" te
.TH pntadm 1M "13 Mar 2001" "SunOS 5.8" "Maintenance Commands"
.SH "NAME"
pntadm \- DHCP network table management utility
.SH "SYNOPSIS"
.PP
\fBpntadm\fR -\fBC\fR  [ -\fBr\fR\ \fIresource\fR ]  [ -\fBp\fR\ \fIpath\fR ]  [ -\fBu\fR\ \fIuninterpreted\fR ]  \fInetwork\fR 
.PP
\fBpntadm\fR -\fBA\fR\ \fIname_IP_address\fR  [ -\fBc\fR\ \fIcomment\fR ]  [ -\fBe\fR\ \fImm/dd/yyyy\fR ]  [ -\fBf\fR\ \fInum\fR | \fIkeywords\fR ]  [  -\fBh\fR\ \fIclient_hostname\fR  ]  [  -\fBi\fR  [ -\fBa\fR ]  \fIclient_ID\fR  ]  [  -\fBm\fR  [ -\fBy\fR ]  \fImacro\fR  ]  [ -\fBs\fR\ \fIserver\fR ]  [ -\fBr\fR\ \fIresource\fR ]  [ -\fBp\fR\ \fIpath\fR ]  [ -\fBu\fR\ \fIuninterpreted\fR ]  \fInetwork\fR 
.PP
\fBpntadm\fR -\fBM\fR\ \fIname_IP_address\fR  [ -\fBc\fR\ \fIcomment\fR ]  [ -\fBe\fR\ \fImm/dd/yyyy\fR ]  [ -\fBf\fR\ \fInum\fR | \fIkeywords\fR ]  [  -\fBh\fR\ \fIclient_hostname\fR  ]  [  -\fBi\fR  [ -\fBa\fR ]  \fIclient\fR  \fI ID\fR  ]  [  -\fBm\fR  [ -\fBy\fR ]  \fImacro\fR  ]  [ -\fBn\fR\ \fInew_client_IP_address\fR ]  [ -\fBs\fR\ \fIserver\fR ]  [ -\fBr\fR\ \fIresource\fR ]  [ -\fBp\fR\ \fIpath\fR ]  [ -\fBu\fR\ uninterpreted ]  \fInetwork\fR 
.PP
\fBpntadm\fR -\fBD\fR\ \fIname_IP_address\fR  [ -\fBy\fR ]  [ -\fBr\fR\ \fIresource\fR ]  [ -\fBp\fR\ \fIpath\fR ]  [ -\fBu\fR\ uninterpreted ]  \fInetwork\fR 
.PP
\fBpntadm\fR -\fBP\fR  [ -\fBv\fR ]  [ -\fBx\fR ]  [ -\fBr\fR\ \fIresource\fR ]  [ -\fBp\fR\ \fIpath\fR ]  [ -\fBu\fR\ uninterpreted ]  \fInetwork\fR 
.PP
\fBpntadm\fR -\fBR\fR  [ -\fBr\fR\ \fIresource\fR ]  [ -\fBp\fR\ \fIpath\fR ]  [ -\fBu\fR\ uninterpreted ]  \fInetwork\fR 
.PP
\fBpntadm\fR -\fBL\fR  [ -\fBr\fR\ \fIresource\fR ]  [ -\fBp\fR\ \fIpath\fR ]  [ -\fBu\fR\ uninterpreted ] 
.PP
\fBpntadm\fR -\fBB\fR  [ -\fBv\fR ]  [ \fIbatchfile\fR ] 
.SH "DESCRIPTION"
.PP
The \fBpntadm\fR command is used to manage the Dynamic Host Configuration
Protocol (\fBDHCP\fR) network tables\&. It is used to add and remove networks under \fBDHCP\fR management, and add, delete, or modify IP address records within network tables, or to view tables\&. For a description of the format of \fBDHCP\fR
network tables, see \fBdhcp_network\fR(4)\&.
.PP
\fBpntadm\fR can be run as root or by other users assigned to the \fBDHCP\fR Management profile\&. See \fBrbac\fR(5) and \fBuser_attr\fR(4)\&.
.PP
If the networks you want to add are subnetted, you need to update the \fBnetmasks\fR(4) table\&. 
.PP
One of the following options (function flags) must be specified with the \fBpntadm\fR command: -\fBA\fR, -\fBB\fR, -\fBC\fR, -\fBD\fR, -\fBL\fR, -\fBM\fR, -\fBP\fR, or-\fBR\fR\&.
.SH "OPTIONS"
.PP
The following options are supported: 
.RS
.IP "-\fBA\fR\fI name_IP_address\fR" 6
Add a client entry with hostname or client IP address, \fIname_IP_address\fR, to the named \fBDHCP\fR network table\&. 
.sp
.RS
The following sub-options are optional:
.RE
.RS
.IP "-\fBc\fR \fIcomment\fR" 6
Comment text\&. The default is \fINULL\fR\&.
.IP "-\fBe\fR \fImm/dd/yyyy\fR" 6
Absolute lease\&. The default is \fB0\fR\&.
.IP "-\fBf\fR \fInum\fR | \fIkeywords\fR" 6
Flag value\&. The default is \fB00\fR\&.
.sp
The flag (-\fBf\fR) option can be specified either as a single number denoting the intended flag value, or as a series of the following keywords, combined using the plus (\fB+\fR) symbol:
.sp
.RS
.IP "\fBDYNAMIC\fR or \fB00\fR" 6
Server manager\&'s assignment\&.
.IP "\fBPERMANENT\fR or \fB01\fR" 6
Lease on entry is permanent\&.
.IP "\fBMANUAL\fR or \fB02\fR" 6
Administrator managed assignment\&.
.IP "\fBUNUSABLE\fR or \fB04\fR" 6
Entry is not valid\&.
.IP "\fBBOOTP\fR or \fB08\fR" 6
Entry reserved for \fBBOOTP\fR clients\&.
.sp
.RE
.RS
For a more detailed description of the flag values, see \fBdhcp_network\fR(4)\&.
.RE
.IP "-\fBh\fR \fIclient_hostname\fR" 6
Client hostname\&. The default is NULL\&.
.sp
.RS
When the -\fBh\fR option is used in this mode, the \fIclient_hostname\fR is added to the hosts table within the resource used for storing host names (files, NIS+ or DNS)\&. The command will fail if this \fIclient_hostname\fR is already present in
the hosts table\&.
.RE
.IP "-\fBi\fR \fIclient_ID\fR [-\fBa\fR]" 6
Client identifier [-\fBa\fR]\&. The default is \fB00\fR\&.
.sp
.RS
The -\fBi\fR option modified with -\fBa\fR specifies that the client identifier is in \fBASCII\fR format, and thus needs to be converted to hexadecimal format before insertion into the table\&.
.RE
.IP "-\fBm\fR \fImacro\fR [-\fBy\fR]" 6
Macro name\&. Default is UNKNOWN\&.
.sp
.RS
The -\fBm\fR option modified with -\fBy\fR verifies the existence of the named macro in the  \fBdhcptab\fR table before adding the entry\&.
.RE
.IP "-\fBs\fR \fIserver\fR" 6
Server IP or name\&. Default is system name (\fBuname\fR -\fBn\fR)\&.
.sp
.RE
.IP "-\fBB\fR" 6
Activate batch mode\&. \fBpntadm\fR will read from the specified file or from standard input a series of \fBpntadm\fR commands and execute them within the same process\&. Processing many \fBpntadm\fR commands
using this method is much faster than running an executable batchfile itself\&. Batch mode is recommended for using \fBpntadm\fR in scripts\&. 
.sp
.RS
The following sub-option is optional:
.RE
.RS
.IP "-\fBv\fR" 6
Display commands to standard output as they are processed\&. 
.sp
.RE
.IP "-\fBC\fR" 6
Create the \fBDHCP\fR network table for the network specified by \fInetwork\fR\&. See \fIOPERANDS\fR\&. For details, see \fBdhcp_network\fR(4) and \fBnetworks\fR(4)\&.
.IP "-\fBD\fR \fI name_IP_address\fR" 6
Delete the specified client entry with hostname or client IP address, \fIname_IP_address\fR, in the named \fBDHCP\fR network table\&. (See \fBdhcp_network\fR(4)\&.) 
.sp
.RS
The following sub-option is optional:
.RE
.RS
.IP "-\fBy\fR" 6
Remove associated host table entry\&. The -\fBy\fR option requests that all hostnames associated with the \fBIP\fR address in the hosts table in the resource be removed\&.
.sp
.RE
.IP "-\fBL\fR" 6
List the \fBDHCP\fR network tables presently configured, one per line, on standard output\&. If none are found, no output is printed and an exit status of \fB0\fR is returned\&.
.IP "-\fBM\fR \fI name_IP_address\fR" 6
Modify the specified client entry with hostname or client IP address, \fIname_IP_address\fR, in the named \fBDHCP\fR network table\&. See \fBdhcp_network\fR(4)\&. The default for the sub-options is what they currently are set to\&. 
.sp
The following sub-options are optional\&.
.sp
.RS
.IP "-\fBc\fR \fIcomment\fR" 6
New comment text\&.
.IP "-\fBe\fR \fImm/dd/yy\fR" 6
New absolute lease expiration date\&. Time defaults to 12:00 AM of the day specified\&.
.IP "-\fBf\fR \fInum\fR | \fIkeyboard\fR" 6
New flag value, see explanation  following the description of the -\fBA\fR option\&. 
.IP "-\fBh\fR \fIhost_name\fR" 6
New client hostname\&.
.sp
.RS
The -\fBh\fR option allows you to change the current \fIhostname\fR associated with the \fBIP\fR address or to add a new \fIhostname\fR to the hosts table if an entry associated with this \fBIP\fR address does not exist\&.
.RE
.IP "-\fBi\fR \fIclient_ID\fR" 6
New client identifier [-\fBa\fR]\&.
.IP "-\fBm\fR \fImacro\fR [-\fBy\fR]" 6
Macro name defined in \fBdhcptab\fR\&.
.IP "-\fBn \fR \fInew_client_IP_address\fR" 6
New \fBIP\fR address\&.
.IP "-\fBs\fR \fIserver\fR" 6
New server \fBIP\fR or name\&.
.sp
.RE
.RS
 For more detailed description of the sub-options and flag values, see \fBdhcp_network\fR(4)\&.
.RE
.IP "-\fBP\fR" 6
Display the named \fBDHCP\fR network table\&. See \fBdhcp_network\fR(4)\&. 
.sp
.RS
The following sub-options are optional:
.RE
.RS
.IP "-\fBv\fR" 6
Display lease time in full verbose format\&.
.IP "-\fBx\fR" 6
Display lease time in raw format\&.
.sp
.RE
.IP "-\fBp\fR\fI path\fR" 6
Override the \fBdhcpsvc\&.conf\fR(4) configuration value for data store
resource path, \fIpath\fR See \fBdhcpsvc\&.conf\fR(4)
.IP "-\fBR\fR" 6
Remove the named DHCP network table\&. See \fBdhcp_network\fR(4)\&.
.IP "-\fBr\fR \fIdata_store_resource\fR" 6
Override the \fB/etc/inet/dhcpsvc\&.conf\fR configuration value for \fBRESOURCE=\fR with the \fIdata_store_resource\fR specified\&. See the \fBdhcpsvc\&.conf\fR(4) man page for more details on resource type, and the \fISolaris DHCP Service Developer\&'s Guide\fR for more information about adding support for
other data stores\&.
.IP "-\fBu\fR uninterpreted" 6
Data which will be ignored by \fBpntadm\fR, but passed to the currently configured public module to be interpreted by the data store\&. This might be used for a database account name or other authentication or authorization
parameters required by a particular data store\&. 
.sp
.RE
.SH "OPERANDS"
.PP
The following operand is supported: 
.IP "\fInetwork\fR" 6
The network address or network name which corresponds to the \fBdhcp network\fR table\&. See \fBdhcp_network\fR(4)\&.
.SH "EXAMPLES"
.PP
\fBExample 1: Creating a Table for the \fB10\&.0\&.0\&.0\fR DHCP Network\fR
.PP
The following command creates a table for the \fB10\&.0\&.0\&.0\fR (subnetted to class C)  \fBDHCP\fR network table\&. Note that if you have an alias for this network in your \fBnetworks\fR(4) table, you can use that value rather than the dotted Internet Address notation\&.
.PP
.nf
example# \fBpntadm -C 10\&.0\&.0\&.0\fR
.fi
.PP
\fBExample 2: Adding an Entry to the \fB10\&.0\&.0\&.0\fR Table\fR
.PP
The following command adds an entry to the \fB10\&.0\&.0\&.0\fR table in the \fBfiles\fR resource in the \fB/var/mydhcp\fR directory:
.PP
.nf
example# \fBpntadm -r SUNWfiles -p /var/mydhcp -A 10\&.0\&.0\&.1 10\&.0\&.0\&.0\fR
.fi
.PP
\fBExample 3: Modifying the \fB10\&.0\&.0\&.1\fR Entry of the \fB10\&.0\&.0\&.0\fR Table\fR
.PP
The following command modifies the \fB10\&.0\&.0\&.1\fR entry of the \fB10\&.0\&.0\&.0\fR table, changing the macro name to \fBGreen\fR, setting the flags field to \fBMANUAL\fR and \fBPERMANENT:\fR
.PP
.nf
example# \fBpntadm -M 10\&.0\&.0\&.1 -m Green -f \&'PERMANENT + MANUAL\&' 10\&.0\&.0\&.0\fR
.fi
.PP
\fBExample 4: Changing the \fB10\&.0\&.0\&.1\fR Entry to \fB10\&.0\&.0\&.2\fR\fR
.PP
The following command changes the \fB10\&.0\&.0\&.1\fR entry to \fB10\&.0\&.0\&.2\fR, making an entry in the \fBhosts\fR(4) table called \fBmyclient\fR:
.PP
.nf
example# \fBpntadm -M 10\&.0\&.0\&.1 -n 10\&.0\&.0\&.2 -h myclient 10\&.0\&.0\&.0\fR
.fi
.PP
\fBExample 5: Setting the Client \fBID\fR as \fBASCII\fR\fR
.PP
The following command sets the client \fBID\fR as \fBASCII\fR \fBaruba\&.foo\&.com\fR for the \fBmyclient\fR entry:
.PP
.nf
example# \fBpntadm -M myclient -i \&'aruba\&.foo\&.com\&' -a 10\&.0\&.0\&.0\fR
.fi
.PP
\fBExample 6: Deleting the \fBmyclient\fREntry from the \fB10\&.0\&.0\&.0\fR Table\fR
.PP
The following command deletes the \fBmyclient\fR (\fB10\&.0\&.0\&.2\fR) entry from the \fB10\&.0\&.0\&.0\fR table:
.PP
.nf
example# \fBpntadm -D myclient 10\&.0\&.0\&.0\fR
.fi
.PP
\fBExample 7: Removing the Named DHCP Network Table \fR
.PP
The following command removes the named \fBDHCP\fR network table in the NIS+ directory specified:
.PP
.nf
example# \fBpntadm -r SUNWnisplus -p Test\&.Nis\&.Plus\&. -R 10\&.0\&.0\&.0\fR
.fi
.PP
\fBExample 8: Listing the Configured DHCP Network Tables\fR
.PP
The following command lists the configured DHCP network tables:
.PP
.nf
example# \fBpntadm -L\fR
\f(CW192\&.168\&.0\&.0
10\&.0\&.0\&.0\fR
.fi
.PP
\fBExample 9: Executing pntadm Commands in Batch Mode\fR
.PP
The following command runs a series of \fBpntadm\fR commands contained in a batch file:
.PP
.nf
example# \fBpntadm -B addclients\fR
.fi
.SH "EXIT STATUS"
.IP "\fB0\fR" 6
Successful completion\&.
.IP "\fB1\fR" 6
Object already exists\&.
.IP "\fB2\fR" 6
Object does not exist\&.
.IP "\fB3\fR" 6
Non-critical error\&.
.IP "\fB4\fR" 6
Critical error\&.
.SH "FILES"
.IP "\fB/etc/inet/dhcpsvc\&.conf\fR" 6
 
.IP "\fB/etc/inet/hosts\fR" 6
 
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
.nr 38 \wSUNWdhcsu
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
.if t .if \n(TW>\n(.li .tm Table at line 283 file Input is too wide - \n(TW units
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
\&\h'|\n(40u'Availability\h'|\n(41u'SUNWdhcsu
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
\fBdhcpconfig\fR(1M), \fBdhcpmgr\fR(1M), \fBdhcp_network\fR(4), , \fBdhcpsvc\&.conf\fR(4), \fBdhcptab\fR(4), \fBhosts\fR(4), \fBnetmasks\fR(4), \fBnetworks\fR(4), \fBuser_attr\fR(4), \fBattributes\fR(5), \fBdhcp\fR(5), \fBdhcp_modules\fR(5), \fBrbac\fR(5)
.PP
\fISolaris DHCP Service Developer\&'s Guide\fR
.PP
\fISystem Administration Guide, Volume 3\fR
.PP
Alexander, S\&., and R\&. Droms, \fIDHCP Options and BOOTP Vendor Extensions\fR, RFC 1533, Lachman Technology, Inc\&., Bucknell University, October 1993\&.
.PP
Droms, R\&., \fIInteroperation Between DHCP and BOOTP\fR, RFC 1534, Bucknell University, October 1993\&.
.PP
Droms, R\&., \fIDynamic Host Configuration Protocol\fR, RFC 1541, Bucknell University, October 1993\&.
.PP
Wimer, W\&., \fIClarifications and Extensions for the Bootstrap Protocol\fR, RFC 1542, Carnegie Mellon University, October 1993\&.
...\" created by instant / solbook-to-man, Thu 08 Mar 2018, 13:59
