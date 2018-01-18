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
package com.taobao.weex.wson;


import android.support.v4.util.LruCache;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.taobao.weex.utils.WXLogUtils;

import java.lang.reflect.Array;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.math.BigDecimal;
import java.math.BigInteger;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Collection;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * fast binary json format for parse map and serialize map
 * Created by efurture on 2017/8/16.
 */
public class Wson {

    /**
     * skip map null values
     * */
    public static final boolean WriteMapNullValue = false;
    /**
     * wson data type
     * */
    private static final byte NULL_TYPE = '0';

    private static final byte STRING_TYPE = 's';

    private static final byte BOOLEAN_TYPE_TRUE = 't';

    private static final byte BOOLEAN_TYPE_FALSE = 'f';

    private static final byte NUMBER_INT_TYPE = 'i';

    private static final byte NUMBER_LONG_TYPE = 'l';

    private static final byte NUMBER_BIG_INTEGER_TYPE = 'g';

    private static final byte NUMBER_BIG_DECIMAL_TYPE = 'e';

    private static final byte NUMBER_DOUBLE_TYPE = 'd';

    private static final byte NUMBER_FLOAT_TYPE = 'F';

    private static final byte ARRAY_TYPE = '[';

    private static final byte MAP_TYPE = '{';

    /**
     * StringUTF-16, byte order with native byte order
     * */
    private static final boolean IS_NATIVE_LITTLE_ENDIAN = (ByteOrder.nativeOrder() == ByteOrder.LITTLE_ENDIAN);


    /**
     * parse wson data  to object, please use WXJsonUtils.parseWson
     * @param  data  byte array
     * */
    public static final Object parse(byte[] data){
        if(data == null){
            return  null;
        }
        try{
            Parser parser =  new Parser(data);
            Object object = parser.parse();
            parser.close();
            return object;
        }catch (Exception e){
            WXLogUtils.e("parseWson", e);
            return  null;
        }
    }


    /**
     * serialize object to wson data, please use WXJsonUtils.wsonWXJSObject
     * */
    public static final byte[] toWson(Object object){
        if(object == null){
            return  null;
        }
        Builder builder = new Builder();
        byte[]  bts  = builder.toWson(object);
        builder.close();
        return bts;
    }


    /**
     * wson data parser
     * */
    private static final class Parser {

        private int position = 0;
        private byte[] buffer;
        private char[]  charsBuffer;

        private Parser(byte[] buffer) {
            this.buffer = buffer;
            charsBuffer = localCharsBufferCache.get();
            if(charsBuffer != null){
                localCharsBufferCache.set(null);
            }else{
                charsBuffer = new char[512];
            }
        }


        private   final Object parse(){
            return  readObject();
        }

        private final void close(){
            position = 0;
            buffer = null;
            if(charsBuffer != null){
                localCharsBufferCache.set(charsBuffer);
            }
            charsBuffer = null;
        }

        private final Object readObject(){
            byte type  = readType();
            switch (type){
                case STRING_TYPE:
                    return readUTF16String();
                case NUMBER_INT_TYPE :
                    return  readVarInt();
                case NUMBER_FLOAT_TYPE :
                    return  readFloat();
                case MAP_TYPE:
                    return readMap();
                case ARRAY_TYPE:
                    return readArray();
                case NUMBER_DOUBLE_TYPE :
                    return readDouble();
                case NUMBER_LONG_TYPE :
                    return  readLong();
                case NUMBER_BIG_INTEGER_TYPE :
                    return  new BigInteger(readUTF16String());
                case NUMBER_BIG_DECIMAL_TYPE :
                    return  new BigDecimal(readUTF16String());
                case BOOLEAN_TYPE_FALSE:
                    return  Boolean.FALSE;
                case BOOLEAN_TYPE_TRUE:
                    return  Boolean.TRUE;
                case NULL_TYPE:
                    return  null;
                default:
                    throw new RuntimeException("wson unhandled type " + type + " " +
                            position  +  " length " + buffer.length);
            }
        }



        private final Object readMap(){
            int size = readUInt();
            Map<String, Object> object = new JSONObject();;
            for(int i=0; i<size; i++){
                String key = readMapKeyUTF16();
                Object value = readObject();
                object.put(key, value);
            }
            return object;
        }

