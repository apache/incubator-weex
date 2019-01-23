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
#pragma once

#include "HandleTypes.h"

namespace JSC {

/*
    A Handle is a smart pointer that updates automatically when the garbage
    collector moves the object to which it points.

    The base Handle class represents a temporary reference to a pointer whose
    lifetime is guaranteed by something else.
*/

template <class T> class Handle;

// Creating a JSValue Handle is invalid
template <> class Handle<JSValue>;

class HandleBase {
    template <typename T> friend class Weak;
    template <typename T> friend class Strong;
    friend class HandleSet;
    friend struct JSCallbackObjectData;

public:
    bool operator!() const { return !m_slot || !*m_slot; }

    explicit operator bool() const { return m_slot && *m_slot; }

    HandleSlot slot() const { return m_slot; }

protected:
    HandleBase(HandleSlot slot)
        : m_slot(slot)
    {
    }
    
    void swap(HandleBase& other) { std::swap(m_slot, other.m_slot); }

    void setSlot(HandleSlot slot)
    {
        m_slot = slot;
    }

private:
    HandleSlot m_slot;
};

template <typename Base, typename T> struct HandleConverter {
    T* operator->()
    {
        return static_cast<Base*>(this)->get();
    }
    const T* operator->() const
    {
        return static_cast<const Base*>(this)->get();
    }

    T* operator*()
    {
        return static_cast<Base*>(this)->get();
    }
    const T* operator*() const
    {
        return static_cast<const Base*>(this)->get();
    }
};

template <typename Base> struct HandleConverter<Base, Unknown> {
    Handle<JSObject> asObject() const;
    bool isObject() const { return jsValue().isObject(); }
    bool getNumber(double number) const { return jsValue().getNumber(number); }
    WTF::String getString(ExecState*) const;
    bool isUndefinedOrNull() const { return jsValue().isUndefinedOrNull(); }

private:
    JSValue jsValue() const
    {
        return static_cast<const Base*>(this)->get();
    }
};

template <typename T> class Handle : public HandleBase, public HandleConverter<Handle<T>, T> {
public:
    template <typename A, typename B> friend struct HandleConverter;
    typedef typename HandleTypes<T>::ExternalType ExternalType;
    template <typename U> Handle(Handle<U> o)
    {
        typename HandleTypes<T>::template validateUpcast<U>();
        setSlot(o.slot());
    }

    void swap(Handle& other) { HandleBase::swap(other); }

    ExternalType get() const { return HandleTypes<T>::getFromSlot(this->slot()); }

protected:
    Handle(HandleSlot slot = 0)
        : HandleBase(slot)
    {
    }
    
private:
    friend class HandleSet;
    friend class WeakBlock;

    static Handle<T> wrapSlot(HandleSlot slot)
    {
        return Handle<T>(slot);
    }
};

template <typename Base> Handle<JSObject> HandleConverter<Base, Unknown>::asObject() const
{
    return Handle<JSObject>::wrapSlot(static_cast<const Base*>(this)->slot());
}

template <typename T, typename U> inline bool operator==(const Handle<T>& a, const Handle<U>& b)
{ 
    return a.get() == b.get(); 
}

template <typename T, typename U> inline bool operator==(const Handle<T>& a, U* b)
{ 
    return a.get() == b; 
}

template <typename T, typename U> inline bool operator==(T* a, const Handle<U>& b) 
{
    return a == b.get(); 
}

template <typename T, typename U> inline bool operator!=(const Handle<T>& a, const Handle<U>& b)
{ 
    return a.get() != b.get(); 
}

template <typename T, typename U> inline bool operator!=(const Handle<T>& a, U* b)
{
    return a.get() != b; 
}

template <typename T, typename U> inline bool operator!=(T* a, const Handle<U>& b)
{ 
    return a != b.get(); 
}

template <typename T, typename U> inline bool operator!=(const Handle<T>& a, JSValue b)
{
    return a.get() != b; 
}

template <typename T, typename U> inline bool operator!=(JSValue a, const Handle<U>& b)
{ 
    return a != b.get(); 
}

} // namespace JSC
