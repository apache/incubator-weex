/*
 * Copyright (C) 2016-2017 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "AirLogRegisterPressure.h"

#if ENABLE(B3_JIT)

#include "AirArgInlines.h"
#include "AirCode.h"
#include "AirInstInlines.h"
#include "AirLiveness.h"

namespace JSC { namespace B3 { namespace Air {

void logRegisterPressure(Code& code)
{
    const unsigned totalColumns = 200;
    const unsigned registerColumns = 100;
    
    RegLiveness liveness(code);

    for (BasicBlock* block : code) {
        RegLiveness::LocalCalc localCalc(liveness, block);

        block->dumpHeader(WTF::dataFile());

        Vector<CString> instDumps;
        for (unsigned instIndex = block->size(); instIndex--;) {
            Inst& inst = block->at(instIndex);
            Inst* prevInst = block->get(instIndex - 1);

            localCalc.execute(instIndex);

            RegisterSet set;
            set.setAll(localCalc.live());
            Inst::forEachDefWithExtraClobberedRegs<Reg>(
                prevInst, &inst,
                [&] (Reg reg, Arg::Role, Bank, Width) {
                    set.set(reg);
                });

            StringPrintStream instOut;
            StringPrintStream lineOut;
            lineOut.print("   ");
            if (set.numberOfSetRegisters()) {
                set.forEach(
                    [&] (Reg reg) {
                        CString text = toCString(" ", reg);
                        if (text.length() + lineOut.length() > totalColumns) {
                            instOut.print(lineOut.toCString(), "\n");
                            lineOut.reset();
                            lineOut.print("       ");
                        }
                        lineOut.print(text);
                    });
                lineOut.print(":");
            }
            if (lineOut.length() > registerColumns) {
                instOut.print(lineOut.toCString(), "\n");
                lineOut.reset();
            }
            while (lineOut.length() < registerColumns)
                lineOut.print(" ");
            lineOut.print(" ");
            lineOut.print(inst);
            instOut.print(lineOut.toCString(), "\n");
            instDumps.append(instOut.toCString());
        }

        for (unsigned i = instDumps.size(); i--;)
            dataLog(instDumps[i]);
        
        block->dumpFooter(WTF::dataFile());
    }
}

} } } // namespace JSC::B3::Air

#endif // ENABLE(B3_JIT)

