/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
package com.taobao.weex.utils;

import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ColorFilter;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PixelFormat;
import android.graphics.PointF;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Region;
import android.graphics.Shader;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Created by moxun on 2017/9/4.
 * Utils for create shadow layer on view
 * Requires Android 4.3 and higher
 *
 * @see <a href="https://www.w3schools.com/cssref/css3_pr_box-shadow.asp">CSS3 box-shadow Property</>
 */

public class BoxShadowUtil {
  private static final String TAG = "BoxShadowUtil";

  public static void setBoxShadow(final View target, String style, float[] radii, int viewPort) {
    final BoxShadowOptions options = parseBoxShadow(style, viewPort);
    if (options == null) {
      WXLogUtils.w(TAG, "Failed to parse box-shadow: " + style);
      return;
    }

    if (target == null) {
      WXLogUtils.w(TAG, "Target view is null!");
      return;
    }

    if (options.isClear && Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
      target.getOverlay().clear();
      WXLogUtils.d(TAG, "Remove box-shadow");
      return;
    }

    if (radii != null) {
      if (radii.length != 8) {
        WXLogUtils.w(TAG, "Length of radii must be 8");
      } else {
        for (int i = 0; i < radii.length; i++) {
          float realRadius = WXViewUtils.getRealSubPxByWidth(radii[i], viewPort);
          radii[i] = realRadius;
        }
        options.radii = radii;
      }
    }

    WXLogUtils.d(TAG, "Set box-shadow: " + options.toString());

    target.post(new Runnable() {
      @Override
      public void run() {
        if (options.isInset) {
          setInsetBoxShadow(target, options);
        } else {
          setNormalBoxShadow(target, options);
        }
      }
    });
  }

  private static Bitmap createShadowBitmap(int viewWidth, int viewHeight,
                                           float[] radii, float shadowRadius,
                                           float shadowSpread,
                                           float dx, float dy, int shadowColor) {

    if (shadowRadius == 0) {
      // 0 can not draw shadow layer
      shadowRadius = 0.01f;
    }

    int canvasWidth = viewWidth + 2 * (int) (shadowRadius + shadowSpread + Math.abs(dx));
    int canvasHeight = viewHeight + 2 * (int) (shadowRadius + shadowSpread + Math.abs(dy));

    Bitmap output = Bitmap.createBitmap(canvasWidth, canvasHeight, Bitmap.Config.ARGB_8888);
    Canvas canvas = new Canvas(output);

    float offsetX = shadowRadius + shadowSpread + Math.abs(dx);
    float offsetY = shadowRadius + shadowSpread + Math.abs(dy);
    RectF selfRect = new RectF(
        offsetX,
        offsetY,
        (float) Math.floor(viewWidth + offsetX),
        (float) Math.floor(viewHeight + offsetY));
    Path contentPath = new Path();
    contentPath.addRoundRect(selfRect, radii, Path.Direction.CCW);
    // can not antialias
    canvas.clipPath(contentPath, Region.Op.DIFFERENCE);

    float shadowLeft, shadowTop;
    if (shadowSpread == 0f) {
      shadowLeft = shadowRadius;
      shadowTop = shadowRadius;
    } else {
      shadowLeft = shadowRadius + dx - shadowSpread;
      shadowTop = shadowRadius + dy - shadowSpread;
    }
    RectF shadowRect = new RectF(
        shadowLeft,
        shadowTop,
        canvasWidth - shadowRadius + shadowSpread,
        canvasHeight - shadowRadius + shadowSpread);

    shadowRect.top += Math.abs(dy);
    shadowRect.bottom -= Math.abs(dy);
    shadowRect.left += Math.abs(dx);
    shadowRect.right -= Math.abs(dx);

    Paint shadowPaint = new Paint();
    shadowPaint.setAntiAlias(true);
    shadowPaint.setColor(shadowColor);
    shadowPaint.setStyle(Paint.Style.FILL);

    float shadowDx = 0f, shadowDy = 0f;
    if (shadowSpread == 0f) {
      shadowDx = dx;
      shadowDy = dy;
    }
    shadowPaint.setShadowLayer(shadowRadius, shadowDx, shadowDy, shadowColor);

    Path shadowPath = new Path();
    float[] shadowRadii = new float[8];
    for (int i = 0; i < radii.length; i++) {
      float contentRadius = radii[i];
      if (contentRadius == 0f) {
        shadowRadii[i] = 0f;
      } else {
        shadowRadii[i] = radii[i] + shadowSpread;
      }
    }
    shadowPath.addRoundRect(shadowRect, shadowRadii, Path.Direction.CCW);
    canvas.drawPath(shadowPath, shadowPaint);
    return output;
  }

