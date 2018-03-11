'\" te
.TH dhcpsvc\&.conf 4 "3 Oct 2001" "SunOS 5.8" "File Formats"
.SH "NAME"
dhcpsvc\&.conf \- file containing service configuration parameters for the DHCP service
.SH "DESCRIPTION"
.PP
The \fBdhcpsvc\&.conf\fR file resides in directory \fB/etc/inet\fR and contains parameters for specifying Dynamic Host Configuration Protocol (\fBDHCP\fR) service configuration settings, including the type and location of \fBDHCP\fR
data store used\&. 
.PP
The description of the \fBdhcpsvc\&.conf\fR file in this man page is informational only\&. The preferred method of setting or modifying values within the \fBdhcpsvc\&.conf\fR file is by using \fBdhcpconfig\fR(1M) or the \fBdhcpmgr\fR(1M) utility\&. Do not edit the \fBdhcpsvc\&.conf\fR file\&.
.PP
The \fBdhcpsvc\&.conf\fR file format is \fBASCII\fR; comment lines begin with the crosshatch (\fB#\fR) character\&. Parameters consist of a keyword followed by an equals (\fB=\fR) sign followed by the parameter value, of the form:
.PP
.PP
.nf
\fIKeyword\fR=\fIValue\fR
.fi
.PP
The following \fIKeyword\fR and \fIValue\fR parameters are supported:
.IP "BOOTP_COMPAT" 6
String\&. \fBautomatic\fR or \fBmanual\fR\&. Enables support of \fBBOOTP\fR clients\&. Default is no \fBBOOTP\fR\&. Value selects \fBBOOTP\fR address allocation method\&.  \fBautomatic\fR
to support all BOOTP clients, \fBmanual\fR to support only registered \fBBOOTP\fR clients\&. \fBserver\fR mode only parameter\&. 
.IP "CACHE_TIMEOUT" 6
Integer\&. Number of seconds the server  will cache data from data store\&. Used to improve performance\&. Default is 10 seconds\&. \fBserver\fR mode only parameter\&.
.IP "CONVER" 6
Integer\&. Container version\&. Used by DHCP administrative tools to identify which version of the public module is being used to administer the data store\&. \fBCONVER\fR should \fInot\fR be changed manually\&.
.IP "DAEMON_ENABLED" 6
\fBTRUE\fR/\fBFALSE\fR\&. If \fBTRUE\fR, the DHCP daemon can be run\&. If \fBFALSE\fR, DHCP daemon process will exit immediately if the daemon is started\&. Default is \fBTRUE\fR\&. Generic
parameter\&.
.IP "HOSTS_DOMAIN" 6
String\&. Defines name service domain that DHCP administration tools use when managing the hosts table\&. Valid only when \fBHOSTS_RESOURCE\fR is set to \fBnisplus\fR or \fBdns\fR\&.
.IP "HOSTS_RESOURCE" 6
String\&. Defines what name service resource should be used by the DHCP administration tools when managing the hosts table\&. Current valid values are \fBfiles\fR, \fBnisplus\fR, and \fBdns\fR\&.
.IP "ICMP_VERIFY" 6
\fBTRUE\fR/\fBFALSE\fR\&. Toggles \fBICMP\fR echo verification of IP  addresses\&. Default is \fBTRUE\fR\&. \fBserver\fR mode only parameter\&.
.IP "INTERFACES" 6
String\&. Comma-separated list of interface names to listen to\&. Generic parameter\&.
.IP "LOGGING_FACILITY" 6
Integer\&. Local facility number (\fB0\fR-\fB7\fR inclusive) to log \fBDHCP\fR events to\&. Default is not to log transactions\&. Generic parameter\&.
.IP "OFFER_CACHE_TIMEOUT" 6
Integer\&. Number of seconds before \fBOFFER\fR cache timeouts occur\&. Default is \fB10\fR  seconds\&. \fBserver\fR mode only parameter\&.  
.IP "PATH" 6
Path to DHCP data tables within the data store specified by the RESOURCE parameter\&. The value of the \fBPATH\fR keyword is specific  to the RESOURCE\&.
.IP "RELAY_DESTINATIONS" 6
String\&. Comma-separated list of host names and/or \fBIP\fR addresses of relay destinations\&. \fBrelay\fR mode only parameter\&.
.IP "RELAY_HOPS " 6
Integer\&. Max number of \fBBOOTP\fR relay hops before packet is dropped\&. Default is \fB4\fR\&. Generic parameter\&.
.IP "RESCAN_INTERVAL" 6
Integer\&. Number of minutes between automatic \fBdhcptab\fR rescans\&. Default is not to do rescans\&. \fBserver\fR mode only parameter\&. 
.IP "RESOURCE" 6
Data store resource used\&. Use this parameter to name the public module\&. See the \fBPATH\fR keyword in \fBdhcp_modules\fR(5)\&.
.IP "RESOURCE_CONFIG" 6
String\&. This might be used for a database account name or other authentication or authorization parameters required by a particular data store\&. \fBdhcp_modules\fR(5)\&.
.sp
.RS
Providers can use the \fBRESOURCE_CONFIG\fR known as \fBconfigure\fR by specifying an optional service provider layer API function:
.nf
.sp
int configure(const char *configp); 
.fi
.sp
 If this function is defined by the public module provider, it is called during module load time by the private layer, with the contents of the \fBRESOURCE_CONFIG\fR string acquired by the administrative interface (in the case of the \fBdhcpmgr\fR, through the use of a public module-specific java bean extending the \fBdhcpmgr\fR to provide a configuration dialog for this information\&.
.RE
.IP "RUN_MODE" 6
\fBserver\fR or \fBrelay\fR\&. Selects daemon run mode\&. Default is \fBserver\fR\&.
.IP "SECONDARY_SERVER_TIMEOUT" 6
Integer\&. The number of seconds a secondary server will wait for a primary server to respond                                before responding itself\&. Default is \fB20\fR seconds\&. This is a server                           
      mode only parameter\&.
.IP "UPDATE_TIMEOUT" 6
Integer\&. Number of minutes to wait for a response from the DNS server before timing out\&. If this parameter is present, the DHCP daemon will update DNS on behalf of DHCP clients, and will wait the number of seconds specified for a response before
timing out\&. You can use \fBUPDATE_TIMEOUT\fR without specifying a number to enable DNS updates with the default timeout of 15 minutes\&. If this parameter is not present, the DHCP daemon will not update DNS for DHCP clients\&. 
.IP "VERBOSE" 6
\fBTRUE\fR/\fBFALSE\fR\&. Toggles verbose mode, determining amount of status and error messages reported by the daemon\&. Default is \fBFALSE\fR\&. Set to \fBTRUE\fR only for debugging\&. Generic parameter\&.
.SH "SEE ALSO"
.PP
\fBdhcpmgr\fR(1M), \fBin\&.dhcpd\fR(1M), \fBdhcp\fR(5), \fBdhcp_modules\fR(5)
.PP
\fISystem Administration Guide, Volume 3\fR
...\" created by instant / solbook-to-man, Thu 08 Mar 2018, 13:49
