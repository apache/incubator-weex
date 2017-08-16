/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
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

#include "config.h"
#include "DataLog.h"
#include <stdarg.h>
#include <string.h>
#include <wtf/FilePrintStream.h>
#include <wtf/LockedPrintStream.h>
#include <wtf/Threading.h>
#include <mutex>
#include <thread>

#if OS(UNIX) || OS(DARWIN)
#include <unistd.h>
#endif

#define DATA_LOG_TO_FILE 0

// Set to 1 to use the temp directory from confstr instead of hardcoded directory.
// The last component of DATA_LOG_FILENAME will still be used.
#define DATA_LOG_TO_DARWIN_TEMP_DIR 0

// Uncomment to force logging to the given file regardless of what the environment variable says.
// Note that we will append ".<pid>.txt" where <pid> is the PID.
// This path won't work on Windows, make sure to change to something like C:\\Users\\<more path>\\log.txt.
#define DATA_LOG_FILENAME "/tmp/WTFLog"

namespace WTF {

static const size_t maxPathLength = 1024;

static PrintStream* s_file;
static uint64_t s_fileData[(sizeof(FilePrintStream) + 7) / 8];
static uint64_t s_lockedFileData[(sizeof(LockedPrintStream) + 7) / 8];

static void initializeLogFileOnce()
{
    const char* filename = nullptr;

    if (s_file)
        return;

#if DATA_LOG_TO_FILE
#if DATA_LOG_TO_DARWIN_TEMP_DIR
    char filenameBuffer[maxPathLength + 1];
#if defined(DATA_LOG_FILENAME)
    char* logBasename = strrchr(DATA_LOG_FILENAME, '/');
    if (!logBasename)
        logBasename = (char*)DATA_LOG_FILENAME;
#else
    const char* logBasename = "WTFLog";
#endif

    bool success = confstr(_CS_DARWIN_USER_TEMP_DIR, filenameBuffer, sizeof(filenameBuffer));
    if (success) {
        // FIXME: Assert that the path ends with a slash instead of adding a slash if it does not exist
        // once <rdar://problem/23579077> is fixed in all iOS Simulator versions that we use.
        size_t lastComponentLength = strlen(logBasename) + 20; // More than enough for ".<pid>.txt"
        size_t dirnameLength = strlen(filenameBuffer);
        bool shouldAddPathSeparator = filenameBuffer[dirnameLength - 1] != '/' && logBasename[0] != '/';
        if (lastComponentLength + shouldAddPathSeparator <= sizeof(filenameBuffer) - dirnameLength - 1) {
            if (shouldAddPathSeparator)
                strncat(filenameBuffer, "/", 1);
            strncat(filenameBuffer, logBasename, sizeof(filenameBuffer) - strlen(filenameBuffer) - 1);
            filename = filenameBuffer;
        }
    }
#elif defined(DATA_LOG_FILENAME)
    filename = DATA_LOG_FILENAME;
#else
    filename = getenv("WTF_DATA_LOG_FILENAME");
#endif
    char actualFilename[maxPathLength + 1];

    if (filename && !strstr(filename, "%pid")) {
#if PLATFORM(WIN)
        _snprintf(actualFilename, sizeof(actualFilename), "%s.%%pid.txt", filename);
#else
        snprintf(actualFilename, sizeof(actualFilename), "%s.%%pid.txt", filename);
#endif
        filename = actualFilename;
    }
#endif // DATA_LOG_TO_FILE

    setDataFile(filename);
}

static void initializeLogFile()
{
    static std::once_flag once;
    std::call_once(
        once,
        [] {
            initializeLogFileOnce();
        });
}

void setDataFile(const char* path)
{
    FilePrintStream* file = nullptr;
    char formattedPath[maxPathLength + 1];
    const char* pathToOpen = path;

    if (path) {
        const char* pidFormat = strstr(path, "%pid");
        if (pidFormat) {
            size_t leadingPathLength = pidFormat - path;
            size_t pathCharactersAvailable = std::min(maxPathLength, leadingPathLength);
            strncpy(formattedPath, path, pathCharactersAvailable);
            char* nextDest = formattedPath + pathCharactersAvailable;
            pathCharactersAvailable = maxPathLength - pathCharactersAvailable;
            if (pathCharactersAvailable) {
                int pidTextLength;
#if PLATFORM(WIN)
                pidTextLength = _snprintf(nextDest, pathCharactersAvailable, "%d", GetCurrentProcessId());
#else
                pidTextLength = snprintf(nextDest, pathCharactersAvailable, "%d", getpid());
#endif
                if (pidTextLength < 0 || static_cast<size_t>(pidTextLength) >= pathCharactersAvailable)
                    pathCharactersAvailable = 0;
                else {
                    pathCharactersAvailable -= static_cast<size_t>(pidTextLength);
                    nextDest += pidTextLength;
                    strncpy(nextDest, pidFormat + 4, pathCharactersAvailable);
                }
            }
            formattedPath[maxPathLength] = '\0';
            pathToOpen = formattedPath;
        }

        file = FilePrintStream::open(pathToOpen, "w").release();
        if (file)
            WTFLogAlways("*** DataLog output to \"%s\" ***\n", pathToOpen);
        else
            WTFLogAlways("Warning: Could not open DataLog file %s for writing.\n", pathToOpen);
    }

    if (!file) {
        // Use placement new; this makes it easier to use dataLog() to debug
        // fastMalloc.
        file = new (s_fileData) FilePrintStream(stderr, FilePrintStream::Borrow);
    }

    setvbuf(file->file(), 0, _IONBF, 0); // Prefer unbuffered output, so that we get a full log upon crash or deadlock.

    if (s_file)
        s_file->flush();

    s_file = new (s_lockedFileData) LockedPrintStream(std::unique_ptr<FilePrintStream>(file));
}

PrintStream& dataFile()
{
    initializeLogFile();
    return *s_file;
}

void dataLogFV(const char* format, va_list argList)
{
    dataFile().vprintf(format, argList);
}

void dataLogF(const char* format, ...)
{
    va_list argList;
    va_start(argList, format);
    dataLogFV(format, argList);
    va_end(argList);
}

void dataLogFString(const char* str)
{
    dataFile().printf("%s", str);
}

} // namespace WTF

