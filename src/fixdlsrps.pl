#!@PERL@
# fixdlsrps: fix DviLaser/PS document to work with PSUtils

# feed this into perl
eval 'exec perl -S $0 "$@"'
   if $running_under_some_shell;

$nesting = 0;
$page = 1;
$infont = 0;

@fonts = ();

while (<>) {
   if (/^XP/) {
      print $_;
      $infont++;
      push(@fonts, $_);
      $infont-- if /PXL.*RP/;
   } elsif ($infont) {
      print $_;
      push(@fonts, $_);
      $infont-- if /PXL.*RP/;
   } elsif (/^%%EndProlog/ && !$nesting) {
      print "\$DviLaser begin/GlobalMode{}bdef/LocalMode{}bdef\
/RES{/Resolution xdef/PxlResolution xdef\
  /RasterScaleFactor PxlResolution Resolution div def\
  InitialMatrix setmatrix 72.0 Resolution div dup scale}bdef\
/DoInitialScaling{}bdef end\n";
      print "\$DviLaser begin /XP {} def /RP {} def end\n";
      print $_;
   } elsif (/^%%BeginPageSetup/ && !$nesting) {
      print "%%Page: $page $page\n";
      $page++;
      print $_;
   } elsif (/^%%EndPageSetup/ && !$nesting) {
      print @fonts;
      print $_;
   } elsif (/^%%BeginDocument/ || /^%%BeginBinary/) {
      print $_;
      $nesting++;
   } elsif (/^%%EndDocument/ || /^%%EndBinary/) {
      print $_;
      $nesting--;
   } elsif (!/^%%PageBoundingBox:/) {
      print $_;
   }
}
