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

#include "JSCJSValue.h"
#include "ProfilerBytecodes.h"
#include "ProfilerCompilation.h"
#include "ProfilerCompilationKind.h"
#include "ProfilerEvent.h"
#include <wtf/FastMalloc.h>
#include <wtf/HashMap.h>
#include <wtf/Lock.h>
#include <wtf/Noncopyable.h>
#include <wtf/SegmentedVector.h>
#include <wtf/ThreadingPrimitives.h>
#include <wtf/text/WTFString.h>

namespace JSC { namespace Profiler {

class Database {
    WTF_MAKE_FAST_ALLOCATED; WTF_MAKE_NONCOPYABLE(Database);
public:
    JS_EXPORT_PRIVATE Database(VM&);
    JS_EXPORT_PRIVATE ~Database();
    
    int databaseID() const { return m_databaseID; }
    
    Bytecodes* ensureBytecodesFor(CodeBlock*);
    void notifyDestruction(CodeBlock*);
    
    void addCompilation(CodeBlock*, Ref<Compilation>&&);
    
    // Converts the database to a JavaScript object that is suitable for JSON stringification.
    // Note that it's probably a good idea to use an ExecState* associated with a global
    // object that is "clean" - i.e. array and object prototypes haven't had strange things
    // done to them. And yes, it should be appropriate to just use a globalExec here.
    JS_EXPORT_PRIVATE JSValue toJS(ExecState*) const;
    
    // Converts the database to a JavaScript object using a private temporary global object,
    // and then returns the JSON representation of that object.
    JS_EXPORT_PRIVATE String toJSON() const;
    
    // Saves the JSON representation (from toJSON()) to the given file. Returns false if the
    // save failed.
    JS_EXPORT_PRIVATE bool save(const char* filename) const;

    void registerToSaveAtExit(const char* filename);
    
    JS_EXPORT_PRIVATE void logEvent(CodeBlock* codeBlock, const char* summary, const CString& detail);
    
private:
    Bytecodes* ensureBytecodesFor(const AbstractLocker&, CodeBlock*);
    
    void addDatabaseToAtExit();
    void removeDatabaseFromAtExit();
    void performAtExitSave() const;
    static Database* removeFirstAtExitDatabase();
    static void atExitCallback();
    
    int m_databaseID;
    VM& m_vm;
    SegmentedVector<Bytecodes> m_bytecodes;
    HashMap<CodeBlock*, Bytecodes*> m_bytecodesMap;
    Vector<Ref<Compilation>> m_compilations;
    HashMap<CodeBlock*, Ref<Compilation>> m_compilationMap;
    Vector<Event> m_events;
    bool m_shouldSaveAtExit;
    CString m_atExitSaveFilename;
    Database* m_nextRegisteredDatabase;
    Lock m_lock;
};

} } // namespace JSC::Profiler
