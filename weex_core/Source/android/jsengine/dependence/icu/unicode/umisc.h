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
**********************************************************************
*   Copyright (C) 1999-2006, International Business Machines
*   Corporation and others.  All Rights Reserved.
**********************************************************************
*   file name:  umisc.h
*   encoding:   US-ASCII
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 1999oct15
*   created by: Markus W. Scherer
*/

#ifndef UMISC_H
#define UMISC_H

#include "unicode/utypes.h"

/**
 * \file
 * \brief  C API:misc definitions 
 *
 *  This file contains miscellaneous definitions for the C APIs. 
 */

U_CDECL_BEGIN

/** A struct representing a range of text containing a specific field 
 *  @stable ICU 2.0
 */
typedef struct UFieldPosition {
  /**
   * The field 
   * @stable ICU 2.0
   */
  int32_t field;
  /**
   * The start of the text range containing field 
   * @stable ICU 2.0
   */
  int32_t beginIndex;
  /** 
   * The limit of the text range containing field 
   * @stable ICU 2.0
   */
  int32_t endIndex;
} UFieldPosition;

#if !UCONFIG_NO_SERVICE
/**
 * Opaque type returned by registerInstance, registerFactory and unregister for service registration.
 * @stable ICU 2.6
 */
typedef const void* URegistryKey;
#endif

U_CDECL_END

#endif
