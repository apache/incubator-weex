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

#include "native_render_object_utils_impl_android.h"
#include <android/log.h>

#include "android/base/string/string_utils.h"
#include "android/jniprebuild/jniheader/NativeRenderObjectUtils_jni.h"
#include "base/android/jni/android_jni.h"
#include "base/android/log_utils.h"
#include "core/render/page/render_page.h"
#include "core/render/manager/render_manager.h"
#include "core/render/node/factory/render_creator.h"
#include "core/render/node/render_object.h"
#include "core/render/node/factory/render_type.h"
#include "core/render/node/render_list.h"
#include "core/config/core_environment.h"
#include "core/manager/weex_core_manager.h"

using namespace WeexCore;

namespace WeexCore {
    bool RegisterJNINativeRenderObjectUtils(JNIEnv *env){
        return RegisterNativesImpl(env);
    }
}



static jlong GetRenderObject(JNIEnv* env, jclass jcaller,
                             jstring instanceId,
                             jstring ref){
    ScopedJStringUTF8 scoped_id(env, instanceId);
    ScopedJStringUTF8 scoped_ref(env, ref);
    return WeexCoreManager::Instance()->getPlatformBridge()->core_side()->GetRenderObject(
            scoped_id.getChars(), scoped_ref.getChars());
}



static void UpdateRenderObjectStyle(JNIEnv* env, jclass jcaller,
                                    jlong ptr,
                                    jstring key,
                                    jstring value){
    ScopedJStringUTF8 scoped_key(env, key);
    ScopedJStringUTF8 scoped_value(env, value);
    WeexCoreManager::Instance()->getPlatformBridge()->core_side()->UpdateRenderObjectStyle(ptr, scoped_key.getChars(), scoped_value.getChars());
}

static void UpdateRenderObjectAttr(JNIEnv* env, jclass jcaller,
                                   jlong ptr,
                                   jstring key,
                                   jstring value){
    ScopedJStringUTF8 scoped_key(env, key);
    ScopedJStringUTF8 scoped_value(env, value);
    WeexCoreManager::Instance()->getPlatformBridge()->core_side()->UpdateRenderObjectAttr(ptr, scoped_key.getChars(), scoped_value.getChars());

}

static jlong CopyRenderObject(JNIEnv* env, jclass jcaller, jlong ptr){
    return WeexCoreManager::Instance()->getPlatformBridge()->core_side()->CopyRenderObject(ptr);
}

static  void showRenderObjectLayout(RenderObject *renderObject, int level){
    LOGE("RenderObject layout %s %d %p %f %f %f %f ", renderObject->type().c_str(),
         renderObject->getStylePositionType(),
         renderObject, renderObject->getLayoutHeight(), renderObject->getLayoutWidth(),
         renderObject->getLayoutPositionLeft(), renderObject->getLayoutPositionRight());

    for(auto it = renderObject->ChildListIterBegin(); it != renderObject->ChildListIterEnd(); it++){
        RenderObject* child = static_cast<RenderObject*>(*it);
        showRenderObjectLayout(child, level + 1);
    }
}

static jint LayoutRenderObject(JNIEnv* env, jclass jcaller,
                               jlong ptr,
                               jfloat width,
                               jfloat height){
    if(width <= 0){
         width = WXCoreEnvironment::getInstance()->DeviceWidth();
    }

    std::pair<float,float> renderPageSize;
    renderPageSize.first= width;

    if(height > 0){
        renderPageSize.second = height;
    }else{
        renderPageSize.second = NAN;
    }

    RenderObject *render = convert_long_to_render_object(ptr);
    if(render->type() == WeexCore::kRenderCell || render->type() == WeexCore::kRenderCellSlot){
        RenderList* renderList = static_cast<RenderList*>(render->getParent());
        if(renderList != nullptr){
            if(renderList->TakeColumnCount() > 1  && renderList->TakeColumnWidth() > 0){
                renderPageSize.first = renderList->TakeColumnWidth();
            }
        }
    }

    if(render->getStyleWidth() != renderPageSize.first){
        render->setStyleWidth(renderPageSize.first, true);
        render->setStyleWidthLevel(CSS_STYLE);
    }

  render->LayoutBeforeImpl();
    render->calculateLayout(renderPageSize);
    render->LayoutAfterImpl();

    return (jint)render->getLayoutHeight();
}

static jint RenderObjectGetLayoutDirectionFromPathNode(JNIEnv* env, jclass jcaller,
                                   jlong ptr){
    RenderObject* renderObject = convert_long_to_render_object(ptr);
    return renderObject->getLayoutDirectionFromPathNode();
}

static jboolean RenderObjectHasNewLayout(JNIEnv* env, jclass jcaller,
                                     jlong ptr){
    RenderObject* renderObject = convert_long_to_render_object(ptr);
    return renderObject->hasNewLayout();
}

static jint RenderObjectChildCount(JNIEnv* env, jclass jcaller,
                                   jlong ptr){
    RenderObject* renderObject = convert_long_to_render_object(ptr);
    return renderObject->getChildCount();
}

static jlong RenderObjectGetChild(JNIEnv* env, jclass jcaller,
                                  jlong ptr,
                                  jint index){
    RenderObject* parent = convert_long_to_render_object(ptr);
    RenderObject* child = (RenderObject *) parent->getChildAt(index);
    return convert_render_object_to_long(child);
}

static void RenderObjectUpdateComponent(JNIEnv* env, jclass jcaller,
                                         jlong ptr,
                                         jobject component){
    RenderObject* renderObject = convert_long_to_render_object(ptr);
    float mTop = renderObject->getLayoutPositionTop();
    float mBottom = renderObject->getLayoutPositionBottom();
    float mRight = renderObject->getLayoutPositionRight();
    float mLeft = renderObject->getLayoutPositionLeft();
    float mHeight = renderObject->getLayoutHeight();
    float mWidth = renderObject->getLayoutWidth();
    Java_NativeRenderObjectUtils_updateComponentSize(env, component,mTop, mBottom,mLeft, mRight, mHeight, mWidth);
    renderObject->setHasNewLayout(false);
}


static void RenderObjectChildWaste(JNIEnv* env, jclass jcaller,
                                   jlong ptr,
                                   jboolean waster){
    RenderObject* renderObject = convert_long_to_render_object(ptr);
    if(waster){
        if(renderObject->getStylePositionType() != WXCorePositionType::kFixed){
            renderObject->setStylePositionType(WXCorePositionType::kFixed);
        }
    }else{
        if(renderObject->getStylePositionType() != WXCorePositionType::kRelative){
            renderObject->setStylePositionType(WXCorePositionType::kRelative);
        }
    }
}




static void AddChildRenderObject(JNIEnv* env, jclass jcaller,
                                 jlong parent,
                                 jlong child){
    RenderObject *renderParent = convert_long_to_render_object(parent);
    RenderObject *renderChild = convert_long_to_render_object(child);
    renderParent->AddRenderObject(-1, renderChild);
}
