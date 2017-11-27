//
// Created by YorkShen on 2017/11/22.
//

#include <gtest/gtest.h>
#include "WXCoreLayout.h"

using namespace WXCoreFlexLayout;

static constexpr float valid_width = 100;
static constexpr float boundary_width = 0;

class HorizontalRelative : public ::testing::Test {
protected:
    virtual void SetUp() {
        positionType = WXCore_PositionType_Relative;
        left = WXCore_PositionEdge_Left;
        right = WXCore_PositionEdge_Right;
        child = WXCoreLayoutNode::newWXCoreNode();
        root = WXCoreLayoutNode::newWXCoreNode();

        child->setStyleWidth(400);
        child->setStyleHeight(100);

        root->setStyleHeight(300);
        root->addChildAt(child, 0);
        root->setStylePositionType(positionType);
    }

    virtual void TearDown() {
        root->freeWXCoreNode();
    }

    WXCoreLayoutNode *root, *child;
    WXCorePositionType positionType;
    WXCorePositionEdge left, right;
};

TEST_F(HorizontalRelative, LNullRNullWNull) {
    root->calculateLayout();
    EXPECT_FLOAT_EQ(400, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(400, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L0RNullWNull) {
    root->setStylePosition(left, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(400, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(400, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L30RNullWNull) {
    const float lvalue = 30;
    root->setStylePosition(left, lvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(400, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(400 + lvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LN10RNullWNull) {
    const float lvalue = -10;
    root->setStylePosition(left, lvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(400, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(400 + lvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LNullR0WNull) {
    root->setStylePosition(right, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(400, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(400, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L0R0WNull) {
    root->setStylePosition(left, 0);
    root->setStylePosition(right, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(400, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(400, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L30R0WNull) {
    const float lvalue = 30;
    root->setStylePosition(left, lvalue);
    root->setStylePosition(right, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(400, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(400 + lvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LN10R0WNull) {
    const float lvalue = -10;
    root->setStylePosition(left, lvalue);
    root->setStylePosition(right, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(400, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(400 + lvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LNullR70WNull) {
    const float rvalue = 70;
    root->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(400, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(0 - rvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(400 - rvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L0R70WNull) {
    root->setStylePosition(left, 0);
    root->setStylePosition(right, -20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(400, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(400, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L30R70WNull) {
    const float lvalue = 30;
    root->setStylePosition(left, lvalue);
    root->setStylePosition(right, 70);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(400, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(400 + lvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LN10R70WNull) {
    const float lvalue = -10;
    root->setStylePosition(left, lvalue);
    root->setStylePosition(right, 70);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(400, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(400 + lvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LNullRN20WNull) {
    const float rvalue = -20;
    root->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(400, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(0 - rvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(400 - rvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L0RN20WNull) {
    root->setStylePosition(left, 0);
    root->setStylePosition(right, -20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(400, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(400, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L30RN20WNull) {
    const float lvalue = 30;
    root->setStylePosition(left, lvalue);
    root->setStylePosition(right, -20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(400, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(400 + lvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LN10RN20WNull) {
    const float lvalue = -10;
    root->setStylePosition(left, lvalue);
    root->setStylePosition(right, -20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(400, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(400 + lvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LNullRNullW100) {
    root->setStyleWidth(valid_width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(valid_width, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L0RNullW100) {
    root->setStyleWidth(valid_width);
    root->setStylePosition(left, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(valid_width, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L30RNullW100) {
    const float lvalue = 30;
    root->setStyleWidth(valid_width);
    root->setStylePosition(left, lvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(valid_width + lvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LN10RNullW100) {
    const float lvalue = -10;
    root->setStyleWidth(valid_width);
    root->setStylePosition(left, lvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(valid_width + lvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LNullR0W100) {
    root->setStyleWidth(valid_width);
    root->setStylePosition(right, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(valid_width, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L0R0W100) {
    root->setStyleWidth(valid_width);
    root->setStylePosition(left, 0);
    root->setStylePosition(right, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(valid_width, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L30R0W100) {
    const float lvalue = 30;
    root->setStyleWidth(valid_width);
    root->setStylePosition(left, lvalue);
    root->setStylePosition(right, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(valid_width + lvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LN10R0W100) {
    const float lvalue = -10;
    root->setStyleWidth(valid_width);
    root->setStylePosition(left, lvalue);
    root->setStylePosition(right, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(valid_width + lvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LNullR70W100) {
    const float rvalue = 70;
    root->setStyleWidth(valid_width);
    root->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(0 - rvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(valid_width - rvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L0R70W100) {
    root->setStyleWidth(valid_width);
    root->setStylePosition(left, 0);
    root->setStylePosition(right, -20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(valid_width, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L30R70W100) {
    const float lvalue = 30;
    root->setStyleWidth(valid_width);
    root->setStylePosition(left, lvalue);
    root->setStylePosition(right, 70);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(valid_width + lvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LN10R70W100) {
    const float lvalue = -10;
    root->setStyleWidth(valid_width);
    root->setStylePosition(left, lvalue);
    root->setStylePosition(right, 70);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(valid_width + lvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LNullRN20W100) {
    const float rvalue = -20;
    root->setStyleWidth(valid_width);
    root->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(0 - rvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(valid_width - rvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L0RN20W100) {
    root->setStyleWidth(valid_width);
    root->setStylePosition(left, 0);
    root->setStylePosition(right, -20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(valid_width, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L30RN20W100) {
    const float lvalue = 30;
    root->setStyleWidth(valid_width);
    root->setStylePosition(left, lvalue);
    root->setStylePosition(right, -20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(valid_width + lvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LN10RN20W100) {
    const float lvalue = -10;
    root->setStyleWidth(valid_width);
    root->setStylePosition(left, lvalue);
    root->setStylePosition(right, -20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(valid_width + lvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}


TEST_F(HorizontalRelative, LNullRNullW0) {
    root->setStyleWidth(boundary_width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(boundary_width, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L0RNullW0) {
    root->setStyleWidth(boundary_width);
    root->setStylePosition(left, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(boundary_width, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L30RNullW0) {
    const float lvalue = 30;
    root->setStyleWidth(boundary_width);
    root->setStylePosition(left, lvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(boundary_width + lvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LN10RNullW0) {
    const float lvalue = -10;
    root->setStyleWidth(boundary_width);
    root->setStylePosition(left, lvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(boundary_width + lvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LNullR0W0) {
    root->setStyleWidth(boundary_width);
    root->setStylePosition(right, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(boundary_width, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L0R0W0) {
    root->setStyleWidth(boundary_width);
    root->setStylePosition(left, 0);
    root->setStylePosition(right, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(boundary_width, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L30R0W0) {
    const float lvalue = 30;
    root->setStyleWidth(boundary_width);
    root->setStylePosition(left, lvalue);
    root->setStylePosition(right, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(boundary_width + lvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LN10R0W0) {
    const float lvalue = -10;
    root->setStyleWidth(boundary_width);
    root->setStylePosition(left, lvalue);
    root->setStylePosition(right, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(boundary_width + lvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LNullR70W0) {
    const float rvalue = 70;
    root->setStyleWidth(boundary_width);
    root->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(0 - rvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(boundary_width - rvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L0R70W0) {
    root->setStyleWidth(boundary_width);
    root->setStylePosition(left, 0);
    root->setStylePosition(right, -20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(boundary_width, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L30R70W0) {
    const float lvalue = 30;
    root->setStyleWidth(boundary_width);
    root->setStylePosition(left, lvalue);
    root->setStylePosition(right, 70);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(boundary_width + lvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LN10R70W0) {
    const float lvalue = -10;
    root->setStyleWidth(boundary_width);
    root->setStylePosition(left, lvalue);
    root->setStylePosition(right, 70);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(boundary_width + lvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LNullRN20W0) {
    const float rvalue = -20;
    root->setStyleWidth(boundary_width);
    root->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(0 - rvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(boundary_width - rvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L0RN20W0) {
    root->setStyleWidth(boundary_width);
    root->setStylePosition(left, 0);
    root->setStylePosition(right, -20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(boundary_width, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L30RN20W0) {
    const float lvalue = 30;
    root->setStyleWidth(boundary_width);
    root->setStylePosition(left, lvalue);
    root->setStylePosition(right, -20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(boundary_width + lvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LN10RN20W0) {
    const float lvalue = -10;
    root->setStyleWidth(boundary_width);
    root->setStylePosition(left, lvalue);
    root->setStylePosition(right, -20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, root->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(boundary_width + lvalue, root->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, root->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(300, root->getLayoutPositionBottom());
}