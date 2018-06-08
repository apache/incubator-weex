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
/*
 * Implementation of the user-space ashmem API for devices, which have our
 * ashmem-enabled kernel. See ashmem-sim.c for the "fake" tmp-based version,
 * used by the simulator.
 */

#ifndef _THIRD_PARTY_ASHMEM_H
#define _THIRD_PARTY_ASHMEM_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int ashmem_create_region(const char* name, size_t size);
int ashmem_set_prot_region(int fd, int prot);
int ashmem_pin_region(int fd, size_t offset, size_t len);
int ashmem_unpin_region(int fd, size_t offset, size_t len);
int ashmem_get_size_region(int fd);
int ashmem_purge_all(void);

#ifdef __cplusplus
}
#endif

#ifndef __ASHMEMIOC /* in case someone included <linux/ashmem.h> too */

#define ASHMEM_NAME_LEN 256

#define ASHMEM_NAME_DEF "dev/ashmem"

/* Return values from ASHMEM_PIN: Was the mapping purged while unpinned? */
#define ASHMEM_NOT_PURGED 0
#define ASHMEM_WAS_PURGED 1

/* Return values from ASHMEM_UNPIN: Is the mapping now pinned or unpinned? */
#define ASHMEM_IS_UNPINNED 0
#define ASHMEM_IS_PINNED 1

#endif /* ! __ASHMEMIOC */

#endif /* _THIRD_PARTY_ASHMEM_H */