  private static void setNormalBoxShadow(View target, BoxShadowOptions options) {
    int h = target.getHeight();
    int w = target.getWidth();

    if (h == 0 || w == 0) {
      Log.w(TAG, "Target view is invisible, ignore set shadow.");
      return;
    }

    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
      Bitmap shadowBitmap = createShadowBitmap(w, h, options.radii, options.blur, options.spread, options.hShadow, options.vShadow, options.color);

      int overflowX = (int) (options.blur + Math.abs(options.hShadow) + options.spread);
      int overflowY = (int) (options.blur + Math.abs(options.vShadow) + options.spread);

      //Drawable's bounds must match the bitmap size, otherwise the shadows will be scaled
      OverflowBitmapDrawable shadowDrawable = new OverflowBitmapDrawable(target.getResources(), shadowBitmap, overflowX, overflowY);
      shadowDrawable.setBounds(-overflowX, -overflowY, w + overflowX, h + overflowY);

      target.getOverlay().clear();
      target.getOverlay().add(shadowDrawable);
      //Relayout to ensure the shadows are fully drawn
      target.getParent().requestLayout();
      ((ViewGroup) target.getParent()).invalidate(shadowDrawable.getBounds());
    } else {
      // I have a dream that one day our minSdkVersion will equals or higher than 21
      Log.w("BoxShadowUtil", "Call setNormalBoxShadow() requires API level 18 or higher.");
    }
  }

  private static void setInsetBoxShadow(View target, BoxShadowOptions options) {
    if (target == null || options == null) {
      WXLogUtils.w(TAG, "Illegal arguments");
      return;
    }

    if (target.getWidth() == 0 || target.getHeight() == 0) {
      WXLogUtils.w(TAG, "Target view is invisible, ignore set shadow.");
      return;
    }

    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
      Drawable shadow = new InsetShadowDrawable(target.getWidth(), target.getHeight(),
          options.hShadow, options.vShadow,
          options.blur, options.spread,
          options.color, options.radii);
      target.getOverlay().clear();
      target.getOverlay().add(shadow);
      target.invalidate();
    } else {
      Log.w(TAG, "Call setInsetBoxShadow() requires API level 18 or higher.");
    }
  }

  public static BoxShadowOptions parseBoxShadow(String boxShadow, int viewport) {
    BoxShadowOptions result = new BoxShadowOptions(viewport);
    if (TextUtils.isEmpty(boxShadow)) {
      result.isClear = true;
      return result;
    }

    String boxShadowCopy = boxShadow;

    // trim rgb() & rgba()
    boxShadowCopy = boxShadowCopy.replaceAll("\\s*,\\s+", ",");

    // match inset first
    if (boxShadowCopy.contains("inset")) {
      result.isInset = true;
      boxShadowCopy = boxShadowCopy.replace("inset", "").trim();
    }

    List<String> params = new ArrayList<>(Arrays.asList(boxShadowCopy.split("\\s+")));

    // match color
    String maybeColor = params.get(params.size() - 1);
    if (!TextUtils.isEmpty(maybeColor)) {
      if (maybeColor.startsWith("#") || maybeColor.startsWith("rgb") || WXResourceUtils.isNamedColor(maybeColor)) {
        result.color = WXResourceUtils.getColor(maybeColor, Color.BLACK);
        params.remove(params.size() - 1);
      }
    }

    try {
      if (params.size() < 2) {
        // Missing required param
        return null;
      } else {
        if (!TextUtils.isEmpty(params.get(0))) {
          float px = WXUtils.getFloat(params.get(0).trim(), 0f);
          result.hShadow = WXViewUtils.getRealSubPxByWidth(px, viewport);
        }

        if (!TextUtils.isEmpty(params.get(1))) {
          float px = WXUtils.getFloat(params.get(1).trim(), 0f);
          result.vShadow = WXViewUtils.getRealPxByWidth(px, viewport);
        }

        for (int i = 2; i < params.size(); i++) {
          int parserIndex = i - 2;
          BoxShadowOptions.IParser parser = result.optionParamParsers.get(parserIndex);
          parser.parse(params.get(i));
        }
      }
    } catch (Throwable t) {
      t.printStackTrace();
    }
    return result;
  }

  private static class OverflowBitmapDrawable extends BitmapDrawable {
    int paddingX;
    int paddingY;

    private OverflowBitmapDrawable(Resources resources, Bitmap bitmap, int paddingX, int paddingY) {
      super(resources, bitmap);
      this.paddingX = paddingX;
      this.paddingY = paddingY;
    }

    @Override
    public void draw(Canvas canvas) {
      Rect newRect = canvas.getClipBounds();
      // Make the Canvas Rect bigger according to the padding.
      newRect.inset(-paddingX * 2, -paddingY * 2);
      canvas.clipRect(newRect, Region.Op.REPLACE);
      super.draw(canvas);
    }
  }

  private static class InsetShadowDrawable extends Drawable {
    private static final int LEFT_TO_RIGHT = 0;
    private static final int TOP_TO_BOTTOM = 1;
    private static final int RIGHT_TO_LEFT = 2;
    private static final int BOTTOM_TO_TOP = 3;

    private float blurRadius;
    private int shadowColor;

    private float[] radii;

    private float width, height;

    private float shadowXSize, shadowYSize;

    private Shader[] shades = new Shader[4];
    private Path[] paths = new Path[4];

    private Paint paint;

    private InsetShadowDrawable(int viewWidth, int viewHeight, float dx, float dy, float blurRadius, float spread, int shadowColor, float[] radii) {
      this.blurRadius = blurRadius;
      this.shadowColor = shadowColor;

      this.width = viewWidth + 2 * dx;
      this.height = viewHeight + 2 * dy;

      this.shadowXSize = dx + spread;
      this.shadowYSize = dy + spread;

      this.radii = radii;

      setBounds(0, 0, viewWidth, viewHeight);
      prepare();
    }

    private void prepare() {
      /*
       *  A +++++++++++++++++++++++++ B
       *  +  E ------------------- F  +
       *  +  |                     |  +
       *  +  |                     |  +
       *  +  |                     |  +
       *  +  H ------------------- G  +
       *  D +++++++++++++++++++++++++ C
       */

      PointF a = new PointF(0, 0);
      PointF b = new PointF(width, 0);
      PointF c = new PointF(b.x, height);
      PointF d = new PointF(a.x, c.y);

      PointF e = new PointF(shadowXSize, shadowYSize);
      PointF f = new PointF(b.x - shadowXSize, e.y);
      PointF g = new PointF(f.x, c.y - shadowYSize);
      PointF h = new PointF(e.x, g.y);

      Shader ltr = new LinearGradient(e.x - blurRadius, e.y, e.x, e.y, shadowColor, Color.TRANSPARENT, Shader.TileMode.CLAMP);
      Shader ttb = new LinearGradient(e.x, e.y - blurRadius, e.x, e.y, shadowColor, Color.TRANSPARENT, Shader.TileMode.CLAMP);
      Shader rtl = new LinearGradient(g.x + blurRadius, g.y, g.x, g.y, shadowColor, Color.TRANSPARENT, Shader.TileMode.CLAMP);
      Shader btt = new LinearGradient(g.x, g.y + blurRadius, g.x, g.y, shadowColor, Color.TRANSPARENT, Shader.TileMode.CLAMP);

      shades[LEFT_TO_RIGHT] = ltr;
      shades[TOP_TO_BOTTOM] = ttb;
      shades[RIGHT_TO_LEFT] = rtl;
      shades[BOTTOM_TO_TOP] = btt;

      Path ltrPath = new Path();
      ltrPath.moveTo(a.x, a.y);
      ltrPath.lineTo(e.x, e.y);
      ltrPath.lineTo(h.x, h.y);
      ltrPath.lineTo(d.x, d.y);
      ltrPath.close();

      Path ttbPath = new Path();
      ttbPath.moveTo(a.x, a.y);
      ttbPath.lineTo(b.x, b.y);
      ttbPath.lineTo(f.x, f.y);
      ttbPath.lineTo(e.x, e.y);
      ttbPath.close();

      Path rtlPath = new Path();
      rtlPath.moveTo(b.x, b.y);
      rtlPath.lineTo(c.x, c.y);
      rtlPath.lineTo(g.x, g.y);
      rtlPath.lineTo(f.x, f.y);
      rtlPath.close();

      Path bttPath = new Path();
      bttPath.moveTo(d.x, d.y);
      bttPath.lineTo(c.x, c.y);
      bttPath.lineTo(g.x, g.y);
      bttPath.lineTo(h.x, h.y);
      bttPath.close();

      paths[LEFT_TO_RIGHT] = ltrPath;
      paths[TOP_TO_BOTTOM] = ttbPath;
      paths[RIGHT_TO_LEFT] = rtlPath;
      paths[BOTTOM_TO_TOP] = bttPath;

      paint = new Paint();
      paint.setAntiAlias(true);
      paint.setStyle(Paint.Style.FILL);
      paint.setColor(shadowColor);
    }

    @Override
    public void draw(Canvas canvas) {
      Path border = new Path();
      RectF rectF = new RectF(0, 0, canvas.getWidth(), canvas.getHeight());
      border.addRoundRect(rectF, radii, Path.Direction.CCW);
      canvas.clipPath(border);
      //TODO: we need clip border-width too

      for (int i = 0; i < 4; i++) {
        Shader shader = shades[i];
        Path path = paths[i];
        paint.setShader(shader);
        canvas.drawPath(path, paint);
      }
    }

    @Override
    public void setAlpha(int alpha) {

    }

    @Override
    public void setColorFilter(ColorFilter cf) {

    }

    @Override
    public int getOpacity() {
      return PixelFormat.OPAQUE;
    }
  }

  public static class BoxShadowOptions {
    private List<IParser> optionParamParsers;
    private int viewport = 750;

    public float hShadow;
    public float vShadow;
    public float blur = 0f;
    public float spread = 0f;
    public float[] radii = new float[]{0, 0, 0, 0, 0, 0, 0, 0};
    public int color = Color.BLACK;
    public boolean isInset = false;

    public boolean isClear = false;

    private BoxShadowOptions(int vp) {
      if (viewport != 0) {
        this.viewport = vp;
      }
      optionParamParsers = new ArrayList<>();

      IParser spreadParser = new IParser() {
        @Override
        public void parse(String param) {
          if (!TextUtils.isEmpty(param)) {
            float px = WXUtils.getFloat(param, 0f);
            spread = WXViewUtils.getRealSubPxByWidth(px, viewport);
            WXLogUtils.w(TAG, "Experimental box-shadow attribute: spread");
          }
        }
      };

      IParser blurParser = new IParser() {
        @Override
        public void parse(String param) {
          if (!TextUtils.isEmpty(param)) {
            float px = WXUtils.getFloat(param, 0f);
            blur = WXViewUtils.getRealSubPxByWidth(px, viewport);
          }
        }
      };

      optionParamParsers.add(blurParser);
      optionParamParsers.add(spreadParser);
    }

    @Override
    public String toString() {

      String r = "[" + radii[0] + "," + radii[2] + "," + radii[4] + "," + radii[6] + "]";

      final StringBuffer sb = new StringBuffer("BoxShadowOptions{");
      sb.append("h-shadow=").append(hShadow);
      sb.append(", v-shadow=").append(vShadow);
      sb.append(", blur=").append(blur);
      sb.append(", spread=").append(spread);
      sb.append(", corner-radius=").append(r);
      sb.append(", color=#").append(Integer.toHexString(color));
      sb.append(", inset=").append(isInset);
      sb.append('}');
      return sb.toString();
    }

    private interface IParser {
      void parse(String param);
    }
  }
}
