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
#ifndef DataLog_h
#define DataLog_h

#include <stdarg.h>
#include <stdio.h>
#include <wtf/PrintStream.h>
#include <wtf/StdLibExtras.h>

namespace WTF {

WTF_EXPORT_PRIVATE PrintStream& dataFile();
WTF_EXPORT_PRIVATE void setDataFile(const char* path);

WTF_EXPORT_PRIVATE void dataLogFV(const char* format, va_list) WTF_ATTRIBUTE_PRINTF(1, 0);
WTF_EXPORT_PRIVATE void dataLogF(const char* format, ...) WTF_ATTRIBUTE_PRINTF(1, 2);
WTF_EXPORT_PRIVATE void dataLogFString(const char*);

template<typename... Types>
void dataLog(const Types&... values)
{
    dataFile().print(values...);
}

template<typename... Types>
void dataLogLn(const Types&... values)
{
    dataLog(values..., "\n");
}

template<typename... Types>
void dataLogIf(bool shouldLog, const Types&... values)
{
    if (shouldLog)
        dataLog(values...);
}

template<typename... Types>
void dataLogLnIf(bool shouldLog, const Types&... values)
{
    if (shouldLog)
        dataLogLn(values...);
}

} // namespace WTF

using WTF::dataLog;
using WTF::dataLogLn;
using WTF::dataLogIf;
using WTF::dataLogLnIf;
using WTF::dataLogF;
using WTF::dataLogFString;

#endif // DataLog_h

