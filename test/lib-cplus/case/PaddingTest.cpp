//
// Created by YorkShen on 2017/11/22.
//

#include <gtest/gtest.h>
#include "../../../android/weex_core_debug/Source/Layout/WXCoreLayout.h"
#include "../../../android/weex_core_debug/Source/Layout/WXCoreFlexEnum.h"
using namespace WXCoreFlexLayout;

class Padding : public ::testing::Test{
protected:
    virtual void SetUp(){
        root  = WXCoreLayoutNode::newWXCoreNode();
        root -> setStyleWidth(100);
        root -> setStyleHeight(300);
        paddingAll = WXCore_Padding_ALL;
        paddingLeft = WXCore_Padding_Left;
        paddingTop = WXCore_Padding_Top;
        paddingRight = WXCore_Padding_Right;
        paddingBottom = WXCore_Padding_Bottom;
    }
    virtual void TearDown(){
        root -> freeWXCoreNode();
    }

    WXCoreLayoutNode *root;
    WXCorePaddingEdge paddingAll, paddingLeft, paddingTop, paddingRight, paddingBottom;
};

TEST_F(Padding, PaddingWidth0){
    float width = 0;
    root -> setPadding(paddingAll, width);
    root -> calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getPaddingLeft());
    EXPECT_FLOAT_EQ(width, root->getPaddingTop());
    EXPECT_FLOAT_EQ(width, root->getPaddingRight());
    EXPECT_FLOAT_EQ(width, root->getPaddingBottom());
}

TEST_F(Padding, PaddingWidth1){
    float width = 1.1;
    root -> setPadding(paddingAll, width);
    root -> calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getPaddingLeft());
    EXPECT_FLOAT_EQ(width, root->getPaddingTop());
    EXPECT_FLOAT_EQ(width, root->getPaddingRight());
    EXPECT_FLOAT_EQ(width, root->getPaddingBottom());
}

TEST_F(Padding, PaddingWidth200){
    float width = 200;
    root -> setPadding(paddingAll, width);
    root -> calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getPaddingLeft());
    EXPECT_FLOAT_EQ(width, root->getPaddingTop());
    EXPECT_FLOAT_EQ(width, root->getPaddingRight());
    EXPECT_FLOAT_EQ(width, root->getPaddingBottom());
}

TEST_F(Padding, PaddingWidthInvalid){
    float width = -30;
    root -> setPadding(paddingAll, width);
    root -> calculateLayout();
}

TEST_F(Padding, PaddingLeftWidth0){
    float width = 0;
    root -> setPadding(paddingLeft, width);
    root -> calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getPaddingLeft());
    EXPECT_FLOAT_EQ(width, root->getPaddingTop());
    EXPECT_FLOAT_EQ(width, root->getPaddingRight());
    EXPECT_FLOAT_EQ(width, root->getPaddingBottom());
}

TEST_F(Padding, PaddingLeftWidth1){
    float width = 1.1;
    root -> setPadding(paddingLeft, width);
    root -> calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getPaddingLeft());
    EXPECT_FLOAT_EQ(0, root->getPaddingTop());
    EXPECT_FLOAT_EQ(0, root->getPaddingRight());
    EXPECT_FLOAT_EQ(0, root->getPaddingBottom());
}

TEST_F(Padding, PaddingLeftWidth200){
    float width = 200;
    root -> setPadding(paddingLeft, width);
    root -> calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getPaddingLeft());
    EXPECT_FLOAT_EQ(0, root->getPaddingTop());
    EXPECT_FLOAT_EQ(0, root->getPaddingRight());
    EXPECT_FLOAT_EQ(0, root->getPaddingBottom());
}

TEST_F(Padding, PaddingLeftWidthInvalid){
    float width = -30;
    root -> setPadding(paddingLeft, width);
    root -> calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getPaddingTop());
    EXPECT_FLOAT_EQ(0, root->getPaddingRight());
    EXPECT_FLOAT_EQ(0, root->getPaddingBottom());
}

TEST_F(Padding, PaddingTopWidth0){
    float width = 0;
    root -> setPadding(paddingTop, width);
    root -> calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getPaddingLeft());
    EXPECT_FLOAT_EQ(width, root->getPaddingTop());
    EXPECT_FLOAT_EQ(width, root->getPaddingRight());
    EXPECT_FLOAT_EQ(width, root->getPaddingBottom());
}

TEST_F(Padding, PaddingTopWidth1){
    float width = 1.1;
    root -> setPadding(paddingTop, width);
    root -> calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getPaddingLeft());
    EXPECT_FLOAT_EQ(width, root->getPaddingTop());
    EXPECT_FLOAT_EQ(0, root->getPaddingRight());
    EXPECT_FLOAT_EQ(0, root->getPaddingBottom());
}

