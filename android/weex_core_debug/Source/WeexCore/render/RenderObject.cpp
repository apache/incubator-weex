#include "RenderObject.h"
#include "RenderPage.h"
#include <WeexCore/platform/android/bridge/ContentBoxMeasurement_Impl_Android.h>
#include <WeexCore/platform/android/bridge/MeasureSize_Impl_Android.h>
#include <WeexCore/platform/android/bridge/MeasureMode_Impl_Android.h>
#include <base/android/jni/scoped_java_ref.h>

namespace WeexCore {

  RenderObject::RenderObject(RenderPage *page)
      : mPage(page) {
    mStyles = new STYLES_MAP();
    mAttributes = new ATTRIBUTES_MAP();
    mPaddings = new PADDING_MAP();
    mMargins = new MARGIN_MAP();
    mBorders = new BORDER_MAP();
    mEvents = new EVENTS_SET();
    mComponent_Impl_Android = nullptr;
    mMeasureFunc_Impl_Android = nullptr;
  }

  RenderObject::~RenderObject() {

    JNIEnv *env = getJNIEnv();

    mPage = nullptr;
    mParentRender = nullptr;

    if (mStyles != nullptr) {
      delete mStyles;
      mStyles = nullptr;
    }
    if (mAttributes != nullptr) {
      delete mAttributes;
      mAttributes = nullptr;
    }
    if (mEvents != nullptr) {
      delete mEvents;
      mEvents = nullptr;
    }

    if (mComponent_Impl_Android != nullptr) {
      env->DeleteGlobalRef(mComponent_Impl_Android);
      mComponent_Impl_Android = nullptr;
    }

    if (mMeasureFunc_Impl_Android != nullptr) {
      env->DeleteGlobalRef(mMeasureFunc_Impl_Android);
      mMeasureFunc_Impl_Android = nullptr;
    }

    freeWXCoreNode();

    for (int i = 0; i < getChildCount(); i++) {
      RenderObject *render = getChild(i);
      if (nullptr != render) {
        delete render;
        render = nullptr;
      }
    }
  }

  void RenderObject::bindComponent_Impl_Android(jobject component_Impl_Android) {
    if (component_Impl_Android == nullptr)
      return;
    this->mComponent_Impl_Android = getJNIEnv()->NewGlobalRef(component_Impl_Android);
  }

  void RenderObject::bindComponent_Impl_iOS(void *component_Impl_iOS) {
    if (component_Impl_iOS == nullptr)
      return;
    this->mComponent_Impl_iOS = component_Impl_iOS;
  }

  WXCoreSize measureFunc_Impl(WXCoreLayoutNode *node, float width, MeasureMode widthMeasureMode,
                              float height, MeasureMode heightMeasureMode) {
    JNIEnv *env = getJNIEnv();
    WXCoreSize size;
    size.height = 0;
    size.width = 0;

    if (node == nullptr || ((RenderObject *) node)->getMeasureFunc_Impl_Android() == nullptr)
      return size;

    int widthMode = getATMOST(env);
    int heightMode = getATMOST(env);
    if (widthMeasureMode == EXACTLY)
      widthMode = getEXACTLY(env);
    if (heightMeasureMode == EXACTLY)
      heightMode = getEXACTLY(env);
    base::android::ScopedLocalJavaRef<jobject> jsize = cumsmeasure_Imple_Android(env,
                                                                                 ((RenderObject *) node)->getMeasureFunc_Impl_Android(),
                                                                                 width, height,
                                                                                 widthMode,
                                                                                 heightMode);
    if (jsize.Get() == nullptr)
      return size;
    size.height = getMeasureHeight(env, jsize.Get());
    size.width = getMeasureWidth(env, jsize.Get());

    return size;
  }

  void RenderObject::bindMeasureFunc_Impl_Android(jobject measureFunc_Impl_Android) {
    if (measureFunc_Impl_Android == nullptr)
      return;
    this->mMeasureFunc_Impl_Android = getJNIEnv()->NewGlobalRef(measureFunc_Impl_Android);
    setMeasureFunc(measureFunc_Impl);
  }

