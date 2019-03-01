package com.taobao.weex.render.platform.component;

import android.text.TextUtils;
import android.view.Gravity;
import android.view.View;

import com.taobao.weex.render.frame.RenderView;
import com.taobao.weex.render.platform.PlatformView;
import com.taobao.weex.render.platform.view.REditText;

import java.util.List;
import java.util.Map;

public class Input extends Component implements PlatformView {

    private REditText mEditText;
    private Map<String, String> mAttrs;
    private Map<String, String> mStyles;





    @Override
    public void createView(RenderView renderView, Map<String, String> attrs, Map<String, String> styles, List<String> events) {
        super.init(renderView, attrs, styles, events);
        if(mEditText == null){
            mEditText = new REditText(renderView.getContext());
            mEditText.setRenderView(renderView);
        }
    }

    @Override
    public View getView() {
        return mEditText;
    }


    @Override
    public void onAttach() {

    }

    @Override
    public void onUpdateAttr(Map<String, String> attrs) {

    }

    @Override
    public void onUpdateStyle(Map<String, String> attrs) {

    }

    @Override
    public void onAddEvent(String event) {

    }

    @Override
    public void onRemoveEvent(String event) {

    }

    @Override
    public void onResume() {

    }

    @Override
    public void onPause() {

    }

    @Override
    public void onDestroy() {

    }


    private int getTextAlign(String textAlign) {
        int align = Gravity.START;
        if (TextUtils.isEmpty(textAlign)) {
            return align;
        }
        if (textAlign.equals(Constants.Attr.Input.Value.TEXT_ALIGN_LEFT)) {
            align = Gravity.START;
        } else if (textAlign.equals(Constants.Attr.Input.Value.TEXT_ALIGN_CENTER)) {
            align = Gravity.CENTER;
        } else if (textAlign.equals(Constants.Attr.Input.Value.TEXT_ALIGN_RIGHT)) {
            align = Gravity.END;
        }
        return align;
    }
}
