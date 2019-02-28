package com.taobao.weex.render.platform.texture;

import android.graphics.SurfaceTexture;
import android.os.Build;

import java.util.concurrent.atomic.AtomicInteger;

public class TextureFactory {

    private static AtomicInteger textureId = new AtomicInteger(0);




    public static SurfaceTexture createSurfaceTexture(){
        SurfaceTexture surfaceTexture  = new SurfaceTexture(textureId.incrementAndGet());
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
            surfaceTexture.detachFromGLContext();
        }
        return surfaceTexture;
    }
}