TEST_F(Padding, PaddingTopWidth200){
    float width = 200;
    root -> setPadding(paddingTop, width);
    root -> calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getPaddingLeft());
    EXPECT_FLOAT_EQ(width, root->getPaddingTop());
    EXPECT_FLOAT_EQ(0, root->getPaddingRight());
    EXPECT_FLOAT_EQ(0, root->getPaddingBottom());
}

TEST_F(Padding, PaddingTopWidthInvalid){
    float width = -30;
    root -> setPadding(paddingTop, width);
    root -> calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getPaddingLeft());
    EXPECT_FLOAT_EQ(0, root->getPaddingRight());
    EXPECT_FLOAT_EQ(0, root->getPaddingBottom());
}

TEST_F(Padding, PaddingRightWidth0){
    float width = 0;
    root -> setPadding(paddingRight, width);
    root -> calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getPaddingLeft());
    EXPECT_FLOAT_EQ(width, root->getPaddingTop());
    EXPECT_FLOAT_EQ(width, root->getPaddingRight());
    EXPECT_FLOAT_EQ(width, root->getPaddingBottom());
}

TEST_F(Padding, PaddingRightWidth1){
    float width = 1.1;
    root -> setPadding(paddingRight, width);
    root -> calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getPaddingLeft());
    EXPECT_FLOAT_EQ(0, root->getPaddingTop());
    EXPECT_FLOAT_EQ(width, root->getPaddingRight());
    EXPECT_FLOAT_EQ(0, root->getPaddingBottom());
}

TEST_F(Padding, PaddingRightWidth200){
    float width = 200;
    root -> setPadding(paddingRight, width);
    root -> calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getPaddingLeft());
    EXPECT_FLOAT_EQ(0, root->getPaddingTop());
    EXPECT_FLOAT_EQ(width, root->getPaddingRight());
    EXPECT_FLOAT_EQ(0, root->getPaddingBottom());
}

TEST_F(Padding, PaddingRightWidthInvalid){
    float width = -30;
    root -> setPadding(paddingRight, width);
    root -> calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getPaddingLeft());
    EXPECT_FLOAT_EQ(0, root->getPaddingTop());
    EXPECT_FLOAT_EQ(0, root->getPaddingBottom());
}

TEST_F(Padding, PaddingBottomWidth0){
    float width = 0;
    root -> setPadding(paddingBottom, width);
    root -> calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getPaddingLeft());
    EXPECT_FLOAT_EQ(width, root->getPaddingTop());
    EXPECT_FLOAT_EQ(width, root->getPaddingRight());
    EXPECT_FLOAT_EQ(width, root->getPaddingBottom());
}

TEST_F(Padding, PaddingBottomWidth1){
    float width = 1.1;
    root -> setPadding(paddingBottom, width);
    root -> calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getPaddingLeft());
    EXPECT_FLOAT_EQ(0, root->getPaddingTop());
    EXPECT_FLOAT_EQ(0, root->getPaddingRight());
    EXPECT_FLOAT_EQ(width, root->getPaddingBottom());
}

TEST_F(Padding, PaddingBottomWidth200){
    float width = 200;
    root -> setPadding(paddingBottom, width);
    root -> calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getPaddingLeft());
    EXPECT_FLOAT_EQ(0, root->getPaddingTop());
    EXPECT_FLOAT_EQ(0, root->getPaddingRight());
    EXPECT_FLOAT_EQ(width, root->getPaddingBottom());
}

TEST_F(Padding, PaddingBottomWidthInvalid){
    float width = -30;
    root -> setPadding(paddingBottom, width);
    root -> calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getPaddingLeft());
    EXPECT_FLOAT_EQ(0, root->getPaddingTop());
    EXPECT_FLOAT_EQ(0, root->getPaddingRight());
}

TEST_F(Padding, PaddingWidthShortHandOverrideEdge){
    root -> setPadding(paddingAll, 200);
    root -> setPadding(paddingTop, 400);
    root -> calculateLayout();
    EXPECT_FLOAT_EQ(200, root->getPaddingLeft());
    EXPECT_FLOAT_EQ(400, root->getPaddingTop());
    EXPECT_FLOAT_EQ(200, root->getPaddingRight());
    EXPECT_FLOAT_EQ(200, root->getPaddingBottom());
}

TEST_F(Padding, PaddingWidthEdgeOverrideShorthand){
    root -> setPadding(paddingTop, 400);
    root -> setPadding(paddingAll, 200);
    root -> calculateLayout();
    EXPECT_FLOAT_EQ(200, root->getPaddingLeft());
    EXPECT_FLOAT_EQ(200, root->getPaddingTop());
    EXPECT_FLOAT_EQ(200, root->getPaddingRight());
    EXPECT_FLOAT_EQ(200, root->getPaddingBottom());
}