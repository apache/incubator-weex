#include "RenderObject.h"
#include "WeexCore/render/page/RenderPage.h"
#include <WeexCore/platform/android/bridge/impl/ContentBoxMeasurement_Impl_Android.h>
#include <WeexCore/platform/android/bridge/impl/MeasureSize_Impl_Android.h>
#include <WeexCore/platform/android/bridge/impl/MeasureMode_Impl_Android.h>
#include <WeexCore/platform/android/base/jni/scoped_java_ref.h>
#include <WeexCore/platform/android/bridge/impl/WeexCore_Impl_Android.h>
#include <WeexCore/platform/android/base/string/StringUtils.h>
#include <WeexCore/css/ConstantsName.h>
#include <WeexCore/css/CssValueGetter.h>
#include <base/ViewUtils.h>

namespace WeexCore {

  RenderObject::RenderObject() {
    mStyles = new StylesMap();
    mAttributes = new AttributesMap();
    mPaddings = new PaddingsMap();
    mMargins = new MarginsMap();
    mBorders = new BordersMap();
    mEvents = new EventsSet();
    mComponent_Impl_Android = nullptr;
    mMeasureFunc_Impl_Android = nullptr;
  }

  RenderObject::~RenderObject() {

    JNIEnv *env = getJNIEnv();

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

    for (Index i = 0; i < getChildCount(); i++) {
      RenderObject *render = getChild(i);
      if (nullptr != render) {
        delete render;
        render = nullptr;
      }
    }
  }

  bool RenderObject::bindComponent_Impl_Android(jobject component_Impl_Android) {
    if (component_Impl_Android == nullptr)
      return false;
    this->mComponent_Impl_Android = getJNIEnv()->NewGlobalRef(component_Impl_Android);
    return true;
  }

  bool RenderObject::bindComponent_Impl_iOS(void *component_Impl_iOS) {
    if (component_Impl_iOS == nullptr)
      return false;
    this->mComponent_Impl_iOS = component_Impl_iOS;
    return true;
  }

