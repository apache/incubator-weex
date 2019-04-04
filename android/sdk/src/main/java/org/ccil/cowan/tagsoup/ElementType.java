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
package org.ccil.cowan.tagsoup;
/**
 This class represents an element type in the schema.
 An element type has a name, a content model vector, a member-of vector,
 a flags vector, default attributes, and a schema to which it belongs.
 @see Schema
 */
public class ElementType {
    private String theName;		// element type name (Qname)
    private String theNamespace;	// element type namespace name
    private String theLocalName;	// element type local name
    private int theModel;		// bitmap: what the element contains
    private int theMemberOf;	// bitmap: what element is contained in
    private int theFlags;		// bitmap: element flags
    private AttributesImpl theAtts;	// default attributes
    private ElementType theParent;	// parent of this element type
    private Schema theSchema;	// schema to which this belongs
    /**
     Construct an ElementType:
     but it's better to use Schema.element() instead.
     The content model, member-of, and flags vectors are specified as ints.
     @param name The element type name
     @param model ORed-together bits representing the content models
     allowed in the content of this element type
     @param memberOf ORed-together bits representing the content models
     to which this element type belongs
     @param flags ORed-together bits representing the flags associated
     with this element type
     @param schema The schema with which this element type will be
     associated
     */
    public ElementType(String name, int model, int memberOf, int flags, Schema schema) {
        theName = name;
        theModel = model;
        theMemberOf = memberOf;
        theFlags = flags;
        theAtts = new AttributesImpl();
        theSchema = schema;
        theNamespace = namespace(name, false);
        theLocalName = localName(name);
    }
    /**
     Return a namespace name from a Qname.
     The attribute flag tells us whether to return an empty namespace
     name if there is no prefix, or use the schema default instead.
     @param name The Qname
     @param attribute True if name is an attribute name
     @return The namespace name
     **/
    public String namespace(String name, boolean attribute) {
        int colon = name.indexOf(':');
        if (colon == -1) {
            return attribute ? "" : theSchema.getURI();
        }
        String prefix = name.substring(0, colon);
        if (prefix.equals("xml")) {
            return "http://www.w3.org/XML/1998/namespace";
        }
        else {
            return ("urn:x-prefix:" + prefix).intern();
        }
    }
    /**
     Return a local name from a Qname.
     @param name The Qname
     @return The local name
     **/
    public String localName(String name) {
        int colon = name.indexOf(':');
        if (colon == -1) {
            return name;
        }
        else {
            return name.substring(colon+1).intern();
        }
    }
    /**
     Returns the name of this element type.
     @return The name of the element type
     */
    public String name() { return theName; }
    /**
     Returns the namespace name of this element type.
     @return The namespace name of the element type
     */
    public String namespace() { return theNamespace; }
    /**
     Returns the local name of this element type.
     @return The local name of the element type
     */
    public String localName() { return theLocalName; }
    /**
     Returns the content models of this element type.
     @return The content models of this element type as a vector of bits
     */
    public int model() { return theModel; }
    /**
     Returns the content models to which this element type belongs.
     @return The content models to which this element type belongs as a
     vector of bits
     */
    public int memberOf() { return theMemberOf; }
    /**
     Returns the flags associated with this element type.
     @return The flags associated with this element type as a vector of bits
     */
    public int flags() { return theFlags; }
    /**
     Returns the default attributes associated with this element type.
     Attributes of type CDATA that don't have default values are
     typically not included.  Other attributes without default values
     have an internal value of <tt>null</tt>.
     The return value is an AttributesImpl to allow the caller to mutate
     the attributes.
     */
    public AttributesImpl atts() {return theAtts;}
    /**
     Returns the parent element type of this element type.
     @return The parent element type
     */
    public ElementType parent() {return theParent;}
    /**
     Returns the schema which this element type is associated with.
     @return The schema
     */
    public Schema schema() {return theSchema;}
    /**
     Returns true if this element type can contain another element type.
     That is, if any of the models in this element's model vector
     match any of the models in the other element type's member-of
     vector.
     @param other The other element type
     */
    public boolean canContain(ElementType other) {
        return (theModel & other.theMemberOf) != 0;
    }
    /**
     Sets an attribute and its value into an AttributesImpl object.
     Attempts to set a namespace declaration are ignored.
     @param atts The AttributesImpl object
     @param name The name (Qname) of the attribute
     @param type The type of the attribute
     @param value The value of the attribute
     */
    public void setAttribute(AttributesImpl atts, String name, String type, String value) {
        if (name.equals("xmlns") || name.startsWith("xmlns:")) {
            return;
        }
        ;
        String namespace = namespace(name, true);
        String localName = localName(name);
        int i = atts.getIndex(name);
        if (i == -1) {
            name = name.intern();
            if (type == null) type = "CDATA";
            if (!type.equals("CDATA")) value = normalize(value);
            atts.addAttribute(namespace, localName, name, type, value);
        }
        else {
            if (type == null) type = atts.getType(i);
            if (!type.equals("CDATA")) value=normalize(value);
            atts.setAttribute(i, namespace, localName, name, type, value);
        }
    }
    /**
     Normalize an attribute value (ID-style).
     CDATA-style attribute normalization is already done.
     @param value The value to normalize
     @return The normalized value
     **/
    public static String normalize(String value) {
        if (value == null) return value;
        value = value.trim();
        if (value.indexOf("  ") == -1) return value;
        boolean space = false;
        int len = value.length();
        StringBuffer b = new StringBuffer(len);
        for (int i = 0; i < len; i++) {
            char v = value.charAt(i);
            if (v == ' ') {
                if (!space) b.append(v);
                space = true;
            }
            else {
                b.append(v);
                space = false;
            }
        }
        return b.toString();
    }
    /**
     Sets an attribute and its value into this element type.
     @param name The name of the attribute
     @param type The type of the attribute
     @param value The value of the attribute
     */
    public void setAttribute(String name, String type, String value) {
        setAttribute(theAtts, name, type, value);
    }
    /**
     Sets the models of this element type.
     @param model The content models of this element type as a vector of bits
     */
    public void setModel(int model) { theModel = model; }
    /**
     Sets the content models to which this element type belongs.
     @param memberOf The content models to which this element type belongs as a vector of bits
     */
    public void setMemberOf(int memberOf) { theMemberOf = memberOf; }
    /**
     Sets the flags of this element type.
     @param flags associated with this element type The flags as a vector of bits
     */
    public void setFlags(int flags) { theFlags = flags; }
    /**
     Sets the parent element type of this element type.
     @param parent The parent element type
     */
    public void setParent(ElementType parent) { theParent = parent; }
}