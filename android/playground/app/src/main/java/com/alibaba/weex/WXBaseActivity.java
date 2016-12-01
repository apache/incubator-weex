package com.alibaba.weex;

import android.content.res.TypedArray;
import android.os.Bundle;
import android.os.PersistableBundle;
import android.support.v7.app.AppCompatActivity;

public abstract class WXBaseActivity extends AppCompatActivity {

  @Override
  public void onCreate(Bundle savedInstanceState, PersistableBundle persistentState) {
    super.onCreate(savedInstanceState, persistentState);
  }

  protected int getActionBarHeight() {
    TypedArray actionbarSizeTypedArray = this.obtainStyledAttributes(new int[]{
        android.R.attr.actionBarSize
    });

    return (int) actionbarSizeTypedArray.getDimension(0, 0);
  }
}
