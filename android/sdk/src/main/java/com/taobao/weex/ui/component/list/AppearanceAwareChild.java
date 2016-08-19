package com.taobao.weex.ui.component.list;

import com.taobao.weex.ui.component.WXComponent;

/**
 * Created by sospartan on 8/19/16.
 */
class AppearanceAwareChild {

  final WXComponent mListDirectChild;
  final WXComponent mAwareChild;

  private boolean isAppear = false;
  /**
   *
   * @param listDirectChild  direct child of list,for locate index.
   * @param awareChild  child to notify when appearance changed.
   */
  AppearanceAwareChild(WXComponent listDirectChild, WXComponent awareChild){
    mListDirectChild = listDirectChild;
    mAwareChild = awareChild;
  }

  public WXComponent getListDirectChild() {
    return mListDirectChild;
  }

  public WXComponent getAwareChild() {
    return mAwareChild;
  }

  public boolean isAppear() {
    return isAppear;
  }

  public void setAppear(boolean appear) {
    isAppear = appear;
  }
}
