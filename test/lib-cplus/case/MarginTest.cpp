//
// Created by YorkShen on 2017/11/22.
//

#include <gtest/gtest.h>
#include "WXCoreLayout.h"
#include "WXCoreFlexEnum.h"

using namespace WXCoreFlexLayout;

class Margin : public ::testing::Test {
protected:
    virtual void SetUp() {
        root = WXCoreLayoutNode::newWXCoreNode();
        root->setStyleWidth(100);
        root->setStyleHeight(300);
        marginAll = WXCore_Margin_ALL;
        marginLeft = WXCore_Margin_Left;
        marginTop = WXCore_Margin_Top;
        marginRight = WXCore_Margin_Right;
        marginBottom = WXCore_Margin_Bottom;
    }

    virtual void TearDown() {
        root->freeWXCoreNode();
    }

    WXCoreLayoutNode *root;
    WXCoreMarginEdge marginAll, marginLeft, marginTop, marginRight, marginBottom;
};

TEST_F(Margin, MarginWidth0) {
    float width = 0;
    root->setMargin(marginAll, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getMarginLeft());
    EXPECT_FLOAT_EQ(width, root->getMarginTop());
    EXPECT_FLOAT_EQ(width, root->getMarginRight());
    EXPECT_FLOAT_EQ(width, root->getMarginBottom());
}

TEST_F(Margin, MarginWidth1) {
    float width = 1.1;
    root->setMargin(marginAll, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getMarginLeft());
    EXPECT_FLOAT_EQ(width, root->getMarginTop());
    EXPECT_FLOAT_EQ(width, root->getMarginRight());
    EXPECT_FLOAT_EQ(width, root->getMarginBottom());
}

TEST_F(Margin, MarginWidth200) {
    float width = 200;
    root->setMargin(marginAll, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getMarginLeft());
    EXPECT_FLOAT_EQ(width, root->getMarginTop());
    EXPECT_FLOAT_EQ(width, root->getMarginRight());
    EXPECT_FLOAT_EQ(width, root->getMarginBottom());
}

TEST_F(Margin, MarginWidthInvalid) {
    float width = -30;
    root->setMargin(marginAll, width);
    root->calculateLayout();
}

TEST_F(Margin, MarginLeftWidth0) {
    float width = 0;
    root->setMargin(marginLeft, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getMarginLeft());
    EXPECT_FLOAT_EQ(width, root->getMarginTop());
    EXPECT_FLOAT_EQ(width, root->getMarginRight());
    EXPECT_FLOAT_EQ(width, root->getMarginBottom());
}

TEST_F(Margin, MarginLeftWidth1) {
    float width = 1.1;
    root->setMargin(marginLeft, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getMarginLeft());
    EXPECT_FLOAT_EQ(0, root->getMarginTop());
    EXPECT_FLOAT_EQ(0, root->getMarginRight());
    EXPECT_FLOAT_EQ(0, root->getMarginBottom());
}

TEST_F(Margin, MarginLeftWidth200) {
    float width = 200;
    root->setMargin(marginLeft, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getMarginLeft());
    EXPECT_FLOAT_EQ(0, root->getMarginTop());
    EXPECT_FLOAT_EQ(0, root->getMarginRight());
    EXPECT_FLOAT_EQ(0, root->getMarginBottom());
}

TEST_F(Margin, MarginLeftWidthInvalid) {
    float width = -30;
    root->setMargin(marginLeft, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getMarginTop());
    EXPECT_FLOAT_EQ(0, root->getMarginRight());
    EXPECT_FLOAT_EQ(0, root->getMarginBottom());
}

TEST_F(Margin, MarginTopWidth0) {
    float width = 0;
    root->setMargin(marginTop, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getMarginLeft());
    EXPECT_FLOAT_EQ(width, root->getMarginTop());
    EXPECT_FLOAT_EQ(width, root->getMarginRight());
    EXPECT_FLOAT_EQ(width, root->getMarginBottom());
}

TEST_F(Margin, MarginTopWidth1) {
    float width = 1.1;
    root->setMargin(marginTop, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getMarginLeft());
    EXPECT_FLOAT_EQ(width, root->getMarginTop());
    EXPECT_FLOAT_EQ(0, root->getMarginRight());
    EXPECT_FLOAT_EQ(0, root->getMarginBottom());
}

TEST_F(Margin, MarginTopWidth200) {
    float width = 200;
    root->setMargin(marginTop, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getMarginLeft());
    EXPECT_FLOAT_EQ(width, root->getMarginTop());
    EXPECT_FLOAT_EQ(0, root->getMarginRight());
    EXPECT_FLOAT_EQ(0, root->getMarginBottom());
}

