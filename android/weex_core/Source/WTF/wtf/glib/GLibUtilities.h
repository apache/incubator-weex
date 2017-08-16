/*
 * Copyright (C) 2010 Igalia, S.L.
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
 */

#ifndef GLibUtilities_h
#define GLibUtilities_h

#include <wtf/Assertions.h>
#include <wtf/text/CString.h>

CString getCurrentExecutablePath();

// These might be added to glib in the future, but in the meantime they're defined here.
#ifndef GULONG_TO_POINTER
#define GULONG_TO_POINTER(ul) ((gpointer) (gulong) (ul))
#endif

#ifndef GPOINTER_TO_ULONG
#define GPOINTER_TO_ULONG(p) ((gulong) (p))
#endif

#endif
