#!/usr/bin/perl -w

# Copyright (C) 2013-2014 Apple Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1.  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES

# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

use strict;
use File::Spec;

my $FeatureDefines = ($ARGV[1] eq 'cairo')  ? 'FeatureDefinesCairo.props' : 'FeatureDefines.props';
my $FeatureDefinesFile = File::Spec->catfile($ARGV[0], 'tools', 'vsprops', $FeatureDefines);

open(FEATURE_DEFINES, '<', $FeatureDefinesFile) or die "Unable to open $FeatureDefinesFile: $!";
my @lines = <FEATURE_DEFINES>;
close(FEATURE_DEFINES);

my @enabled = grep(/<ENABLE_/, @lines);
@enabled = grep(!/\/>/, @enabled);
for (@enabled) {
    s/<\/.*>//;
    s/<.*>//;
    s/^\s+//;
    s/\s+$//;
    chomp();
}

my $result = join(' ', @enabled);

print "$result\n";
