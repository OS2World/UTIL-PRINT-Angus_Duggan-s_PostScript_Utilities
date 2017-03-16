extproc perl -Sx 
#!perl
# fixpsditps: fix psdit output for use in psutils

# feed this into perl
eval 'exec perl -S $0 "$@"'
   if $running_under_some_shell;

$nesting = 0;

while (<>) {
   if (/^\/p{pop showpage pagesave restore \/pagesave save def}def$/) {
      print "/p{pop showpage pagesave restore}def\n";
   } elsif (/^%%BeginDocument/ || /^%%BeginBinary/) {
      print $_;
      $nesting++;
   } elsif (/^%%EndDocument/ || /^%%EndBinary/) {
      print $_;
      $nesting--;
   } elsif (/^%%Page:/ && $nesting == 0) {
      print $_;
      print "xi\n";
   } elsif (! /^xi$/) {
      print $_;
   }
}
