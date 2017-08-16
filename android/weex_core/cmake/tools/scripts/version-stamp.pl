#!/usr/bin/perl -w

use strict;
use Cwd 'abs_path';
use File::Basename;
use File::Spec;
use POSIX;

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

sub exitStatus($);

my $rawPath = $0;

my $thisDirectory = dirname(abs_path($rawPath));

my $VERSION_STAMPER = File::Spec->catfile($thisDirectory, '..', 'VersionStamper', 'VersionStamper.exe');

unless (-e $VERSION_STAMPER) {
    print "No $VERSION_STAMPER executable. Exiting.\n";
    exit(0);
}

my ($intdir, $target);
if (scalar(@ARGV) > 1) {
    $intdir = $ARGV[0];
    $target = $ARGV[1];
} else {
    my @arguments = split(/\s+/, $ARGV[0]) or die "Couldn't parse $ARGV[0]";
    die "You must supply the build output directory and the target to be stamped with version information.\n" if (scalar(@arguments) < 2);
    $intdir = $arguments[0];
    $target = $arguments[1];
}

# Make sure we don't have any leading or trailing quote characters.
$intdir =~ s/^\"//;
$intdir =~ s/\"$//;

$target =~ s/^\"//;
$target =~ s/\"$//;

my %components =
(
    '__VERSION_TEXT__' => undef,
    '__VERSION_MAJOR__' => undef,
    '__VERSION_MINOR__' => undef,
    '__VERSION_TINY__' => undef,
    '__VERSION_BUILD__' => undef,
    '__FULL_BUILD_MAJOR_VERSION__' => undef,
);

my $VERSION_FILE = File::Spec->catfile(File::Spec->canonpath($intdir), 'include', 'autoversion.h');

open(VERSION_INFO, '<', $VERSION_FILE) or die "Unable to open $VERSION_FILE: $!\n";
while (my $line = <VERSION_INFO>) {
    foreach my $componentKey (keys %components) {
        if ($line !~ m/$componentKey/) {
            next;
        }

        $line =~ s/#define $componentKey//;
        $line =~ s/^\s*(.*)\s*$/$1/;
        $line =~ s/^"(.*)"$/$1/;
        chomp($line);

        $components{$componentKey} = $line;
    }
}
close(VERSION_INFO);

print "Processing version $components{'__VERSION_TEXT__'} for $target\n";

my $TARGET_PATH = File::Spec->canonpath($target);

# Replace forward slashes with backslashes in case we are running Cygwin Perl.
$TARGET_PATH =~ s/\//\\/g;

print "Adjusting RC_PROJECTSOURCEVERSION and RC_ProjectSourceVersion to be safe for VersionStamper.\n";

my $SAFE_PROJECT_VERSION = "$components{'__FULL_BUILD_MAJOR_VERSION__'}.$components{'__VERSION_MINOR__'}.$components{'__VERSION_TINY__'}";
my $SAFE_BUILD_VERSION = $ENV{RC_ProjectBuildVersion} || $ENV{RC_PROJECTBUILDVERSION} || $components{'__VERSION_BUILD__'};

print "Using RC_PROJECTSOURCEVERSION=$SAFE_PROJECT_VERSION and RC_PROJECTBUILDVERSION=$SAFE_BUILD_VERSION\n";

# Note: These environment settings only affect this script and its child processes:
$ENV{RC_PROJECTSOURCEVERSION} = $SAFE_PROJECT_VERSION;
$ENV{RC_ProjectSourceVersion} = $SAFE_PROJECT_VERSION;

my $rc = system($VERSION_STAMPER, '--verbose', $TARGET_PATH, '--fileMajor', $components{'__FULL_BUILD_MAJOR_VERSION__'},
    '--fileMinor', $components{'__VERSION_MINOR__'}, '--fileRevision', $components{'__VERSION_TINY__'},
    '--fileBuild', $SAFE_BUILD_VERSION, '--productMajor', $components{'__FULL_BUILD_MAJOR_VERSION__'},
    '--productMinor', $components{'__VERSION_MINOR__'}, '--productRevision', $components{'__VERSION_TINY__'},
    '--productBuild', $components{'__VERSION_BUILD__'}); 

exit(exitStatus($rc));

# This method is for portability. Return the system-appropriate exit
# status of a child process.
#
# Args: pass the child error status returned by the last pipe close,
#       for example "$?".
sub exitStatus($)
{
    my ($returnvalue) = @_;
    if ($^O eq "MSWin32") {
        return $returnvalue >> 8;
    }
    if (!WIFEXITED($returnvalue)) {
        return 254;
    }
    return WEXITSTATUS($returnvalue);
}
