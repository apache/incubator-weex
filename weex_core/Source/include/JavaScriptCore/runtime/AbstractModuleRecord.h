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

#include "Identifier.h"
#include "JSDestructibleObject.h"
#include "SourceCode.h"
#include "VariableEnvironment.h"
#include <wtf/HashMap.h>
#include <wtf/ListHashSet.h>
#include <wtf/Optional.h>

namespace JSC {

class JSModuleEnvironment;
class JSModuleNamespaceObject;
class JSMap;

// Based on the Source Text Module Record
// http://www.ecma-international.org/ecma-262/6.0/#sec-source-text-module-records
class AbstractModuleRecord : public JSDestructibleObject {
    friend class LLIntOffsetsExtractor;
public:
    typedef JSDestructibleObject Base;

    // https://tc39.github.io/ecma262/#sec-source-text-module-records
    struct ExportEntry {
        enum class Type {
            Local,
            Indirect
        };

        static ExportEntry createLocal(const Identifier& exportName, const Identifier& localName);
        static ExportEntry createIndirect(const Identifier& exportName, const Identifier& importName, const Identifier& moduleName);

        Type type;
        Identifier exportName;
        Identifier moduleName;
        Identifier importName;
        Identifier localName;
    };

    struct ImportEntry {
        Identifier moduleRequest;
        Identifier importName;
        Identifier localName;

        bool isNamespace(VM& vm) const
        {
            return importName == vm.propertyNames->timesIdentifier;
        }
    };

    typedef WTF::ListHashSet<RefPtr<UniquedStringImpl>, IdentifierRepHash> OrderedIdentifierSet;
    typedef HashMap<RefPtr<UniquedStringImpl>, ImportEntry, IdentifierRepHash, HashTraits<RefPtr<UniquedStringImpl>>> ImportEntries;
    typedef HashMap<RefPtr<UniquedStringImpl>, ExportEntry, IdentifierRepHash, HashTraits<RefPtr<UniquedStringImpl>>> ExportEntries;

    DECLARE_EXPORT_INFO;

    void appendRequestedModule(const Identifier&);
    void addStarExportEntry(const Identifier&);
    void addImportEntry(const ImportEntry&);
    void addExportEntry(const ExportEntry&);

    std::optional<ImportEntry> tryGetImportEntry(UniquedStringImpl* localName);
    std::optional<ExportEntry> tryGetExportEntry(UniquedStringImpl* exportName);

    const Identifier& moduleKey() const { return m_moduleKey; }
    const OrderedIdentifierSet& requestedModules() const { return m_requestedModules; }
    const ExportEntries& exportEntries() const { return m_exportEntries; }
    const ImportEntries& importEntries() const { return m_importEntries; }
    const OrderedIdentifierSet& starExportEntries() const { return m_starExportEntries; }

    void dump();

    struct Resolution {
        enum class Type { Resolved, NotFound, Ambiguous, Error };

        static Resolution notFound();
        static Resolution error();
        static Resolution ambiguous();

        Type type;
        AbstractModuleRecord* moduleRecord;
        Identifier localName;
    };

    Resolution resolveExport(ExecState*, const Identifier& exportName);
    Resolution resolveImport(ExecState*, const Identifier& localName);

    AbstractModuleRecord* hostResolveImportedModule(ExecState*, const Identifier& moduleName);

    JSModuleNamespaceObject* getModuleNamespace(ExecState*);
    
    JSModuleEnvironment* moduleEnvironment()
    {
        ASSERT(m_moduleEnvironment);
        return m_moduleEnvironment.get();
    }

protected:
    AbstractModuleRecord(VM&, Structure*, const Identifier&);
    void finishCreation(ExecState*, VM&);

    static void visitChildren(JSCell*, SlotVisitor&);
    static void destroy(JSCell*);

    WriteBarrier<JSModuleEnvironment> m_moduleEnvironment;

private:
    struct ResolveQuery;
    static Resolution resolveExportImpl(ExecState*, const ResolveQuery&);
    std::optional<Resolution> tryGetCachedResolution(UniquedStringImpl* exportName);
    void cacheResolution(UniquedStringImpl* exportName, const Resolution&);

    // The loader resolves the given module name to the module key. The module key is the unique value to represent this module.
    Identifier m_moduleKey;

    // Currently, we don't keep the occurrence order of the import / export entries.
    // So, we does not guarantee the order of the errors.
    // e.g. The import declaration that occurr later than the another import declaration may
    //      throw the error even if the former import declaration also has the invalid content.
    //
    //      import ... // (1) this has some invalid content.
    //      import ... // (2) this also has some invalid content.
    //
    //      In the above case, (2) may throw the error earlier than (1)
    //
    // But, in all the cases, we will throw the syntax error. So except for the content of the syntax error,
    // there are no difference.

    // Map localName -> ImportEntry.
    ImportEntries m_importEntries;

    // Map exportName -> ExportEntry.
    ExportEntries m_exportEntries;

    // Save the occurrence order since resolveExport requires it.
    OrderedIdentifierSet m_starExportEntries;

    // Save the occurrence order since the module loader loads and runs the modules in this order.
    // http://www.ecma-international.org/ecma-262/6.0/#sec-moduleevaluation
    OrderedIdentifierSet m_requestedModules;

    WriteBarrier<JSMap> m_dependenciesMap;
    
    WriteBarrier<JSModuleNamespaceObject> m_moduleNamespaceObject;

    // We assume that all the AbstractModuleRecord are retained by JSModuleLoader's registry.
    // So here, we don't visit each object for GC. The resolution cache map caches the once
    // looked up correctly resolved resolution, since (1) we rarely looked up the non-resolved one,
    // and (2) if we cache all the attempts the size of the map becomes infinitely large.
    typedef HashMap<RefPtr<UniquedStringImpl>, Resolution, IdentifierRepHash, HashTraits<RefPtr<UniquedStringImpl>>> Resolutions;
    Resolutions m_resolutionCache;
};

} // namespace JSC
