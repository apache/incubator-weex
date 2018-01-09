//
// Created by YorkShen on 2017/11/22.
//

#include <gtest/gtest.h>
#include "WXCoreLayout.h"

using namespace WeexCore;

static constexpr float valid_width = 100;
static constexpr float boundary_width = 0;
static constexpr float rootWidth = 700;
static constexpr float targetWidth = 300;
static constexpr float targetHeight = 400;
static constexpr float fooWidth = 200;

class HorizontalRelative : public ::testing::Test {
protected:
    virtual void SetUp() {
        direction = kFlexDirectionRow;
        positionType = kRelative;
        left = kPositionEdgeLeft;
        right = kPositionEdgeRight;
        foo = WXCoreLayoutNode::newWXCoreNode();
        targetChild = WXCoreLayoutNode::newWXCoreNode();
        target = WXCoreLayoutNode::newWXCoreNode();
        root = WXCoreLayoutNode::newWXCoreNode();

        foo->setStyleWidth(fooWidth);
        foo->setStyleHeight(100);
        targetChild->setStyleWidth(targetWidth);
        targetChild->setStyleHeight(50);

        target->setStyleHeight(targetHeight);
        target->setStylePositionType(positionType);
        root->setFlexDirection(direction);
        root->setStyleHeight(300);
        root->setStyleWidth(rootWidth);

        target->appendChild(targetChild);
        root->appendChild(foo);
        root->appendChild(target);
    }

    virtual void TearDown() {
        root->freeWXCoreNode();
        foo->freeWXCoreNode();
        target->freeWXCoreNode();
        targetChild->freeWXCoreNode();
    }

    WXCoreLayoutNode *root, *foo, *targetChild, *target;
    WXCoreFlexDirection direction;
    WXCorePositionType positionType;
    WXCorePositionEdge left, right;
};

