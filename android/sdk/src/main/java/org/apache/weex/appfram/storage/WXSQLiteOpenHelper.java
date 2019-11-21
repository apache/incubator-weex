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
package org.apache.weex.appfram.storage;

import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteException;
import android.database.sqlite.SQLiteOpenHelper;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;

import org.apache.weex.utils.WXLogUtils;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

public class WXSQLiteOpenHelper extends SQLiteOpenHelper {

    private static final String DATABASE_NAME = "WXStorage";
    private static final int DATABASE_VERSION = 2;
    static final String TAG_STORAGE = "weex_storage";

    private long mMaximumDatabaseSize = 5 * 10 * 1024 * 1024L;//50mb
    static SimpleDateFormat sDateFormatter = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.getDefault());
    private Context mContext;
    private SQLiteDatabase mDb;


    static final String TABLE_STORAGE = "default_wx_storage";
    static final String COLUMN_KEY = "key";
    static final String COLUMN_VALUE = "value";
    static final String COLUMN_TIMESTAMP = "timestamp";
    static final String COLUMN_PERSISTENT = "persistent";

    private static final int SLEEP_TIME_MS = 30;

    private static final String STATEMENT_CREATE_TABLE = "CREATE TABLE IF NOT EXISTS " + TABLE_STORAGE + " ("
            + COLUMN_KEY
            + " TEXT PRIMARY KEY,"
            + COLUMN_VALUE
            + " TEXT NOT NULL,"
            + COLUMN_TIMESTAMP
            + " TEXT NOT NULL,"
            + COLUMN_PERSISTENT
            + " INTEGER DEFAULT 0"
            + ")";


    public WXSQLiteOpenHelper(Context context) {
        super(context, DATABASE_NAME, null, DATABASE_VERSION);
        this.mContext = context;
    }

    /**
     * retrieve sqlite database
     *
     * @return a {@link SQLiteDatabase} instance or null if retrieve fails.
     * */
    public @Nullable SQLiteDatabase getDatabase() {
        ensureDatabase();
        return mDb;
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        db.execSQL(STATEMENT_CREATE_TABLE);
    }


    /**
     * version 1:
     *
     *   ----------------
     *   | key | value |
     *   ---------------
     *
     * version 2:
     *
     *  ----------------------------------------
     *  | key | value | timestamp | persistent |
     *  ----------------------------------------
     **/
    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        if (oldVersion != newVersion) {
            if(newVersion == 2 && oldVersion == 1){
                WXLogUtils.d(TAG_STORAGE,"storage is updating from version "+oldVersion+" to version "+newVersion);
                boolean updateResult = true;
                try {
                    long start = System.currentTimeMillis();

                    db.beginTransaction();
                    // update table structure
                    String SQL_ADD_COLUMN_TIMESTAMP = "ALTER TABLE "+TABLE_STORAGE+" ADD COLUMN "+COLUMN_TIMESTAMP+" TEXT;";
                    WXLogUtils.d(TAG_STORAGE,"exec sql : "+ SQL_ADD_COLUMN_TIMESTAMP);
                    db.execSQL(SQL_ADD_COLUMN_TIMESTAMP);

                    String SQL_ADD_COLUMN_PERSISTENT = "ALTER TABLE "+TABLE_STORAGE+" ADD COLUMN "+COLUMN_PERSISTENT+" INTEGER;";
                    WXLogUtils.d(TAG_STORAGE,"exec sql : "+ SQL_ADD_COLUMN_PERSISTENT);
                    db.execSQL(SQL_ADD_COLUMN_PERSISTENT);

                    // update timestamp & persistent
                    String SQL_UPDATE_TABLE = "UPDATE "+TABLE_STORAGE+" SET "+ COLUMN_TIMESTAMP+" = '"+sDateFormatter.format(new Date())+"' , "+ COLUMN_PERSISTENT +" = 0";
                    WXLogUtils.d(TAG_STORAGE,"exec sql : "+ SQL_UPDATE_TABLE);
                    db.execSQL(SQL_UPDATE_TABLE);

                    db.setTransactionSuccessful();
                    long time = System.currentTimeMillis() - start;
                    WXLogUtils.d(TAG_STORAGE,"storage updated success ("+time+"ms)");
                }catch (Exception e){
                    WXLogUtils.d(TAG_STORAGE,"storage updated failed from version "+oldVersion+" to version "+newVersion+","+e.getMessage());
                    updateResult = false;
                }finally {
                    db.endTransaction();
                }
                //rollback
                if(!updateResult){
                    WXLogUtils.d(TAG_STORAGE,"storage is rollback,all data will be removed");
                    deleteDB();
                    onCreate(db);
                }
            }else{
                deleteDB();
                onCreate(db);
            }
        }
    }



    synchronized void ensureDatabase() {
        if (mDb != null && mDb.isOpen()) {
            return;
        }

        try {
            // Sometimes retrieving the database fails. We do 2 retries: first without database deletion
            // and then with deletion.
            for (int tries = 0; tries < 2; tries++) {
                try {
                    if (tries > 0) {
                        //delete db and recreate
                        deleteDB();
                    }
                    mDb = getWritableDatabase();
                    break;
                } catch (SQLiteException e) {
                    e.printStackTrace();
                }
                // Wait before retrying.
                try {
                    Thread.sleep(SLEEP_TIME_MS);
                } catch (InterruptedException ie) {
                    Thread.currentThread().interrupt();
                }
            }
            if(mDb == null){
                return;
            }

            createTableIfNotExists(mDb);

            mDb.setMaximumSize(mMaximumDatabaseSize);
        } catch (Throwable e) {
            mDb = null;
            WXLogUtils.d(TAG_STORAGE,"ensureDatabase failed, throwable = " + e.getMessage());
        }

    }

    public synchronized void setMaximumSize(long size) {
        mMaximumDatabaseSize = size;
        if (mDb != null) {
            mDb.setMaximumSize(mMaximumDatabaseSize);
        }
    }

    private boolean deleteDB() {
        closeDatabase();
        return mContext.deleteDatabase(DATABASE_NAME);
    }

    public synchronized void closeDatabase() {
        if (mDb != null && mDb.isOpen()) {
            mDb.close();
            mDb = null;
        }
    }

    private void createTableIfNotExists(@NonNull SQLiteDatabase db) {
        Cursor cursor = null;
        try {
            cursor = db.rawQuery("SELECT DISTINCT tbl_name FROM sqlite_master WHERE tbl_name = '"+TABLE_STORAGE+"'", null);
            if(cursor != null && cursor.getCount() > 0) {
                return;
            }
            db.execSQL(STATEMENT_CREATE_TABLE);
        }catch (Exception e){
            e.printStackTrace();
        }finally {
            if(cursor != null){
                cursor.close();
            }
        }
    }


}
