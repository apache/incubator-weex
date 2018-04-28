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
import android.graphics.BlurMaskFilter;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ColorFilter;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PixelFormat;
import android.graphics.Point;
import android.graphics.PointF;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Region;
import android.graphics.Shader;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.LayerDrawable;
import android.os.Build;
import android.support.annotation.IntRange;
import android.support.annotation.Nullable;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewParent;

import com.taobao.weex.WXEnvironment;
import com.taobao.weex.common.WXThread;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Created by moxun on 2017/9/4.
 * Utils for create shadow layer on view
 * Requires Android 4.3 and higher
 *
 * @see <a href="https://www.w3schools.com/cssref/css3_pr_box-shadow.asp">CSS3 box-shadow Property</>
 */

public class BoxShadowUtil {
  private static final String TAG = "BoxShadowUtil";
  private static boolean sBoxShadowEnabled = true /*disable box-shadow temporary*/;

  private static Pattern sColorPattern;

  public static void setBoxShadowEnabled(boolean enabled) {
    sBoxShadowEnabled = enabled;
    WXLogUtils.w(TAG, "Switch box-shadow status: " + enabled);
  }

  public static boolean isBoxShadowEnabled() {
    return sBoxShadowEnabled;
  }

  public static void setBoxShadow(final View target, String style, final float[] radii, int viewPort, final float quality) {
    if (!sBoxShadowEnabled) {
      WXLogUtils.w(TAG, "box-shadow was disabled by config");
      return;
    }

    if (target == null) {
      WXLogUtils.w(TAG, "Target view is null!");
      return;
    }

    if (TextUtils.isEmpty(style) && Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
      target.getOverlay().clear();
      WXLogUtils.d(TAG, "Remove all box-shadow");
      return;
    }

    final BoxShadowOptions[] shadows = parseBoxShadows(style, viewPort);
    if (shadows == null || shadows.length == 0) {
      WXLogUtils.w(TAG, "Failed to parse box-shadow: " + style);
      return;
    }

    final List<BoxShadowOptions> normalShadows = new ArrayList<>(), insetShadows = new ArrayList<>();
    for (BoxShadowOptions shadow : shadows) {
      if (shadow != null) {
        if (shadow.isInset) {
          insetShadows.add(0, shadow);
        } else {
          normalShadows.add(0, shadow);
        }
      }
    }

    if (radii != null) {
      if (radii.length != 8) {
        WXLogUtils.w(TAG, "Length of radii must be 8");
      } else {
        for (int i = 0; i < radii.length; i++) {
          float realRadius = WXViewUtils.getRealSubPxByWidth(radii[i], viewPort);
          radii[i] = realRadius;
        }
      }
    }

    target.post(WXThread.secure(new Runnable() {
      @Override
      public void run() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
          target.getOverlay().clear();
          if (normalShadows.size() > 0) {
            setNormalBoxShadow(target, normalShadows, quality, radii);
          }

          if (insetShadows.size() > 0) {
            setInsetBoxShadow(target, insetShadows, quality, radii);
          }
        }
      }
    }));
  }

  private static void drawShadow(Canvas canvas, BoxShadowOptions options) {
    RectF shadowRect = new RectF(
        0f, 0f,
        options.viewWidth + 2f * options.spread, options.viewHeight + 2f * options.spread
    );

    if (options.topLeft != null) {
      shadowRect.offset(options.topLeft.x, options.topLeft.y);
    }

    float shadowDx = options.blur;
    float shadowDy = options.blur;
    if (options.hShadow > 0) {
      shadowDx = shadowDx + 2f * options.hShadow;
    }
    if (options.vShadow > 0) {
      shadowDy = shadowDy + 2f * options.vShadow;
    }
    shadowRect.offset(shadowDx, shadowDy);

    Paint shadowPaint = new Paint();
    shadowPaint.setAntiAlias(true);
    shadowPaint.setColor(options.color);
    shadowPaint.setStyle(Paint.Style.FILL);

    if (options.blur > 0) {
      shadowPaint.setMaskFilter(new BlurMaskFilter(options.blur, BlurMaskFilter.Blur.NORMAL));
    }

    Path shadowPath = new Path();
    float[] shadowRadii = new float[8];
    for (int i = 0; i < options.radii.length; i++) {
      float contentRadius = options.radii[i];
      if (contentRadius == 0f) {
        shadowRadii[i] = 0f;
      } else {
        shadowRadii[i] = options.radii[i] + options.spread;
      }
    }
    shadowPath.addRoundRect(shadowRect, shadowRadii, Path.Direction.CCW);
    canvas.drawPath(shadowPath, shadowPaint);

    if (false && WXEnvironment.isApkDebugable()) {
      Paint paint = new Paint();
      paint.setAntiAlias(true);
      paint.setColor(Color.BLACK);
      paint.setStyle(Paint.Style.STROKE);
      canvas.drawRect(shadowRect, paint);
    }
  }

  private static void setNormalBoxShadow(View target, List<BoxShadowOptions> options, float quality, float[] radii) {
    int h = target.getHeight();
    int w = target.getWidth();

    ViewGroup.LayoutParams p = target.getLayoutParams();

    if (h == 0 || w == 0) {
      Log.w(TAG, "Target view is invisible, ignore set shadow.");
      return;
    }

    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
      int maxWidth = 0, maxHeight = 0;
      for (BoxShadowOptions option : options) {
        option.viewWidth = w;
        option.viewHeight = h;
        option.radii = radii;

        Rect rect = option.getTargetCanvasRect();
        if (maxWidth < rect.width()) {
          maxWidth = rect.width();
        }

        if (maxHeight < rect.height()) {
          maxHeight = rect.height();
        }
      }

      int canvasWidth = (int) (maxWidth * quality);
      int canvasHeight = (int) (maxHeight * quality);
      Bitmap output = Bitmap.createBitmap(canvasWidth, canvasHeight, Bitmap.Config.ARGB_4444);
      if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
        WXLogUtils.d(TAG, "Allocation memory for box-shadow: " + (output.getAllocationByteCount() / 1024) + " KB");
      }
      Canvas canvas = new Canvas(output);

      if (false && WXEnvironment.isApkDebugable()) {
        // Using for debug
        Paint strokePaint = new Paint();
        strokePaint.setColor(Color.BLACK);
        strokePaint.setStrokeWidth(2);
        strokePaint.setStyle(Paint.Style.STROKE);
        canvas.drawRect(canvas.getClipBounds(), strokePaint);
      }

      for (BoxShadowOptions option : options) {
        Rect rect = option.getTargetCanvasRect();
        float left = (maxWidth - rect.width()) / 2f;
        float top = (maxHeight - rect.height()) / 2f;
        option.topLeft = new PointF(left, top);

        BoxShadowOptions scaledOption = option.scale(quality);
        drawShadow(canvas, scaledOption);
      }

      //Drawable's bounds must match the bitmap size, otherwise the shadows will be scaled
      int paddingX = (maxWidth - w) / 2;
      int paddingY = (maxHeight - h) / 2;
      OverflowBitmapDrawable shadowDrawable = new OverflowBitmapDrawable(target.getResources(),
          output, new Point(paddingX, paddingY), new Rect(0, 0, w, h), radii);

      target.getOverlay().add(shadowDrawable);
      //Relayout to ensure the shadows are fully drawn
      ViewParent parent = target.getParent();
      if (parent != null) {
        parent.requestLayout();
        if (parent instanceof ViewGroup) {
          ((ViewGroup) parent).invalidate(shadowDrawable.getBounds());
        }
      }
    } else {
      // I have a dream that one day our minSdkVersion will equals or higher than 21
      Log.w("BoxShadowUtil", "Call setNormalBoxShadow() requires API level 18 or higher.");
    }
  }

  private static void setInsetBoxShadow(View target, List<BoxShadowOptions> options, float quality, float[] radii) {
    if (target == null || options == null) {
      WXLogUtils.w(TAG, "Illegal arguments");
      return;
    }

    if (target.getWidth() == 0 || target.getHeight() == 0) {
      WXLogUtils.w(TAG, "Target view is invisible, ignore set shadow.");
      return;
    }

    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
      Drawable[] drawables = new Drawable[options.size()];
      for (int i = 0; i < options.size(); i++) {
        BoxShadowOptions option = options.get(i);
        Drawable shadow = new InsetShadowDrawable(target.getWidth(), target.getHeight(),
            option.hShadow, option.vShadow,
            option.blur, option.spread,
            option.color, radii);
        drawables[i] = shadow;
      }

      LayerDrawable layerDrawable = new LayerDrawable(drawables);
      target.getOverlay().add(layerDrawable);
      target.invalidate();
    } else {
      Log.w(TAG, "Call setInsetBoxShadow() requires API level 18 or higher.");
    }
  }

  public static BoxShadowOptions[] parseBoxShadows(String boxShadowStyle, int viewport) {
    // normalization color expression to #AARRGGBB
    if (sColorPattern == null) {
      sColorPattern = Pattern.compile("([rR][gG][bB][aA]?)\\((\\d+\\s*),\\s*(\\d+\\s*),\\s*(\\d+\\s*)(?:,\\s*(\\d+(?:\\.\\d+)?))?\\)");
    }

    Matcher matcher = sColorPattern.matcher(boxShadowStyle);

    String processedStyle = boxShadowStyle;
    while (matcher.find()) {
      String color = matcher.group();
      processedStyle = processedStyle.replace(color, "#" + String.format("%8s", Integer.toHexString(WXResourceUtils.getColor(color, Color.BLACK))).replaceAll("\\s", "0"));
    }

    String[] styles = processedStyle.split(",");
    if (styles != null && styles.length > 0) {
      BoxShadowOptions[] result = new BoxShadowOptions[styles.length];
      for (int i = 0; i < styles.length; i++) {
        result[i] = parseBoxShadow(styles[i], viewport);
      }
      return result;
    }
    return null;
  }

  private static BoxShadowOptions parseBoxShadow(String boxShadow, int viewport) {
    BoxShadowOptions result = new BoxShadowOptions(viewport);
    if (TextUtils.isEmpty(boxShadow)) {
      return null;
    }

    String boxShadowCopy = boxShadow;

    // trim rgb() & rgba()
    boxShadowCopy = boxShadowCopy.replaceAll("\\s*,\\s+", ",");

    // match inset first
    if (boxShadowCopy.contains("inset")) {
      result.isInset = true;
      boxShadowCopy = boxShadowCopy.replace("inset", "");
    }

    boxShadowCopy = boxShadowCopy.trim();
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
    private int paddingX;
    private int paddingY;
    private Rect viewRect;
    private float[] radii;

    private OverflowBitmapDrawable(Resources resources, Bitmap bitmap, Point topLeft, Rect viewRect, float[] radii) {
      super(resources, bitmap);
      this.paddingX = topLeft.x;
      this.paddingY = topLeft.y;
      this.viewRect = viewRect;
      this.radii = radii;

      setBounds(-paddingX, -paddingY, viewRect.width() + paddingX, viewRect.height() + paddingY);
    }

    public void draw(Canvas canvas) {
      Rect bounds = canvas.getClipBounds();
      Rect newRect = new Rect(bounds);
      // Make the Canvas Rect bigger according to the padding.
      newRect.inset(-paddingX * 2, -paddingY * 2);
      canvas.clipRect(newRect, Region.Op.REPLACE);

      Path contentPath = new Path();
      // the content area map must be aligned with bounds
      RectF rectF = new RectF(bounds);
      contentPath.addRoundRect(rectF, radii, Path.Direction.CCW);
      // can not antialias
      canvas.clipPath(contentPath, Region.Op.DIFFERENCE);

      // translate the canvas to a suitable position and then draw the bitmap, otherwise draw from the origin(0, 0)
      canvas.translate(bounds.left, bounds.top);

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
      Rect bounds = canvas.getClipBounds();
      Path border = new Path();
      RectF rectF = new RectF(bounds);
      border.addRoundRect(rectF, radii, Path.Direction.CCW);
      canvas.clipPath(border);
      //TODO: we need clip border-width too

      // translate the canvas to the right place and then draw the inner shadow
      canvas.translate(bounds.left, bounds.top);

      for (int i = 0; i < 4; i++) {
        Shader shader = shades[i];
        Path path = paths[i];
        paint.setShader(shader);
        canvas.drawPath(path, paint);
      }
    }

    @Override
    public void setAlpha(@IntRange(from = 0, to = 255) int alpha) {

    }

    @Override
    public void setColorFilter(@Nullable ColorFilter colorFilter) {

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

    public int viewWidth = 0;
    public int viewHeight = 0;
    public PointF topLeft = null;

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

    public BoxShadowOptions scale(float scale) {
      if (scale > 0f && scale <= 1f) {
        BoxShadowOptions scaledOptions = new BoxShadowOptions(viewport);
        scaledOptions.hShadow = hShadow * scale;
        scaledOptions.vShadow = vShadow * scale;
        scaledOptions.blur = blur * scale;
        scaledOptions.spread = spread * scale;
        for (int i = 0; i < radii.length ; i++) {
          scaledOptions.radii[i] = radii[i] * scale;
        }
        scaledOptions.viewHeight = (int) (viewHeight * scale);
        scaledOptions.viewWidth = (int) (viewWidth * scale);

        if (topLeft != null) {
          scaledOptions.topLeft = new PointF();
          scaledOptions.topLeft.x = topLeft.x * scale;
          scaledOptions.topLeft.y = topLeft.y * scale;
        }

        scaledOptions.color = color;
        scaledOptions.isInset = isInset;
        WXLogUtils.d(TAG, "Scaled BoxShadowOptions: [" + scale + "] " + scaledOptions);
        return scaledOptions;
      }
      return null;
    }

    public Rect getTargetCanvasRect() {
      int canvasWidth = viewWidth + 2 * (int) (blur + spread + Math.abs(hShadow));
      int canvasHeight = viewHeight + 2 * (int) (blur + spread + Math.abs(vShadow));
      return new Rect(0, 0, canvasWidth, canvasHeight);
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
