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

// This file's only purpose is to collect commonly used *Inlines.h files, so that you don't
// have to include all of them in every .cpp file. Instead you just include this. It's good
// style to make sure that every .cpp file includes JSCInlines.h.
//
// JSC should never include this file, or any *Inline.h file, from interface headers, since
// this could lead to a circular dependency.
//
// WebCore, or any other downstream client of JSC, is allowed to include this file in headers.
// In fact, it can make a lot of sense: outside of JSC, this file becomes a kind of umbrella
// header that pulls in most (all?) of the interesting things in JSC.

#include "ExceptionHelpers.h"
#include "GCIncomingRefCountedInlines.h"
#include "HeapInlines.h"
#include "IdentifierInlines.h"
#include "JSArrayBufferViewInlines.h"
#include "JSCJSValueInlines.h"
#include "JSFunctionInlines.h"
#include "JSGlobalObjectInlines.h"
#include "JSObjectInlines.h"
#include "JSProxy.h"
#include "JSString.h"
#include "Operations.h"
#include "SlotVisitorInlines.h"
#include "StructureInlines.h"
#include "ThrowScope.h"
#include "WeakGCMapInlines.h"
