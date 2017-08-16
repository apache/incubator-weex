/*
 * Copyright (C) 2014 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "config.h"
#import "StringView.h"

#import "RetainPtr.h"

namespace WTF {

RetainPtr<NSString> StringView::createNSString() const
{
    if (is8Bit())
        return adoptNS([[NSString alloc] initWithBytes:const_cast<LChar*>(characters8()) length:length() encoding:NSISOLatin1StringEncoding]);

    return adoptNS([[NSString alloc] initWithCharacters:const_cast<UChar*>(characters16()) length:length()]);
}

RetainPtr<NSString> StringView::createNSStringWithoutCopying() const
{
    if (is8Bit())
        return adoptNS([[NSString alloc] initWithBytesNoCopy:const_cast<LChar*>(characters8()) length:length() encoding:NSISOLatin1StringEncoding freeWhenDone:NO]);

    return adoptNS([[NSString alloc] initWithCharactersNoCopy:const_cast<UChar*>(characters16()) length:length() freeWhenDone:NO]);
}

}
