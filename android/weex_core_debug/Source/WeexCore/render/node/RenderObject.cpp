#include <WeexCore/render/node/RenderObject.h>
#include <WeexCore/platform/android/bridge/impl/ContentBoxMeasurement_Impl_Android.h>
#include <WeexCore/platform/android/bridge/impl/MeasureSize_Impl_Android.h>
#include <WeexCore/platform/android/bridge/impl/MeasureMode_Impl_Android.h>
#include <WeexCore/platform/android/base/jni/scoped_java_ref.h>
#include <WeexCore/platform/android/bridge/impl/WeexCore_Impl_Android.h>
#include <WeexCore/platform/android/base/string/StringUtils.h>
#include <string.h>

using namespace std;
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

    LOGEDEBUG("[~RenderObject] Delete tihs=%p",this);
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

    for (Index i = 0; i < getChildCount(); i++) {
      RenderObject *render = GetChild(i);
      if (nullptr != render) {
        delete render;
        render = nullptr;
      }
    }
  }

  void RenderObject::ApplyDefaultStyle() {
    std::map<std::string, std::string> *style = GetDefaultStyle();

    if (style == nullptr)
      return;

    StylesIterator style_it = style->begin();
    StylesIterator style_end = style->end();

    for (; style_it != style_end; ++style_it)
      AddStyle(style_it->first, style_it->second);

    if (style != nullptr) {
      delete style;
      style = nullptr;
    }
  }

  void RenderObject::ApplyDefaultAttr() {
    std::map<std::string, std::string> *attrs = GetDefaultAttr();

    if (attrs == nullptr)
      return;

    AttributesIterator attr_it = AttrItBegin();
    AttributesIterator attr_end = AttrItEnd();

    for (; attr_it != attr_end; ++attr_it) {
        UpdateAttr(attr_it->first, attr_it->second);
    }

    if (attrs != nullptr) {
      delete attrs;
      attrs = nullptr;
    }
  }

  bool RenderObject::BindComponentImplAndroid(jobject component_impl_android) {
    if (component_impl_android == nullptr)
      return false;
    this->mComponent_Impl_Android = getJNIEnv()->NewGlobalRef(component_impl_android);
    return true;
  }

  bool RenderObject::BindComponentImplIOS(void *component_impl_ios) {
    if (component_impl_ios == nullptr)
      return false;
    this->mComponent_Impl_iOS = component_impl_ios;
    return true;
  }

  WXCoreSize measureFunc_Impl(WXCoreLayoutNode *node, float width, MeasureMode widthMeasureMode,
                              float height, MeasureMode heightMeasureMode) {
    JNIEnv *env = getJNIEnv();
    WXCoreSize size;
    size.height = 0;
    size.width = 0;

    if (node == nullptr || ((RenderObject *) node)->GetMeasureFuncImplAndroid() == nullptr)
      return size;

    int widthMode = Unspecified(env);
    int heightMode = Unspecified(env);
    if (widthMeasureMode == kExactly)
      widthMode = Exactly(env);
    if (heightMeasureMode == kExactly)
      heightMode = Exactly(env);
    base::android::ScopedLocalJavaRef<jobject> jsize = cumsmeasure_Imple_Android(env,
                                                                                 ((RenderObject *) node)->GetMeasureFuncImplAndroid(),
                                                                                 width, height,
                                                                                 widthMode,
                                                                                 heightMode);
    if (jsize.Get() == nullptr)
      return size;
    size.height = GetMeasureHeight(env, jsize.Get());
    size.width = GetMeasureWidth(env, jsize.Get());

    return size;
  }

  bool RenderObject::BindMeasureFuncImplAndroid(jobject measureFunc_impl_android) {
    if (measureFunc_impl_android == nullptr)
      return false;
    this->mMeasureFunc_Impl_Android = getJNIEnv()->NewGlobalRef(measureFunc_impl_android);
    setMeasureFunc(measureFunc_Impl);
    return true;
  }

  bool RenderObject::BindMeasureFuncImplIOS(WXCoreMeasureFunc measureFunc_impl_ios) {
    if (measureFunc_impl_ios == nullptr)
      return false;
    setMeasureFunc(measureFunc_impl_ios);
    return true;
  }

  void RenderObject::onLayoutBefore() {
    if (this->GetMeasureFuncImplAndroid() == nullptr)
      return;
    JNIEnv *env = getJNIEnv();
    LayoutBeforeImplAndroid(env, this->GetMeasureFuncImplAndroid());
  }

  void RenderObject::onLayoutAfter(float width, float height) {
    if (this->GetMeasureFuncImplAndroid() == nullptr)
      return;
    JNIEnv *env = getJNIEnv();
    LayoutAfterImplAndroid(env, this->GetMeasureFuncImplAndroid(), width, height);
  }

  void RenderObject::PrintRenderMsg() {
    std::string result("ref: ");
    result.append(Ref());
    result.append("\n");

    result.append("type: ");
    result.append(Type());
    result.append("\n");


    result.append("attr:\n");

    AttributesIterator attr_it = AttrItBegin();
    AttributesIterator attr_end = AttrItEnd();
    for (; attr_it != attr_end; ++attr_it) {
      result.append("   ");
      result.append(attr_it->first);
      result.append(":");
      result.append(attr_it->second);
      result.append("\n");
    }


    result.append("style:\n");

    StylesIterator style_it = StyleItBegin();
    StylesIterator style_end = StyleItEnd();
    for (; style_it != style_end; ++style_it) {
      result.append("   ");
      result.append(style_it->first);
      result.append(":");
      result.append(style_it->second);
      result.append("\n");
    }


    if (mEvents != nullptr) {
      result.append("event:\n");

      EventsIterator event_it = EventItBegin();
      EventsIterator event_end = EventItEnd();

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

  void RenderObject::PrintLayoutMsg() {
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
