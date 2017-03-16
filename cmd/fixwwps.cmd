extproc perl -Sx 
#!perl
# fixwwps: get semi-conforming PostScript out of Windows Write file

# feed this into perl
eval 'exec perl -S $0 "$@"'
   if $running_under_some_shell;

$page = 1;

while (<>) {
   if (/^(%!.*) EPSF-\d.\d/) {
      print $1, "\n";
   } elsif (/^SS/) {
      print "%%Page: $page $page\n";
      print $_;
      $page++;
   } else {
      print $_;
   }
}
