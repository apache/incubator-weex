//
// Created by YorkShen on 2017/11/22.
//

#include <gtest/gtest.h>
#include "WXCoreLayout.h"
#include "WXCoreFlexEnum.h"

using namespace WXCoreFlexLayout;

class Border : public ::testing::Test {
protected:
    virtual void SetUp() {
        root = WXCoreLayoutNode::newWXCoreNode();
        root->setStyleWidth(100);
        root->setStyleHeight(300);
        borderAll = WXCore_Border_Width_ALL;
        borderLeft = WXCore_Border_Width_Left;
        borderTop = WXCore_Border_Width_Top;
        borderRight = WXCore_Border_Width_Right;
        borderBottom = WXCore_Border_Width_Bottom;
    }

    virtual void TearDown() {
        root->freeWXCoreNode();
    }

    WXCoreLayoutNode *root;
    WXCoreBorderWidthEdge borderAll, borderLeft, borderTop, borderRight, borderBottom;
};

TEST_F(Border, BorderWidth0) {
    float width = 0;
    root->setBorderWidth(borderAll, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getBorderWidthLeft());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthTop());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthRight());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthBottom());
}

TEST_F(Border, BorderWidth1) {
    float width = 1.1;
    root->setBorderWidth(borderAll, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getBorderWidthLeft());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthTop());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthRight());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthBottom());
}

TEST_F(Border, BorderWidth200) {
    float width = 200;
    root->setBorderWidth(borderAll, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getBorderWidthLeft());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthTop());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthRight());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthBottom());
}

TEST_F(Border, BorderWidthInvalid) {
    float width = -30;
    root->setBorderWidth(borderAll, width);
    root->calculateLayout();
}

TEST_F(Border, BorderLeftWidth0) {
    float width = 0;
    root->setBorderWidth(borderLeft, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getBorderWidthLeft());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthTop());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthRight());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthBottom());
}

TEST_F(Border, BorderLeftWidth1) {
    float width = 1.1;
    root->setBorderWidth(borderLeft, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getBorderWidthLeft());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthTop());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthRight());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthBottom());
}

TEST_F(Border, BorderLeftWidth200) {
    float width = 200;
    root->setBorderWidth(borderLeft, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getBorderWidthLeft());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthTop());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthRight());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthBottom());
}

TEST_F(Border, BorderLeftWidthInvalid) {
    float width = -30;
    root->setBorderWidth(borderLeft, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getBorderWidthTop());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthRight());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthBottom());
}

TEST_F(Border, BorderTopWidth0) {
    float width = 0;
    root->setBorderWidth(borderTop, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getBorderWidthLeft());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthTop());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthRight());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthBottom());
}

TEST_F(Border, BorderTopWidth1) {
    float width = 1.1;
    root->setBorderWidth(borderTop, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getBorderWidthLeft());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthTop());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthRight());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthBottom());
}

TEST_F(Border, BorderTopWidth200) {
    float width = 200;
    root->setBorderWidth(borderTop, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getBorderWidthLeft());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthTop());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthRight());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthBottom());
}

TEST_F(Border, BorderTopWidthInvalid) {
    float width = -30;
    root->setBorderWidth(borderTop, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getBorderWidthLeft());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthRight());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthBottom());
}

TEST_F(Border, BorderRightWidth0) {
    float width = 0;
    root->setBorderWidth(borderRight, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getBorderWidthLeft());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthTop());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthRight());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthBottom());
}

TEST_F(Border, BorderRightWidth1) {
    float width = 1.1;
    root->setBorderWidth(borderRight, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getBorderWidthLeft());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthTop());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthRight());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthBottom());
}

TEST_F(Border, BorderRightWidth200) {
    float width = 200;
    root->setBorderWidth(borderRight, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getBorderWidthLeft());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthTop());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthRight());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthBottom());
}

TEST_F(Border, BorderRightWidthInvalid) {
    float width = -30;
    root->setBorderWidth(borderRight, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getBorderWidthLeft());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthTop());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthBottom());
}

TEST_F(Border, BorderBottomWidth0) {
    float width = 0;
    root->setBorderWidth(borderBottom, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(width, root->getBorderWidthLeft());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthTop());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthRight());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthBottom());
}

TEST_F(Border, BorderBottomWidth1) {
    float width = 1.1;
    root->setBorderWidth(borderBottom, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getBorderWidthLeft());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthTop());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthRight());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthBottom());
}

TEST_F(Border, BorderBottomWidth200) {
    float width = 200;
    root->setBorderWidth(borderBottom, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getBorderWidthLeft());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthTop());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthRight());
    EXPECT_FLOAT_EQ(width, root->getBorderWidthBottom());
}

TEST_F(Border, BorderBottomWidthInvalid) {
    float width = -30;
    root->setBorderWidth(borderBottom, width);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getBorderWidthLeft());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthTop());
    EXPECT_FLOAT_EQ(0, root->getBorderWidthRight());
}

TEST_F(Border, BorderWidthShortHandOverrideEdge) {
    root->setBorderWidth(borderAll, 200);
    root->setBorderWidth(borderTop, 400);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(200, root->getBorderWidthLeft());
    EXPECT_FLOAT_EQ(400, root->getBorderWidthTop());
    EXPECT_FLOAT_EQ(200, root->getBorderWidthRight());
    EXPECT_FLOAT_EQ(200, root->getBorderWidthBottom());
}

TEST_F(Border, BorderWidthEdgeOverrideShorthand) {
    root->setBorderWidth(borderTop, 400);
    root->setBorderWidth(borderAll, 200);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(200, root->getBorderWidthLeft());
    EXPECT_FLOAT_EQ(200, root->getBorderWidthTop());
    EXPECT_FLOAT_EQ(200, root->getBorderWidthRight());
    EXPECT_FLOAT_EQ(200, root->getBorderWidthBottom());
}


TEST_F(Border, BorderDimension){
    root->setBorderWidth(borderAll, 200);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(root->getLayoutWidth(), 100);
    EXPECT_FLOAT_EQ(root->getLayoutHeight(), 300);
    EXPECT_FLOAT_EQ(root->getLayoutPositionLeft(), 0);
    EXPECT_FLOAT_EQ(root->getLayoutPositionRight(), 100);
    EXPECT_FLOAT_EQ(root->getLayoutPositionTop(), 0);
    EXPECT_FLOAT_EQ(root->getLayoutPositionBottom(), 300);
}