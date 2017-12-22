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
        FormattingContext bfc = NON_BFC;
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
        WXCoreAlignItems alignItems = get<2>(GetParam());
        WXCoreAlignSelf alignSelf = get<3>(GetParam());
        float height = 0;
        float alignSelfWithoutCrossSizeBottom = -1;
        if (get<1>(GetParam()) == WXCore_Wrap_Wrap) {
            auto flexCap = static_cast<int>(targetWidth / childWidth);
            auto linesBeforeLast = childCount / flexCap - (childCount % flexCap == 0 ? 1 : 0);
            auto itemsBeforeLast = childCount % flexCap == 0 ? flexCap - 1 : childCount % flexCap - 1;
            float alignItemsWithCrossSizeTop, alignItemsWithCrossSizeBottom;
            if (flexCap >= childCount) {
                if (alignSelf == WXCore_AlignSelf_Stretch ||
                    (alignItems == WXCore_AlignItems_Stretch && alignSelf == WXCore_AlignSelf_Auto)) {
                    height = targetHeight;
                    alignSelfWithoutCrossSizeBottom = targetHeight;
                }

                switch (alignItems) {
                    case WXCore_AlignItems_Stretch:
                    case WXCore_AlignItems_Flex_Start:
                        alignItemsWithCrossSizeTop = 0;
                        alignItemsWithCrossSizeBottom = childHeight;
                        break;
                    case WXCore_AlignItems_Center:
                        alignItemsWithCrossSizeTop = targetHeight / 2 - childHeight / 2;
                        alignItemsWithCrossSizeBottom = targetHeight / 2 + childHeight / 2;
                        break;
                    case WXCore_AlignItems_Flex_End:
                        alignItemsWithCrossSizeTop = targetHeight - childHeight;
                        alignItemsWithCrossSizeBottom = targetHeight;
                        break;
                    default:
                        alignItemsWithCrossSizeTop = -1;
                        alignItemsWithCrossSizeBottom = -1;
                        break;
                }
            } else {
                if (alignSelf == WXCore_AlignSelf_Stretch ||
                    (alignItems == WXCore_AlignItems_Stretch && alignSelf == WXCore_AlignSelf_Auto)) {
                    height = childHeight;
                    alignSelfWithoutCrossSizeBottom = childHeight;
                }
                alignItemsWithCrossSizeTop = linesBeforeLast * childHeight;
                alignItemsWithCrossSizeBottom = (linesBeforeLast + 1) * childHeight;
            }

            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutWidth(), childWidth);
            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutHeight(), height);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutWidth(), childWidth);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutHeight(), childHeight);

            if (alignItemsWithCrossSizeTop != -1 && alignItemsWithCrossSizeBottom != -1) {
                EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionLeft(), itemsBeforeLast * childWidth);
                EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionRight(), (itemsBeforeLast + 1) * childWidth);
            }
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionTop(), linesBeforeLast * childHeight);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionBottom(), (linesBeforeLast + 1) * childHeight);

            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutPositionLeft(), 0);
            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutPositionRight(), childWidth);
            if (alignSelfWithoutCrossSizeBottom != -1) {
                EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutPositionTop(), 0);
                EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutPositionBottom(), alignSelfWithoutCrossSizeBottom);
            }
        } else if (get<1>(GetParam()) == WXCore_Wrap_NoWrap) {
            if (alignSelf == WXCore_AlignSelf_Stretch ||
                (alignItems == WXCore_AlignItems_Stretch && alignSelf == WXCore_AlignSelf_Auto)) {
                height = targetHeight;
                alignSelfWithoutCrossSizeBottom = targetHeight;
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

            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutPositionLeft(), 0);
            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutPositionRight(), childWidth);
            if (alignSelfWithoutCrossSizeBottom != -1) {
                EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutPositionTop(), 0);
                EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutPositionBottom(), alignSelfWithoutCrossSizeBottom);
            }
        }
    }
}