TEST_F(HorizontalRelative, LNullRNullWNull) {
    constexpr float lvalue = 0;
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + targetWidth, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L0RNullWNull) {
    constexpr float lvalue = 0;
    target->setStylePosition(left, lvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + targetWidth, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L30RNullWNull) {
    constexpr float lvalue = 30;
    target->setStylePosition(left, lvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + targetWidth, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LN10RNullWNull) {
    constexpr float lvalue = -10;
    target->setStylePosition(left, lvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + targetWidth, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LNullR0WNull) {
    constexpr float lvalue = 0;
    constexpr float rvalue = 0;
    target->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + rvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + rvalue + fooWidth + targetWidth, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L0R0WNull) {
    constexpr float lvalue = 0;
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + targetWidth, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L30R0WNull) {
    constexpr float lvalue = 30;
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + targetWidth, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LN10R0WNull) {
    constexpr float lvalue = -10;
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + targetWidth, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LNullR70WNull) {
    constexpr float lvalue = 0;
    constexpr float rvalue = 70;
    target->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(fooWidth - rvalue, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(fooWidth - rvalue + targetWidth, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L0R70WNull) {
    constexpr float lvalue = 0;
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, -20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + targetWidth, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L30R70WNull) {
    constexpr float lvalue = 30;
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, 70);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + targetWidth, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LN10R70WNull) {
    constexpr float lvalue = -10;
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, 70);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + targetWidth, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LNullRN20WNull) {
    constexpr float lvalue = 0;
    constexpr float rvalue = -20;
    target->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue - rvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue - rvalue + fooWidth + targetWidth, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L0RN20WNull) {
    constexpr float lvalue = 0;
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, -20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + targetWidth, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L30RN20WNull) {
    constexpr float lvalue = 30;
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, -20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + targetWidth, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LN10RN20WNull) {
    constexpr float lvalue = -10;
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, -20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + targetWidth, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LNullRNullW100) {
    constexpr float lvalue = 0;
    target->setStyleWidth(valid_width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + valid_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L0RNullW100) {
    constexpr float lvalue = 0;
    target->setStyleWidth(valid_width);
    target->setStylePosition(left, lvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + valid_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L30RNullW100) {
    constexpr float lvalue = 30;
    target->setStyleWidth(valid_width);
    target->setStylePosition(left, lvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + valid_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LN10RNullW100) {
    constexpr float lvalue = -10;
    target->setStyleWidth(valid_width);
    target->setStylePosition(left, lvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + valid_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LNullR0W100) {
    constexpr float lvalue = 0;
    constexpr float rvalue = 0;
    target->setStyleWidth(valid_width);
    target->setStylePosition(right, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + rvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + rvalue + fooWidth + valid_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L0R0W100) {
    constexpr float lvalue = 0;
    target->setStyleWidth(valid_width);
    target->setStylePosition(left, 0);
    target->setStylePosition(right, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + valid_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L30R0W100) {
    constexpr float lvalue = 30;
    target->setStyleWidth(valid_width);
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + valid_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LN10R0W100) {
    constexpr float lvalue = -10;
    target->setStyleWidth(valid_width);
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + valid_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LNullR70W100) {
    constexpr float lvalue = 0;
    constexpr float rvalue = 70;
    target->setStyleWidth(valid_width);
    target->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(fooWidth - rvalue, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(fooWidth - rvalue + valid_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L0R70W100) {
    constexpr float lvalue = 0;
    target->setStyleWidth(valid_width);
    target->setStylePosition(left, 0);
    target->setStylePosition(right, -20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + valid_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L30R70W100) {
    constexpr float lvalue = 30;
    target->setStyleWidth(valid_width);
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, 70);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + valid_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LN10R70W100) {
    constexpr float lvalue = -10;
    target->setStyleWidth(valid_width);
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, 70);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + valid_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LNullRN20W100) {
    constexpr float lvalue = 0;
    constexpr float rvalue = -20;
    target->setStyleWidth(valid_width);
    target->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue - rvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue - rvalue + fooWidth + valid_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L0RN20W100) {
    constexpr float lvalue = 0;
    target->setStyleWidth(valid_width);
    target->setStylePosition(left, 0);
    target->setStylePosition(right, -20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + valid_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L30RN20W100) {
    constexpr float lvalue = 30;
    target->setStyleWidth(valid_width);
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, -20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + valid_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LN10RN20W100) {
    constexpr float lvalue = -10;
    target->setStyleWidth(valid_width);
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, -20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(valid_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + valid_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}


TEST_F(HorizontalRelative, LNullRNullW0) {
    constexpr float lvalue = 0;
    target->setStyleWidth(boundary_width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + boundary_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L0RNullW0) {
    constexpr float lvalue = 0;
    target->setStyleWidth(boundary_width);
    target->setStylePosition(left, lvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + boundary_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L30RNullW0) {
    constexpr float lvalue = 30;
    target->setStyleWidth(boundary_width);
    target->setStylePosition(left, lvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + boundary_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LN10RNullW0) {
    constexpr float lvalue = -10;
    target->setStyleWidth(boundary_width);
    target->setStylePosition(left, lvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + boundary_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LNullR0W0) {
    constexpr float lvalue = 0;
    constexpr float rvalue = 0;
    target->setStyleWidth(boundary_width);
    target->setStylePosition(right, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + rvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + rvalue + fooWidth + boundary_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L0R0W0) {
    constexpr float lvalue = 0;
    target->setStyleWidth(boundary_width);
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + boundary_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L30R0W0) {
    constexpr float lvalue = 30;
    target->setStyleWidth(boundary_width);
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + boundary_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LN10R0W0) {
    constexpr float lvalue = -10;
    target->setStyleWidth(boundary_width);
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, 0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + boundary_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LNullR70W0) {
    constexpr float lvalue = 0;
    constexpr float rvalue = 70;
    target->setStyleWidth(boundary_width);
    target->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(fooWidth - rvalue, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(fooWidth - rvalue + boundary_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L0R70W0) {
    constexpr float lvalue = 0;
    target->setStyleWidth(boundary_width);
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, -20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + boundary_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L30R70W0) {
    constexpr float lvalue = 30;
    target->setStyleWidth(boundary_width);
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, 70);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + boundary_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LN10R70W0) {
    constexpr float lvalue = -10;
    target->setStyleWidth(boundary_width);
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, 70);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + boundary_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LNullRN20W0) {
    constexpr float lvalue = 0;
    constexpr float rvalue = -20;
    target->setStyleWidth(boundary_width);
    target->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue - rvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue - rvalue + fooWidth + boundary_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L0RN20W0) {
    constexpr float lvalue = 0;
    target->setStyleWidth(boundary_width);
    target->setStylePosition(left, 0);
    target->setStylePosition(right, -20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + boundary_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, L30RN20W0) {
    constexpr float lvalue = 30;
    target->setStyleWidth(boundary_width);
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, -20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + boundary_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalRelative, LN10RN20W0) {
    constexpr float lvalue = -10;
    target->setStyleWidth(boundary_width);
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, -20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(boundary_width, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue + fooWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + fooWidth + boundary_width, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}