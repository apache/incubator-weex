//
// Created by YorkShen on 2017/12/5.
//


#include <gtest/gtest.h>
#include "WXCoreLayout.h"

using namespace WeexCore;
using namespace testing;
using namespace std;

constexpr float childWidth = 150;
constexpr float childHeight = 100;
constexpr float targetWidth = 700;
constexpr float targetHeight = 1000;
constexpr int childCount = 10;

class AlignmentItemsAlignSelf
        : public TestWithParam<tuple<WXCoreFlexDirection, WXCoreFlexWrap, WXCoreAlignItems, WXCoreAlignSelf>> {
protected:
    AlignmentItemsAlignSelf() {
        root = WXCoreLayoutNode::newWXCoreNode();
        target = WXCoreLayoutNode::newWXCoreNode();
        alignSelfWithoutCrossSize = nullptr;

        root->setStyleWidth(750);
        root->setStyleHeight(1200);
        target->setStyleWidth(targetWidth);
        target->setStyleHeight(targetHeight);
        target->setFlexDirection(get<0>(GetParam()));
        target->setFlexWrap(get<1>(GetParam()));
        target->setAlignItems(get<2>(GetParam()));

        for (auto i = 0; i < childCount; i++) {
            WXCoreLayoutNode *child = WXCoreLayoutNode::newWXCoreNode();
            if (alignSelfWithoutCrossSize == nullptr) {
                alignSelfWithoutCrossSize = child;
                if (get<0>(GetParam()) == WXCore_Flex_Direction_Row) {
                    child->setStyleWidth(childWidth);
                } else if (get<0>(GetParam()) == WXCore_Flex_Direction_Column) {
                    child->setStyleHeight(childHeight);
                }
            } else {
                child->setStyleWidth(childWidth);
                child->setStyleHeight(childHeight);
            }
            alignItemsWithCrossSize = child;
            target->appendChild(child);
        }
        WXCoreAlignSelf alignSelf = get<3>(GetParam());
        if (alignSelf != WXCore_AlignSelf_Auto) {
            alignSelfWithoutCrossSize->setAlignSelf(alignSelf);
        }
        root->appendChild(target);
        root->calculateLayout();
    }

    ~AlignmentItemsAlignSelf() override {
        root->freeWXCoreNode();
        target->freeWXCoreNode();
        FormatingContext bfc = NON_BFC;
        for (auto i = 0; i < root->getChildCount(bfc); i++) {
            root->getChildAt(bfc, i)->freeWXCoreNode();
        }
    }

    WXCoreLayoutNode *root, *target, *alignSelfWithoutCrossSize, *alignItemsWithCrossSize;
};


TEST_P(AlignmentItemsAlignSelf, row) {
    string expectedCase("Row/AlignmentItemsAlignSelf");
    string actualCase(UnitTest::GetInstance()->current_test_info()->test_case_name());
    if (expectedCase == actualCase) {
        if (get<1>(GetParam()) == WXCore_Wrap_Wrap) {
            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutWidth(), childWidth);
            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutHeight(), 0);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutWidth(), childWidth);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutHeight(), childHeight);

            auto flexCap = static_cast<int>(targetWidth / childWidth);
            auto linesBeforeLast = childCount / flexCap - (childCount % flexCap == 0 ? 1 : 0);
            auto itemsBeforeLast = childCount % flexCap == 0 ? flexCap - 1 : childCount % flexCap - 1;

            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionLeft(), itemsBeforeLast * childWidth);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionRight(), (itemsBeforeLast + 1) * childWidth);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionTop(), linesBeforeLast * childHeight);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionBottom(), (linesBeforeLast + 1) * childHeight);
        } else if (get<1>(GetParam()) == WXCore_Wrap_NoWrap) {
            WXCoreAlignItems alignItems = get<2>(GetParam());
            WXCoreAlignSelf alignSelf = get<3>(GetParam());
            float height = 0;
            if (alignSelf == WXCore_AlignSelf_Stretch ||
                (alignItems == WXCore_AlignItems_Stretch && alignSelf == WXCore_AlignSelf_Auto)) {
                height = targetHeight;
            }

            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutWidth(), childWidth);
            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutHeight(), height);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutWidth(), childWidth);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutHeight(), childHeight);

            float alignItemsWithCrossSizeTop = 0;
            if (alignItems == WXCore_AlignItems_Flex_End) {
                alignItemsWithCrossSizeTop = targetHeight - childHeight;
            } else if (alignItems == WXCore_AlignItems_Center) {
                alignItemsWithCrossSizeTop = targetHeight / 2 - childHeight / 2;
            }

            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionLeft(), (childCount - 1) * childWidth);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionRight(), childCount * childWidth);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionTop(), alignItemsWithCrossSizeTop);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionBottom(),
                            alignItemsWithCrossSizeTop + childHeight);

            float alignSelfWithoutCrossSizeTop = -1;
            if (alignSelf == WXCore_AlignSelf_Auto) {
                alignSelfWithoutCrossSizeTop = alignItemsWithCrossSizeTop;
            } else if (alignSelf == WXCore_AlignSelf_Stretch) {
                alignSelfWithoutCrossSizeTop = targetHeight;
            }

            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutPositionLeft(), 0);
            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutPositionRight(), childWidth);
            if (alignSelfWithoutCrossSizeTop != -1) {
                EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutPositionTop(), alignSelfWithoutCrossSizeTop);
                EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutPositionBottom(),
                                alignSelfWithoutCrossSizeTop + height);
            }
        }
    }
}

