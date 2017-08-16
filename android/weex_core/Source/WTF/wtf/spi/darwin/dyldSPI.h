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

#pragma once

#if USE(APPLE_INTERNAL_SDK)

#include <mach-o/dyld_priv.h>

#if PLATFORM(IOS) && __IPHONE_OS_VERSION_MIN_REQUIRED < 100000
#define DYLD_IOS_VERSION_10_0 0x000A0000
#endif

#if PLATFORM(MAC) && __MAC_OS_X_VERSION_MIN_REQUIRED < 101200
#define DYLD_MACOSX_VERSION_10_12 0x000A0C00
#endif

#ifndef DYLD_IOS_VERSION_11_0
#define DYLD_IOS_VERSION_11_0 0x000B0000
#endif

#ifndef DYLD_MACOSX_VERSION_10_13
#define DYLD_MACOSX_VERSION_10_13 0x000A0D00
#endif

#else

#define DYLD_IOS_VERSION_3_0 0x00030000
#define DYLD_IOS_VERSION_4_2 0x00040200
#define DYLD_IOS_VERSION_5_0 0x00050000
#define DYLD_IOS_VERSION_6_0 0x00060000
#define DYLD_IOS_VERSION_7_0 0x00070000
#define DYLD_IOS_VERSION_9_0 0x00090000
#define DYLD_IOS_VERSION_10_0 0x000A0000
#define DYLD_IOS_VERSION_11_0 0x000B0000

#define DYLD_MACOSX_VERSION_10_11 0x000A0B00
#define DYLD_MACOSX_VERSION_10_12 0x000A0C00
#define DYLD_MACOSX_VERSION_10_13 0x000A0D00

#endif

WTF_EXTERN_C_BEGIN

uint32_t dyld_get_program_sdk_version();

WTF_EXTERN_C_END
