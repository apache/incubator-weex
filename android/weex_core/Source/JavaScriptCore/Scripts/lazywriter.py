#!/usr/bin/env python
#
# Copyright (c) 2015 Apple Inc. All rights reserved.
# Copyright (c) 2009 Google Inc. All rights reserved.
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

# A writer that only updates file if it actually changed.


class LazyFileWriter:
    def __init__(self, filepath, force_output):
        self._filepath = filepath
        self._output = ""
        self.force_output = force_output

    def write(self, text):
        self._output += text

    def close(self):
        text_changed = True
        self._output = self._output.rstrip() + "\n"

        try:
            if self.force_output:
                raise

            read_file = open(self._filepath, "r")
            old_text = read_file.read()
            read_file.close()
            text_changed = old_text != self._output
        except:
            # Ignore, just overwrite by default
            pass

        if text_changed or self.force_output:
            out_file = open(self._filepath, "w")
            out_file.write(self._output)
            out_file.close()
