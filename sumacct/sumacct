#!/usr/bin/perl -w
#
# usage: sumacct [pacctfile]
#

use strict;
use POSIX;

# Location of default pacct file.
my($g_pacct_file)="/var/adm/pacct";

# Structure definition of a pacct entry.
my($g_pacct_t)="b8 C C C l l L l B16 B16 B16 B16 B16 B16 A8";
my($g_pacct_len)=length(pack($g_pacct_t));

# Hashes to store the records.
my(%g_userhash,%g_userprochash);
my $g_totalproc=0;
my $g_totaltime=0;
my(%g_cmds,%g_usercmds,%g_cmdusers);

&main;

sub main
{
  &readfile();

  &printusers();

  print("\n");

  &printcommands();
}

sub readfile
{
  my($pacct_entry,$ac_uid,$ac_utime,$ac_stime,$ac_comm);
  my($usertime,$systemtime,$username);

  if (@ARGV >= 1){
    $g_pacct_file=$ARGV[0];
  }

  if (!open(PACCT_FILE,$g_pacct_file)){
    print("Couldn't open the file $g_pacct_file: $!.\n");
    exit(0);
  }

  while (read(PACCT_FILE,$pacct_entry,$g_pacct_len)==40){
    ($ac_uid,$ac_utime,$ac_stime,$ac_comm)=
      (unpack($g_pacct_t,$pacct_entry))[4,8,9,14];

    $username=getpwuid($ac_uid);
    if (!defined $username){
      $username="$ac_uid";
    }

    $usertime=&comp_ttofloat($ac_utime)/100;
    $systemtime=&comp_ttofloat($ac_stime)/100;

    $g_totaltime=$g_totaltime+$usertime+$systemtime;
    $g_totalproc=$g_totalproc+1;

    if (!defined $g_userhash{$username}){
      $g_userhash{$username}=$usertime+$systemtime;
      $g_userprochash{$username}=1;
    }
    else{
      $g_userhash{$username}=$g_userhash{$username}+$usertime+$systemtime;
      $g_userprochash{$username}=$g_userprochash{$username}+1;
    }

    if (!defined $g_cmds{$ac_comm}){
      $g_cmds{$ac_comm}=1;
    }
    else{
      $g_cmds{$ac_comm}=$g_cmds{$ac_comm}+1;
    }

    if (!defined $g_cmdusers{$ac_comm}{$username}){
      $g_cmdusers{$ac_comm}{$username}=1;
    }
    else{
      $g_cmdusers{$ac_comm}{$username}=$g_cmdusers{$ac_comm}{$username}+1;
    }

    if (!defined $g_usercmds{$username}{$ac_comm}){
      $g_usercmds{$username}{$ac_comm}=1;
    }
    else{
      $g_usercmds{$username}{$ac_comm}=$g_usercmds{$username}{$ac_comm}+1;
    }
  }
  
  close(PACCT_FILE);
}

sub printusers
{
  my($username);
  my($numprocs,$numcmds,$totaltime);

  $~="PROCFORMAT";

  print("username   procs    cmds       cpu\n");
  print("--------  -------  ------  ----------\n");

  foreach $username (sort keys(%g_userhash)){
    $numprocs=$g_userprochash{$username};
    $numcmds=keys %{$g_usercmds{$username}};
    $totaltime=&getcpuspan($g_userhash{$username});
    
    write;
  }

  print("--------  -------  ------  ----------\n");

  $username="totals:";
  $numprocs=$g_totalproc;
  $numcmds=keys %g_cmds;
  $totaltime=&getcpuspan($g_totaltime);
  write;

  return;

format PROCFORMAT =
@<<<<<<<  @>>>>>>  @>>>>> @>>>>>>>>>>
$username,$numprocs,$numcmds,$totaltime
.
}

sub printcommands
{
  my($command);
  my($numtimes,$numusers,$users);

  $~="CMDFORMAT";

  print("command    procs users  user list\n");
  print("--------  ------ -----  ---------------------------------------\n");

  foreach $command (sort keys(%g_cmds)){
    $numtimes=$g_cmds{$command};
    $numusers=keys %{$g_cmdusers{$command}};
    $users="";
    my($key);
    foreach $key (sort {-($g_cmdusers{$command}{$a} <=> 
			  $g_cmdusers{$command}{$b})} 
		  (keys %{$g_cmdusers{$command}}) ){
      $users=$users . "$key (" . $g_cmdusers{$command}{$key} . ") ";
    }
    
    write;
  }

  return;

format CMDFORMAT =
@<<<<<<<  @>>>>> @>>>>  @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
$command,$numtimes,$numusers,$users
.
}

sub comp_ttofloat
{
  # convert a comp_t type number to a normal floating point represenatation
  # typedef ushort_t comp_t; 
  #   pseudo "floating point representation" 3 bit base-8 exponent in the 
  #   high order bits, and a 13-bit fraction  in the low order bits. 
  my($bitstring)=@_;
  my $value=0;
  my($exponent,$mantissa);

  $exponent="00000".substr($bitstring,0,3);
  $mantissa="000".substr($bitstring,3,13);

  $exponent=unpack("C",pack("B8",$exponent));
  $mantissa=unpack("S",pack("B16",$mantissa));

  $value=$mantissa*(8**$exponent);

  return $value;
}

sub getcpuspan
{
  my($timespanf)=@_;

  my $timespan=int($timespanf);
  my $frac=($timespanf-$timespan)*100;
  my $days=int $timespan/(24*60*60); # number of complete days
  my $day=$timespan-$days*(24*60*60); # last (incomplete) day in seconds
  my $hours=int $day/(60*60); # number of complete hours
  my $hour=$day-$hours*(60*60); # last (incomplete) hour in seconds
  my $mins=int $hour/(60); # number of complete minutes
  my $secs=$hour-($mins*60);

  my $spanstr;
  if ($days){
    $spanstr=sprintf("$days+%02d:%02dd",$hours,$mins);
  }
  elsif ($hours){
    $spanstr=sprintf("%2d:%02d:%02dh",$hours,$mins,$secs);
  }
  elsif ($mins){
    $spanstr=sprintf("%2d:%02d.%02d ",$mins,$secs,$frac);
  }
  else{
    $spanstr=sprintf("%2d.%02d ",$secs,$frac);
  }

  return $spanstr;
}
