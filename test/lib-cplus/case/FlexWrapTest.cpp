//
// Created by YorkShen on 2017/11/22.
//

#include <gtest/gtest.h>
#include "WXCoreLayout.h"
using namespace WXCoreFlexLayout;

class FlexWrap : public ::testing::Test{
protected:
    virtual void SetUp(){
        root  = WXCoreLayoutNode::newWXCoreNode();
        root->setStyleWidth(700);
        root->setStyleHeight(1000);
        for(int i=0;i<10;i++){
            WXCoreLayoutNode *child = WXCoreLayoutNode::newWXCoreNode();
            child->setStyleWidth(200);
            child->setStyleHeight(300);
            root->addChildAt(child, i);
        }

    }
    virtual void TearDown(){
        root->freeWXCoreNode();
        FormatingContext bfc = NON_BFC;
        for(int i=0;i< root->getChildCount(bfc);i++){
            root->getChildAt(bfc, i)->freeWXCoreNode();
        }
    }

    WXCoreLayoutNode *root;
};

TEST_F(FlexWrap, Nan){
}