        private final Object readArray(){
            int length = readUInt();
            List<Object> array = new JSONArray(length);
            for(int i=0; i<length; i++){
                array.add(readObject());
            }
            return  array;
        }

        private  final byte readType(){
            byte type = buffer[position];
            position ++;
            return  type;
        }


        private final String readMapKeyUTF16() {
            int length = readUInt();
            length = length/2;
            if(charsBuffer.length < length){
                charsBuffer = new char[length];
            }
            int hash = 5381;
            if(IS_NATIVE_LITTLE_ENDIAN){
                for(int i=0; i<length; i++){
                    char ch = (char) ((buffer[position] & 0xFF) +
                            (buffer[position + 1] << 8));
                    charsBuffer[i] = (ch);
                    hash = ((hash << 5) + hash)  + ch;
                    position+=2;
                }
            }else{
                for(int i=0; i<length; i++){
                    char ch = (char) ((buffer[position + 1] & 0xFF) +
                            (buffer[position] << 8));
                    charsBuffer[i] = (ch);
                    hash = ((hash << 5) + hash)  + ch;
                    position+=2;
                }
            }
            int globalIndex = (globalStringBytesCache.length - 1)&hash;
            String cache = globalStringBytesCache[globalIndex];
            if(cache != null
                    && cache.length() == length){
                boolean isStringEqual  = true;
                for(int i=0; i<length; i++){
                    if(charsBuffer[i] != cache.charAt(i)){
                        isStringEqual = false;
                        break;
                    }
                }
                if(isStringEqual) {
                    return cache;
                }
            }
            cache = new String(charsBuffer, 0, length);
            if(length < 64) {
                globalStringBytesCache[globalIndex] = cache;
            }
            return  cache;
        }

        private final String readUTF16String(){
            int length = readUInt()/2;
            if(charsBuffer.length < length){
                charsBuffer = new char[length];
            }
            if(IS_NATIVE_LITTLE_ENDIAN){
                for(int i=0; i<length; i++){
                    char ch = (char) ((buffer[position] & 0xFF) +
                            (buffer[position + 1] << 8));
                    charsBuffer[i] = (ch);
                    position+=2;
                }
            }else{
                for(int i=0; i<length; i++){
                    char ch = (char) ((buffer[position + 1] & 0xFF) +
                            (buffer[position] << 8));
                    charsBuffer[i] = (ch);
                    position+=2;
                }
            }
            return  new String(charsBuffer, 0, length);
        }





        private   final int readVarInt(){
            int raw = readUInt();
            // This undoes the trick in putVarInt()
            int num = (((raw << 31) >> 31) ^ raw) >> 1;
            // This extra step lets us deal with the largest signed values by treating
            // negative results from read unsigned methods as like unsigned values.
            // Must re-flip the top bit if the original read value had it set.
            return num ^ (raw & (1 << 31));
        }

        private final  int readUInt(){
            int value = 0;
            int i = 0;
            int b;
            while (((b = buffer[position]) & 0x80) != 0) {
                value |= (b & 0x7F) << i;
                i += 7;
                position+=1;
                if (i > 35) {
                    throw new IllegalArgumentException("Variable length quantity is too long");
                }
            }
            position+=1;
            return value | (b << i);
        }

        private final long readLong(){
            long number = (((buffer[position + 7] & 0xFFL)      ) +
                    ((buffer[position + 6] & 0xFFL) <<  8) +
                    ((buffer[position + 5] & 0xFFL) << 16) +
                    ((buffer[position + 4] & 0xFFL) << 24) +
                    ((buffer[position + 3] & 0xFFL) << 32) +
                    ((buffer[position + 2] & 0xFFL) << 40) +
                    ((buffer[position + 1] & 0xFFL) << 48) +
                    (((long) buffer[position])      << 56));
            position += 8;
            return  number;
        }

        private  final Object readDouble(){
            double number = Double.longBitsToDouble(readLong());
            if(number > Integer.MAX_VALUE){
                long numberLong = (long) number;
                double doubleLong = (numberLong);
                if(number - doubleLong < Double.MIN_NORMAL){
                    return numberLong;
                }
            }
            return  number;
        }

        private Object readFloat() {
            int number = (((buffer[position + 3] & 0xFF)      ) +
                    ((buffer[position + 2] & 0xFF) <<  8) +
                    ((buffer[position + 1] & 0xFF) << 16) +
                    ((buffer[position  ] & 0xFF) << 24));
            position +=4;
            return  Float.intBitsToFloat(number);
        }
    }

