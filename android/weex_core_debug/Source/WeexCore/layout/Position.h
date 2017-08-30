#ifndef WEEXV8_POSITION_H
#define WEEXV8_POSITION_H

namespace WeexCore {

  class Position {
  public:
    float mLeft;
    float mTop;
    float mRight;
    float mBottom;

  public:
    Position() : mLeft(0), mTop(0), mRight(0), mBottom(0) {}

    Position(int l, int t, int r, int b) : mLeft(l), mTop(t), mRight(r), mBottom(b) {}

    ~Position() {}

    bool Update(Position &position) {
      if (mLeft == position.mLeft &&
          mTop == position.mTop &&
          mRight == position.mRight &&
          mBottom == position.mBottom) {
        return false;
      }

      mLeft = position.mLeft;
      mTop = position.mTop;
      mRight = position.mRight;
      mBottom = position.mBottom;
      return true;
    }

    bool Update(int left, int top, int right, int bottom) {
      if (mLeft == left &&
          mTop == top &&
          mRight == right &&
          mBottom == bottom) {
        return false;
      }

      mLeft = left;
      mTop = top;
      mRight = right;
      mBottom = bottom;
      return true;
    }
  };

}

#endif //WEEXV8_POSITION_H
