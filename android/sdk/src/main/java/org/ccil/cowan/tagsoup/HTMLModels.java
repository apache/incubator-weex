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
// This file is part of TagSoup and is Copyright 2002-2008 by John Cowan.
//
// TagSoup is licensed under the Apache License,
// Version 2.0.  You may obtain a copy of this license at
// http://www.apache.org/licenses/LICENSE-2.0 .  You may also have
// additional legal rights not granted by this license.
//
// TagSoup is distributed in the hope that it will be useful, but
// unless required by applicable law or agreed to in writing, TagSoup
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
// OF ANY KIND, either express or implied; not even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// 
// 
// Defines models for HTMLSchema
/**
 This interface contains generated constants representing HTML content
 models.  Logically, it is part of HTMLSchema, but it is more
 convenient to generate the constants into a separate interface.
 */
package org.ccil.cowan.tagsoup;
public interface HTMLModels {
    // Start of model definitions
    public static final int M_AREA = 1 << 1;
    public static final int M_BLOCK = 1 << 2;
    public static final int M_BLOCKINLINE = 1 << 3;
    public static final int M_BODY = 1 << 4;
    public static final int M_CELL = 1 << 5;
    public static final int M_COL = 1 << 6;
    public static final int M_DEF = 1 << 7;
    public static final int M_FORM = 1 << 8;
    public static final int M_FRAME = 1 << 9;
    public static final int M_HEAD = 1 << 10;
    public static final int M_HTML = 1 << 11;
    public static final int M_INLINE = 1 << 12;
    public static final int M_LEGEND = 1 << 13;
    public static final int M_LI = 1 << 14;
    public static final int M_NOLINK = 1 << 15;
    public static final int M_OPTION = 1 << 16;
    public static final int M_OPTIONS = 1 << 17;
    public static final int M_P = 1 << 18;
    public static final int M_PARAM = 1 << 19;
    public static final int M_TABLE = 1 << 20;
    public static final int M_TABULAR = 1 << 21;
    public static final int M_TR = 1 << 22;
    // End of model definitions
}