    /**
     * wson builder
     * */
    private static final class Builder {

        private byte[] buffer;
        private int position;
        private ArrayList refs;
        private final static ThreadLocal<byte[]> bufLocal = new ThreadLocal<byte[]>();
        private final static ThreadLocal<ArrayList> refsLocal = new ThreadLocal<ArrayList>();



        private Builder(){
            buffer =  bufLocal.get();
            if(buffer != null) {
                bufLocal.set(null);
            }else{
                buffer = new byte[1024];
            }
            refs = refsLocal.get();
            if(refs != null){
                refsLocal.set(null);
            }else{
                refs = new ArrayList<>(16);
            }
        }


        private final byte[] toWson(Object object){
            writeObject(object);
            byte[] bts = new byte[position];
            System.arraycopy(buffer, 0, bts, 0, position);
            return  bts;
        }

        private final void close(){
            if(buffer.length <= 1024*16){
                bufLocal.set(buffer);
            }
            if(refs.isEmpty()){
                refsLocal.set(refs);
            }else{
                refs.clear();
            }
            refs = null;
            buffer = null;
            position = 0;
        }

        private final void writeObject(Object object) {
            if(object instanceof  CharSequence){
                ensureCapacity(2);
                writeByte(STRING_TYPE);
                writeUTF16String((CharSequence) object);
                return;
            }else if (object instanceof Map){
                if(refs.contains(object)){
                    ensureCapacity(2);
                    writeByte(NULL_TYPE);
                    return;
                }
                refs.add(object);
                Map map = (Map) object;
                writeMap(map);
                refs.remove(refs.size()-1);
                return;
            }else if (object instanceof List){
                if(refs.contains(object)){
                    ensureCapacity(2);
                    writeByte(NULL_TYPE);
                    return;
                }
                refs.add(object);
                ensureCapacity(8);
                List list = (List) object;
                writeByte(ARRAY_TYPE);
                writeUInt(list.size());
                for(Object value : list){
                    writeObject(value);
                }
                refs.remove(refs.size()-1);
                return;
            }else if (object instanceof Number){
                Number number = (Number) object;
                writeNumber(number);
                return;
            }else if (object instanceof  Boolean){
                ensureCapacity(2);
                Boolean value  = (Boolean) object;
                if(value){
                    writeByte(BOOLEAN_TYPE_TRUE);
                }else{
                    writeByte(BOOLEAN_TYPE_FALSE);
                }
                return;
            }else if(object == null){
                ensureCapacity(2);
                writeByte(NULL_TYPE);
                return;
            }else if (object.getClass().isArray()){
                if(refs.contains(object)){
                    ensureCapacity(2);
                    writeByte(NULL_TYPE);
                    return;
                }
                refs.add(object);
                ensureCapacity(8);
                int length = Array.getLength(object);
                writeByte(ARRAY_TYPE);
                writeUInt(length);
                for(int i=0; i<length; i++){
                    Object value = Array.get(object, i);
                    writeObject(value);
                }
                refs.remove(refs.size()-1);
                return;
            }else  if(object instanceof  Date){
                ensureCapacity(10);
                double date = ((Date)object).getTime();
                writeByte(NUMBER_DOUBLE_TYPE);
                writeDouble(date);
            }else  if(object instanceof  Calendar){
                ensureCapacity(10);
                double date = ((Calendar)object).getTime().getTime();
                writeByte(NUMBER_DOUBLE_TYPE);
                writeDouble(date);
            }else  if(object instanceof  Collection){
                if(refs.contains(object)){
                    ensureCapacity(2);
                    writeByte(NULL_TYPE);
                    return;
                }
                refs.add(object);
                ensureCapacity(8);
                Collection list = (Collection) object;
                writeByte(ARRAY_TYPE);
                writeUInt(list.size());
                for(Object value : list){
                    writeObject(value);
                }
                refs.remove(refs.size()-1);
            }else{
                if(refs.contains(object)){
                    ensureCapacity(2);
                    writeByte(NULL_TYPE);
                }else {
                    refs.add(object);
                    if(object.getClass().isEnum()){
                        writeObject(JSON.toJSONString(object));
                    }else{
                        writeMap(toMap(object));
                    }
                    refs.remove(refs.size()-1);
                }
                return;
            }
        }

