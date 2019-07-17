/*
 * Copyright (C) 2012 Patrick Gansterer <paroga@paroga.com>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WindowsExtras_h
#define WindowsExtras_h

#if OS(WINDOWS)

#include <windows.h>
#include <objbase.h>
#include <shlwapi.h>

#ifndef HWND_MESSAGE
const HWND HWND_MESSAGE = 0;
#endif

namespace WTF {

inline HRESULT getRegistryValue(HKEY hkey, LPCWSTR pszSubKey, LPCWSTR pszValue, LPDWORD pdwType, LPVOID pvData, LPDWORD pcbData)
{
    return ::SHGetValueW(hkey, pszSubKey, pszValue, pdwType, pvData, pcbData);
}

inline void* getWindowPointer(HWND hWnd, int index)
{
    return reinterpret_cast<void*>(::GetWindowLongPtr(hWnd, index));
}

inline void* setWindowPointer(HWND hWnd, int index, void* value)
{
    return reinterpret_cast<void*>(::SetWindowLongPtr(hWnd, index, reinterpret_cast<LONG_PTR>(value)));
}

} // namespace WTF

using WTF::getRegistryValue;
using WTF::getWindowPointer;
using WTF::setWindowPointer;

#endif // OS(WINDOWS)

#endif // WindowsExtras_h
