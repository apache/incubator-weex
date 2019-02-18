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
package com.taobao.weex.appfram.storage;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteFullException;
import android.database.sqlite.SQLiteStatement;
import android.support.annotation.Nullable;

import com.taobao.weex.common.WXThread;
import com.taobao.weex.utils.WXLogUtils;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class DefaultWXStorage implements IWXStorageAdapter {

    private WXSQLiteOpenHelper mDatabaseSupplier;

    private ExecutorService mExecutorService;

    private void execute(@Nullable final Runnable runnable) {
        if (mExecutorService == null) {
            mExecutorService = Executors.newSingleThreadExecutor();
        }

        if(runnable != null && !mExecutorService.isShutdown() && !mExecutorService.isTerminated()) {
            mExecutorService.execute(WXThread.secure(runnable));
        }
    }

    public DefaultWXStorage(Context context) {
        this.mDatabaseSupplier = new WXSQLiteOpenHelper(context);
    }


    @Override
    public void setItem(final String key, final String value, final OnResultReceivedListener listener) {
        execute(new Runnable() {
            @Override
            public void run() {
                Map<String, Object> data = StorageResultHandler.setItemResult(performSetItem(key, value, false, true));
                if (listener == null) {
                    return;
                }
                listener.onReceived(data);
            }
        });
    }

    @Override
    public void getItem(final String key, final OnResultReceivedListener listener) {
        execute(new Runnable() {
            @Override
            public void run() {
                Map<String, Object> data = StorageResultHandler.getItemResult(performGetItem(key));
                if (listener == null) {
                    return;
                }
                listener.onReceived(data);
            }
        });
    }

    @Override
    public void removeItem(final String key, final OnResultReceivedListener listener) {
        execute(new Runnable() {
            @Override
            public void run() {
                Map<String, Object> data = StorageResultHandler.removeItemResult(performRemoveItem(key));
                if (listener == null) {
                    return;
                }
                listener.onReceived(data);
            }
        });
    }

    @Override
    public void length(final OnResultReceivedListener listener) {
        execute(new Runnable() {
            @Override
            public void run() {
                Map<String, Object> data = StorageResultHandler.getLengthResult(performGetLength());
                if (listener == null) {
                    return;
                }
                listener.onReceived(data);
            }
        });
    }

    @Override
    public void getAllKeys(final OnResultReceivedListener listener) {
        execute(new Runnable() {
            @Override
            public void run() {
                Map<String, Object> data = StorageResultHandler.getAllkeysResult(performGetAllKeys());
                if (listener == null) {
                    return;
                }
                listener.onReceived(data);
            }
        });
    }

    @Override
    public void setItemPersistent(final String key, final String value, final OnResultReceivedListener listener) {
        execute(new Runnable() {
            @Override
            public void run() {
                Map<String, Object> data = StorageResultHandler.setItemResult(performSetItem(key, value, true, true));
                if (listener == null) {
                    return;
                }
                listener.onReceived(data);
            }
        });
    }

    @Override
    public void close() {
        final ExecutorService needCloseService = mExecutorService;
        execute(new Runnable() {
            @Override
            public void run() {
                try {
                    mDatabaseSupplier.closeDatabase();
                    if (needCloseService != null) {
                        needCloseService.shutdown();
                    }
                } catch (Exception e) {
                    WXLogUtils.e(WXSQLiteOpenHelper.TAG_STORAGE, e.getMessage());
                }
            }
        });
        mExecutorService = null;
    }

    private boolean performSetItem(String key, String value, boolean isPersistent, boolean allowRetryWhenFull) {
        SQLiteDatabase database = mDatabaseSupplier.getDatabase();
        if (database == null) {
            return false;
        }

        WXLogUtils.d(WXSQLiteOpenHelper.TAG_STORAGE, "set k-v to storage(key:" + key + ",value:" + value + ",isPersistent:" + isPersistent + ",allowRetry:" + allowRetryWhenFull + ")");
        String sql = "INSERT OR REPLACE INTO " + WXSQLiteOpenHelper.TABLE_STORAGE + " VALUES (?,?,?,?);";
        SQLiteStatement statement = null;
        String timeStamp = WXSQLiteOpenHelper.sDateFormatter.format(new Date());
        try {
            statement = database.compileStatement(sql);
            statement.clearBindings();
            statement.bindString(1, key);
            statement.bindString(2, value);
            statement.bindString(3, timeStamp);
            statement.bindLong(4, isPersistent ? 1 : 0);
            statement.execute();
            return true;
        } catch (Exception e) {
            WXLogUtils.e(WXSQLiteOpenHelper.TAG_STORAGE, "DefaultWXStorage occurred an exception when execute setItem :" + e.getMessage());
            if (e instanceof SQLiteFullException) {
                if (allowRetryWhenFull && trimToSize()) {
                    //try again
                    //setItem/setItemPersistent method only allow try once when occurred a sqliteFullException.
                    WXLogUtils.d(WXSQLiteOpenHelper.TAG_STORAGE, "retry set k-v to storage(key:" + key + ",value:" + value + ")");
                    return performSetItem(key, value, isPersistent, false);
                }
            }

            return false;
        } finally {
            if(statement != null) {
                statement.close();
            }
        }
    }

    /**
     * remove 10% of total record(at most) ordered by timestamp.
     * */
    private boolean trimToSize() {
        SQLiteDatabase database = mDatabaseSupplier.getDatabase();
        if (database == null) {
            return false;
        }

        List<String> toEvict = new ArrayList<>();
        int num = 0;

        Cursor c = database.query(WXSQLiteOpenHelper.TABLE_STORAGE, new String[]{WXSQLiteOpenHelper.COLUMN_KEY, WXSQLiteOpenHelper.COLUMN_PERSISTENT}, null, null, null, null, WXSQLiteOpenHelper.COLUMN_TIMESTAMP + " ASC");
        try {
            int evictSize = c.getCount() / 10;
            while (c.moveToNext()) {
                String key = c.getString(c.getColumnIndex(WXSQLiteOpenHelper.COLUMN_KEY));
                boolean persistent = c.getInt(c.getColumnIndex(WXSQLiteOpenHelper.COLUMN_PERSISTENT)) == 1;
                if (!persistent && key != null) {
                    num++;
                    toEvict.add(key);
                    if (num == evictSize) {
                        break;
                    }
                }
            }
        } catch (Exception e) {
            WXLogUtils.e(WXSQLiteOpenHelper.TAG_STORAGE, "DefaultWXStorage occurred an exception when execute trimToSize:" + e.getMessage());
        } finally {
            c.close();
        }

        if (num <= 0) {
            return false;
        }

        for (String key : toEvict) {
            performRemoveItem(key);
        }
        WXLogUtils.e(WXSQLiteOpenHelper.TAG_STORAGE, "remove " + num + " items by lru");
        return true;
    }

    private String performGetItem(String key) {
        SQLiteDatabase database = mDatabaseSupplier.getDatabase();
        if (database == null) {
            return null;
        }

        Cursor c = database.query(WXSQLiteOpenHelper.TABLE_STORAGE,
                new String[]{WXSQLiteOpenHelper.COLUMN_VALUE},
                WXSQLiteOpenHelper.COLUMN_KEY + "=?",
                new String[]{key},
                null, null, null);
        try {
            if (c.moveToNext()) {
                ContentValues values = new ContentValues();
                //update timestamp
                values.put(WXSQLiteOpenHelper.COLUMN_TIMESTAMP, WXSQLiteOpenHelper.sDateFormatter.format(new Date()));
                int updateResult = mDatabaseSupplier.getDatabase().update(WXSQLiteOpenHelper.TABLE_STORAGE, values, WXSQLiteOpenHelper.COLUMN_KEY + "= ?", new String[]{key});

                WXLogUtils.d(WXSQLiteOpenHelper.TAG_STORAGE, "update timestamp " + (updateResult == 1 ? "success" : "failed") + " for operation [getItem(key = " + key + ")]");
                return c.getString(c.getColumnIndex(WXSQLiteOpenHelper.COLUMN_VALUE));
            } else {
                return null;
            }
        } catch (Exception e) {
            WXLogUtils.e(WXSQLiteOpenHelper.TAG_STORAGE, "DefaultWXStorage occurred an exception when execute getItem:" + e.getMessage());
            return null;
        } finally {
            c.close();
        }
    }

    private boolean performRemoveItem(String key) {
        SQLiteDatabase database = mDatabaseSupplier.getDatabase();
        if (database == null) {
            return false;
        }

        int count = 0;
        try {
            count = database.delete(WXSQLiteOpenHelper.TABLE_STORAGE,
                    WXSQLiteOpenHelper.COLUMN_KEY + "=?",
                    new String[]{key});
        } catch (Exception e) {
            WXLogUtils.e(WXSQLiteOpenHelper.TAG_STORAGE, "DefaultWXStorage occurred an exception when execute removeItem:" + e.getMessage());
            return false;
        }
        return count == 1;
    }

    private long performGetLength() {
        SQLiteDatabase database = mDatabaseSupplier.getDatabase();
        if (database == null) {
            return 0;
        }

        String sql = "SELECT count(" + WXSQLiteOpenHelper.COLUMN_KEY + ") FROM " + WXSQLiteOpenHelper.TABLE_STORAGE;
        SQLiteStatement statement = null;
        try {
            statement = database.compileStatement(sql);
            return statement.simpleQueryForLong();
        } catch (Exception e) {
            WXLogUtils.e(WXSQLiteOpenHelper.TAG_STORAGE, "DefaultWXStorage occurred an exception when execute getLength:" + e.getMessage());
            return 0;
        } finally {
            if(statement != null) {
                statement.close();
            }
        }
    }

    private List<String> performGetAllKeys() {
        SQLiteDatabase database = mDatabaseSupplier.getDatabase();
        if (database == null) {
            return null;
        }

        List<String> result = new ArrayList<>();
        Cursor c = database.query(WXSQLiteOpenHelper.TABLE_STORAGE, new String[]{WXSQLiteOpenHelper.COLUMN_KEY}, null, null, null, null, null);
        try {
            while (c.moveToNext()) {
                result.add(c.getString(c.getColumnIndex(WXSQLiteOpenHelper.COLUMN_KEY)));
            }
            return result;
        } catch (Exception e) {
            WXLogUtils.e(WXSQLiteOpenHelper.TAG_STORAGE, "DefaultWXStorage occurred an exception when execute getAllKeys:" + e.getMessage());
            return result;
        } finally {
            c.close();
        }
    }

}
