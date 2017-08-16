# Copyright (C) 2011 Apple Inc. All rights reserved.
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

require "config"

GPRS =
    [
     "t0",
     "t1",
     "t2",
     "t3",
     "t4",
     "t5",
     "cfr",
     "a0",
     "a1",
     "a2",
     "a3",
     "r0",
     "r1",
     "sp",
     "lr",
     "pc",
     # 64-bit only registers:
     "csr0",
     "csr1",
     "csr2",
     "csr3",
     "csr4",
     "csr5",
     "csr6",
     "csr7",
     "csr8",
     "csr9"
    ]

FPRS =
    [
     "ft0",
     "ft1",
     "ft2",
     "ft3",
     "ft4",
     "ft5",
     "fa0",
     "fa1",
     "fa2",
     "fa3",
     "csfr0",
     "csfr1",
     "csfr2",
     "csfr3",
     "csfr4",
     "csfr5",
     "csfr6",
     "csfr7",
     "fr"
    ]

REGISTERS = GPRS + FPRS

GPR_PATTERN = Regexp.new('\\A((' + GPRS.join(')|(') + '))\\Z')
FPR_PATTERN = Regexp.new('\\A((' + FPRS.join(')|(') + '))\\Z')

REGISTER_PATTERN = Regexp.new('\\A((' + REGISTERS.join(')|(') + '))\\Z')
