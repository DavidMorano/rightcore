NUMBERS

This command (KSH shell built-in) prints out a specified function of
of given numbers. The govin input nymbers are specified in a certain
form in order to identify the function to be applied to them.

Synopsis:
$ numbers [-w] [<nums(s)> ...] [-af <afile>] [-V]

Arguments:
[-w]		with repitition
<num(s)>	number(s) to operate on:
		    <n>E<k>	n Exponetial k
		    <n>P<k>	n Permuations k
		    <n>PM<k>	n Multi-Permutations k
		    <n>C<k>	n Combinations k
		    <n>CM<k>	n Multi-Combination k
		    <n>F	n Factorial
-af <afile>	take spec(s) from file
-V		print command version to standard-error and then exit
