/*
 * Copyright (C) 2003, 2005, 2006, 2010, 2011, 2016 Apple Inc. All rights reserved.
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
#import "PlatformUserPreferredLanguages.h"

#import "BlockObjCExceptions.h"
#import <mutex>
#import <unicode/uloc.h>
#import <wtf/Assertions.h>
#import <wtf/Lock.h>
#import <wtf/NeverDestroyed.h>
#import <wtf/RetainPtr.h>
#import <wtf/spi/cf/CFBundleSPI.h>
#import <wtf/text/WTFString.h>

namespace WTF {

static void (*callback)();

void setPlatformUserPreferredLanguagesChangedCallback(void (*passedCallback)())
{
    callback = passedCallback;
}

static StaticLock preferredLanguagesMutex;

static Vector<String>& preferredLanguages()
{
    static NeverDestroyed<Vector<String>> languages;
    return languages;
}

}

@interface WTFLanguageChangeObserver : NSObject
@end

@implementation WTFLanguageChangeObserver

+ (void)languagePreferencesDidChange:(NSNotification *)notification
{
    UNUSED_PARAM(notification);

    {
        std::lock_guard<StaticLock> lock(WTF::preferredLanguagesMutex);
        WTF::preferredLanguages().clear();
    }
    
    if (WTF::callback)
        WTF::callback();
}

@end

namespace WTF {

static String httpStyleLanguageCode(NSString *language)
{
    SInt32 languageCode;
    SInt32 regionCode;
    SInt32 scriptCode;
    CFStringEncoding stringEncoding;

    // FIXME: This transformation is very wrong:
    // 1. There is no reason why CFBundle localization names would be at all related to language names as used on the Web.
    // 2. Script Manager codes cannot represent all languages that are now supported by the platform, so the conversion is lossy.
    // 3. This should probably match what is sent by the network layer as Accept-Language, but currently, that's implemented separately.
    CFBundleGetLocalizationInfoForLocalization((CFStringRef)language, &languageCode, &regionCode, &scriptCode, &stringEncoding);
    RetainPtr<CFStringRef> preferredLanguageCode = adoptCF(CFBundleCopyLocalizationForLocalizationInfo(languageCode, regionCode, scriptCode, stringEncoding));
    if (preferredLanguageCode)
        language = (NSString *)preferredLanguageCode.get();

    // Turn a '_' into a '-' if it appears after a 2-letter language code
    if ([language length] >= 3 && [language characterAtIndex:2] == '_') {
        RetainPtr<NSMutableString> mutableLanguageCode = adoptNS([language mutableCopy]);
        [mutableLanguageCode.get() replaceCharactersInRange:NSMakeRange(2, 1) withString:@"-"];
        return mutableLanguageCode.get();
    }

    return language;
}

Vector<String> platformUserPreferredLanguages()
{
#if PLATFORM(MAC)
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        [[NSDistributedNotificationCenter defaultCenter] addObserver:[WTFLanguageChangeObserver self] selector:@selector(languagePreferencesDidChange:) name:@"AppleLanguagePreferencesChangedNotification" object:nil];
    });
#endif

    BEGIN_BLOCK_OBJC_EXCEPTIONS;

    std::lock_guard<StaticLock> lock(preferredLanguagesMutex);
    Vector<String>& userPreferredLanguages = preferredLanguages();

    if (userPreferredLanguages.isEmpty()) {
        RetainPtr<CFArrayRef> languages = adoptCF(CFLocaleCopyPreferredLanguages());
        CFIndex languageCount = CFArrayGetCount(languages.get());
        if (!languageCount)
            userPreferredLanguages.append("en");
        else {
            for (CFIndex i = 0; i < languageCount; i++)
                userPreferredLanguages.append(httpStyleLanguageCode((NSString *)CFArrayGetValueAtIndex(languages.get(), i)));
        }
    }

    Vector<String> userPreferredLanguagesCopy;
    userPreferredLanguagesCopy.reserveInitialCapacity(userPreferredLanguages.size());

    for (auto& language : userPreferredLanguages)
        userPreferredLanguagesCopy.uncheckedAppend(language.isolatedCopy());

    return userPreferredLanguagesCopy;

    END_BLOCK_OBJC_EXCEPTIONS;

    return Vector<String>();
}

} // namespace WTF
