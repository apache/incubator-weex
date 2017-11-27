//
// Created by YorkShen on 2017/11/27.
//

#include <gtest/gtest.h>
#include "WXCoreLayout.h"

using namespace WXCoreFlexLayout;

static constexpr float validWidth = 100;
static constexpr float boundaryWidth = 0;
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
    constexpr auto lvalue = 0;
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + targetWidth, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}

TEST_F(HorizontalAbsolute, WNullRNullL0) {
    constexpr auto lvalue = 0;
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
    constexpr auto lvalue = 30;
    target->setStylePosition(left, lvalue);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutHeight());
    EXPECT_FLOAT_EQ(lvalue, target->getLayoutPositionLeft());
    EXPECT_FLOAT_EQ(lvalue + targetWidth, target->getLayoutPositionRight());
    EXPECT_FLOAT_EQ(0, target->getLayoutPositionTop());
    EXPECT_FLOAT_EQ(targetHeight, target->getLayoutPositionBottom());
}