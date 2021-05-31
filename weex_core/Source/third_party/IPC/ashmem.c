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

#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ashmem.h"
#include <linux/ashmem.h>
#include <stdlib.h>
#include <sys/system_properties.h>
#include <dlfcn.h>
#include <pthread.h>

#define ASHMEM_DEVICE "/dev/ashmem"

static int system_property_get_int(const char* name) {
    int result = 0;
    char value[PROP_VALUE_MAX] = {};
    if (__system_property_get(name, value) >= 1)
        result = atoi(value);
    return result;
}

static int device_api_level() {
    static int s_api_level = -1;
    if (s_api_level < 0)
        s_api_level = system_property_get_int("ro.build.version.sdk");
    return s_api_level;
}

typedef int(*ASharedMemory_createFunc)(const char *, size_t);
typedef size_t(*ASharedMemory_getSizeFunc)(int fd);
typedef int(*ASharedMemory_setProtFunc)(int fd, int prot);

// Function pointers to shared memory functions.
typedef struct {
    ASharedMemory_createFunc create;
    ASharedMemory_getSizeFunc getSize;
    ASharedMemory_setProtFunc setProt;
} ASharedMemoryFuncs;

static ASharedMemoryFuncs s_ashmem_funcs = {
        NULL, NULL, NULL
};
static pthread_once_t s_ashmem_funcs_once = PTHREAD_ONCE_INIT;


/*
 * ashmem_create_region - creates a new ashmem region and returns the file
 * descriptor, or <0 on error
 *
 * `name' is an optional label to give the region (visible in /proc/pid/maps)
 * `size' is the size of the region, in page-aligned bytes
 */
static int ashmem_dev_create_region(const char* name, size_t size)
{
    int fd, ret;

    fd = open(ASHMEM_DEVICE, O_RDWR);
    if (fd < 0)
        return fd;

    if (name) {
        char buf[ASHMEM_NAME_LEN];

        strlcpy(buf, name, sizeof(buf));
        ret = ioctl(fd, ASHMEM_SET_NAME, buf);
        if (ret < 0)
            goto error;
    }

    ret = ioctl(fd, ASHMEM_SET_SIZE, size);
    if (ret < 0)
        goto error;

    return fd;

error:
    close(fd);
    return ret;
}

static int ashmem_dev_set_prot_region(int fd, int prot)
{
    return ioctl(fd, ASHMEM_SET_PROT_MASK, prot);
}

static size_t ashmem_dev_get_size_region(int fd)
{
    return (size_t)ioctl(fd, ASHMEM_GET_SIZE, NULL);
}

static void ashmem_init_funcs() {
    ASharedMemoryFuncs* funcs = &s_ashmem_funcs;
    // ANDROID_API_Q
    if (device_api_level() >= 29) {
        void * lib = dlopen("libandroid.so", RTLD_NOW);
        funcs->create =
                (ASharedMemory_createFunc)dlsym(lib, "ASharedMemory_create");
        funcs->getSize =
                (ASharedMemory_getSizeFunc)dlsym(lib, "ASharedMemory_getSize");
        funcs->setProt =
                (ASharedMemory_setProtFunc)dlsym(lib, "ASharedMemory_setProt");
    }

    if(funcs->create == NULL) {
        funcs->create = &ashmem_dev_create_region;
    }
    if(funcs->getSize == NULL) {
        funcs->getSize = &ashmem_dev_get_size_region;
    }
    if(funcs->setProt == NULL) {
        funcs->setProt = &ashmem_dev_set_prot_region;
    }
}

static const ASharedMemoryFuncs* ashmem_get_funcs() {
    pthread_once(&s_ashmem_funcs_once, ashmem_init_funcs);
    return &s_ashmem_funcs;
}

int ashmem_create_region(const char *name, size_t size)
 {
    return ashmem_get_funcs()->create(name, size);
}

int ashmem_set_prot_region(int fd, int prot)
{
    return ashmem_get_funcs()->setProt(fd, prot);
}

int ashmem_get_size_region(int fd)
{
    return (int)ashmem_get_funcs()->getSize(fd);
}