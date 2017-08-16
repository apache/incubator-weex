/*
 * Copyright (C) 2004, 2005, 2006, 2007, 2008, 2013 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Patrick Gansterer <paroga@paroga.com>
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"
#include "AtomicStringTable.h"

#include <wtf/HashSet.h>
#include <wtf/MainThread.h>
#include <wtf/WTFThreadData.h>

namespace WTF {

void AtomicStringTable::create(WTFThreadData& data)
{
#if USE(WEB_THREAD)
    // On iOS, one AtomicStringTable is shared between the main UI thread and the WebThread.
    static AtomicStringTable* sharedStringTable = new AtomicStringTable;

    bool currentThreadIsWebThread = isWebThread();
    if (currentThreadIsWebThread || isUIThread())
        data.m_defaultAtomicStringTable = sharedStringTable;
    else
        data.m_defaultAtomicStringTable = new AtomicStringTable;

    // We do the following so that its destruction happens only
    // once - on the main UI thread.
    if (!currentThreadIsWebThread)
        data.m_atomicStringTableDestructor = AtomicStringTable::destroy;
#else
    data.m_defaultAtomicStringTable = new AtomicStringTable;
    data.m_atomicStringTableDestructor = AtomicStringTable::destroy;
#endif // USE(WEB_THREAD)
}

AtomicStringTable::~AtomicStringTable()
{
    for (auto* string : m_table)
        string->setIsAtomic(false);
}

void AtomicStringTable::destroy(AtomicStringTable* table)
{
    delete table;
}

}
