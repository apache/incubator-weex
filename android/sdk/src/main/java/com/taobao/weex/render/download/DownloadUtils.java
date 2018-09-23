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
package com.taobao.weex.render.download;

import android.util.Log;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.math.BigInteger;
import java.net.URL;
import java.net.URLConnection;
import java.security.MessageDigest;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

/**
 * Created by furture on 2018/9/21.
 */

public class DownloadUtils {

    public static boolean download(String urlStr, String savePath){
        try {
            URL url = new URL(urlStr);
            URLConnection urlConnection = url.openConnection();
            urlConnection.connect();
            InputStream input = new BufferedInputStream(url.openStream());
            File saveFile = new File(savePath);
            if(saveFile.getParentFile() != null && !saveFile.getParentFile().exists()){
                saveFile.getParentFile().mkdirs();
            }
            OutputStream output = new FileOutputStream(saveFile);
            byte data[] = new byte[1024];
            int count;
            while ((count = input.read(data)) != -1) {
                output.write(data, 0, count);
            }
            output.flush();
            output.close();
            input.close();
            return true;
        } catch (Exception e) {
            return false;
        }
    }


    public static boolean unzip(String zipFile, String dir){
        try{
            FileInputStream fin = new FileInputStream(zipFile);
            ZipInputStream zin = new ZipInputStream(fin);
            ZipEntry ze = null;
            while ((ze = zin.getNextEntry()) != null) {
                if(ze.isDirectory()) {
                    continue;
                } else {
                    File file = new File(dir  + File.separator + ze.getName());
                    if(file.getParentFile() != null){
                        if(file.getParentFile().exists()){
                            file.getParentFile().mkdirs();
                        }
                    }
                    FileOutputStream fout = new FileOutputStream(file);
                    byte[] buffer = new byte[1024];
                    int len;
                    while ((len = zin.read(buffer)) != -1) {
                        fout.write(buffer, 0, len);
                    }
                    fout.close();
                    zin.closeEntry();
                }

            }
            zin.close();
            return true;
        }catch (Exception e){
            return false;
        }
    }

    public static boolean deleteFile(String filePath){
        File file = new File(filePath);
        return file.delete();
    }


    public static String fileMd5(String filename){
        try{
            InputStream fis =  new FileInputStream(filename);
            byte[] buffer = new byte[1024];
            MessageDigest complete = MessageDigest.getInstance("MD5");
            int numRead;
            do {
                numRead = fis.read(buffer);
                if (numRead > 0) {
                    complete.update(buffer, 0, numRead);
                }
            } while (numRead != -1);

            fis.close();
            BigInteger bigInt = new BigInteger(1, complete.digest());
            return  bigInt.toString(16).toLowerCase();
        }catch (Exception e){
            return "";
        }
    }
}