        private final void writeNumber(Number number) {
            ensureCapacity(12);
            if(number instanceof  Integer){
                writeByte(NUMBER_INT_TYPE);
                writeVarInt(number.intValue());
                return;
            }

            if(number instanceof Float){
                writeByte(NUMBER_FLOAT_TYPE);
                writeFloat(number.floatValue());
                return;
            }
            if(number instanceof  Double){
                writeByte(NUMBER_DOUBLE_TYPE);
                writeDouble(number.doubleValue());
                return;
            }

            if(number instanceof  Long){
                writeByte(NUMBER_LONG_TYPE);
                writeLong(number.longValue());
                return;
            }

            if(number instanceof  Short
                    || number instanceof  Byte){
                writeByte(NUMBER_INT_TYPE);
                writeVarInt(number.intValue());
                return;
            }

            if(number instanceof BigInteger){
                writeByte(NUMBER_BIG_INTEGER_TYPE);
                writeUTF16String(number.toString());
                return;
            }

            if(number instanceof BigDecimal){
                String value = number.toString();
                double doubleValue = number.doubleValue();
                if(value.equals(Double.toString(doubleValue))){
                    writeByte(NUMBER_DOUBLE_TYPE);
                    writeDouble(doubleValue);
                }else {
                    writeByte(NUMBER_BIG_DECIMAL_TYPE);
                    writeUTF16String(value);
                }
                return;
            }
            writeByte(STRING_TYPE);
            writeUTF16String(number.toString());

        }

        private final  void writeMap(Map map) {
            if(WriteMapNullValue){
                ensureCapacity(8);
                writeByte(MAP_TYPE);
                writeUInt(map.size());
                Set<Map.Entry<Object,Object>>  entries = map.entrySet();
                for(Map.Entry<Object,Object> entry : entries){
                    writeMapKeyUTF16(entry.getKey().toString());
                    writeObject(entry.getValue());
                }
            }else{
                Set<Map.Entry<Object,Object>>  entries = map.entrySet();
                int nullValueSize = 0;
                for(Map.Entry<Object,Object> entry : entries){
                    if(entry.getValue() == null){
                        nullValueSize++;
                    }
                }

                ensureCapacity(8);
                writeByte(MAP_TYPE);
                writeUInt(map.size()-nullValueSize);
                for(Map.Entry<Object,Object> entry : entries){
                    if(entry.getValue() == null){
                        continue;
                    }
                    writeMapKeyUTF16(entry.getKey().toString());
                    writeObject(entry.getValue());
                }
            }
        }


        private final void writeByte(byte type){
            buffer[position] = type;
            position++;
        }

        private  final Map  toMap(Object object){
            Map map = new JSONObject();
            try {
                Class<?> targetClass = object.getClass();
                String key = targetClass.getName();
                List<Method> methods = getBeanMethod(key, targetClass);
                for (Method method : methods) {
                    String methodName = method.getName();
                    if (methodName.startsWith(METHOD_PREFIX_GET)) {
                        Object value = method.invoke(object);
                        if(value != null){
                            StringBuilder builder = new StringBuilder(method.getName().substring(3));
                            builder.setCharAt(0, Character.toLowerCase(builder.charAt(0)));
                            map.put(builder.toString(), (Object) value);
                        }
                    }else if(methodName.startsWith(METHOD_PREFIX_IS)){
                        Object value = method.invoke(object);
                        if(value != null){
                            StringBuilder builder = new StringBuilder(method.getName().substring(2));
                            builder.setCharAt(0, Character.toLowerCase(builder.charAt(0)));
                            map.put(builder.toString(), value);
                        }
                    }
                }
                List<Field> fields = getBeanFields(key, targetClass);
                for(Field field : fields){
                    String fieldName = field.getName();
                    if(map.containsKey(fieldName)){
                        continue;
                    }
                    Object value  = field.get(object);
                    if(value == null){
                        continue;
                    }
                    map.put(fieldName, value);
                }
            }catch (Exception e){
                throw  new RuntimeException(e);
            }
            return  map;
        }

        private  final void writeMapKeyUTF16(String value){
            writeUTF16String(value);
        }




