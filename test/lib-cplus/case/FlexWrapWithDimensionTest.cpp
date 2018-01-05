//
// Created by YorkShen on 2017/11/22.
//

#include <gtest/gtest.h>
#include "WXCoreLayout.h"

using namespace WeexCore;
using namespace testing;
using namespace std;

constexpr float margin = 10;
constexpr float childWidth = 150;
constexpr float childHeight = 100;
constexpr float targetWidth = 700;
constexpr float targetHeight = 1000;
constexpr int childCount = 10;

class FlexWrap : public Test {
protected:
    FlexWrap() {
        root = WXCoreLayoutNode::newWXCoreNode();
        target = WXCoreLayoutNode::newWXCoreNode();
        firstChildInTarget = nullptr;

        root->setStyleWidth(750);
        root->setStyleHeight(1200);
        target->setStyleWidth(targetWidth);
        target->setStyleHeight(targetHeight);

        for (auto i = 0; i < childCount; i++) {
            WXCoreLayoutNode *child = WXCoreLayoutNode::newWXCoreNode();
            if (firstChildInTarget == nullptr) {
                firstChildInTarget = child;
            }
            child->setMargin(kMarginALL, margin);
            child->setPadding(kPaddingALL, 10);
            child->setBorderWidth(kBorderWidthALL, 10);
            child->setStyleWidth(childWidth);
            child->setStyleHeight(childHeight);
            lastChildInTarget = child;
            target->appendChild(child);
        }
        root->appendChild(target);
    }

    ~FlexWrap() override {
        root->freeWXCoreNode();
        target->freeWXCoreNode();
        FormattingContext bfc = kNonBFC;
        for (auto i = 0; i < root->getChildCount(bfc); i++) {
            root->getChildAt(bfc, i)->freeWXCoreNode();
        }
    }

    WXCoreLayoutNode *root, *target, *firstChildInTarget, *lastChildInTarget;
};

TEST_F(FlexWrap, NoWrapRow) {
    target->setFlexWrap(kNoWrap);
    target->setFlexDirection(kFlexDirectionRow);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionLeft(), margin);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionRight(), margin + childWidth);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionTop(), margin);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionBottom(), margin + childHeight);

    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionLeft(), (childCount - 1) * (childWidth + 2 * margin) + margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionRight(), (childCount) * (childWidth + 2 * margin) - margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionTop(), margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionBottom(), margin + childHeight);
}

TEST_F(FlexWrap, NoWrapRowReverse) {
    target->setFlexWrap(kNoWrap);
    target->setFlexDirection(kFlexDirectionRowReverse);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionLeft(), targetWidth - margin - childWidth);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionRight(), targetWidth - margin);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionTop(), margin);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionBottom(), margin + childHeight);

    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionLeft(),
                    targetWidth - (childCount) * (childWidth + 2 * margin) + margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionRight(),
                    targetWidth - (childCount - 1) * (childWidth + 2 * margin) - margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionTop(), margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionBottom(), margin + childHeight);
}

TEST_F(FlexWrap, NoWrapColumn) {
    target->setFlexWrap(kNoWrap);
    target->setFlexDirection(kFlexDirectionColumn);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionLeft(), margin);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionRight(), margin + childWidth);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionTop(), margin);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionBottom(), margin + childHeight);

    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionLeft(), margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionRight(), margin + childWidth);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionTop(), (childCount - 1) * (childHeight + 2 * margin) + margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionBottom(), (childCount) * (childHeight + 2 * margin) - margin);
}

TEST_F(FlexWrap, NoWrapColumnReverse) {
    target->setFlexWrap(WXCore_Wrap_NoWrap);
    target->setFlexDirection(WXCore_Flex_Direction_Column_Reverse);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionLeft(), margin);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionRight(), margin + childWidth);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionTop(), targetHeight - margin - childHeight);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionBottom(), targetHeight - margin);

    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionLeft(), margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionRight(), margin + childWidth);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionTop(),
                    targetHeight - (childCount) * (childHeight + 2 * margin) + margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionBottom(),
                    targetHeight - (childCount - 1) * (childHeight + 2 * margin) - margin);
}


