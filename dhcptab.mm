'\" te
.TH dhcptab 4 "13 Mar 2001" "SunOS 5.8" "File Formats"
.SH "NAME"
dhcptab \- DHCP configuration parameter table
.SH "DESCRIPTION"
.PP
The \fBdhcptab\fR configuration table allows network administrators to organize groups of configuration parameters as macro definitions, which can then be referenced in the definition of other useful macros\&. These macros are then used by the \fBDHCP\fR server to return their
values to \fBDHCP\fR and \fBBOOTP\fR clients\&.
.PP
The preferred method of managing the \fBdhcptab\fR is through the  use of the \fBdhcpmgr\fR(1M) or \fBdhtadm\fR(1M) utility\&. The description of \fBdhcptab\fR entries included in this manual page is intended for informational purposes only, and should not be used to manually edit entries\&. 
.PP
You can view the contents of the \fBdhcptab\fR using the DHCP manager\&'s tabs for Macros and Options, or using the \fBdhtadm -P\fR command\&. 
.SS "Syntax of dhcptab Entries"
.PP
The format of a \fBdhcptab\fR table depends on the data store used to maintain it\&. However, any \fBdhcptab\fR must contain the following fields in each record:
.IP "\fBName\fR" 6
This field identifies the macro or symbol record and is used as a search key into the \fBdhcptab\fR table\&. The name of a macro or symbol must consist of \fBASCII\fR characters, with the length limited to 128 characters\&. Names can include spaces, except at the end of the name\&. The name is not case-sensitive\&.
.IP "\fBType\fR" 6
This field specifies the type of record and is used as a search key into the \fBdhcptab\fR\&. Currently, there are only two legal values for \fBType\fR:
.RS
.IP "\fBm\fR" 6
This record is a \fBDHCP\fR macro definition\&.
.IP "\fBs\fR" 6
This record is a \fBDHCP\fR symbol definition\&. It is used to define vendor and site-specific options\&.
.sp
.RE
.IP "\fBValue\fR" 6
This field contains the value for the specified type of record\&. For the \fBm\fR type, the value will consist of a series of symbol=value pairs, separated by the colon (\fB:\fR) character\&. For the \fBs\fR type, the value will consist of a series of fields, separated by a comma (\fB,\fR), which define a symbol\&'s characteristics\&. Once defined, a symbol can be used in macro definitions\&.
.SS "Symbol Characteristics"
.PP
The Value field of a symbols definition contain the following fields describing the characteristics of a symbol:
.IP "\fBContext\fR" 6
This field defines the context in which the symbol definition is to be used\&. It can have one of the following values:
.RS
.IP "\fBSite\fR" 6
This symbol defines a site-specific option, codes 128-254\&.
.IP "\fBVendor=Client Class \&.\&.\&.\fR" 6
This symbol defines a vendor-specific option, codes 1-254\&. The Vendor context takes \fBASCII\fR string arguments which identify the client class that this vendor option is associated with\&. Multiple
client class names can be specified, separated by white space\&. Only those clients whose client class matches one of these values will see this option\&. For Sun machines, the Vendor client class matches the value returned by the command \fBuname -i\fR on the client, with periods replacing
commas\&.
.sp
.RE
.IP "\fBCode\fR" 6
This field specifies the option code number associated with this symbol\&. Valid values are 128-254 for site-specific options, and 1-254 for vendor-specific options\&.
.IP "\fBType\fR" 6
This field defines the type of data expected as a value for this symbol, and is not case-sensitive\&. Legal values are:
.RS
.IP "\fBASCII\fR" 6
\fBNVT ASCII\fR text\&. Value is enclosed in double-quotes (\fB"\fR)\&. Granularity setting has no effect on symbols of this type,
since \fBASCII\fR strings have a natural granularity of one (1)\&.
.IP "\fBBOOLEAN\fR" 6
No value is associated with this data type\&. Presence of symbols of this type denote boolean \fBTRUE,\fR whereas absence denotes \fBFALSE\&.\fR
Granularity and Miximum values have no meaning for symbols of this type\&.
.IP "\fBIP\fR" 6
Dotted decimal form of an Internet address\&. Multi-IP address granularity is supported\&.
.IP "\fBNUMBER\fR" 6
An unsigned number with a supported granularity of \fB1\fR, \fB2\fR, \fB4\fR, and \fB8\fR octets\&.
.sp
.RS
Valid \fBNUMBER\fR types are: \fBUNUMBER8\fR, \fBSNUMBER8\fR, \fBUNUMBER16\fR, \fBSNUMBER16\fR, \fBUNUMBER32\fR, \fBSNUMBER32\fR, \fBUNUMBER64\fR, and \fBSNUMBER64\fR\&. See \fBdhcp_inittab\fR(4) for details\&. 
.RE
.IP "\fBOCTET\fR" 6
Uninterpreted \fBASCII\fR representation of binary data\&. The client identifier is one example of an \fBOCTET\fR string\&. Valid characters are 0-9, [a-f] [A-F]\&. One \fBASCII\fR character represents one nibble (4 bits), thus two \fBASCII\fR characters are needed to represent an 8 bit quantity\&. The granularity setting has no effect on symbols of this type, since \fBOCTET\fR strings have a natural granularity of one (1)\&.
.sp
.RE
.IP "\fBGranularity\fR" 6
This value specifies how many objects of \fBType\fR define a single \fBinstance\fR of the symbol value\&. For example, the static route option is defined to be a variable list of routes\&. Each route
consists of two \fBIP\fR addresses, so the \fBType\fR is defined to be \fBIP\fR, and the data\&'s granularity is defined to be \fB2\fR \fBIP\fR addresses\&. The granularity field affects the \fBIP\fR and \fBNUMBER\fR data types\&.
.IP "\fBMaximum\fR" 6
This value specifies the maximum items of \fBGranularity\fR which are permissible in a definition using this symbol\&. For example, there can only be one \fBIP\fR address specified for a subnet mask,
so the  \fBMaximum\fR number of items in this case is one (\fB1\fR)\&. A  \fBMaximum\fR value of zero (\fB0\fR) means that a variable number of items is permitted\&.
.PP
The following example defines a site-specific option (symbol) called \fBMystatRt\fR, of code \fB130\fR, type \fBIP,\fR and granularity \fB2\fR, and a \fBMaximum\fR of \fB0\fR\&. This
definition corresponds to the internal definition of the static route option (\fBStaticRt\fR)\&.
.sp
.PP
.nf
\f(CWMystatRt s Site,130,IP,2,0\fR
.fi
.PP
.sp
.SS "Macro Definitions"
.PP
The following example illustrates a macro defined using the \fBMystatRt\fR site option symbol just defined:
.sp
.PP
.nf
\f(CW10netnis m :MystatRt=3\&.0\&.0\&.0 10\&.0\&.0\&.30:\fR
.fi
.PP
.sp
Macros can be specified in the \fBMacro\fR field in \fBDHCP\fR network tables (see \fBdhcp_network\fR(4)), which will bind particular
macro definitions to specific \fBIP addresses\&.\fR
.PP
Up to four macro definitions are consulted by the \fBDHCP\fR server to determine the options that are returned to the requesting client\&.
.PP
These macros are processed in the following order:
.IP "\fBClient Class\fR" 6
A macro named using the \fBASCII\fR representation of the client class (e\&.g\&. \fBSUNW\&.Ultra-30\fR) is searched for in the \fBdhcptab\fR\&.
If found, its symbol/value pairs will be selected for delivery to the client\&. This mechanism permits the network administrator to select configuration parameters to be returned to all clients of the same class\&.
.IP "\fBNetwork\fR" 6
A macro named by the dotted Internet form of the network address of the client\&'s network (for example, \fB10\&.0\&.0\&.0\fR) is searched for in the \fBdhcptab\fR\&. If found, its symbol/value pairs will be combined
with those of the \fBClient Class\fR macro\&. If a symbol exists in both macros, then the \fBNetwork\fR macro value overrides the value defined in the \fBClient Class\fR macro\&. This mechanism permits the network administrator to select configuration parameters to be
returned to all clients on the same network\&.
.IP "\fBIP Address\fR" 6
This macro may be named anything, but must be specified in the \fBDHCP\fR network table for the IP address record assigned to the requesting client\&. If this macro is found in the \fBdhcptab\fR, then its symbol/value pairs will be combined with those of the \fBClient Class\fR macro and the \fBNetwork\fR macro\&. This mechanism permits the network administrator to select configuration parameters to be returned to clients using a particular \fBIP\fR address\&. It can also be used to deliver a macro defined to include "server-specific" information by including this macro definition in all \fBDHCP\fR network table entries owned by a specific server\&.
.IP "\fBClient Identifier\fR" 6
A macro named by the \fBASCII\fR representation of the client\&'s unique identifier as shown in the \fBDHCP\fR network table (see \fBdhcp_network\fR(4))\&. If found, its symbol/value pairs are combined to the sum of the \fBClient Class\fR, \fBNetwork\fR, and \fBIP Address\fR macros\&. Any symbol collisions are replaced with those
specified in the client identifier macro\&. The client mechanism permits the network administrator to select configuration parameters to be returned to a particular client, regardless of what network that client is connected to\&.
.PP
Refer to \fISystem Administration Guide, Volume 3\fR for more information about macro processing\&.
.PP
Refer to the \fBdhcp_inittab\fR(4) man page for more information about symbols used in Solaris DHCP\&.
.SH "SEE ALSO"
.PP
\fBdhcpmgr\fR(1M), \fBdhtadm\fR(1M), \fBin\&.dhcpd\fR(1M), \fBdhcp_inittab\fR(4), \fBdhcp_network\fR(4), \fBdhcp\fR(5)
.PP
\fISystem Administration Guide, Volume 3\fR
.PP
Alexander, S\&., and R\&. Droms, \fIDHCP Options and BOOTP Vendor Extensions\fR, RFC 2132, Silicon Graphics, Inc\&., Bucknell University, March 1997\&.
.PP
Droms, R\&., \fIInteroperation Between DHCP and BOOTP\fR, RFC 1534, Bucknell University, October 1993\&.
.PP
Droms, R\&., \fIDynamic Host Configuration Protocol\fR, RFC 2131, Bucknell University, March 1997\&.
.PP
Wimer, W\&., \fIClarifications and Extensions for the Bootstrap Protocol\fR, RFC 1542, Carnegie Mellon University, October 1993\&.
...\" created by instant / solbook-to-man, Thu 08 Mar 2018, 14:25
