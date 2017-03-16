#!@PERL@
# fixfmps: get conforming PostScript out of FrameMaker version 2 file
# move all FMDEFINEFONTs to start of pages

# feed this into perl
eval 'exec perl -S $0 "$@"'
   if $running_under_some_shell;

%fonts=();

while (<>) {
   if (/^([0-9]+) [0-9]+ .* FMDEFINEFONT$/) {
      $fonts{$1} = $_;
   } elsif (/^[0-9.]+ [0-9.]+ [0-9]+ FMBEGINPAGE$/) {
      print $_, join('',values(%fonts));
   } else {
      print $_;
   }
}