TEST_F(FlexWrap, WrapRow) {
    target->setFlexWrap(WXCore_Wrap_Wrap);
    target->setFlexDirection(WXCore_Flex_Direction_Row);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionLeft(), margin);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionRight(), margin + childWidth);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionTop(), margin);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionBottom(), margin + childHeight);

    auto flexItemCap = static_cast<int>(targetWidth / (childWidth + 2 * margin));
    int linesBeforeLast = childCount / flexItemCap;
    int itemsBeforeLast = childCount % flexItemCap - 1;

    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionLeft(), itemsBeforeLast * (childWidth + 2 * margin) + margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionRight(),
                    (itemsBeforeLast + 1) * (childWidth + 2 * margin) - margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionTop(), linesBeforeLast * (childHeight + margin * 2) + margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionBottom(),
                    (linesBeforeLast + 1) * (childHeight + margin * 2) - margin);
}

TEST_F(FlexWrap, WrapRowReverse) {
    target->setFlexWrap(WXCore_Wrap_Wrap);
    target->setFlexDirection(WXCore_Flex_Direction_Row_Reverse);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionLeft(), targetWidth - margin - childWidth);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionRight(), targetWidth - margin);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionTop(), margin);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionBottom(), margin + childHeight);

    auto flexItemCap = static_cast<int>(targetWidth / (childWidth + 2 * margin));
    int linesBeforeLast = childCount / flexItemCap;
    int itemsBeforeLast = childCount % flexItemCap - 1;

    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionLeft(),
                    targetWidth - (itemsBeforeLast + 1) * (childWidth + 2 * margin) + margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionRight(),
                    targetWidth - itemsBeforeLast * (childWidth + 2 * margin) - margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionTop(), linesBeforeLast * (childHeight + margin * 2) + margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionBottom(),
                    (linesBeforeLast + 1) * (childHeight + margin * 2) - margin);
}

TEST_F(FlexWrap, WrapColumn) {
    target->setFlexWrap(WXCore_Wrap_Wrap);
    target->setFlexDirection(WXCore_Flex_Direction_Column);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionLeft(), margin);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionRight(), margin + childWidth);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionTop(), margin);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionBottom(), margin + childHeight);

    auto flexItemCap = static_cast<int>(targetHeight / (childHeight + 2 * margin));
    int linesBeforeLast = childCount / flexItemCap;
    int itemsBeforeLast = childCount % flexItemCap - 1;

    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionLeft(), linesBeforeLast * (childWidth + margin * 2) + margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionRight(),
                    (linesBeforeLast + 1) * (childWidth + margin * 2) - margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionTop(), itemsBeforeLast * (childHeight + 2 * margin) + margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionBottom(),
                    (itemsBeforeLast + 1) * (childHeight + 2 * margin) - margin);
}

TEST_F(FlexWrap, WrapColumnReverse) {
    target->setFlexWrap(WXCore_Wrap_Wrap);
    target->setFlexDirection(WXCore_Flex_Direction_Column_Reverse);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionLeft(), margin);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionRight(), margin + childWidth);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionTop(), targetHeight - margin - childHeight);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionBottom(), targetHeight - margin);

    auto flexItemCap = static_cast<int>(targetHeight / (childHeight + 2 * margin));
    int linesBeforeLast = childCount / flexItemCap;
    int itemsBeforeLast = childCount % flexItemCap - 1;

    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionLeft(), linesBeforeLast * (childWidth + margin * 2) + margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionRight(),
                    (linesBeforeLast + 1) * (childWidth + margin * 2) - margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionTop(),
                    targetHeight - (itemsBeforeLast + 1) * (childHeight + 2 * margin) + margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionBottom(),
                    targetHeight - (itemsBeforeLast) * (childHeight + 2 * margin) - margin);
}


