//
// Created by YorkShen on 2017/11/27.
//
#include <gtest/gtest.h>
#include "WXCoreLayout.h"

using namespace WeexCore;

static constexpr float rootHeight = 700;
static constexpr float targetHeight = 300;
static constexpr float targetWidth = 400;
static constexpr float fooHeight = 200;

class PositionVertical : public ::testing::TestWithParam<::testing::tuple<float, float, float, WXCorePositionType>> {
protected:
    PositionVertical() {
        WXCorePositionEdge top, bottom;
        top = WXCore_PositionEdge_Top;
        bottom = WXCore_PositionEdge_Bottom;
        foo = WXCoreLayoutNode::newWXCoreNode();
        targetChild = WXCoreLayoutNode::newWXCoreNode();
        target = WXCoreLayoutNode::newWXCoreNode();
        root = WXCoreLayoutNode::newWXCoreNode();
        WXCorePositionType positionType;
        float height, tvalue, bvalue;
        height = ::testing::get<0>(GetParam());
        tvalue = ::testing::get<1>(GetParam());
        bvalue = ::testing::get<2>(GetParam());
        positionType = ::testing::get<3>(GetParam());

        foo->setStyleWidth(100);
        foo->setStyleHeight(fooHeight);
        targetChild->setStyleWidth(50);
        targetChild->setStyleHeight(targetHeight);

        target->setStyleWidth(targetWidth);
        target->setStylePositionType(positionType);
        root->setStyleWidth(300);
        root->setStyleHeight(rootHeight);

        if (!isnan(height))
            target->setStyleHeight(height);
        if (!isnan(tvalue))
            target->setStylePosition(top, tvalue);
        if (!isnan(bvalue))
            target->setStylePosition(bottom, bvalue);

        target->appendChild(targetChild);
        root->appendChild(foo);
        root->appendChild(target);

        root->calculateLayout();
    }

    ~PositionVertical() override {
        root->freeWXCoreNode();
        foo->freeWXCoreNode();
        target->freeWXCoreNode();
        targetChild->freeWXCoreNode();
    }

    WXCoreLayoutNode *root, *foo, *targetChild, *target;
};


TEST_P(PositionVertical, relative) {
    const ::testing::TestInfo *const test_info =
            ::testing::UnitTest::GetInstance()->current_test_info();
    std::string actualCase{test_info->test_case_name()};
    std::string expectedCase{"Relative/PositionVertical"};
    if (expectedCase == actualCase) {
        float height, tvalue, bvalue, top, bottom;
        if (isnan(height = ::testing::get<0>(GetParam())))
            height = targetHeight;
        tvalue = ::testing::get<1>(GetParam());
        bvalue = ::testing::get<2>(GetParam());

        if (isnan(tvalue)) {
            if (isnan(bvalue)) {
                top = fooHeight;
            } else {
                top = fooHeight - bvalue;
            }
        } else {
            top = fooHeight + tvalue;
        }

        if (isnan(tvalue)) {
            if (isnan(bvalue)) {
                bottom = fooHeight + height;
            } else {
                bottom = fooHeight + height - bvalue;
            }
        } else {
            bottom = fooHeight + height + tvalue;
        }

        EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
        EXPECT_FLOAT_EQ(height, target->getLayoutHeight());
        EXPECT_FLOAT_EQ(0, target->getLayoutPositionLeft());
        EXPECT_FLOAT_EQ(targetWidth, target->getLayoutPositionRight());
        EXPECT_FLOAT_EQ(top, target->getLayoutPositionTop());
        EXPECT_FLOAT_EQ(bottom, target->getLayoutPositionBottom());
    }
}


TEST_P(PositionVertical, absolute) {
    const ::testing::TestInfo *const test_info =
            ::testing::UnitTest::GetInstance()->current_test_info();
    std::string actualCase{test_info->test_case_name()};
    std::string expectedCase{"Absolute/PositionVertical"};
    if (expectedCase == actualCase) {
        float height, tvalue, bvalue, top, bottom;

        height = ::testing::get<0>(GetParam());
        tvalue = ::testing::get<1>(GetParam());
        bvalue = ::testing::get<2>(GetParam());

        if (isnan(height)) {
            if (!isnan(tvalue) && !isnan(bvalue)) {
                height = rootHeight - tvalue - bvalue;
            } else {
                height = targetHeight;
            }
        }

        if (isnan(tvalue)) {
            if (isnan(bvalue)) {
                top = 0;
                bottom = height;
            } else {
                top = rootHeight - bvalue - targetHeight;
                bottom = rootHeight - bvalue;
            }
        } else {
            top = tvalue;
            if (isnan(bvalue)) {
                bottom = tvalue + targetHeight;
            } else {
                bottom = rootHeight - bvalue;
            }
        }

        EXPECT_FLOAT_EQ(targetWidth, target->getLayoutWidth());
        EXPECT_FLOAT_EQ(height, target->getLayoutHeight());
        EXPECT_FLOAT_EQ(0, target->getLayoutPositionLeft());
        EXPECT_FLOAT_EQ(targetWidth, target->getLayoutPositionRight());
        EXPECT_FLOAT_EQ(top, target->getLayoutPositionTop());
        EXPECT_FLOAT_EQ(bottom, target->getLayoutPositionBottom());
    }
}

INSTANTIATE_TEST_CASE_P(Relative, PositionVertical,
                        ::testing::Combine(::testing::Values(NAN, 100, 0), ::testing::Values(NAN, 0, 30, -10),
                                           ::testing::Values(NAN, 0, 70, -20),
                                           ::testing::Values(WXCore_PositionType_Relative)));

INSTANTIATE_TEST_CASE_P(Absolute, PositionVertical,
                        ::testing::Combine(::testing::Values(NAN, 100, 0), ::testing::Values(NAN, 0, 30, -10),
                                           ::testing::Values(NAN, 0, 70, -20),
                                           ::testing::Values(WXCore_PositionType_Absolute)));