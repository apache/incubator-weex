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
package com.alibaba.weex;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.DividerItemDecoration;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.TextView;

import com.taobao.weex.IWXRenderListener;
import com.taobao.weex.WXSDKInstance;
import com.taobao.weex.common.WXRenderStrategy;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class SliceTestActivity extends AppCompatActivity {
  private static final String LOG_TAG = "SliceTestActivity";
  private RecyclerView mRecyclerView;
  private TextView mReportTextView;

  private final List<String> mData = new ArrayList<>();
  private WXInstanceAdapter mAdapter;
  private final Set<WXSDKInstance> mInstances = new HashSet<>();

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_slice_test);
    mRecyclerView = (RecyclerView) findViewById(R.id.recycler_view);
    mReportTextView = (TextView) findViewById(R.id.report_text);

    mRecyclerView.setLayoutManager(new LinearLayoutManager(this));
    mRecyclerView.addItemDecoration(new DividerItemDecoration(this, DividerItemDecoration.VERTICAL));
    mAdapter = new WXInstanceAdapter();
    mAdapter.setHasStableIds(true);
    mRecyclerView.setAdapter(mAdapter);
  }

  public void addCellClick(View view) {
    mData.add("{}");
    mAdapter.notifyItemInserted(mData.size() - 1);
//    mAdapter.notifyDataSetChanged();
  }

  private class WXInstanceAdapter extends RecyclerView.Adapter<WXViewHolder> {
    @Override
    public WXViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
      Log.d(LOG_TAG, "onCreateViewHolder");
      FrameLayout itemView = new FrameLayout(SliceTestActivity.this);
      itemView.setLayoutParams(new RecyclerView.LayoutParams(800,120));
      return new WXViewHolder(itemView);
    }

    @Override
    public void onBindViewHolder(WXViewHolder holder, int position) {
      Log.d(LOG_TAG, "onBindViewHolder " + position);
      String data = mData.get(position);
      if (!holder.isRendered()) {
        holder.render(data, position);
      } else {
        holder.refresh(data, position);
      }
    }

    @Override
    public long getItemId(int position) {
      return position;
    }

    @Override
    public int getItemViewType(int position) {
      return 1;
    }

    @Override
    public int getItemCount() {
      return mData.size();
    }
  }

  @Override
  protected void onDestroy() {
    super.onDestroy();
    for (WXSDKInstance instance : mInstances) {
      instance.destroy();
    }
    mInstances.clear();
  }

  private class WXViewHolder extends RecyclerView.ViewHolder implements IWXRenderListener {
    private WXSDKInstance mInstance;
    private boolean mRendered;

    private TextView mTextView;

    public WXViewHolder(View itemView) {
      super(itemView);
      mInstance = new WXSDKInstance(SliceTestActivity.this);
      mInstance.registerRenderListener(this);
      mInstances.add(mInstance);
      mTextView = new TextView(SliceTestActivity.this);
      FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
      params.gravity = Gravity.RIGHT;
      ((ViewGroup) itemView).addView(mTextView, params);
    }

    public void render(String initData, int position) {
      mInstance.render(
          "testPage",
          "// { \"framework\": \"Rax\" }",
          null,
          initData,
          WXRenderStrategy.APPEND_ONCE
      );
      mRendered = true;
      mTextView.setText(String.valueOf(position));
    }

    public boolean isRendered() {
      return mRendered;
    }

    public void refresh(String initData, int position) {
      mInstance.refreshInstance(initData);
      mTextView.setText(String.valueOf(position));
    }

    @Override
    public void onViewCreated(WXSDKInstance instance, View view) {
      ((ViewGroup) itemView).addView(view, ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
    }

    @Override
    public void onRenderSuccess(WXSDKInstance instance, int width, int height) {
    }

    @Override
    public void onRefreshSuccess(WXSDKInstance instance, int width, int height) {

    }

    @Override
    public void onException(WXSDKInstance instance, String errCode, String msg) {

    }
  }
}
