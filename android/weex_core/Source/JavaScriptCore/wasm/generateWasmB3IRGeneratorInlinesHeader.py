#!/usr/bin/env python

# Copyright (C) 2016 Apple Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:n
#
# 1.  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# This tool has a couple of helpful macros to process Wasm files from the wasm.json.

from generateWasm import *
import optparse
import sys
import re

parser = optparse.OptionParser(usage="usage: %prog <wasm.json> <WasmOps.h>")
(options, args) = parser.parse_args(sys.argv[0:])
if len(args) != 3:
    parser.error(parser.usage)

wasm = Wasm(args[0], args[1])
opcodes = wasm.opcodes
wasmB3IRGeneratorHFile = open(args[2], "w")

opcodeRegex = re.compile('([a-zA-Z0-9]+)')
argumentRegex = re.compile('(\@[0-9]+)')
decimalRegex = re.compile('([-]?[0-9]+)')
whitespaceRegex = re.compile('\s+')
commaRegex = re.compile('(,)')
oparenRegex = re.compile('(\()')
cparenRegex = re.compile('(\))')


class Source:
    def __init__(self, contents, offset=0):
        self.contents = contents
        self.offset = offset


def read(regex, source):
    match = regex.match(source.contents, source.offset)
    if not match:
        return None
    source.offset = match.end()
    return match.group()


def lex(source):
    result = []
    while source.offset != len(source.contents):
        read(whitespaceRegex, source)
        opcode = read(opcodeRegex, source)
        if opcode:
            result.append(opcode)
            continue

        argument = read(argumentRegex, source)
        if argument:
            result.append(argument)
            continue

        number = read(decimalRegex, source)
        if number:
            result.append(int(number))
            continue

        oparen = read(oparenRegex, source)
        if oparen:
            result.append(oparen)
            continue

        cparen = read(cparenRegex, source)
        if cparen:
            result.append(cparen)
            continue

        comma = read(commaRegex, source)
        if comma:
            # Skip commas
            continue

        raise Exception("Lexing Error: could not lex token from: " + source.contents + " at offset: " + str(source.offset) + " (" + source.contents[source.offset:] + "). With tokens: [" + ", ".join(result) + "]")
    return result


class CodeGenerator:
    def __init__(self, tokens):
        self.tokens = tokens
        self.index = 0
        self.code = []

    def advance(self):
        self.index += 1

    def token(self):
        return self.tokens[self.index]

    def parseError(self, string):
        raise Exception("Parse error " + string)

    def consume(self, string):
        if self.token() != string:
            self.parseError("Expected " + string + " but got " + self.token())
        self.advance()

    def generateParameters(self):
        self.advance()
        params = []
        tokens = self.tokens
        while self.index < len(tokens):
            if self.token() == ")":
                self.advance()
                return params
            params.append(self.generateOpcode())
        self.parseError("Parsing arguments fell off end")

    def generateOpcode(self):
        result = None
        if self.token() == "i32" or self.token() == "i64":
            type = "Int32"
            if self.token() == "i64":
                type = "Int64"
            self.advance()
            self.consume("(")
            self.code.append(generateConstCode(self.index, self.token(), type))
            result = temp(self.index)
            self.advance()
            self.consume(")")
        elif argumentRegex.match(self.token()):
            result = "arg" + self.token()[1:]
            self.advance()
        else:
            op = self.token()
            index = self.index
            self.advance()
            params = self.generateParameters()
            self.code.append(generateB3OpCode(index, op, params))
            result = temp(index)

        return result

    def generate(self, wasmOp):
        if len(self.tokens) == 1:
            params = ["arg" + str(param) for param in range(len(wasmOp["parameter"]))]
            return "    result = m_currentBlock->appendNew<Value>(m_proc, B3::" + self.token() + ", Origin(), " + ", ".join(params) + ")"
        result = self.generateOpcode()
        self.code.append("result = " + result)
        return "    " + "    \n".join(self.code)


def temp(index):
    return "temp" + str(index)


def generateB3OpCode(index, op, params):
    return "Value* " + temp(index) + " = m_currentBlock->appendNew<Value>(m_proc, B3::" + op + ", Origin(), " + ", ".join(params) + ");"


def generateConstCode(index, value, type):
    return "Value* " + temp(index) + " = m_currentBlock->appendIntConstant(m_proc, Origin(), B3::" + type + ", " + value + ");"


def generateB3Code(wasmOp, source):
    tokens = lex(Source(source))
    parser = CodeGenerator(tokens)
    return parser.generate(wasmOp)


def generateSimpleCode(op):
    opcode = op["opcode"]
    b3op = opcode["b3op"]
    args = ["ExpressionType arg" + str(param) for param in range(len(opcode["parameter"]))]
    args.append("ExpressionType& result")
    return """
template<> auto B3IRGenerator::addOp<OpType::""" + wasm.toCpp(op["name"]) + ">(" + ", ".join(args) + """) -> PartialResult
{
""" + generateB3Code(opcode, b3op) + """;
    return { };
}
"""


definitions = [generateSimpleCode(op) for op in wasm.opcodeIterator(lambda op: isSimple(op) and (isBinary(op) or isUnary(op)))]
contents = wasm.header + """

#pragma once

#if ENABLE(WEBASSEMBLY)

namespace JSC { namespace Wasm {

""" + "".join(definitions) + """

} } // namespace JSC::Wasm

#endif // ENABLE(WEBASSEMBLY)

"""

wasmB3IRGeneratorHFile.write(contents)
wasmB3IRGeneratorHFile.close()
