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
#ifndef WTF_DisallowCType_h
#define WTF_DisallowCType_h

// The behavior of many of the functions in the <ctype.h> header is dependent
// on the current locale. But almost all uses of these functions are for
// locale-independent, ASCII-specific purposes. In WebKit code we use our own
// ASCII-specific functions instead. This header makes sure we get a compile-time
// error if we use one of the <ctype.h> functions by accident.

// this breaks compilation of <QFontDatabase>, at least, so turn it off for now
// Also generates errors on wx on Windows, presumably because these functions
// are used from wx headers. On GTK+ for Mac many GTK+ files include <libintl.h>
// or <glib/gi18n-lib.h>, which in turn include <xlocale/_ctype.h> which uses
// isacii(). 
#if !(OS(DARWIN) && PLATFORM(GTK)) && !defined(_LIBCPP_VERSION) && defined(__GLIBC__)

#include <ctype.h>

#undef isalnum
#undef isalpha
#undef isascii
#undef isblank
#undef iscntrl
#undef isdigit
#undef isgraph
#undef islower
#undef isprint
#undef ispunct
#undef isspace
#undef isupper
#undef isxdigit
#undef toascii
#undef tolower
#undef toupper

#define isalnum isalnum_WTF_Please_use_ASCIICType_instead_of_ctype_see_comment_in_ASCIICType_h
#define isalpha isalpha_WTF_Please_use_ASCIICType_instead_of_ctype_see_comment_in_ASCIICType_h
#define isascii isascii_WTF_Please_use_ASCIICType_instead_of_ctype_see_comment_in_ASCIICType_h
#define isblank isblank_WTF_Please_use_ASCIICType_instead_of_ctype_see_comment_in_ASCIICType_h
#define iscntrl iscntrl_WTF_Please_use_ASCIICType_instead_of_ctype_see_comment_in_ASCIICType_h
#define isdigit isdigit_WTF_Please_use_ASCIICType_instead_of_ctype_see_comment_in_ASCIICType_h
#define isgraph isgraph_WTF_Please_use_ASCIICType_instead_of_ctype_see_comment_in_ASCIICType_h
#define islower islower_WTF_Please_use_ASCIICType_instead_of_ctype_see_comment_in_ASCIICType_h
#define isprint isprint_WTF_Please_use_ASCIICType_instead_of_ctype_see_comment_in_ASCIICType_h
#define ispunct ispunct_WTF_Please_use_ASCIICType_instead_of_ctype_see_comment_in_ASCIICType_h
#define isspace isspace_WTF_Please_use_ASCIICType_instead_of_ctype_see_comment_in_ASCIICType_h
#define isupper isupper_WTF_Please_use_ASCIICType_instead_of_ctype_see_comment_in_ASCIICType_h
#define isxdigit isxdigit_WTF_Please_use_ASCIICType_instead_of_ctype_see_comment_in_ASCIICType_h
#define toascii toascii_WTF_Please_use_ASCIICType_instead_of_ctype_see_comment_in_ASCIICType_h
#define tolower tolower_WTF_Please_use_ASCIICType_instead_of_ctype_see_comment_in_ASCIICType_h
#define toupper toupper_WTF_Please_use_ASCIICType_instead_of_ctype_see_comment_in_ASCIICType_h

#endif

#endif
