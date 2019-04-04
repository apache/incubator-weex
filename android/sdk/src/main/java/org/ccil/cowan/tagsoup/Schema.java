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
// Model of document
package org.ccil.cowan.tagsoup;
import java.util.HashMap;
import java.util.Locale;
/**
 Abstract class representing a TSSL schema.
 Actual TSSL schemas are compiled into concrete subclasses of this class.
 **/
public abstract class Schema {
    public static final int M_ANY = 0xFFFFFFFF;
    public static final int M_EMPTY = 0;
    public static final int M_PCDATA = 1 << 30;
    public static final int M_ROOT = 1 << 31;
    public static final int F_RESTART = 1;
    public static final int F_CDATA = 2;
    public static final int F_NOFORCE = 4;
    private HashMap theEntities =
            new HashMap();		// String -> Character
    private HashMap theElementTypes =
            new HashMap();		// String -> ElementType
    private String theURI = "";
    private String thePrefix = "";
    private ElementType theRoot = null;
    /**
     Add or replace an element type for this schema.
     @param name Name (Qname) of the element
     @param model Models of the element's content as a vector of bits
     @param memberOf Models the element is a member of as a vector of bits
     @param flags Flags for the element
     **/
    public void elementType(String name, int model, int memberOf, int flags) {
        ElementType e = new ElementType(name, model, memberOf, flags, this);
        theElementTypes.put(name.toLowerCase(Locale.ROOT), e);
        if (memberOf == M_ROOT) theRoot = e;
    }
    /**
     Get the root element of this schema
     **/
    public ElementType rootElementType() {
        return theRoot;
    }
    /**
     Add or replace a default attribute for an element type in this schema.
     @param elemName Name (Qname) of the element type
     @param attrName Name (Qname) of the attribute
     @param type Type of the attribute
     @param value Default value of the attribute; null if no default
     **/
    public void attribute(String elemName, String attrName,
                          String type, String value) {
        ElementType e = getElementType(elemName);
        if (e == null) {
            throw new Error("Attribute " + attrName +
                    " specified for unknown element type " +
                    elemName);
        }
        e.setAttribute(attrName, type, value);
    }
    /**
     Specify natural parent of an element in this schema.
     @param name Name of the child element
     @param parentName Name of the parent element
     **/
    public void parent(String name, String parentName) {
        ElementType child = getElementType(name);
        ElementType parent = getElementType(parentName);
        if (child == null) {
            throw new Error("No child " + name + " for parent " + parentName);
        }
        if (parent == null) {
            throw new Error("No parent " + parentName + " for child " + name);
        }
        child.setParent(parent);
    }
    /**
     Add to or replace a character entity in this schema.
     @param name Name of the entity
     @param value Value of the entity
     **/
    public void entity(String name, int value) {
        theEntities.put(name, new Integer(value));
    }
    /**
     Get an ElementType by name.
     @param name Name (Qname) of the element type
     @return The corresponding ElementType
     **/
    public ElementType getElementType(String name) {
        return (ElementType)(theElementTypes.get(name.toLowerCase(Locale.ROOT)));
    }
    /**
     Get an entity value by name.
     @param name Name of the entity
     @return The corresponding character, or 0 if none
     **/
    public int getEntity(String name) {
//		System.err.println("%% Looking up entity " + name);
        Integer ch = (Integer)theEntities.get(name);
        if (ch == null) return 0;
        return ch.intValue();
    }
    /**
     Return the URI (namespace name) of this schema.
     **/
    public String getURI() {
        return theURI;
    }
    /**
     Return the prefix of this schema.
     **/
    public String getPrefix() {
        return thePrefix;
    }
    /**
     Change the URI (namespace name) of this schema.
     **/
    public void setURI(String uri) {
        theURI = uri;
    }
    /**
     Change the prefix of this schema.
     **/
    public void setPrefix(String prefix) {
        thePrefix = prefix;
    }
}