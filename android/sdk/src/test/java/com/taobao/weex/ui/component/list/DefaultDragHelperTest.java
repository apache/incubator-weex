package com.taobao.weex.ui.component.list;

import com.taobao.weappplus_sdk.BuildConfig;
import com.taobao.weex.WXSDKInstanceTest;
import com.taobao.weex.dom.TestDomObject;
import com.taobao.weex.dom.WXDomObject;
import com.taobao.weex.dom.WXListDomObject;
import com.taobao.weex.ui.SimpleComponentHolder;
import com.taobao.weex.ui.component.ComponentTest;
import com.taobao.weex.ui.component.WXComponent;
import com.taobao.weex.ui.component.WXDiv;
import com.taobao.weex.ui.component.WXDivTest;
import com.taobao.weex.ui.component.WXVContainer;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.robolectric.RobolectricGradleTestRunner;
import org.robolectric.annotation.Config;

import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.List;

import static org.junit.Assert.assertEquals;
import static org.mockito.Matchers.anyMap;
import static org.mockito.Matchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;

/**
 * Description:
 * <p>
 * Created by rowandjj(chuyi)<br/>
 */
@RunWith(RobolectricGradleTestRunner.class)
@Config(constants = BuildConfig.class, sdk = 19)
@PowerMockIgnore( {"org.mockito.*", "org.robolectric.*", "android.*"})
public class DefaultDragHelperTest {

    WXListComponent listComponent;

    WXCell fakeCell;

    private DragHelper mFakeDragHelper;

    private EventTrigger mockedEventTrigger;

    private WXComponent c1;
    private WXComponent c2;
    private WXComponent c3;

    private List<WXComponent> mFakeDataSource;

    public static WXListComponent create(WXVContainer parent) throws IllegalAccessException, InstantiationException, InvocationTargetException {
        return create(parent,new WXListDomObject());
    }

    public static WXListComponent create(WXVContainer parent, WXDomObject dom) throws IllegalAccessException, InstantiationException, InvocationTargetException {
        return (WXListComponent) new SimpleComponentHolder(WXListComponent.class).createInstance(WXSDKInstanceTest.createInstance(), dom, parent);
    }

    @Before
    public void setUp() throws Exception {
        WXDiv div = WXDivTest.create();
        ComponentTest.create(div);
        listComponent = create(div);
        ComponentTest.create(listComponent);

        fakeCell = new WXCell(WXSDKInstanceTest.createInstance(),new TestDomObject(),null,false);

        c1 = new WXCell(WXSDKInstanceTest.createInstance(),new TestDomObject(),null,false);
        c2 = new WXCell(WXSDKInstanceTest.createInstance(),new TestDomObject(),null,false);
        c3 = new WXCell(WXSDKInstanceTest.createInstance(),new TestDomObject(),null,false);

        mFakeDataSource = new ArrayList<>();
        mFakeDataSource.add(c1);
        mFakeDataSource.add(c2);
        mFakeDataSource.add(c3);

        mockedEventTrigger = mock(EventTrigger.class);

        mFakeDragHelper = new DefaultDragHelper(mFakeDataSource,listComponent.getHostView().getInnerView(), mockedEventTrigger);
    }

    @After
    public void tearDown() throws Exception {
        listComponent.destroy();
    }

    @Test
    public void onDragStart() throws Exception {
        WXComponent c = new WXCell(WXSDKInstanceTest.createInstance(),new TestDomObject(),null,false);
        mFakeDragHelper.onDragStart(c,3);
        verify(mockedEventTrigger).triggerEvent(eq("dragstart"),anyMap());
    }

    @Test
    public void onDragEnd() throws Exception {
        WXComponent c = new WXCell(WXSDKInstanceTest.createInstance(),new TestDomObject(),null,false);
        mFakeDragHelper.onDragEnd(c,1,2);
        verify(mockedEventTrigger).triggerEvent(eq("dragend"),anyMap());
    }

    @Test
    public void onDragging() throws Exception {
        assertEquals(mFakeDataSource.get(0),c1);
        assertEquals(mFakeDataSource.get(1),c2);

        mFakeDragHelper.onDragging(0,1);

        assertEquals(mFakeDataSource.get(0),c2);
        assertEquals(mFakeDataSource.get(1),c1);
    }


}