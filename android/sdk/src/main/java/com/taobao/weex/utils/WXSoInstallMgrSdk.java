/*
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
package com.taobao.weex.utils;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.os.Build;
import android.text.TextUtils;

import com.taobao.weex.IWXStatisticsListener;
import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.WXEnvironment;
import com.taobao.weex.adapter.IWXSoLoaderAdapter;
import com.taobao.weex.adapter.IWXUserTrackAdapter;
import com.taobao.weex.common.WXErrorCode;
import com.taobao.weex.common.WXPerformance;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Field;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.util.Enumeration;
import java.util.zip.ZipEntry;
import java.util.zip.ZipException;
import java.util.zip.ZipFile;

import dalvik.system.PathClassLoader;


/**
 * Utility class for managing so library, including load native library and version management.
 * <ol>
 *   <li>Load library<br>
 *     It Will try to use {@link System#loadLibrary(String)} to load native library. If it successes,
 *     the Android Framework will be responsible for managing library and library version.
 *     If it fails in case of some ceratin armebi-v7a architecture device, it will try to extract
 *     native library from apk and copy it the data directory of the app. Then load it using
 *     {@link System#load(String)}.
 *     </li>
 *  <li>
 *       Version control for extracting native library from apk.
 *  </li>
 * </ol>
 */
public class WXSoInstallMgrSdk {

  final static String LOGTAG = "INIT_SO";
  //below is the CPU string types
  private final static String ARMEABI = "armeabi"; //default
  private final static String X86 = "x86";
  private final static String MIPS = "mips";

  private final static int ARMEABI_Size = 3583820;
  private final static int X86_Size = 4340864;

  static Context mContext = null;
  private static IWXSoLoaderAdapter mSoLoader = null;
  private static IWXStatisticsListener mStatisticsListener = null;

  public static void init(Context c,
                          IWXSoLoaderAdapter loader,
                          IWXStatisticsListener listener) {
    mContext = c;
    mSoLoader = loader;
    mStatisticsListener = listener;
  }

  public static boolean isX86(){
    String cpuType = _cpuType();
    return cpuType.equalsIgnoreCase(X86);
  }

  public static boolean isCPUSupport(){
    String cpuType = _cpuType();
    return !cpuType.equalsIgnoreCase(MIPS);
  }

  /**
   * Load so library.
   *
   * If a library loader adapter exists, use this adapter to load library,
   * otherwise use {@link System#loadLibrary(String)} to load library.
   * If failed to load library, try to extract the so library and load it
   * from arembi in the .apk
   *
   * @param libName library name, like webp, not necessary to be libwep.so
   * @param version the version of the so library
   */
  public static boolean initSo(String libName, int version, IWXUserTrackAdapter utAdapter) {
    String cpuType = _cpuType();
    if (cpuType.equalsIgnoreCase(MIPS) ) {
      return false;
    }

    boolean InitSuc = false;
    if (checkSoIsValid(libName, BuildConfig.ARMEABI_Size) ||checkSoIsValid(libName, BuildConfig.X86_Size)) {

      /**
       * Load library with {@link System#loadLibrary(String)}
       */
      try {
        // If a library loader adapter exists, use this adapter to load library
        // instead of System.loadLibrary.
        if (mSoLoader != null) {
          mSoLoader.doLoadLibrary(libName);
        } else {
          System.loadLibrary(libName);
        }
        commit(utAdapter, null, null);

        InitSuc = true;
      } catch (Exception | Error e2) {
        if (cpuType.contains(ARMEABI) || cpuType.contains(X86)) {
          commit(utAdapter, WXErrorCode.WX_ERR_LOAD_SO.getErrorCode(), WXErrorCode.WX_ERR_LOAD_SO.getErrorMsg() + ":" + e2.getMessage());
        }
        InitSuc = false;
      }

      try {

        if (!InitSuc) {

          //File extracted from apk already exists.
          if (isExist(libName, version)) {
            boolean res = _loadUnzipSo(libName, version, utAdapter);
            if (res) {
              return res;
            } else {
              //Delete the corrupt so library, and extract it again.
              removeSoIfExit(libName, version);
            }
          }

          //Fail for loading file from libs, extract so library from so and load it.
          if (cpuType.equalsIgnoreCase(MIPS)) {
            return false;
          } else {
            try {
              InitSuc = unZipSelectedFiles(libName, version, utAdapter);
            } catch (IOException e2) {
              e2.printStackTrace();
            }
          }

        }
      } catch (Exception | Error e) {
        InitSuc = false;
        e.printStackTrace();
      }
    }
    return InitSuc;
  }

  private static String _getFieldReflectively(Build build, String fieldName) {
    try {
      final Field field = Build.class.getField(fieldName);
      return field.get(build).toString();
    } catch (Exception ex) {
      return "Unknown";
    }
  }

  private static String _cpuType() {

    String abi = _getFieldReflectively(new Build(), "CPU_ABI");
    if (abi == null || abi.length() == 0 || abi.equals("Unknown")) {
      abi = ARMEABI;
    }
    abi = abi.toLowerCase();
    return abi;
  }

