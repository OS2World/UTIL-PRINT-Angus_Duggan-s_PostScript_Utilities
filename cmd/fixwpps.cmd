extproc perl -Sx 
#!perl
# fixwpps: get semi-conforming PostScript out of WordPerfect 5.0 file

# feed this into perl
eval 'exec perl -S $0 "$@"'
   if $running_under_some_shell;

$page = 1;
$nesting = 0;

while (<>) {
   s/([^\/]_t)([0-9]+)/\1 \2/g;	# fix wp 5.0 bug
   if (m!/_[be][dp]! || m!_bp \d+ \d+ roll!) {
      print $_;
   } elsif (/^(.*)(_bp.*)$/) {
      print "$1\n" if $1 ne "";
      print "%%Page: $page $page\n";
      print "$2\n";
      $page++;
      $nesting++;
   } elsif (/_ep$/) {
      print $_;
      $nesting--;
   } elsif (/^(.*)(_ed.*)/) {
      print "$1\n" if $1 ne "";
      print "%%Trailer:\n";
      print "$2\n";
   } else {
      print $_;
   }
}
