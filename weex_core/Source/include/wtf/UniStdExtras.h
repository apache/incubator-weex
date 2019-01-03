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
#ifndef UniStdExtras_h
#define UniStdExtras_h

#include <errno.h>
#include <unistd.h>

namespace WTF {

bool setCloseOnExec(int fileDescriptor);
int dupCloseOnExec(int fileDescriptor);

inline int closeWithRetry(int fileDescriptor)
{
    int ret;
#if OS(LINUX)
    // Workaround for the Linux behavior of closing the descriptor
    // unconditionally, even if the close() call is interrupted.
    // See https://bugs.webkit.org/show_bug.cgi?id=117266 for more
    // details.
    if ((ret = close(fileDescriptor)) == -1 && errno == EINTR)
        return 0;
#else
    while ((ret = close(fileDescriptor)) == -1 && errno == EINTR) { }
#endif
    return ret;
}

} // namespace WTF

using WTF::closeWithRetry;
using WTF::setCloseOnExec;
using WTF::dupCloseOnExec;

#endif // UniStdExtras_h
