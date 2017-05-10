BEGIN		{ f = 0 }
/{*}/		{
		   if ( $0 == "{reposname}" ) {
			f = 1 
		   } else
			f = 0
		}
		{
		   if (f) {
			sub("enabled=1","enabled=0")
		   } 
			print $0
		}