TEST_F(FlexWrap, WrapReverseRow) {
    target->setFlexWrap(WXCore_Wrap_WrapReverse);
    target->setFlexDirection(WXCore_Flex_Direction_Row);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionLeft(), margin);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionRight(), margin + childWidth);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionTop(), targetHeight - margin - childHeight);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionBottom(), targetHeight - margin);

    auto flexItemCap = static_cast<int>(targetWidth / (childWidth + 2 * margin));
    int linesBeforeLast = childCount / flexItemCap;
    int itemsBeforeLast = childCount % flexItemCap - 1;

    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionLeft(), itemsBeforeLast * (childWidth + 2 * margin) + margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionRight(),
                    (itemsBeforeLast + 1) * (childWidth + 2 * margin) - margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionTop(),
                    targetHeight - (linesBeforeLast + 1) * (childHeight + margin * 2) + margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionBottom(),
                    targetHeight - linesBeforeLast * (childHeight + margin * 2) - margin);
}

TEST_F(FlexWrap, WrapReverseRowReverse) {
    target->setFlexWrap(WXCore_Wrap_WrapReverse);
    target->setFlexDirection(WXCore_Flex_Direction_Row_Reverse);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionLeft(), targetWidth - margin - childWidth);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionRight(), targetWidth - margin);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionTop(), targetHeight - margin - childHeight);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionBottom(), targetHeight - margin);

    auto flexItemCap = static_cast<int>(targetWidth / (childWidth + 2 * margin));
    int linesBeforeLast = childCount / flexItemCap;
    int itemsBeforeLast = childCount % flexItemCap - 1;

    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionLeft(),
                    targetWidth - (itemsBeforeLast + 1) * (childWidth + 2 * margin) + margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionRight(),
                    targetWidth - itemsBeforeLast * (childWidth + 2 * margin) - margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionTop(),
                    targetHeight - (linesBeforeLast + 1) * (childHeight + margin * 2) + margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionBottom(),
                    targetHeight - linesBeforeLast * (childHeight + margin * 2) - margin);
}

TEST_F(FlexWrap, WrapReverseColumn) {
    target->setFlexWrap(WXCore_Wrap_WrapReverse);
    target->setFlexDirection(WXCore_Flex_Direction_Column);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionLeft(), targetWidth - margin - childWidth);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionRight(), targetWidth - margin);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionTop(), margin);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionBottom(), margin + childHeight);

    auto flexItemCap = static_cast<int>(targetHeight / (childHeight + 2 * margin));
    int linesBeforeLast = childCount / flexItemCap;
    int itemsBeforeLast = childCount % flexItemCap - 1;

    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionLeft(),
                    targetWidth - (linesBeforeLast + 1) * (childWidth + margin * 2) + margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionRight(),
                    targetWidth - linesBeforeLast * (childWidth + margin * 2) - margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionTop(), itemsBeforeLast * (childHeight + 2 * margin) + margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionBottom(),
                    (itemsBeforeLast + 1) * (childHeight + 2 * margin) - margin);
}

TEST_F(FlexWrap, WrapReverseColumnReverse) {
    target->setFlexWrap(WXCore_Wrap_WrapReverse);
    target->setFlexDirection(WXCore_Flex_Direction_Column_Reverse);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionLeft(), targetWidth - margin - childWidth);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionRight(), targetWidth - margin);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionTop(), targetHeight - margin - childHeight);
    EXPECT_FLOAT_EQ(firstChildInTarget->getLayoutPositionBottom(), targetHeight - margin);

    auto flexItemCap = static_cast<int>(targetHeight / (childHeight + 2 * margin));
    int linesBeforeLast = childCount / flexItemCap;
    int itemsBeforeLast = childCount % flexItemCap - 1;

    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionLeft(),
                    targetWidth - (linesBeforeLast + 1) * (childWidth + margin * 2) + margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionRight(),
                    targetWidth - linesBeforeLast * (childWidth + margin * 2) - margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionTop(),
                    targetHeight - (itemsBeforeLast + 1) * (childHeight + 2 * margin) + margin);
    EXPECT_FLOAT_EQ(lastChildInTarget->getLayoutPositionBottom(),
                    targetHeight - (itemsBeforeLast) * (childHeight + 2 * margin) - margin);
}