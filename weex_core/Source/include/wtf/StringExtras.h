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
#ifndef WTF_StringExtras_h
#define WTF_StringExtras_h

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#if HAVE(STRINGS_H) 
#include <strings.h> 
#endif 

#if COMPILER(MSVC)

// FIXME: We should stop using these entirely and use suitable versions of equalIgnoringASCIICase instead.

inline int strncasecmp(const char* s1, const char* s2, size_t len)
{
    return _strnicmp(s1, s2, len);
}

inline int strcasecmp(const char* s1, const char* s2)
{
    return _stricmp(s1, s2);
}

#endif

#if !HAVE(STRNSTR)

inline char* strnstr(const char* buffer, const char* target, size_t bufferLength)
{
    size_t targetLength = strlen(target);
    if (targetLength == 0)
        return const_cast<char*>(buffer);
    for (const char* start = buffer; *start && start + targetLength <= buffer + bufferLength; start++) {
        if (*start == *target && strncmp(start + 1, target + 1, targetLength - 1) == 0)
            return const_cast<char*>(start);
    }
    return 0;
}

#endif

#endif // WTF_StringExtras_h