TEST_F(Margin, MarginTopWidthInvalid) {
    float width = -30;
    root->setMargin(marginTop, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getMarginLeft());
    EXPECT_FLOAT_EQ(0, root->getMarginRight());
    EXPECT_FLOAT_EQ(0, root->getMarginBottom());
}

TEST_F(Margin, MarginRightWidth0) {
    float width = 0;
    root->setMargin(marginRight, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getMarginLeft());
    EXPECT_FLOAT_EQ(width, root->getMarginTop());
    EXPECT_FLOAT_EQ(width, root->getMarginRight());
    EXPECT_FLOAT_EQ(width, root->getMarginBottom());
}

TEST_F(Margin, MarginRightWidth1) {
    float width = 1.1;
    root->setMargin(marginRight, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getMarginLeft());
    EXPECT_FLOAT_EQ(0, root->getMarginTop());
    EXPECT_FLOAT_EQ(width, root->getMarginRight());
    EXPECT_FLOAT_EQ(0, root->getMarginBottom());
}

TEST_F(Margin, MarginRightWidth200) {
    float width = 200;
    root->setMargin(marginRight, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getMarginLeft());
    EXPECT_FLOAT_EQ(0, root->getMarginTop());
    EXPECT_FLOAT_EQ(width, root->getMarginRight());
    EXPECT_FLOAT_EQ(0, root->getMarginBottom());
}

TEST_F(Margin, MarginRightWidthInvalid) {
    float width = -30;
    root->setMargin(marginRight, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getMarginLeft());
    EXPECT_FLOAT_EQ(0, root->getMarginTop());
    EXPECT_FLOAT_EQ(0, root->getMarginBottom());
}

TEST_F(Margin, MarginBottomWidth0) {
    float width = 0;
    root->setMargin(marginBottom, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getMarginLeft());
    EXPECT_FLOAT_EQ(width, root->getMarginTop());
    EXPECT_FLOAT_EQ(width, root->getMarginRight());
    EXPECT_FLOAT_EQ(width, root->getMarginBottom());
}

TEST_F(Margin, MarginBottomWidth1) {
    float width = 1.1;
    root->setMargin(marginBottom, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getMarginLeft());
    EXPECT_FLOAT_EQ(0, root->getMarginTop());
    EXPECT_FLOAT_EQ(0, root->getMarginRight());
    EXPECT_FLOAT_EQ(width, root->getMarginBottom());
}

TEST_F(Margin, MarginBottomWidth200) {
    float width = 200;
    root->setMargin(marginBottom, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getMarginLeft());
    EXPECT_FLOAT_EQ(0, root->getMarginTop());
    EXPECT_FLOAT_EQ(0, root->getMarginRight());
    EXPECT_FLOAT_EQ(width, root->getMarginBottom());
}

TEST_F(Margin, MarginBottomWidthInvalid) {
    float width = -30;
    root->setMargin(marginBottom, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getMarginLeft());
    EXPECT_FLOAT_EQ(0, root->getMarginTop());
    EXPECT_FLOAT_EQ(0, root->getMarginRight());
}

TEST_F(Margin, MarginWidthShortHandOverrideEdge) {
    root->setMargin(marginAll, 200);
    root->setMargin(marginTop, 400);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(200, root->getMarginLeft());
    EXPECT_FLOAT_EQ(400, root->getMarginTop());
    EXPECT_FLOAT_EQ(200, root->getMarginRight());
    EXPECT_FLOAT_EQ(200, root->getMarginBottom());
}

TEST_F(Margin, MarginWidthEdgeOverrideShorthand) {
    root->setMargin(marginTop, 400);
    root->setMargin(marginAll, 200);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(200, root->getMarginLeft());
    EXPECT_FLOAT_EQ(200, root->getMarginTop());
    EXPECT_FLOAT_EQ(200, root->getMarginRight());
    EXPECT_FLOAT_EQ(200, root->getMarginBottom());
}

TEST_F(Margin, MarginDimension) {
    root->setMargin(marginAll, 200);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(root->getLayoutWidth(), 300);
    EXPECT_FLOAT_EQ(root->getLayoutHeight(), 500);
    EXPECT_FLOAT_EQ(root->getLayoutPositionLeft(), 200);
    EXPECT_FLOAT_EQ(root->getLayoutPositionRight(), 300);
    EXPECT_FLOAT_EQ(root->getLayoutPositionTop(), 500);
    EXPECT_FLOAT_EQ(root->getLayoutPositionBottom(), 500);
}