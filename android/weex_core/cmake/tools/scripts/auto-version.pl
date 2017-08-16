#!/usr/bin/perl -w

use strict;
use File::Path qw(make_path);
use File::Spec;
use File::Basename;
use Cwd 'abs_path';
use Win32;

# Copyright (C) 2007, 2009, 2014 Apple Inc.  All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 

sub splitVersion($);

die "You must supply an output path as the argument.\n" if ($#ARGV < 0);

my $rawPath = $0;

if ($^O eq "cygwin") {
    chomp($rawPath = `/usr/bin/cygpath -u "$0"`);
}

my $thisDirectory = dirname(abs_path($rawPath));

my $FALLBACK_VERSION_PATH = File::Spec->catfile($thisDirectory, 'VERSION');
open(FALLBACK_VERSION_FILE, '<', $FALLBACK_VERSION_PATH) or die "Unable to open $FALLBACK_VERSION_PATH: $!";
my $FALLBACK_VERSION = <FALLBACK_VERSION_FILE>;
close FALLBACK_VERSION_FILE;
chomp($FALLBACK_VERSION);

my $COPYRIGHT_END_YEAR_PATH = File::Spec->catfile($thisDirectory, 'COPYRIGHT-END-YEAR');
open(COPYRIGHT_END_YEAR_FILE, '<', $COPYRIGHT_END_YEAR_PATH) or die "Unable to open $COPYRIGHT_END_YEAR_PATH: $!";
my $COPYRIGHT_END_YEAR = <COPYRIGHT_END_YEAR_FILE>;
close COPYRIGHT_END_YEAR_FILE;
chomp($COPYRIGHT_END_YEAR);

# Make sure we don't have any leading or trailing quote
$ARGV[0] =~ s/^\"//;
$ARGV[0] =~ s/\"$//;

my $OUTPUT_DIR = File::Spec->catdir(File::Spec->canonpath($ARGV[0]), 'include');
unless (-d $OUTPUT_DIR) {
    make_path($OUTPUT_DIR) or die "Couldn't create $OUTPUT_DIR: $!";
}

my $OUTPUT_FILE = File::Spec->catfile($OUTPUT_DIR, 'autoversion.h');

# Take the initial version number from RC_ProjectSourceVersion if it
# exists, otherwise fall back to the version number stored in the source.
my $ENVIRONMENT_VERSION = $ENV{'RC_ProjectSourceVersion'} || $ENV{'RC_PROJECTSOURCEVERSION'};
my $PROPOSED_VERSION = $ENVIRONMENT_VERSION || $FALLBACK_VERSION;
chomp($PROPOSED_VERSION);

my ($BUILD_MAJOR_VERSION, $BUILD_MINOR_VERSION, $BUILD_TINY_VERSION, $BUILD_VARIANT_VERSION, $ADJUSTED_PROPOSED_VERSION, $FULL_BUILD_MAJOR_VERSION) = splitVersion($PROPOSED_VERSION);

my $TINY_VERSION = $BUILD_TINY_VERSION;
my $VERSION_TEXT = $ADJUSTED_PROPOSED_VERSION;
my $VERSION_TEXT_SHORT = $VERSION_TEXT;

my $SVN_REVISION = '';
if (!$ENVIRONMENT_VERSION) {
    # If we didn't pull the version number from the environment then we're doing
    # an engineering build and we'll stamp the build with some more information.

    my $BUILD_DATE = localtime(time);
    my $SVN_REVISION = `svn info`;
    $SVN_REVISION =~ m/Revision: (\d+)/;
    $SVN_REVISION= $1;

    chomp($BUILD_DATE);
    chomp($SVN_REVISION);

    $VERSION_TEXT_SHORT .= "+";
    my $USER = Win32::LoginName;
    $VERSION_TEXT = "$VERSION_TEXT_SHORT $USER - $BUILD_DATE - r$SVN_REVISION";
}

open(OUTPUT_FILE, '>', $OUTPUT_FILE) or die "Couldn't open $OUTPUT_FILE: $!";
print OUTPUT_FILE <<EOF;
#define __VERSION_TEXT__ "$VERSION_TEXT"
#define __BUILD_NUMBER__ "$VERSION_TEXT"
#define __BUILD_NUMBER_SHORT__ "$VERSION_TEXT_SHORT"
#define __VERSION_MAJOR__ $BUILD_MAJOR_VERSION
#define __VERSION_MINOR__ $BUILD_MINOR_VERSION
#define __VERSION_TINY__ $BUILD_TINY_VERSION
#define __VERSION_BUILD__ $BUILD_VARIANT_VERSION
#define __BUILD_NUMBER_MAJOR__ $BUILD_MAJOR_VERSION
#define __BUILD_NUMBER_MINOR__ $BUILD_MINOR_VERSION
#define __BUILD_NUMBER_VARIANT__ $BUILD_TINY_VERSION
#define __SVN_REVISION__ $SVN_REVISION
#define __FULL_BUILD_MAJOR_VERSION__  $FULL_BUILD_MAJOR_VERSION
EOF

if (defined $COPYRIGHT_END_YEAR) {
    print OUTPUT_FILE "#define __COPYRIGHT_YEAR_END_TEXT__ \"$COPYRIGHT_END_YEAR\"\n";
}
close(OUTPUT_FILE);

sub packTwoValues($$)
{
    my $first = shift;
    my $second = shift;

    die "First version component ($first) is too large. Must be between 0 and 99" if ($first > 99);
    die "Second version component ($second) is too large. Must be between 0 and 999" if ($second > 999);

    return $first * 1000 + $second;
}

sub splitVersion($)
{
    my $PROPOSED_VERSION = shift;

    $PROPOSED_VERSION =~ s/^\s+//g; # Get rid of any leading whitespace
    $PROPOSED_VERSION =~ s/\s+$//g; # Get rid of any trailing whitespace

    # Split out the components of the dotted version number.
    my @components = split(/\./, $PROPOSED_VERSION) or die "Couldn't parse $PROPOSED_VERSION";
    my $componentCount = scalar(@components);

    my $BUILD_MAJOR_VERSION = $components[0];

    # Have the minor and tiny components default to zero if not present.
    my $BUILD_MINOR_VERSION = 0;
    my $BUILD_TINY_VERSION = 0;
    my $BUILD_MICRO_VERSION = 0;
    my $BUILD_NANO_VERSION = 0;
    if ($componentCount > 1) {
        $BUILD_MINOR_VERSION = $components[1];
    }
    if ($componentCount > 2) {
        $BUILD_TINY_VERSION = $components[2];
    }
    if ($componentCount > 3) {
        $BUILD_MICRO_VERSION = $components[3];
    }

    my $RETURN_NANO_VERSION = $ENV{'RC_ProjectBuildVersion'} || $ENV{'RC_PROJECTBUILDVERSION'} || $BUILD_MICRO_VERSION;
    if ($componentCount > 4) {
        $BUILD_NANO_VERSION = $components[4];
        $RETURN_NANO_VERSION = $BUILD_NANO_VERSION;
    }

    # Cut the major component down to three characters by dropping any
    # extra leading digits, then adjust the major version portion of the
    # version string to match.
    my $originalLength = length($BUILD_MAJOR_VERSION);
    my $FULL_BUILD_MAJOR_VERSION = $BUILD_MAJOR_VERSION;
    $BUILD_MAJOR_VERSION =~ s/^.*(\d\d\d)$/$1/;

    my $charactersToRemove = $originalLength - length($BUILD_MAJOR_VERSION);

    $PROPOSED_VERSION = substr($PROPOSED_VERSION, $charactersToRemove);

    return ($BUILD_MAJOR_VERSION, packTwoValues($BUILD_MINOR_VERSION, $BUILD_TINY_VERSION), packTwoValues($BUILD_MICRO_VERSION, $BUILD_NANO_VERSION), $RETURN_NANO_VERSION, $PROPOSED_VERSION, $FULL_BUILD_MAJOR_VERSION);
}