        /**
         * writeString UTF-16
         * */
        private  final void writeUTF16String(CharSequence value){
            int length = value.length();
            ensureCapacity(length*2 + 8);
            writeUInt(length*2);
            if(IS_NATIVE_LITTLE_ENDIAN){
                for(int i=0; i<length; i++){
                    char ch = value.charAt(i);
                    buffer[position] = (byte) (ch);
                    buffer[position+1] = (byte) (ch >>> 8);
                    position+=2;
                }
            }else{
                for(int i=0; i<length; i++){
                    char ch = value.charAt(i);
                    buffer[position + 1] = (byte) (ch      );
                    buffer[position] = (byte) (ch >>> 8);
                    position+=2;
                }
            }
        }


        private final void writeDouble(double value){
            writeLong(Double.doubleToLongBits(value));
        }

        private final void writeFloat(float value){
            int val = Float.floatToIntBits(value);
            buffer[position + 3] = (byte) (val       );
            buffer[position + 2] = (byte) (val >>>  8);
            buffer[position + 1] = (byte) (val >>> 16);
            buffer[position ] = (byte) (val >>> 24);
            position += 4;
        }

        private final void writeLong(long val){
            buffer[position + 7] = (byte) (val       );
            buffer[position + 6] = (byte) (val >>>  8);
            buffer[position + 5] = (byte) (val >>> 16);
            buffer[position + 4] = (byte) (val >>> 24);
            buffer[position + 3] = (byte) (val >>> 32);
            buffer[position + 2] = (byte) (val >>> 40);
            buffer[position + 1] = (byte) (val >>> 48);
            buffer[position    ] = (byte) (val >>> 56);
            position += 8;
        }

        private final void writeVarInt(int value){
            writeUInt((value << 1) ^ (value >> 31));
        }

        private final void  writeUInt(int value){
            while ((value & 0xFFFFFF80) != 0) {
                buffer[position] = (byte)((value & 0x7F) | 0x80);
                position++;
                value >>>= 7;
            }
            buffer[position] = (byte)(value & 0x7F);
            position++;
        }


        private final void ensureCapacity(int minCapacity) {
            minCapacity += position;
            // overflow-conscious code
            if (minCapacity - buffer.length > 0){
                int oldCapacity = buffer.length;
                int newCapacity = oldCapacity << 1;
                if(newCapacity < 1024*16){
                    newCapacity = 1024*16;
                }
                if (newCapacity - minCapacity < 0) {
                    newCapacity = minCapacity;
                }
                buffer = Arrays.copyOf(buffer, newCapacity);
            }
        }
    }


    /**
     * cache json property key, most of them all same
     * */
    private static final int GLOBAL_STRING_CACHE_SIZE = 2*1024;
    private static final ThreadLocal<char[]> localCharsBufferCache = new ThreadLocal<>();
    private static final String[] globalStringBytesCache = new String[GLOBAL_STRING_CACHE_SIZE];




    /**
     * lru cache, to map helper
     * */
    private static final String METHOD_PREFIX_GET = "get";
    private static final String METHOD_PREFIX_IS = "is";
    private static LruCache<String, List<Method>> methodsCache = new LruCache<>(128);
    private static LruCache<String, List<Field>> fieldsCache = new LruCache<>(128);


    private static final List<Method> getBeanMethod(String key, Class targetClass){
        List<Method> methods = methodsCache.get(key);
        if(methods == null){
            methods = new ArrayList<>();
            Method[]  allMethods = targetClass.getMethods();
            for(Method method : allMethods){
                if(method.getDeclaringClass() == Object.class){
                    continue;
                }
                if( (method.getModifiers() & Modifier.STATIC) != 0){
                    continue;
                }
                String methodName = method.getName();
                if(methodName.startsWith(METHOD_PREFIX_GET)
                        || methodName.startsWith(METHOD_PREFIX_IS)) {
                    methods.add(method);
                }
            }
            methodsCache.put(key, methods);
        }
        return methods;
    }



    private static  final List<Field> getBeanFields(String key, Class targetClass){
        List<Field> fieldList = fieldsCache.get(key);
        if(fieldList == null) {
            Field[] fields = targetClass.getFields();
            fieldList = new ArrayList<>(fields.length);
            for(Field field : fields){
                if((field.getModifiers() & Modifier.STATIC) != 0){
                    continue;
                }
                fieldList.add(field);
            }
            fieldsCache.put(key, fieldList);
        }
        return  fieldList;
    }

}
