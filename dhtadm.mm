'\" te
.TH dhtadm 1M "13 Mar 2001" "SunOS 5.8" "Maintenance Commands"
.SH "NAME"
dhtadm \- DHCP configuration table management utility
.SH "SYNOPSIS"
.PP
\fBdhtadm\fR -\fBC\fR  [ -\fBr\fR\ \fIresource\fR ]  [ -\fBp\fR\ \fIpath\fR ]  [ -\fBu\fR\ \fIuninterpreted\fR ] 
.PP
\fBdhtadm\fR -\fBA\fR  -\fBs\fR\ \fIsymbol_name\fR  -\fBd\fR\ \fIdefinition\fR  [ -\fBr\fR\ \fIresource\fR ]  [ -\fBp\fR\ \fIpath\fR ]  [ -\fBu\fR\ \fIuninterpreted\fR ] 
.PP
\fBdhtadm\fR -\fBA\fR  -\fBm\fR\ \fImacro_name\fR  -\fBd\fR\ \fIdefinition\fR  [ -\fBr\fR\ \fIresource\fR ]  [ -\fBp\fR\ \fIpath\fR ]  [ -\fBu\fR\ \fIuninterpreted\fR ] 
.PP
\fBdhtadm\fR -\fBM\fR  -\fBs\fR\ \fIsymbol_name\fR  -\fBd\fR\ \fIdefinition\fR  [ -\fBr\fR\ \fIresource\fR ]  [ -\fBp\fR\ \fIpath\fR ]  [ -\fBu\fR\ \fIuninterpreted\fR ] 
.PP
\fBdhtadm\fR -\fBM\fR  -\fBs\fR\ \fIsymbol_name\fR  -\fBn\fR\ \fInew_name\fR  [ -\fBr\fR\ \fIresource\fR ]  [ -\fBp\fR\ \fIpath\fR ]  [ -\fBu\fR\ \fIuninterpreted\fR ] 
.PP
\fBdhtadm\fR -\fBM\fR  -\fBm\fR\ \fImacro_name\fR  -\fBn\fR\ \fInew_name\fR  [ -\fBr\fR\ \fIresource\fR ]  [ -\fBp\fR\ \fIpath\fR ]  [ -\fBu\fR\ \fIuninterpreted\fR ] 
.PP
\fBdhtadm\fR -\fBM\fR  -\fBm\fR\ \fImacro_name\fR  -\fBd\fR\ \fIdefinition\fR  [ -\fBr\fR\ \fIresource\fR ]  [ -\fBp\fR\ \fIpath\fR ]  [ -\fBu\fR\ \fIuninterpreted\fR ] 
.PP
\fBdhtadm\fR -\fBM\fR  -\fBm\fR\ \fImacro_name\fR  -\fBe\fR\ \fIsymbol=value\fR  [ -\fBr\fR\ \fIresource\fR ]  [ -\fBp\fR\ \fIpath\fR ]  [ -\fBu\fR\ \fIuninterpreted\fR ] 
.PP
\fBdhtadm\fR -\fBD\fR  -\fBs\fR\ \fIsymbol_name\fR  [ -\fBr\fR\ \fIresource\fR ]  [ -\fBp\fR\ \fIpath\fR ]  [ -\fBu\fR\ \fIuninterpreted\fR ] 
.PP
\fBdhtadm\fR -\fBD\fR  -\fBm\fR\ \fImacro_name\fR  [ -\fBr\fR\ \fIresource\fR ]  [ -\fBp\fR\ \fIpath\fR ]  [ -\fBu\fR\ \fIuninterpreted\fR ] 
.PP
\fBdhtadm\fR -\fBP\fR  [ -\fBr\fR\ \fIresource\fR ]  [ -\fBp\fR\ \fIpath\fR ]  [ -\fBu\fR\ \fIuninterpreted\fR ] 
.PP
\fBdhtadm\fR -\fBR\fR  [ -\fBr\fR\ \fIresource\fR ]  [ -\fBp\fR\ \fIpath\fR ]  [ -\fBu\fR\ \fIuninterpreted\fR ] 
.PP
\fBdhtadm\fR -\fBB\fR  [ -\fBv\fR ]  [ \fIbatchfile\fR ] 
.SH "DESCRIPTION"
.PP
\fBdhtadm\fR manages the Dynamic Host Configuration Protocol (\fBDHCP\fR) service configuration table, \fBdhcptab\fR\&. You can use it to add, delete, or modify \fBDHCP\fR configuration macros or options
or view the table\&. For a description of the table format, see \fBdhcptab\fR(4)\&.)
.PP
The \fBdhtadm\fR command can be run by root, or by other users assigned to the DHCP Management profile\&. See \fBrbac\fR(5) and \fBuser_attr\fR(4)\&.
.PP
After you make changes with \fBdhtadm\fR, you should issue a \fBSIGHUP\fR to the DHCP server, causing it to read the \fBdhcptab\fR and pick up the changes\&. Do this using the command using the \fBpkill -HUP in\&.dhcpd\fR command\&. See \fBin\&.dhcpd\fR(1M)\&.
.SH "OPTIONS"
.PP
One of the following function flags must be specified with the \fBdhtadm\fR command: -\fBA\fR, -\fBB\fR, -\fBC\fR, -\fBD\fR, -\fBM\fR, -\fBP\fR or -\fBR\fR\&.
.br 
.PP
The following options are supported:
.IP "-\fBA\fR " 6
Add a symbol or macro definition to the \fBdhcptab\fR table\&. 
.sp
.RS
The following sub-options are required:
.RE
.RS
.IP "-\fBd\fR \fIdefinition\fR" 6
Specify a macro or symbol definition\&.
.sp
.RS
\fIdefinition\fR must be enclosed in single quotation marks\&. For macros, use the form -\fBd\fR \fB\&'\fIsymbol\fR=\fIvalue\fR:\fIsymbol\fR=\fIvalue\fR:\&'\fR\&. For symbols,
the definition is a series of fields that define a symbol\&'s characteristics\&. The fields are separated by commas\&. Use the form -\fBd\fR \fB\&'\fIcontext\fR,\fIcode\fR,\fItype\fR,\fIgranularity\fR,\fImaximum\fR\&'\fR\&. See \fBdhcptab\fR(4) for information about these fields\&.
.RE
.IP "-\fBm\fR \fImacro_name\fR" 6
Specify the name of the macro to be added\&. 
.sp
.RS
The -\fBd\fR option must be used with the -\fBm\fR option\&. The -\fBs\fR option cannot be used with the -\fBm\fR option\&.
.RE
.IP "-\fBs\fR \fIsymbol_name\fR" 6
Specify the name of the symbol to be added\&.
.sp
.RS
The -\fBd\fR option must be used with the -\fBs\fR option\&. The -\fBm\fR option cannot be used with the -\fBs\fR option\&.
.RE
.sp
.RE
.IP "-\fBB\fR" 6
Batch process \fBdhtadm\fR commands\&. \fBdhtadm\fR will read from the specified file or from standard input a series of \fBdhtadm\fR commands and execute them within the same process\&. Processing many \fBdhtadm\fR commands using this method is much faster than running an executable batchfile itself\&. Batch mode is recommended for using \fBdhtadm\fR in scripts\&. 
.sp
.RS
The following sub-option is optional:
.RE
.RS
.IP "-\fBv\fR" 6
Display commands to standard output as they are processed\&.
.sp
.RE
.IP "-\fBC\fR " 6
Create the \fBDHCP\fR service configuration table, \fBdhcptab\fR\&. 
.IP "-\fBD\fR " 6
Delete a symbol or macro definition\&.
.sp
.RS
The following sub-options are required:
.RE
.RS
.IP "-\fBm\fR \fImacro_name\fR" 6
Delete the specified macro\&.
.IP "-\fBs\fR \fIsymbol_name\fR" 6
Delete the specified symbol\&.
.sp
.RE
.IP "-\fBM\fR " 6
Modify an existing symbol or macro definition\&.
.sp
.RS
The following sub-options are required: 
.RE
.RS
.IP "-\fBd\fR \fIdefinition\fR" 6
Specify a macro or symbol definition to modify\&. 
.sp
.RS
The definition must be enclosed in single quotation marks\&. For macros, use the form -\fBd\fR \fB\&'\fIsymbol\fR=\fIvalue\fR:\fIsymbol\fR=\fIvalue\fR:\&'\fR\&. For symbols, the definition is a
series of fields that define a symbol\&'s characteristics\&. The fields are separated by commas\&. Use the form -\fBd\fR \fB\&'\fIcontext\fR,\fIcode\fR,\fItype\fR,\fIgranularity\fR,\fImaximum\fR\&'\fR\&. See \fBdhcptab\fR(4) for information about these fields\&.
.RE
.IP "-\fBe\fR" 6
This sub-option uses the \fIsymbol\fR \fB=\fR\fIvalue\fR argument\&. Use it to edit a \fIsymbol\fR/\fIvalue\fR pair within a macro\&. To add a symbol
which does not have an associate value, enter: 
.sp
.nf
.sp
\fIsymbol\fR\fB=NULL_VALUE_\fR
.fi
.sp
.sp
To delete a symbol definition from a macro, enter: 
.sp
.nf
.sp
\fIsymbol\fR\fB=\fR
.fi
.sp
.sp
.IP "-\fBm\fR" 6
This sub-option uses the \fImacro_name\fR argument\&. The -\fBn\fR, -\fBd\fR, or -\fBe\fR sub-options are legal companions for this sub-option\&.\&.
.IP "-\fBn\fR" 6
This sub-option uses the \fInew_name\fR argument and modifies the name of the object specified by the -\fBm\fR or -\fBs\fR sub-option\&. It is not limited to macros\&. \&. Use it to specify a new macro name
or symbol name\&.
.IP "-\fBs\fR" 6
This sub-option uses the \fIsymbol_name\fR argument\&. Use it to specify a symbol\&. The -\fBd\fR sub-option is a legal companion\&.
.sp
.RE
.IP "-\fBp\fR\fI path\fR " 6
Override the \fBdhcpsvc\&.conf\fR(4) configuration value for \fBPATH=\fR with \fIpath\fR\&. See \fBdhcpsvc\&.conf\fR(4) for more details regarding \fIpath\fR\&. See \fBdhcp_modules\fR(5) for information regarding data storage modules for the \fBDHCP\fR service\&.
.IP "-\fBP\fR " 6
Print (display) the \fBdhcptab\fR table\&.
.IP "-\fBr\fR\fI data_store_resource\fR " 6
Override the \fBdhcpsvc\&.conf\fR(4) configuration value for \fBRESOURCE=\fR with the \fIdata_store_resource\fR specified\&. See \fBdhcpsvc\&.conf\fR(4) for more details on resource type\&. See\fISolaris DHCP Service Developer\&'s Guide\fR
for more information about adding support for other data stores\&. See \fBdhcp_modules\fR(5) for information regarding data storage modules for the \fBDHCP\fR
service\&.
.IP "-\fBR\fR " 6
Remove the \fBdhcptab\fR table\&.
.IP "-\fBu\fR\fI uninterpreted\fR" 6
Data which will be ignored by \fBdhtadm\fR, but passed to currently configured public module, to be interpreted by the data store\&. This might be used for a database account name or other
authentication or authorization parameters required by a particular data store\&. Uninterpreted data is stored within \fBRESOURCE_CONFIG\fR keyword of \fBdhcpsvc\&.conf\fR(4)\&.
See \fBdhcp_modules\fR(5) for information regarding data storage modules for the \fBDHCP\fR service\&.
.SH "EXAMPLES"
.PP
\fBExample 1: Creating the \fBDHCP\fR Service Configuration Table\fR
.PP
The following command creates the \fBDHCP\fR service configuration table, \fBdhcptab\fR: 
.PP
.nf
# dhtadm -\fBC\fR
.fi
.PP
\fBExample 2: Adding a Symbol Definition\fR
.PP
The following command adds a \fBVendor\fR option symbol definition for a new symbol called \fBMySym\fR to the \fBdhcptab\fR table in the \fBSUNWfiles\fR resource in the \fB/var/mydhcp\fR directory:
.PP
.nf
# dhtadm -\fBA\fR -\fBs\fR MySym 
     -\fBd\fR \&'Vendor=SUNW\&.PCW\&.LAN,20,IP,1,0\&'
		  -\fBr\fR SUNWfiles -\fBp\fR /var/mydhcp
