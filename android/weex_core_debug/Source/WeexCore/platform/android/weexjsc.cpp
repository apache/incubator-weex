/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2004-2017 Apple Inc. All rights reserved.
 *  Copyright (C) 2006 Bjoern Graf (bjoern.graf@gmail.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */
#include "weexjsc.h"

namespace WEEXJSC {
extern "C" void soLoad(JavaVM *vm,
    jclass* jBridgeClazz,
    jclass* jWXJSObject,
    jclass* jWXLogUtils)
{
    sVm = vm;
    // for android bridge
    // if (mBridgeAndroid.get())
    // {
    //     mBridgeAndroid->init(sVm, *jBridgeClazz, *jWXJSObject, *jWXLogUtils);
    // }
    __android_log_print(ANDROID_LOG_ERROR, "shiwentao", "weexjsc soLoad");
}
}
