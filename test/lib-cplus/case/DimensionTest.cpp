//
// Created by YorkShen on 2017/11/22.
//

#include <gtest/gtest.h>
#include "WXCoreLayout.h"

using namespace WXCoreFlexLayout;

class Dimension : public ::testing::Test {
protected:
    virtual void SetUp() {
        root = WXCoreLayoutNode::newWXCoreNode();
        child = WXCoreLayoutNode::newWXCoreNode();
        grandChild = WXCoreLayoutNode::newWXCoreNode();
        root->addChildAt(child, 0);
        child->addChildAt(grandChild, 0);
    }

    virtual void TearDown() {
        root->freeWXCoreNode();
        child->freeWXCoreNode();
        grandChild->freeWXCoreNode();
    }

    WXCoreLayoutNode *root, *child, *grandChild;
};

TEST_F(Dimension, W0H0) {
    root->setStyleWidth(0);
    root->setStyleHeight(0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(0, root->getLayoutHeight());
}

TEST_F(Dimension, W0H1) {
    root->setStyleWidth(0);
    root->setStyleHeight(1);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(1, root->getLayoutHeight());
}

TEST_F(Dimension, W0H100) {
    root->setStyleWidth(0);
    root->setStyleHeight(100);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(0, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(100, root->getLayoutHeight());
}

TEST_F(Dimension, W1H0) {
    root->setStyleWidth(1);
    root->setStyleHeight(0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(1, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(0, root->getLayoutHeight());
}

TEST_F(Dimension, W1H1) {
    root->setStyleWidth(1);
    root->setStyleHeight(1);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(1, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(1, root->getLayoutHeight());
}

TEST_F(Dimension, W1H300) {
    root->setStyleWidth(1);
    root->setStyleHeight(300);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(1, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
}

TEST_F(Dimension, W100H0) {
    root->setStyleWidth(100);
    root->setStyleHeight(0);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(100, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(0, root->getLayoutHeight());
}

TEST_F(Dimension, W100H1) {
    root->setStyleWidth(100);
    root->setStyleHeight(1);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(100, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(1, root->getLayoutHeight());
}

TEST_F(Dimension, W100H300) {
    root->setStyleWidth(100);
    root->setStyleHeight(300);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(100, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
}

TEST_F(Dimension, WInvalidH300) {
    root->setStyleWidth(-10);
    root->setStyleHeight(300);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
}

TEST_F(Dimension, W100HInvalid) {
    root->setStyleWidth(100);
    root->setStyleHeight(-20);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(100, root->getLayoutWidth());
}

TEST_F(Dimension, ChildOverflow) {
    root->setStyleWidth(200);
    root->setStyleHeight(300);
    child->setStyleWidth(100);
    child->setStyleHeight(400);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(200, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(100, child->getLayoutWidth());
    EXPECT_FLOAT_EQ(400, child->getLayoutHeight());
}

TEST_F(Dimension, GrandChildOverflow) {
    root->setStyleWidth(200);
    root->setStyleHeight(300);
    child->setStyleWidth(100);
    child->setStyleHeight(100);
    grandChild->setStyleWidth(50);
    grandChild->setStyleHeight(400);
    root->calculateLayout();
    EXPECT_FLOAT_EQ(200, root->getLayoutWidth());
    EXPECT_FLOAT_EQ(300, root->getLayoutHeight());
    EXPECT_FLOAT_EQ(100, child->getLayoutWidth());
    EXPECT_FLOAT_EQ(100, child->getLayoutHeight());
    EXPECT_FLOAT_EQ(50, grandChild->getLayoutWidth());
    EXPECT_FLOAT_EQ(400, grandChild->getLayoutHeight());
}