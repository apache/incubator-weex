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

#include <stddef.h>

namespace std {
template<typename T> class optional;
}

namespace WTF {

template<typename T> class Function;
template<typename T> class LazyNeverDestroyed;
template<typename T> class NeverDestroyed;
template<typename T> class OptionSet;
template<typename T> class PassRefPtr;
template<typename T> class Ref;
template<typename T> class RefPtr;
template<typename T> class StringBuffer;

template<typename... T> class Variant;
template<typename T, size_t inlineCapacity, typename OverflowHandler, size_t minCapacity> class Vector;

class AtomicString;
class AtomicStringImpl;
class BinarySemaphore;
class CString;
class FunctionDispatcher;
class OrdinalNumber;
class PrintStream;
class String;
class StringBuilder;
class StringImpl;
class StringView;
class TextPosition;

}

using WTF::AtomicString;
using WTF::AtomicStringImpl;
using WTF::BinarySemaphore;
using WTF::CString;
using WTF::Function;
using WTF::FunctionDispatcher;
using WTF::LazyNeverDestroyed;
using WTF::NeverDestroyed;
using WTF::OptionSet;
using WTF::OrdinalNumber;
using WTF::PassRefPtr;
using WTF::PrintStream;
using WTF::Ref;
using WTF::RefPtr;
using WTF::String;
using WTF::StringBuffer;
using WTF::StringBuilder;
using WTF::StringImpl;
using WTF::StringView;
using WTF::TextPosition;
using WTF::Variant;
using WTF::Vector;
