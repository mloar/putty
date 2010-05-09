#!/usr/bin/perl

# A script to call nmake with the appropriate VER settings.  Note that this
# assumes that the git branch 'upstream' has git-svn-id annotations.
#
# Usage: mkMakefile.pl x86|x64

use POSIX qw(strftime);

my $date = strftime("%Y-%m-%d", gmtime);

open GIT, "git log -1 upstream|" or die "could not run git";
while (<GIT>)
{
  my $value = $_;
  if (grep /git-svn-id: /, $value)
  {
    my @fields = split /@/, $value;
    my $rev = substr ($fields[1], 0, 4);

    open VERSION, "../LATEST.VER" or die "could not open version file";
    my $version = <VERSION>;
    close VERSION;
    chomp $version;

    open MAKEFILE, ">", "Makefile" or die "could not open Makefile for writing";
    print MAKEFILE "VER=/DSNAPSHOT=$date /DSVN_REV=$rev\n";
    print MAKEFILE "!INCLUDE Makefile.vc\n";
    print MAKEFILE "PuTTY.wixobj: PuTTY.wxs ..\\LATEST.VER\n\tcandle /dProcessorArchitecture=$ARGV[0] /dProductVersion=$version.$rev PuTTY.wxs\n\n";
    print MAKEFILE "PuTTY.msi: all PuTTY.wixobj\n\tlight -sval -ext WixUiExtension.dll -cultures:en-us PuTTY.wixobj\n\n";
    print MAKEFILE "reallyclean: clean\n\t-del *.msi\n\t-del *.wixobj\n";
    close MAKEFILE;

    last;
  }
}
close GIT;
