extproc perl -Sx 
#!perl
# fixtpps: fix tpscript document to work with PSUtils

# feed this into perl
eval 'exec perl -S $0 "$@"'
   if $running_under_some_shell;

$nesting = 0;
$header = 1;

while (<>) {
   if (/^%%Page:/ && $nesting == 0) {
      print $_;
      print "save home\n";
      $header = 0;
   } elsif (/^%%BeginDocument/ || /^%%BeginBinary/) {
      print $_;
      $nesting++;
   } elsif (/^%%EndDocument/ || /^%%EndBinary/) {
      print $_;
      $nesting--;
   } elsif (/save home/) {
      s/save home//;
      print $_;
   } elsif (!$header || (! /^save$/ && ! /^home$/)) {
      print $_;
   }
}
