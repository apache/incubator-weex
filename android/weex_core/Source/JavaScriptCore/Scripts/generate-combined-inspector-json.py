#!/usr/bin/python

# Copyright (C) 2013 Apple Inc. All rights reserved.
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
# THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGE.

import glob
import json
import os
import sys

if len(sys.argv) < 2:
    print("usage: %s [json files or directory of json files ...]" % os.path.basename(sys.argv[0]))
    sys.exit(1)

files = []
for arg in sys.argv[1:]:
    if not os.access(arg, os.F_OK):
        raise Exception("File \"%s\" not found" % arg)
    elif os.path.isdir(arg):
        files.extend(glob.glob(os.path.join(arg, "*.json")))
    else:
        files.append(arg)
files.sort()

# To keep as close to the original JSON formatting as possible, just
# dump each JSON input file unmodified into an array of "domains".
# Validate each file is valid JSON and that there is a "domain" key.

first = True
print("{\"domains\":[")
for file in files:
    if first:
        first = False
    else:
        print(",")

    string = open(file).read()

    try:
        dictionary = json.loads(string)
        if not "domain" in dictionary:
            raise Exception("File \"%s\" does not contains a \"domain\" key." % file)
    except ValueError:
        sys.stderr.write("File \"%s\" does not contain valid JSON:\n" % file)
        raise

    print(string.rstrip())
print("]}")