TEST_P(AlignmentItemsAlignSelf, column) {
    string expectedCase("Column/AlignmentItemsAlignSelf");
    string actualCase(UnitTest::GetInstance()->current_test_info()->test_case_name());
    if (expectedCase == actualCase) {
        if (get<1>(GetParam()) == WXCore_Wrap_Wrap) {
            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutWidth(), 0);
            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutHeight(), childHeight);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutWidth(), childWidth);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutHeight(), childHeight);

            auto flexCap = static_cast<int>(targetHeight / childHeight);
            auto linesBeforeLast = childCount / flexCap - (childCount % flexCap == 0 ? 1 : 0);
            auto itemsBeforeLast = childCount % flexCap == 0 ? flexCap - 1 : childCount % flexCap - 1;

            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionLeft(), linesBeforeLast * childWidth);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionRight(), (linesBeforeLast + 1) * childWidth);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionTop(), itemsBeforeLast * childHeight);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionBottom(), (itemsBeforeLast + 1) * childHeight);
        } else if (get<1>(GetParam()) == WXCore_Wrap_NoWrap) {
            WXCoreAlignItems alignItems = get<2>(GetParam());
            WXCoreAlignSelf alignSelf = get<3>(GetParam());
            float width = 0;
            if (alignSelf == WXCore_AlignSelf_Stretch ||
                (alignItems == WXCore_AlignItems_Stretch && alignSelf == WXCore_AlignSelf_Auto)) {
                width = targetWidth;
            }

            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutWidth(), width);
            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutHeight(), childHeight);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutWidth(), childWidth);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutHeight(), childHeight);

            float alignItemsWithCrossSizeLeft = 0;
            if (alignItems == WXCore_AlignItems_Flex_End) {
                alignItemsWithCrossSizeLeft = targetWidth - childWidth;
            } else if (alignItems == WXCore_AlignItems_Center) {
                alignItemsWithCrossSizeLeft = targetWidth / 2 - childWidth / 2;
            }

            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionLeft(), alignItemsWithCrossSizeLeft);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionRight(),
                            alignItemsWithCrossSizeLeft + childWidth);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionTop(), (childCount - 1) * childHeight);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionBottom(), childCount * childHeight);

            float alignSelfWithoutCrossSizeLeft = -1;
            if (alignSelf == WXCore_AlignSelf_Auto) {
                alignSelfWithoutCrossSizeLeft = alignItemsWithCrossSizeLeft;
            } else if (alignSelf == WXCore_AlignSelf_Stretch) {
                alignSelfWithoutCrossSizeLeft = targetWidth;
            }


            if (alignSelfWithoutCrossSizeLeft != -1) {
                EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutPositionLeft(), alignSelfWithoutCrossSizeLeft);
                EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutPositionRight(),
                                alignSelfWithoutCrossSizeLeft + childWidth);

            }
            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutPositionTop(), 0);
            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutPositionBottom(), childHeight);
        }
    }
}

INSTANTIATE_TEST_CASE_P(Row, AlignmentItemsAlignSelf,
                        Combine(Values(WXCore_Flex_Direction_Row),
                                Values(WXCore_Wrap_NoWrap, WXCore_Wrap_Wrap),
                                Values(WXCore_AlignItems_Flex_Start,
                                       WXCore_AlignItems_Center,
                                       WXCore_AlignItems_Flex_End,
                                       WXCore_AlignItems_Stretch),
                                Values(WXCore_AlignSelf_Auto,
                                       WXCore_AlignSelf_Flex_Start,
                                       WXCore_AlignSelf_Center,
                                       WXCore_AlignSelf_Flex_End,
                                       WXCore_AlignSelf_Stretch)));


INSTANTIATE_TEST_CASE_P(Column, AlignmentItemsAlignSelf,
                        Combine(Values(WXCore_Flex_Direction_Column),
                                Values(WXCore_Wrap_NoWrap, WXCore_Wrap_Wrap),
                                Values(WXCore_AlignItems_Flex_Start,
                                       WXCore_AlignItems_Center,
                                       WXCore_AlignItems_Flex_End,
                                       WXCore_AlignItems_Stretch),
                                Values(WXCore_AlignSelf_Auto,
                                       WXCore_AlignSelf_Flex_Start,
                                       WXCore_AlignSelf_Center,
                                       WXCore_AlignSelf_Flex_End,
                                       WXCore_AlignSelf_Stretch)));