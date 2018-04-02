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
    mEvents = new EventsSet();
    mComponent_Impl_Android = nullptr;
    mMeasureFunc_Impl_Android = nullptr;
    mIsRootRender = false;
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

    for(auto it = ChildListIterBegin(); it != ChildListIterEnd(); it++) {
      RenderObject* child = static_cast<RenderObject*>(*it);
      if (child != nullptr) {
        delete child;
        child = nullptr;
      }
    }
  }

  void RenderObject::ApplyDefaultStyle() {
    std::map<std::string, std::string> *style = GetDefaultStyle();

    if (style == nullptr)
      return;

    for (auto iter = style->cbegin(); iter != style->cend(); iter++)
      AddStyle(iter->first, iter->second);

    if (style != nullptr) {
      delete style;
      style = nullptr;
    }
  }

  void RenderObject::ApplyDefaultAttr() {
    std::map<std::string, std::string> *attrs = GetDefaultAttr();

    if (attrs == nullptr)
      return;

    for (auto iter = attrs->cbegin(); iter != attrs->cend(); iter++) {
        UpdateAttr(iter->first, iter->second);
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
} //end WeexCore
