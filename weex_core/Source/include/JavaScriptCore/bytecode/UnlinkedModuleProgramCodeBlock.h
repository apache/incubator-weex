/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#pragma once

#include "UnlinkedGlobalCodeBlock.h"

namespace JSC {

class UnlinkedModuleProgramCodeBlock final : public UnlinkedGlobalCodeBlock {
public:
    typedef UnlinkedGlobalCodeBlock Base;
    static const unsigned StructureFlags = Base::StructureFlags | StructureIsImmortal;

    static UnlinkedModuleProgramCodeBlock* create(VM* vm, const ExecutableInfo& info, DebuggerMode debuggerMode)
    {
        UnlinkedModuleProgramCodeBlock* instance = new (NotNull, allocateCell<UnlinkedModuleProgramCodeBlock>(vm->heap)) UnlinkedModuleProgramCodeBlock(vm, vm->unlinkedModuleProgramCodeBlockStructure.get(), info, debuggerMode);
        instance->finishCreation(*vm);
        return instance;
    }

    static void destroy(JSCell*);

    static void visitChildren(JSCell*, SlotVisitor&);

    // This offset represents the constant register offset to the stored symbol table that represents the layout of the
    // module environment. This symbol table is created by the byte code generator since the module environment includes
    // the top-most lexical captured variables inside the module code. This means that, once the module environment is
    // allocated and instantiated from this symbol table, it is titely coupled with the specific unlinked module program
    // code block and the stored symbol table. So before executing the module code, we should not clear the unlinked module
    // program code block in the module executable. This requirement is met because the garbage collector only clears
    // unlinked code in (1) unmarked executables and (2) function executables.
    //
    // Since the function code may be executed repeatedly and the environment of each function execution is different,
    // the function code need to allocate and instantiate the environment in the prologue of the function code. On the
    // other hand, the module code is executed only once. So we can instantiate the module environment outside the module
    // code. At that time, we construct the module environment by using the symbol table that is held by the module executable.
    // The symbol table held by the executable is the cloned one from one in the unlinked code block. Instantiating the module
    // environment before executing and linking the module code is required to link the imported bindings between the modules.
    //
    // The unlinked module program code block only holds the pre-cloned symbol table in its constant register pool. It does
    // not hold the instantiated module environment. So while the module environment requires the specific unlinked module
    // program code block, the unlinked module code block can be used for the module environment instantiated from this
    // unlinked code block. There is 1:N relation between the unlinked module code block and the module environments. So the
    // unlinked module program code block can be cached.
    //
    // On the other hand, the linked code block for the module environment includes the resolved references to the imported
    // bindings. The imported binding references the other module environment, so the linked code block is titly coupled
    // with the specific set of the module environments. Thus, the linked code block should not be cached.
    int moduleEnvironmentSymbolTableConstantRegisterOffset() { return m_moduleEnvironmentSymbolTableConstantRegisterOffset; }
    void setModuleEnvironmentSymbolTableConstantRegisterOffset(int offset)
    {
        m_moduleEnvironmentSymbolTableConstantRegisterOffset = offset;
    }

private:
    UnlinkedModuleProgramCodeBlock(VM* vm, Structure* structure, const ExecutableInfo& info, DebuggerMode debuggerMode)
        : Base(vm, structure, ModuleCode, info, debuggerMode)
    {
    }

    int m_moduleEnvironmentSymbolTableConstantRegisterOffset { 0 };

public:
    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue proto)
    {
        return Structure::create(vm, globalObject, proto, TypeInfo(UnlinkedModuleProgramCodeBlockType, StructureFlags), info());
    }

    DECLARE_INFO;
};

}
