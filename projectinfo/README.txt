PROJECTINFO

This program retrieves information from the UNIX® system PROJECT database.

Synopsis:
$ projectinfo [{<projectname>|<pjid>|-}] [{<query(s)>|-a}] [-q] [-V]

Arguments:
<projectname>	project-name to query
<pjid>		project ID
-		user default project
<query(s)>	query keyword (listed below)
-a		list all users who belong to the project one way or another
-q		do not print anything out (just return exit-code)
-V		print command version to standard-error and then exit

Queries:
pjid		return the project ID
projectname	return the project name
users		return users within the project
groups		return groups within the project
comment		project comment (alternate query 'description')