.fi
.PP
\fBExample 3: Adding a Macro Definition\fR
.PP
The following command adds the \fBaruba\fR macro definition to the \fBdhcptab\fR table\&. Note that symbol/value pairs are bracketed with colons (\fB:\fR)\&. 
.PP
.nf
# dhtadm -\fBA\fR -\fBm\fR aruba 
     -\fBd\fR \&':Timeserv=10\&.0\&.0\&.10 10\&.0\&.0\&.11:DNSserv=10\&.0\&.0\&.1:\&'
.fi
.PP
\fBExample 4: Modifying a Macro Definition\fR
.PP
The following command modifies the \fBLocale\fR macro definition, setting the value of the \fBUTCOffst\fR symbol to 18000 seconds\&. Note that any macro definition which includes the definition of the \fBLocale\fR macro will inherit this change\&.
.PP
.nf
# dhtadm -\fBM\fR -\fBm\fR Locale -\fBe\fR \&'UTCOffst=18000\&'
.fi
.PP
\fBExample 5: Deleting a Symbol\fR
.PP
The following command deletes the \fBTimeserv\fR symbol from the \fBaruba\fR macro\&. Note that any macro definition which includes the definition of the \fBaruba\fR macro will inherit this change\&.
.PP
.nf
# dhtadm -\fBM\fR -\fBm\fR aruba -\fBe\fR \&'Timeserv=\&'
.fi
.PP
\fBExample 6: Adding a Symbol to a Macro\fR
.PP
The following command adds the \fBHostname\fR symbol to the \fBaruba\fR macro\&. Note that the \fBHostname\fR symbol takes no value, and thus requires the special value \fB_NULL_VALUE_\fR\&. Note also that any macro definition which includes the definition
of the \fBaruba\fR macro will inherit this change\&.
.PP
.nf
# dhtadm -\fBM\fR -\fBm\fR aruba -\fBe\fR \&'Hostname=_NULL_VALUE_\&'
.fi
.PP
\fBExample 7: Renaming a Macro\fR
.PP
The following command renames the \fBLocale\fR macro to \fBMyLocale\fR\&. Note that any \fBInclude\fR statements in macro definitions which include the \fBLocale\fR macro will also need to be changed\&.
.PP
.nf
# dhtadm -\fBM\fR -\fBm\fR Locale -\fBn\fR MyLocale
.fi
.PP
\fBExample 8: Deleting a Symbol Definition\fR
.PP
The following command deletes the \fBMySym\fR symbol definition\&. Note that any macro definitions which use \fBMySym\fR will need to be modified\&.
.PP
.nf
# dhtadm -\fBD\fR -\fBs\fR MySym
.fi
.PP
\fBExample 9: Removing a dhcptab\fR
.PP
The following command removes the \fBdhcptab\fR table in the NIS+ directory specified\&.
.PP
.nf
# dhtadm -\fBR\fR -\fBr\fR SUNWnisplus -\fBp\fR Test\&.Nis\&.Plus\&.
.fi
.PP
\fBExample 10: Printing a dhcptab\fR
.PP
The following command prints to standard output the contents of the \fBdhcptab\fR that is located in the data store and path indicated in the \fBdhcpsvc\&.conf\fR file:\&.
.PP
.nf
# dhtadm -\fBP\fR 
.fi
.PP
\fBExample 11: Executing dhtadm in Batch Mode\fR
.PP
The following command runs a series of \fBdhtadm\fR commands contained in a batch file:
.PP
.nf
# \fBdhtadm -B addmacros\fR
.fi
.SH "EXIT STATUS"
.IP "\fB0\fR " 6
Successful completion\&.
.IP "\fB1\fR " 6
Object already exists\&.
.IP "\fB2\fR " 6
Object does not exist\&.
.IP "\fB3\fR " 6
Non-critical error\&.
.IP "\fB4\fR " 6
Critical error\&.
.SH "FILES"
.IP "\fB/etc/inet/dhcpsvc\&.conf\fR " 6
 
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
.nr 38 \w\fBATTRIBUTE TYPE\fR
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
.nr 38 \w\fBATTRIBUTE VALUE\fR
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
.if t .if \n(TW>\n(.li .tm Table at line 275 file Input is too wide - \n(TW units
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
\&\h'|\n(40u'\fBATTRIBUTE TYPE\fR\h'|\n(41u'\fBATTRIBUTE VALUE\fR
.br 
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
.if \n-(b.=0 .nr c. \n(.c-\n(d.-9
.sp
.SH "SEE ALSO"
.PP
\fBdhcpconfig\fR(1M), \fBdhcpmgr\fR(1M), \fBin\&.dhcpd\fR(1M), \fBdhcpsvc\&.conf\fR(4), \fBdhcp_network\fR(4), \fBdhcptab\fR(4), \fBhosts\fR(4), \fBuser_attr\fR(4), \fBattributes\fR(5), \fBdhcp\fR(5), \fBdhcp_modules\fR(5)\fBrbac\fR(5)
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
...\" created by instant / solbook-to-man, Thu 08 Mar 2018, 13:57
