/*
 *  Copyright (C) 2006, 2009, 2011 Apple Inc. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
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
