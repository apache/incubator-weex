//
// Created by YorkShen on 2017/11/22.
//

#include <gtest/gtest.h>
#include "../../../android/weex_core_debug/Source/Layout/WXCoreLayout.h"
using namespace WXCoreFlexLayout;

class Vertical: public ::testing::Test{
protected:
    virtual void SetUp(){
        root  = WXCoreLayoutNode::newWXCoreNode();
        root->setStyleHeight(300);
    }
    virtual void TearDown(){
        root -> freeWXCoreNode();
    }

    WXCoreLayoutNode *root;
};

TEST_F(Vertical, RelativeLNullRNullWNull){
    root->calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
}