  WXCoreSize measureFunc_Impl(WXCoreLayoutNode *node, float width, MeasureMode widthMeasureMode,
                              float height, MeasureMode heightMeasureMode) {
    JNIEnv *env = getJNIEnv();
    WXCoreSize size;
    size.height = 0;
    size.width = 0;

    if (node == nullptr || ((RenderObject *) node)->getMeasureFunc_Impl_Android() == nullptr)
      return size;

    int widthMode = getUnspecified(env);
    int heightMode = getUnspecified(env);
    if (widthMeasureMode == kExactly)
      widthMode = getExactly(env);
    if (heightMeasureMode == kExactly)
      heightMode = getExactly(env);
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

  bool RenderObject::bindMeasureFunc_Impl_Android(jobject measureFunc_Impl_Android) {
    if (measureFunc_Impl_Android == nullptr)
      return false;
    this->mMeasureFunc_Impl_Android = getJNIEnv()->NewGlobalRef(measureFunc_Impl_Android);
    setMeasureFunc(measureFunc_Impl);
    return true;
  }

  bool RenderObject::bindMeasureFunc_Impl_iOS(WXCoreMeasureFunc measureFunc_Impl_iOS) {
    if (measureFunc_Impl_iOS == nullptr)
      return false;
    setMeasureFunc(measureFunc_Impl_iOS);
    return true;
  }

  void RenderObject::onLayoutBefore() {
    if (this->getMeasureFunc_Impl_Android() == nullptr)
      return;
    JNIEnv *env = getJNIEnv();
    layoutBefore_Impl_Android(env, this->getMeasureFunc_Impl_Android());
  }

  void RenderObject::onLayoutAfter(float width, float height) {
    if (this->getMeasureFunc_Impl_Android() == nullptr)
      return;
    JNIEnv *env = getJNIEnv();
    layoutAfter_Impl_Android(env, this->getMeasureFunc_Impl_Android(), width, height);
  }

  StyleType RenderObject::applyStyle(const std::string &key, const std::string &value) {
    if (key == ALIGN_ITEMS) {
      setAlignItems(getWXCoreAlignItem(value));
      return kTypeLayout;
    } else if (key == ALIGN_SELF) {
      setAlignSelf(getWXCoreAlignSelf(value));
      return kTypeLayout;
    } else if (key == FLEX) {
      setFlex(atof(value.c_str()));
      return kTypeLayout;
    } else if (key == FLEX_DIRECTION) {
      setFlexDirection(getWXCoreFlexDirection(value));
      return kTypeLayout;
    } else if (key == JUSTIFY_CONTENT) {
      setJustifyContent(getWXCoreJustifyContent(value));
      return kTypeLayout;
    } else if (key == FLEX_WRAP) {
      setFlexWrap(getWXCoreFlexWrap(value));
      return kTypeLayout;
    } else if (key == MIN_WIDTH) {
      setMinWidth(getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      return kTypeLayout;
    } else if (key == MIN_HEIGHT) {
      setMinHeight(getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      return kTypeLayout;
    } else if (key == MAX_WIDTH) {
      setMaxWidth(getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      return kTypeLayout;
    } else if (key == MAX_HEIGHT) {
      setMaxHeight(getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      return kTypeLayout;
    } else if (key == HEIGHT) {
      setStyleHeight(getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      return kTypeLayout;
    } else if (key == WIDTH) {
      setStyleWidth(getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      return kTypeLayout;
    } else if (key == POSITION) {
      setStylePositionType(getWXCorePositionType(value));
      return kTypeLayout;
    } else if (key == LEFT) {
      setStylePosition(kPositionEdgeLeft, getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      return kTypeLayout;
    } else if (key == TOP) {
      setStylePosition(kPositionEdgeTop, getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      return kTypeLayout;
    } else if (key == RIGHT) {
      setStylePosition(kPositionEdgeRight, getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      return kTypeLayout;
    } else if (key == BOTTOM) {
      setStylePosition(kPositionEdgeBottom, getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      return kTypeLayout;
    } else if (key == MARGIN) {
      setMargin(kMarginALL, getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      mMargins->insert(std::pair<std::string, std::string>(key, value));
      return kTypeMargin;
    } else if (key == MARGIN_LEFT) {
      setMargin(kMarginLeft, getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      mMargins->insert(std::pair<std::string, std::string>(key, value));
      return kTypeMargin;
    } else if (key == MARGIN_TOP) {
      setMargin(kMarginTop, getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      mMargins->insert(std::pair<std::string, std::string>(key, value));
      return kTypeMargin;
    } else if (key == MARGIN_RIGHT) {
      setMargin(kMarginRight, getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      mMargins->insert(std::pair<std::string, std::string>(key, value));
      return kTypeMargin;
    } else if (key == MARGIN_BOTTOM) {
      setMargin(kMarginBottom, getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      mMargins->insert(std::pair<std::string, std::string>(key, value));
      return kTypeMargin;
    } else if (key == BORDER_WIDTH) {
      setBorderWidth(kBorderWidthALL, getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      mBorders->insert(std::pair<std::string, std::string>(key, value));
      return kTypeBorder;
    } else if (key == BORDER_TOP_WIDTH) {
      setBorderWidth(kBorderWidthTop, getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      mBorders->insert(std::pair<std::string, std::string>(key, value));
      return kTypeBorder;
    } else if (key == BORDER_RIGHT_WIDTH) {
      setBorderWidth(kBorderWidthRight, getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      mBorders->insert(std::pair<std::string, std::string>(key, value));
      return kTypeBorder;
    } else if (key == BORDER_BOTTOM_WIDTH) {
      setBorderWidth(kBorderWidthBottom, getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      mBorders->insert(std::pair<std::string, std::string>(key, value));
      return kTypeBorder;
    } else if (key == BORDER_LEFT_WIDTH) {
      setBorderWidth(kBorderWidthLeft, getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      mBorders->insert(std::pair<std::string, std::string>(key, value));
      return kTypeBorder;
    } else if (key == PADDING) {
      setPadding(kPaddingALL, getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      mPaddings->insert(std::pair<std::string, std::string>(key, value));
      return kTypePadding;
    } else if (key == PADDING_LEFT) {
      setPadding(kPaddingLeft, getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      mPaddings->insert(std::pair<std::string, std::string>(key, value));
      return kTypePadding;
    } else if (key == PADDING_TOP) {
      setPadding(kPaddingTop, getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      mPaddings->insert(std::pair<std::string, std::string>(key, value));
      return kTypePadding;
    } else if (key == PADDING_RIGHT) {
      setPadding(kPaddingRight, getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      mPaddings->insert(std::pair<std::string, std::string>(key, value));
      return kTypePadding;
    } else if (key == PADDING_BOTTOM) {
      setPadding(kPaddingBottom, getRealPxByWidth(atof(value.c_str()), getRenderPage()->getViewPortWidth()));
      mPaddings->insert(std::pair<std::string, std::string>(key, value));
      return kTypePadding;
    } else {
      mStyles->insert(std::pair<std::string, std::string>(key, value));
      return kTypeStyle;
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

    AttributesIterator attr_it = getAttrItBegin();
    AttributesIterator attr_end = getAttrItEnd();
    for (; attr_it != attr_end; ++attr_it) {
      result.append("   ");
      result.append(attr_it->first);
      result.append(":");
      result.append(attr_it->second);
      result.append("\n");
    }


    result.append("style:\n");

    StylesIterator style_it = getStyleItBegin();
    StylesIterator style_end = getStyleItEnd();
    for (; style_it != style_end; ++style_it) {
      result.append("   ");
      result.append(style_it->first);
      result.append(":");
      result.append(style_it->second);
      result.append("\n");
    }


    if (mEvents != nullptr) {
      result.append("event:\n");

      EventsIterator event_it = getEventItBegin();
      EventsIterator event_end = getEventItEnd();

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
