#ifndef WEEXV8_RENDERSIZE_H
#define WEEXV8_RENDERSIZE_H

namespace WeexCore {
  class RenderSize {
  public:
    float mWidth;
    float mHeight;

  public:
    RenderSize() : mWidth(0), mHeight(0) {}

    RenderSize(int w, int h) : mWidth(w), mHeight(h) {}

    ~RenderSize() {}

    bool Update(RenderSize &renderSize) {
      if (mWidth == renderSize.mWidth &&
          mHeight == renderSize.mHeight) {
        return false;
      }

      mWidth = renderSize.mWidth;
      mHeight = renderSize.mHeight;
      return true;
    }

    bool Update(int width, int height) {
      if (mWidth == width && mHeight == height) {
        return false;
      }

      mWidth = width;
      mHeight = height;
      return true;
    }
  };
}

#endif //WEEXV8_RENDERSIZE_H