  /**
   *
   * @param libName lib name
   * @param size  the right size of lib
   * @return true for valid  ; false for InValid
   */
  static boolean checkSoIsValid(String libName, long size) {
    Context context = mContext;
    if (null == context) {
      return false;
    }
    try{
      long start=System.currentTimeMillis();
      if(WXSoInstallMgrSdk.class.getClassLoader() instanceof PathClassLoader ) {

        String path = ((PathClassLoader) (WXSoInstallMgrSdk.class.getClassLoader())).findLibrary(libName);
        if(TextUtils.isEmpty(path) ){
          return false;
        }
        File file = new File(path);

        if (!file.exists() || size == file.length()) {
          WXLogUtils.w("weex so size check path :" + path+"   "+(System.currentTimeMillis() - start));
          return true;
        } else {
          return false;
        }
      }
    }catch(Throwable e ){
      WXLogUtils.e("weex so size check fail exception :"+e.getMessage());
    }

    return true;
  }

  /**
   * Concatenate the path of the so library, including directory.
   * @param libName the raw name of the lib
   * @param version the version of the so library
   * @return the path of the so library
   */
  static String _targetSoFile(String libName, int version) {
    Context context = mContext;
    if (null == context) {
      return "";
    }

    String path = "/data/data/" + context.getPackageName() + "/files";

    File f = context.getFilesDir();
    if (f != null) {
      path = f.getPath();
    }
    return path + "/lib" + libName + "bk" + version + ".so";

  }

  /**
   * Remove the so library if it had been extracted.
   * @param libName
   * @param version
   */
  static void removeSoIfExit(String libName, int version) {

    String file = _targetSoFile(libName, version);
    File a = new File(file);
    if (a.exists()) {
      a.delete();
    }

  }

  /**
   * Tell whether the so is extracted.
   */
  static boolean isExist(String libName, int version) {

    String file = _targetSoFile(libName, version);
    File a = new File(file);
    return a.exists();

  }


  /**
   * Load .so library
   */
  static boolean _loadUnzipSo(String libName,
                              int version,
                              IWXUserTrackAdapter utAdapter) {
    boolean initSuc = false;
    try {
      if (isExist(libName, version)) {
        // If a library loader adapter exists, use this adapter to load library
        // instead of System.load.
        if (mSoLoader != null) {
          mSoLoader.doLoad(_targetSoFile(libName, version));
        } else {
          System.load(_targetSoFile(libName, version));
        }
        commit(utAdapter, "2000", "Load file extract from apk successfully.");
      }
      initSuc = true;
    } catch (Throwable e) {
      commit(utAdapter,
             WXErrorCode.WX_ERR_COPY_FROM_APK.getErrorCode(),
             WXErrorCode.WX_ERR_COPY_FROM_APK.getErrorMsg() + ":" + e.getMessage());
      initSuc = false;
      WXLogUtils.e("", e);
    }
    return initSuc;
  }

  static boolean unZipSelectedFiles(String libName,
      int version,
      IWXUserTrackAdapter utAdapter) throws ZipException, IOException {
    String sourcePath = "lib/armeabi/lib" + libName + ".so";

    String zipPath = "";
    Context context = mContext;
    if (context == null) {
      return false;
    }

    ApplicationInfo aInfo = context.getApplicationInfo();
    if (null != aInfo) {
      zipPath = aInfo.sourceDir;
    }

    ZipFile zf;
    zf = new ZipFile(zipPath);
    try {

      for (Enumeration<?> entries = zf.entries(); entries.hasMoreElements(); ) {
        ZipEntry entry = ((ZipEntry) entries.nextElement());
        if (entry.getName().startsWith(sourcePath)) {

          InputStream in = null;
          FileOutputStream os = null;
          FileChannel channel = null;
          int total = 0;
          try {

            //Make sure the old library is deleted.
            removeSoIfExit(libName, version);

            //Copy file
            in = zf.getInputStream(entry);
            os = context.openFileOutput("lib" + libName + "bk" + version + ".so",
                                        Context.MODE_PRIVATE);
            channel = os.getChannel();

            byte[] buffers = new byte[1024];
            int realLength;

            while ((realLength = in.read(buffers)) > 0) {
              //os.write(buffers);
              channel.write(ByteBuffer.wrap(buffers, 0, realLength));
              total += realLength;

            }
          } finally {
            if (in != null) {
              try {
                in.close();
              } catch (Exception e) {
                e.printStackTrace();
              }
            }

            if (channel != null) {
              try {
                channel.close();
              } catch (Exception e) {
                e.printStackTrace();
              }
            }

            if (os != null) {
              try {
                os.close();
              } catch (Exception e) {
                e.printStackTrace();
              }
            }

            if (zf != null) {
              zf.close();
              zf = null;
            }
          }

          if (total > 0) {
            return _loadUnzipSo(libName, version, utAdapter);
          } else {
            return false;
          }
        }
      }
    } catch (java.io.IOException e) {
      e.printStackTrace();

    } finally {

      if (zf != null) {
        zf.close();
        zf = null;
      }
    }
    return false;
  }

  static void commit(IWXUserTrackAdapter utAdapter, String errCode, String errMsg) {
    if (mStatisticsListener != null) {
      mStatisticsListener.onException("0", errCode, errMsg);
    }

    if (utAdapter == null) {
      return;
    }
    if (errCode != null && errMsg != null) {
      WXPerformance p = new WXPerformance();
      p.errCode = errCode;
      p.errMsg = errMsg;
      utAdapter.commit(null, null, WXEnvironment.ENVIRONMENT, p, null);
    } else {
      utAdapter.commit(null, null, WXEnvironment.ENVIRONMENT, null, null);

    }
  }

}
