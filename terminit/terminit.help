TERMINIT

This command sets or queries configuration values from a terminal device.

Synopsis:
$ terminit [-s] <device> [<req(s)>[=<value>]] [-db <db>] [-V]

Arguments:
<device>	is the device to lookup 
<req(s)>	request keys: term, label
-s		set value for specified key
<value>		value for a key when setting
-db <db>	specified the database file

The default action is to retrieve the value for a specified key. If no key is
present in the database, an empty value is returned.

Example:
Set the speed and the terminal-type for device '/dev/console': 
$ terminit -s /dev/console term=sun label=C38400

