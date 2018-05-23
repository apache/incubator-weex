/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Implementation of the user-space ashmem API for devices, which have our
 * ashmem-enabled kernel. See ashmem-sim.c for the "fake" tmp-based version,
 * used by the simulator.
 */

#ifndef IPCCHECK_H
#define IPCCHECK_H
#include <stdlib.h>
#if defined(NDEBUG)
#define IPC_DCHECK(b)
#define IPC_UNREACHABLE() abort()
#else
void reportCheckFailed(const char* msg, const char* file, int line);
void reportUnreachable(const char* file, int line);
#define IPC_DCHECK(b)                                                                \
    if (!(b)) {                                                                      \
        reportCheckFailed("CHECK FAILED: " #b " at line %s %d", __FILE__, __LINE__); \
    }
#define IPC_UNREACHABLE() reportUnreachable(__FILE__, __LINE__);
#endif
#endif /* IPCCHECK_H */
