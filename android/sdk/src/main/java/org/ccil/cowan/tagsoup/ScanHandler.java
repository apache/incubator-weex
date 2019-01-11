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
// Scanner handler
package org.ccil.cowan.tagsoup;
import org.xml.sax.SAXException;
/**
 An interface that Scanners use to report events in the input stream.
 **/
public interface ScanHandler {
    /**
     Reports an attribute name without a value.
     **/
    public void adup(char[] buff, int offset, int length) throws SAXException;
    /**
     Reports an attribute name; a value will follow.
     **/
    public void aname(char[] buff, int offset, int length) throws SAXException;
    /**
     Reports an attribute value.
     **/
    public void aval(char[] buff, int offset, int length) throws SAXException;
    /**
     * Reports the content of a CDATA section (not a CDATA element)
     */
    public void cdsect(char[] buff, int offset, int length) throws SAXException;
    /**
     * Reports a <!....> declaration - typically a DOCTYPE
     */
    public void decl(char[] buff, int offset, int length) throws SAXException;
    /**
     Reports an entity reference or character reference.
     **/
    public void entity(char[] buff, int offset, int length) throws SAXException;
    /**
     Reports EOF.
     **/
    public void eof(char[] buff, int offset, int length) throws SAXException;
    /**
     Reports an end-tag.
     **/
    public void etag(char[] buff, int offset, int length) throws SAXException;
    /**
     Reports the general identifier (element type name) of a start-tag.
     **/
    public void gi(char[] buff, int offset, int length) throws SAXException;
    /**
     Reports character content.
     **/
    public void pcdata(char[] buff, int offset, int length) throws SAXException;
    /**
     Reports the data part of a processing instruction.
     **/
    public void pi(char[] buff, int offset, int length) throws SAXException;
    /**
     Reports the target part of a processing instruction.
     **/
    public void pitarget(char[] buff, int offset, int length) throws SAXException;
    /**
     Reports the close of a start-tag.
     **/
    public void stagc(char[] buff, int offset, int length) throws SAXException;
    /**
     Reports the close of an empty-tag.
     **/
    public void stage(char[] buff, int offset, int length) throws SAXException;
    /**
     Reports a comment.
     **/
    public void cmnt(char[] buff, int offset, int length) throws SAXException;
    /**
     Returns the value of the last entity or character reference reported.
     **/
    public int getEntity();
}