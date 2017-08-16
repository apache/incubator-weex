/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY GOOGLE INC. AND ITS CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL GOOGLE INC.
 * OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "InspectorProtocolObjects.h"
#include <wtf/Vector.h>
#include <wtf/text/TextPosition.h>
#include <wtf/text/WTFString.h>

namespace JSC { namespace Yarr {
class RegularExpression;
} }

namespace Inspector {

namespace ContentSearchUtilities {

JS_EXPORT_PRIVATE JSC::Yarr::RegularExpression createSearchRegex(const String& query, bool caseSensitive, bool isRegex);
JS_EXPORT_PRIVATE int countRegularExpressionMatches(const JSC::Yarr::RegularExpression&, const String&);
JS_EXPORT_PRIVATE Ref<Inspector::Protocol::Array<Inspector::Protocol::GenericTypes::SearchMatch>> searchInTextByLines(const String& text, const String& query, const bool caseSensitive, const bool isRegex);
JS_EXPORT_PRIVATE TextPosition textPositionFromOffset(size_t offset, const Vector<size_t>& lineEndings);
JS_EXPORT_PRIVATE std::unique_ptr<Vector<size_t>> lineEndings(const String&);

JS_EXPORT_PRIVATE String findStylesheetSourceMapURL(const String& content);

} // namespace ContentSearchUtilities

} // namespace Inspector
