//
// Created by YorkShen on 2017/11/22.
//

#include <gtest/gtest.h>
#include "WXCoreLayout.h"

using namespace WXCoreFlexLayout;
using namespace testing;

class FlexWrap : public TestWithParam<tuple<WXCoreFlexDirection, WXCoreFlexWrap>> {
protected:
    FlexWrap() {
        WXCoreFlexDirection direction = get<0>(GetParam());
        WXCoreFlexWrap wrap = get<1>(GetParam());
        root = WXCoreLayoutNode::newWXCoreNode();
        root->setStyleWidth(700);
        root->setStyleHeight(1000);
        root->setFlexDirection(direction);
        root->setFlexWrap(wrap);
        for (auto i = 0; i < 10; i++) {
            WXCoreLayoutNode *child = WXCoreLayoutNode::newWXCoreNode();
            child->setStyleWidth(200);
            child->setStyleHeight(300);
            root->addChildAt(child, i);
        }

    }

    ~FlexWrap() override {
        root->freeWXCoreNode();
        FormatingContext bfc = NON_BFC;
        for (auto i = 0; i < root->getChildCount(bfc); i++) {
            root->getChildAt(bfc, i)->freeWXCoreNode();
        }
    }

    WXCoreLayoutNode *root;
};

