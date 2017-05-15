package com.taobao.weex.ui.view.listview;

import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.StaggeredGridLayoutManager;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.utils.WXLogUtils;

/**
 * Created by zhengshihan on 2017/5/5.
 */

public class ExtendedStaggeredGridLayoutManager extends StaggeredGridLayoutManager{

  public ExtendedStaggeredGridLayoutManager(int spanCount, int orientation) {
    super(spanCount, orientation);
  }

  @Override
  public void onItemsRemoved(RecyclerView recyclerView, int positionStart, int itemCount) {
    if(positionStart ==-1){
      WXLogUtils.e("ExtendedStaggeredGridLayoutManager: onItemsRemoved  Error Invalid Index : positionStart :"+positionStart +"  itemCount:"+ itemCount);
      return ;
    }else {
      if(WXEnvironment.isApkDebugable()){
        WXLogUtils.e("ExtendedStaggeredGridLayoutManager: onItemsRemoved  positionStart :"+positionStart+"  itemCount:"+ itemCount);
      }
    }
    super.onItemsRemoved(recyclerView, positionStart, itemCount);
  }
}