  void RenderObject::bindMeasureFunc_Impl_iOS(WXCoreMeasureFunc measureFunc_Impl_iOS) {
    if (measureFunc_Impl_iOS == nullptr)
      return;
    setMeasureFunc(measureFunc_Impl_iOS);
  }

  void RenderObject::onLayoutBefore() {
    JNIEnv *env = getJNIEnv();
    layoutBefore_Impl_Android(env, this->getMeasureFunc_Impl_Android());
  }

  void RenderObject::onLayoutAfter(float width, float height) {
    JNIEnv *env = getJNIEnv();
    layoutAfter_Impl_Android(env, this->getMeasureFunc_Impl_Android(), width, height);
  }

/**
 * Synchronize the style of RenderObject to LayoutNode
 */
  void RenderObject::applyStyle(std::string key, std::string value) {
    if (key == ALIGN_ITEMS) {
      setAlignItems(getWXCoreAlignItem(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == ALIGN_SELF) {
      setAlignSelf(getWXCoreAlignSelf(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == FLEX) {
      setFlex(stringToNum<float>(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == FLEX_DIRECTION) {
      setFlexDirection(getWXCoreFlexDirection(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == JUSTIFY_CONTENT) {
      setJustifyContent(getWXCoreJustifyContent(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == FLEX_WRAP) {
      setFlexWrap(getWXCoreFlexWrap(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == MIN_WIDTH) {
      setMinWidth(stringToNum<float>(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == MIN_HEIGHT) {
      setMinHeight(stringToNum<float>(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == MAX_WIDTH) {
      setMaxWidth(stringToNum<float>(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == MAX_HEIGHT) {
      setMaxHeight(stringToNum<float>(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == DEFAULT_HEIGHT || key == HEIGHT) {
      setStyleHeight(stringToNum<float>(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == DEFAULT_WIDTH | key == WIDTH) {
      setStyleWidth(stringToNum<float>(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == POSITION) {
      setStylePositionType(getWXCorePositionType(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == LEFT) {
      setStylePosition(WXCore_PositionEdge_Left, stringToNum<float>(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == TOP) {
      setStylePosition(WXCore_PositionEdge_Top, stringToNum<float>(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == RIGHT) {
      setStylePosition(WXCore_PositionEdge_Right, stringToNum<float>(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == BOTTOM) {
      setStylePosition(WXCore_PositionEdge_Bottom, stringToNum<float>(value));
      mStyles->insert(pair<std::string, std::string>(key, value));
    } else if (key == MARGIN) {
      setMargin(WXCore_Margin_ALL, stringToNum<float>(value));
      mMargins->insert(pair<std::string, std::string>(key, value));
    } else if (key == MARGIN_LEFT) {
      setMargin(WXCore_Margin_Left, stringToNum<float>(value));
      mMargins->insert(pair<std::string, std::string>(key, value));
    } else if (key == MARGIN_TOP) {
      setMargin(WXCore_Margin_Top, stringToNum<float>(value));
      mMargins->insert(pair<std::string, std::string>(key, value));
    } else if (key == MARGIN_RIGHT) {
      setMargin(WXCore_Margin_Right, stringToNum<float>(value));
      mMargins->insert(pair<std::string, std::string>(key, value));
    } else if (key == MARGIN_BOTTOM) {
      setMargin(WXCore_Margin_Bottom, stringToNum<float>(value));
      mMargins->insert(pair<std::string, std::string>(key, value));
    } else if (key == BORDER_WIDTH) {
      setBorderWidth(WXCore_Border_Width_ALL, stringToNum<float>(value));
      mBorders->insert(pair<std::string, std::string>(key, value));
    } else if (key == BORDER_TOP_WIDTH) {
      setBorderWidth(WXCore_Border_Width_Top, stringToNum<float>(value));
      mBorders->insert(pair<std::string, std::string>(key, value));
    } else if (key == BORDER_RIGHT_WIDTH) {
      setBorderWidth(WXCore_Border_Width_Right, stringToNum<float>(value));
      mBorders->insert(pair<std::string, std::string>(key, value));
    } else if (key == BORDER_BOTTOM_WIDTH) {
      setBorderWidth(WXCore_Border_Width_Bottom, stringToNum<float>(value));
      mBorders->insert(pair<std::string, std::string>(key, value));
    } else if (key == BORDER_LEFT_WIDTH) {
      setBorderWidth(WXCore_Border_Width_Left, stringToNum<float>(value));
      mBorders->insert(pair<std::string, std::string>(key, value));
    } else if (key == PADDING) {
      setPadding(WXCore_Padding_ALL, stringToNum<float>(value));
      mPaddings->insert(pair<std::string, std::string>(key, value));
    } else if (key == PADDING_LEFT) {
      setPadding(WXCore_Padding_Left, stringToNum<float>(value));
      mPaddings->insert(pair<std::string, std::string>(key, value));
    } else if (key == PADDING_TOP) {
      setPadding(WXCore_Padding_Top, stringToNum<float>(value));
      mPaddings->insert(pair<std::string, std::string>(key, value));
    } else if (key == PADDING_RIGHT) {
      setPadding(WXCore_Padding_Right, stringToNum<float>(value));
      mPaddings->insert(pair<std::string, std::string>(key, value));
    } else if (key == PADDING_BOTTOM) {
      setPadding(WXCore_Padding_Bottom, stringToNum<float>(value));
      mPaddings->insert(pair<std::string, std::string>(key, value));
    } else {
      mStyles->insert(pair<std::string, std::string>(key, value));
    }
  }

  void RenderObject::printRenderMsg() {
    std::string result("ref: ");
    result.append(mRef);
    result.append("\n");

    result.append("type: ");
    result.append(mType);
    result.append("\n");


    result.append("attr:\n");

    ATTR_IT attr_it = getAttrItBegin();
    ATTR_IT attr_end = getAttrItEnd();
    for (; attr_it != attr_end; ++attr_it) {
      result.append("   ");
      result.append(attr_it->first);
      result.append(":");
      result.append(attr_it->second);
      result.append("\n");
    }


    result.append("style:\n");

    STYLE_IT style_it = getStyleItBegin();
    STYLE_IT style_end = getStyleItEnd();
    for (; style_it != style_end; ++style_it) {
      result.append("   ");
      result.append(style_it->first);
      result.append(":");
      result.append(style_it->second);
      result.append("\n");
    }


    if (mEvents != nullptr) {
      result.append("event:\n");

      EVENT_IT event_it = getEventItBegin();
      EVENT_IT event_end = getEventItEnd();

      for (; event_it != event_end; ++event_it) {
        result.append("   ");
        result.append(*event_it);
        result.append("\n");
      }
    }

//    LOGE("[RenderObject::Render tree] \n %s", result.c_str());

//    for (CHILD_LIST_IT it = getChildListItBegin();
//         it != getChildListItEnd(); it++) {
//      if (nullptr != *it) {
//        (*it)->printRenderMsg();
//      }
//    }
  }

  void RenderObject::printYGNodeMsg() {
//    LOGE("yoga ref: %s\n", mRef.c_str());
//    LOGE("yoga type: %s\n", mType.c_str());
//    YGNodePrint(mYGNode, YGPrintOptionsLayout);
//    YGNodePrint(mYGNode, YGPrintOptionsStyle);
//    LOGE("\n\n");

//    for (CHILD_LIST_IT it = getChildListItBegin();
//         it != getChildListItEnd(); it++) {
//      if (nullptr != *it) {
//        (*it)->printYGNodeMsg();
//      }
//    }
  }
} //end WeexCore