TEST_P(AlignmentItemsAlignSelf, column) {
    string expectedCase("Column/AlignmentItemsAlignSelf");
    string actualCase(UnitTest::GetInstance()->current_test_info()->test_case_name());
    if (expectedCase == actualCase) {
        WXCoreAlignItems alignItems = get<2>(GetParam());
        WXCoreAlignSelf alignSelf = get<3>(GetParam());
        float width = 0;
        float alignSelfWithoutCrossSizeRight = -1;
        if (get<1>(GetParam()) == WXCore_Wrap_Wrap) {
            auto flexCap = static_cast<int>(targetHeight / childHeight);
            auto linesBeforeLast = childCount / flexCap - (childCount % flexCap == 0 ? 1 : 0);
            auto itemsBeforeLast = childCount % flexCap == 0 ? flexCap - 1 : childCount % flexCap - 1;
            float alignItemsWithCrossSizeLeft, alignItemsWithCrossSizeRight;
            if (flexCap >= childCount) {
                if (alignSelf == WXCore_AlignSelf_Stretch ||
                    (alignItems == WXCore_AlignItems_Stretch && alignSelf == WXCore_AlignSelf_Auto)) {
                    width = targetWidth;
                    alignSelfWithoutCrossSizeRight = targetWidth;
                }

                switch (alignItems) {
                    case WXCore_AlignItems_Stretch:
                    case WXCore_AlignItems_Flex_Start:
                        alignItemsWithCrossSizeLeft = 0;
                        alignItemsWithCrossSizeRight = childWidth;
                        break;
                    case WXCore_AlignItems_Center:
                        alignItemsWithCrossSizeLeft = targetWidth / 2 - childWidth / 2;
                        alignItemsWithCrossSizeRight = targetWidth / 2 + childWidth / 2;
                        break;
                    case WXCore_AlignItems_Flex_End:
                        alignItemsWithCrossSizeLeft = targetWidth - childWidth;
                        alignItemsWithCrossSizeRight = targetWidth;
                        break;
                    default:
                        alignItemsWithCrossSizeLeft = -1;
                        alignItemsWithCrossSizeRight = -1;
                        break;
                }
            } else {
                if (alignSelf == WXCore_AlignSelf_Stretch ||
                    (alignItems == WXCore_AlignItems_Stretch && alignSelf == WXCore_AlignSelf_Auto)) {
                    width = childWidth;
                    alignSelfWithoutCrossSizeRight = childWidth;
                }
                alignItemsWithCrossSizeLeft = linesBeforeLast * childWidth;
                alignItemsWithCrossSizeRight = (linesBeforeLast + 1) * childWidth;
            }

            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutWidth(), width);
            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutHeight(), childHeight);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutWidth(), childWidth);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutHeight(), childHeight);

            if (alignItemsWithCrossSizeLeft != -1 && alignItemsWithCrossSizeRight != -1) {
                EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionLeft(), alignItemsWithCrossSizeLeft);
                EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionRight(), alignItemsWithCrossSizeRight);
            }
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionTop(), itemsBeforeLast * childHeight);
            EXPECT_FLOAT_EQ(alignItemsWithCrossSize->getLayoutPositionBottom(), (itemsBeforeLast + 1) * childHeight);

            if (alignSelfWithoutCrossSizeRight != -1) {
                EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutPositionLeft(), 0);
                EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutPositionRight(), alignSelfWithoutCrossSizeRight);
            }
            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutPositionTop(), 0);
            EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutPositionBottom(), childHeight);
        } else if (get<1>(GetParam()) == WXCore_Wrap_NoWrap) {
            if (alignSelf == WXCore_AlignSelf_Stretch ||
                (alignItems == WXCore_AlignItems_Stretch && alignSelf == WXCore_AlignSelf_Auto)) {
                width = targetWidth;
                alignSelfWithoutCrossSizeRight = targetWidth;
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

            if (alignSelfWithoutCrossSizeRight != -1) {
                EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutPositionLeft(), 0);
                EXPECT_FLOAT_EQ(alignSelfWithoutCrossSize->getLayoutPositionRight(), alignSelfWithoutCrossSizeRight);
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