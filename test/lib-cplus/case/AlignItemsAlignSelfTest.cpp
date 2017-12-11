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

class Alignment : public TestWithParam<tuple<WXCoreFlexDirection, WXCoreFlexWrap, WXCoreAlignItems, WXCoreAlignSelf>> {
protected:
    Alignment() {
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
                    child->setStyleHeight(childHeight);
                } else if (get<0>(GetParam()) == WXCore_Flex_Direction_Column) {
                    child->setStyleWidth(childWidth);
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

    ~Alignment() override {
        root->freeWXCoreNode();
        target->freeWXCoreNode();
        FormatingContext bfc = NON_BFC;
        for (auto i = 0; i < root->getChildCount(bfc); i++) {
            root->getChildAt(bfc, i)->freeWXCoreNode();
        }
    }

    WXCoreLayoutNode *root, *target, *alignSelfWithoutCrossSize, *alignItemsWithCrossSize;
};


TEST_P(Alignment, row) {
    string expectedCase("Row/Alignment");
    string actualCase(UnitTest::GetInstance()->current_test_info()->test_case_name());
    if (expectedCase == actualCase) {
        if (get<1>(GetParam()) == WXCore_Wrap_Wrap) {
            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutWidth(), 0);
            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutHeight(), childHeight);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutWidth(), childWidth);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutHeight(), childHeight);

            auto flexCap = static_cast<int>(targetWidth / childWidth);
            int linesBeforeLast = (childCount - 1) / flexCap - ((childCount - 1) % flexCap == 0 ? 1 : 0);
            int itemsBeforeLast = (childCount - 1) % flexCap == 0 ? flexCap - 1 : (childCount - 1) % flexCap - 1;

            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionLeft(), itemsBeforeLast * childWidth);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionRight(), (itemsBeforeLast + 1) * childWidth);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionTop(), linesBeforeLast * childHeight);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionBottom(), (linesBeforeLast + 1) * childHeight);
        } else if (get<1>(GetParam()) == WXCore_Wrap_NoWrap) {

        }
    }
}

TEST_P(Alignment, column) {
    string expectedCase("Column/Alignment");
    string actualCase(UnitTest::GetInstance()->current_test_info()->test_case_name());
    if (expectedCase == actualCase) {
        if (get<1>(GetParam()) == WXCore_Wrap_Wrap) {
            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutWidth(), childWidth);
            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutHeight(), 0);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutWidth(), childWidth);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutHeight(), childHeight);

        } else if (get<1>(GetParam()) == WXCore_Wrap_NoWrap) {

        }
    }
}

INSTANTIATE_TEST_CASE_P(Row, Alignment,
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


INSTANTIATE_TEST_CASE_P(Column, Alignment,
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