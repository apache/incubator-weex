//
// Created by YorkShen on 2017/11/27.
//

#include <gtest/gtest.h>
#include "WXCoreLayout.h"

using namespace WXCoreFlexLayout;

static constexpr float validWidth = 100;
static constexpr float boundaryWidth = 0;
static constexpr float rootWidth = 700;
static constexpr float targetWidth = 300;
static constexpr float targetHeight = 400;
static constexpr float fooWidth = 200;

class HorizontalAbsolute : public ::testing::Test {
protected:
    virtual void SetUp() {
        direction = WXCore_Flex_Direction_Row;
        positionType = WXCore_PositionType_Absolute;
        left = WXCore_PositionEdge_Left;
        right = WXCore_PositionEdge_Right;
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
    }

    WXCoreLayoutNode *root, *foo, *targetChild, *target;
    WXCoreFlexDirection direction;
    WXCorePositionType positionType;
    WXCorePositionEdge left, right;
};


TEST_F(HorizontalAbsolute, WNullRNullLNull) {
    constexpr float lvalue = 0;
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + targetWidth, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalAbsolute, WNullRNullL0) {
    constexpr float lvalue = 0;
    target->setStylePosition(left, lvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + targetWidth, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalAbsolute, WNullRNullL30) {
    constexpr float lvalue = 30;
    target->setStylePosition(left, lvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + targetWidth, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalAbsolute, WNullRNullLN10) {
    constexpr float lvalue = -10;
    target->setStylePosition(left, lvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + targetWidth, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalAbsolute, WNullR0LNull) {
    constexpr float lvalue = 0;
    constexpr float rvalue = 0;
    target->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(rootWidth - rvalue - targetWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(rootWidth - rvalue, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalAbsolute, WNullR0L0) {
    constexpr float lvalue = 0;
    constexpr float rvalue = 0;
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(rootWidth - rvalue - lvalue, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(rootWidth - rvalue, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalAbsolute, WNullR0L30) {
    constexpr float lvalue = 30;
    constexpr float rvalue = 0;
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(rootWidth - rvalue - lvalue, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(rootWidth - rvalue, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalAbsolute, WNullR0LN10) {
    constexpr float lvalue = -10;
    constexpr float rvalue = 0;
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(rootWidth - rvalue - lvalue, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(rootWidth - rvalue, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalAbsolute, WNullR70LNull) {
    constexpr float lvalue = 0;
    constexpr float rvalue = 70;
    target->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(rootWidth - rvalue - targetWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(rootWidth - rvalue, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalAbsolute, WNullR70L0) {
    constexpr float lvalue = 0;
    constexpr float rvalue = 70;
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(rootWidth - rvalue - lvalue, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(rootWidth - rvalue, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalAbsolute, WNullR70L30) {
    constexpr float lvalue = 30;
    constexpr float rvalue = 70;
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(rootWidth - rvalue - lvalue, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(rootWidth - rvalue, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalAbsolute, WNullR70LN10) {
    constexpr float lvalue = -10;
    constexpr float rvalue = 70;
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(rootWidth - rvalue - lvalue, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(rootWidth - rvalue, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalAbsolute, WNullRN20LNull) {
    constexpr float lvalue = 0;
    constexpr float rvalue = -20;
    target->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(rootWidth - rvalue - targetWidth, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(rootWidth - rvalue, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalAbsolute, WNullRN20L0) {
    constexpr float lvalue = 0;
    constexpr float rvalue = -20;
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(rootWidth - rvalue - lvalue, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(rootWidth - rvalue, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalAbsolute, WNullRN20L30) {
    constexpr float lvalue = 30;
    constexpr float rvalue = -20;
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(rootWidth - rvalue - lvalue, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(rootWidth - rvalue, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalAbsolute, WNullRN20LN10) {
    constexpr float lvalue = -10;
    constexpr float rvalue = -20;
    target->setStylePosition(left, lvalue);
    target->setStylePosition(right, rvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(rootWidth - rvalue - lvalue, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(rootWidth - rvalue, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}