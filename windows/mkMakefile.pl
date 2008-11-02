#!/usr/bin/perl

# A script to call nmake with the appropriate VER settings.  Note that this
# assumes we are in a mercurial branch where the changesets in inline branch
# 'putty' have log entries of the format '[svn r1234] blah'.
use POSIX qw(strftime);

my $date = strftime("%Y-%m-%d", gmtime);

open HG, "hg log -b putty --limit 1|" or die "could not run hg";
while (<HG>)
{
  my $value = $_;
  if (grep /summary: /, $value)
  {
    my @fields = split / +/, $value;
    my $rev = substr ($fields[2], 1, -1);

    open VERSION, "../LATEST.VER" or die "could not open version file";
    my $version = <VERSION>;
    close VERSION;
    chomp $version;

    open MAKEFILE, ">", "Makefile" or die "could not open Makefile for writing";
    print MAKEFILE "VER=/DSNAPSHOT=$date /DSVN_REV=$rev\n";
    print MAKEFILE "!INCLUDE Makefile.vc\n";
    print MAKEFILE "PuTTY.wixobj: PuTTY.wxs ..\\LATEST.VER\n\tcandle /dProcessorArchitecture=$ARGV[0] /dProductVersion=$version.$rev PuTTY.wxs\n\n";
    print MAKEFILE "PuTTY.msi: all PuTTY.wixobj\n\tlight -sval -ext \$(UTIL)\\WixUiExtension.dll -cultures:en-us PuTTY.wixobj\n\n";
    print MAKEFILE "reallyclean: clean\n\t-del *.msi\n\t-del *.wixobj\n";
    close MAKEFILE;
  }
}
close HG;
