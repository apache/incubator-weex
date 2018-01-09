//
// Created by YorkShen on 2017/12/2.
//

#include <gtest/gtest.h>
#include "WXCoreLayout.h"

using namespace WeexCore;
using namespace testing;
using namespace std;

constexpr float childWidth = 150;
constexpr float childHeight = 100;
constexpr float rootWidth = 750;
constexpr int childCount = 20;

class FlexWrapDimension : public Test {
protected:
    FlexWrapDimension() {
        root = WXCoreLayoutNode::newWXCoreNode();
        target = WXCoreLayoutNode::newWXCoreNode();

        root->setStyleWidth(750);
        root->setStyleHeight(1200);

        for (auto i = 0; i < childCount; i++) {
            WXCoreLayoutNode *child = WXCoreLayoutNode::newWXCoreNode();
            child->setStyleWidth(childWidth);
            child->setStyleHeight(childHeight);
            lastChildInTarget = child;
            target->appendChild(child);
        }
        root->appendChild(target);
    }

    ~FlexWrapDimension() override {
        root->freeWXCoreNode();
        target->freeWXCoreNode();
        FormattingContext bfc = kNonBFC;
        for (auto i = 0; i < root->getChildCount(bfc); i++) {
            root->getChildAt(bfc, i)->freeWXCoreNode();
        }
    }

    WXCoreLayoutNode *root, *target, *lastChildInTarget;
};

TEST_F(FlexWrapDimension, RowNoWrap) {
    target->setFlexDirection(kFlexDirectionRow);
    target->setFlexWrap(kNoWrap);
    root->calculateLayout();

    EXPECT_FLOAT_EQ(root->getLayoutWidth(), rootWidth);
    EXPECT_FLOAT_EQ(target->getLayoutHeight(), childHeight);

    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionTop(), 0);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionLeft(), (childCount - 1) * childWidth);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionRight(), childCount * childWidth);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionBottom(), childHeight);
}

TEST_F(FlexWrapDimension, RowWrap) {
    target->setFlexDirection(kFlexDirectionRow);
    target->setFlexWrap(kWrap);
    root->calculateLayout();

    auto flexCap = static_cast<int>(rootWidth / childWidth);
    int linesBeforeLast = childCount / flexCap - (childCount % flexCap == 0 ? 1 : 0);
    int itemsBeforeLast = childCount % flexCap == 0 ? flexCap - 1 : (childCount % flexCap - 1);

    EXPECT_FLOAT_EQ(target->getLayoutWidth(), rootWidth);
    EXPECT_FLOAT_EQ(target->getLayoutHeight(), (linesBeforeLast + 1) * childHeight);

    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionTop(), linesBeforeLast * childHeight);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionBottom(), (linesBeforeLast + 1) * childHeight);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionLeft(), itemsBeforeLast * childWidth);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionRight(), (itemsBeforeLast + 1) * childWidth);
}

TEST_F(FlexWrapDimension, ColumnNoWrap) {
    target->setFlexDirection(kFlexDirectionColumn);
    target->setFlexWrap(kNoWrap);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(root->getLayoutWidth(), rootWidth);
    EXPECT_FLOAT_EQ(target->getLayoutHeight(), childHeight * childCount);

    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionLeft(), 0);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionRight(), childWidth);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionTop(), (childCount - 1) * childHeight);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionBottom(), childCount * childHeight);
}

TEST_F(FlexWrapDimension, ColumnWrap) {
    target->setFlexDirection(kFlexDirectionColumn);
    target->setFlexWrap(kWrap);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(root->getLayoutWidth(), rootWidth);
    EXPECT_FLOAT_EQ(target->getLayoutHeight(), childHeight * childCount);

    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionLeft(), 0);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionRight(), childWidth);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionTop(), (childCount - 1) * childHeight);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionBottom(), childCount * childHeight);
}
