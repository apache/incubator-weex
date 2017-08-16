/*
 * Copyright (C) 2015-2016 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "MathCommon.h"

#include "PureNaN.h"

namespace JSC {

#if PLATFORM(IOS) && CPU(ARM_THUMB2)

// The following code is taken from netlib.org:
//   http://www.netlib.org/fdlibm/fdlibm.h
//   http://www.netlib.org/fdlibm/e_pow.c
//   http://www.netlib.org/fdlibm/s_scalbn.c
//
// And was originally distributed under the following license:

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
/*
 * ====================================================
 * Copyright (C) 2004 by Sun Microsystems, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/* __ieee754_pow(x,y) return x**y
 *
 *              n
 * Method:  Let x =  2   * (1+f)
 *    1. Compute and return log2(x) in two pieces:
 *        log2(x) = w1 + w2,
 *       where w1 has 53-24 = 29 bit trailing zeros.
 *    2. Perform y*log2(x) = n+y' by simulating muti-precision
 *       arithmetic, where |y'|<=0.5.
 *    3. Return x**y = 2**n*exp(y'*log2)
 *
 * Special cases:
 *    1.  (anything) ** 0  is 1
 *    2.  (anything) ** 1  is itself
 *    3.  (anything) ** NAN is NAN
 *    4.  NAN ** (anything except 0) is NAN
 *    5.  +-(|x| > 1) **  +INF is +INF
 *    6.  +-(|x| > 1) **  -INF is +0
 *    7.  +-(|x| < 1) **  +INF is +0
 *    8.  +-(|x| < 1) **  -INF is +INF
 *    9.  +-1         ** +-INF is NAN
 *    10. +0 ** (+anything except 0, NAN)               is +0
 *    11. -0 ** (+anything except 0, NAN, odd integer)  is +0
 *    12. +0 ** (-anything except 0, NAN)               is +INF
 *    13. -0 ** (-anything except 0, NAN, odd integer)  is +INF
 *    14. -0 ** (odd integer) = -( +0 ** (odd integer) )
 *    15. +INF ** (+anything except 0,NAN) is +INF
 *    16. +INF ** (-anything except 0,NAN) is +0
 *    17. -INF ** (anything)  = -0 ** (-anything)
 *    18. (-anything) ** (integer) is (-1)**(integer)*(+anything**integer)
 *    19. (-anything except 0 and inf) ** (non-integer) is NAN
 *
 * Accuracy:
 *    pow(x,y) returns x**y nearly rounded. In particular
 *            pow(integer,integer)
 *    always returns the correct integer provided it is
 *    representable.
 *
 * Constants :
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

#define __HI(x) *(1+(int*)&x)
#define __LO(x) *(int*)&x

static const double
bp[] = {1.0, 1.5,},
dp_h[] = { 0.0, 5.84962487220764160156e-01,}, /* 0x3FE2B803, 0x40000000 */
dp_l[] = { 0.0, 1.35003920212974897128e-08,}, /* 0x3E4CFDEB, 0x43CFD006 */
zero    =  0.0,
one    =  1.0,
two    =  2.0,
two53    =  9007199254740992.0,    /* 0x43400000, 0x00000000 */
huge    =  1.0e300,
tiny    =  1.0e-300,
/* for scalbn */
two54   =  1.80143985094819840000e+16, /* 0x43500000, 0x00000000 */
twom54  =  5.55111512312578270212e-17, /* 0x3C900000, 0x00000000 */
/* poly coefs for (3/2)*(log(x)-2s-2/3*s**3 */
L1  =  5.99999999999994648725e-01, /* 0x3FE33333, 0x33333303 */
L2  =  4.28571428578550184252e-01, /* 0x3FDB6DB6, 0xDB6FABFF */
L3  =  3.33333329818377432918e-01, /* 0x3FD55555, 0x518F264D */
L4  =  2.72728123808534006489e-01, /* 0x3FD17460, 0xA91D4101 */
L5  =  2.30660745775561754067e-01, /* 0x3FCD864A, 0x93C9DB65 */
L6  =  2.06975017800338417784e-01, /* 0x3FCA7E28, 0x4A454EEF */
P1   =  1.66666666666666019037e-01, /* 0x3FC55555, 0x5555553E */
P2   = -2.77777777770155933842e-03, /* 0xBF66C16C, 0x16BEBD93 */
P3   =  6.61375632143793436117e-05, /* 0x3F11566A, 0xAF25DE2C */
P4   = -1.65339022054652515390e-06, /* 0xBEBBBD41, 0xC5D26BF1 */
P5   =  4.13813679705723846039e-08, /* 0x3E663769, 0x72BEA4D0 */
lg2  =  6.93147180559945286227e-01, /* 0x3FE62E42, 0xFEFA39EF */
lg2_h  =  6.93147182464599609375e-01, /* 0x3FE62E43, 0x00000000 */
lg2_l  = -1.90465429995776804525e-09, /* 0xBE205C61, 0x0CA86C39 */
ovt =  8.0085662595372944372e-0017, /* -(1024-log2(ovfl+.5ulp)) */
cp    =  9.61796693925975554329e-01, /* 0x3FEEC709, 0xDC3A03FD =2/(3ln2) */
cp_h  =  9.61796700954437255859e-01, /* 0x3FEEC709, 0xE0000000 =(float)cp */
cp_l  = -7.02846165095275826516e-09, /* 0xBE3E2FE0, 0x145B01F5 =tail of cp_h*/
ivln2    =  1.44269504088896338700e+00, /* 0x3FF71547, 0x652B82FE =1/ln2 */
ivln2_h  =  1.44269502162933349609e+00, /* 0x3FF71547, 0x60000000 =24b 1/ln2*/
ivln2_l  =  1.92596299112661746887e-08; /* 0x3E54AE0B, 0xF85DDF44 =1/ln2 tail*/

inline double fdlibmScalbn (double x, int n)
{
    int  k,hx,lx;
    hx = __HI(x);
    lx = __LO(x);
    k = (hx&0x7ff00000)>>20;        /* extract exponent */
    if (k==0) {                /* 0 or subnormal x */
        if ((lx|(hx&0x7fffffff))==0) return x; /* +-0 */
        x *= two54;
        hx = __HI(x);
        k = ((hx&0x7ff00000)>>20) - 54;
        if (n< -50000) return tiny*x;     /*underflow*/
    }
    if (k==0x7ff) return x+x;        /* NaN or Inf */
    k = k+n;
    if (k >  0x7fe) return huge*copysign(huge,x); /* overflow  */
    if (k > 0)                 /* normal result */
    {__HI(x) = (hx&0x800fffff)|(k<<20); return x;}
    if (k <= -54) {
        if (n > 50000)     /* in case integer overflow in n+k */
            return huge*copysign(huge,x);    /*overflow*/
        else return tiny*copysign(tiny,x);     /*underflow*/
    }
    k += 54;                /* subnormal result */
    __HI(x) = (hx&0x800fffff)|(k<<20);
    return x*twom54;
}

static double fdlibmPow(double x, double y)
{
    double z,ax,z_h,z_l,p_h,p_l;
    double y1,t1,t2,r,s,t,u,v,w;
    int i0,i1,i,j,k,yisint,n;
    int hx,hy,ix,iy;
    unsigned lx,ly;

    i0 = ((*(int*)&one)>>29)^1; i1=1-i0;
    hx = __HI(x); lx = __LO(x);
    hy = __HI(y); ly = __LO(y);
    ix = hx&0x7fffffff;  iy = hy&0x7fffffff;

    /* y==zero: x**0 = 1 */
    if((iy|ly)==0) return one;

    /* +-NaN return x+y */
    if(ix > 0x7ff00000 || ((ix==0x7ff00000)&&(lx!=0)) ||
       iy > 0x7ff00000 || ((iy==0x7ff00000)&&(ly!=0)))
        return x+y;

    /* determine if y is an odd int when x < 0
     * yisint = 0    ... y is not an integer
     * yisint = 1    ... y is an odd int
     * yisint = 2    ... y is an even int
     */
    yisint  = 0;
    if(hx<0) {
        if(iy>=0x43400000) yisint = 2; /* even integer y */
        else if(iy>=0x3ff00000) {
            k = (iy>>20)-0x3ff;       /* exponent */
            if(k>20) {
                j = ly>>(52-k);
                if(static_cast<unsigned>(j<<(52-k))==ly) yisint = 2-(j&1);
            } else if(ly==0) {
                j = iy>>(20-k);
                if((j<<(20-k))==iy) yisint = 2-(j&1);
            }
        }
    }

    /* special value of y */
    if(ly==0) {
        if (iy==0x7ff00000) {    /* y is +-inf */
            if(((ix-0x3ff00000)|lx)==0)
                return  y - y;    /* inf**+-1 is NaN */
            else if (ix >= 0x3ff00000)/* (|x|>1)**+-inf = inf,0 */
                return (hy>=0)? y: zero;
            else            /* (|x|<1)**-,+inf = inf,0 */
                return (hy<0)?-y: zero;
        }
        if(iy==0x3ff00000) {    /* y is  +-1 */
            if(hy<0) return one/x; else return x;
        }
        if(hy==0x40000000) return x*x; /* y is  2 */
        if(hy==0x3fe00000) {    /* y is  0.5 */
            if(hx>=0)    /* x >= +0 */
                return sqrt(x);
        }
    }

    ax   = fabs(x);
    /* special value of x */
    if(lx==0) {
        if(ix==0x7ff00000||ix==0||ix==0x3ff00000){
            z = ax;            /*x is +-0,+-inf,+-1*/
            if(hy<0) z = one/z;    /* z = (1/|x|) */
            if(hx<0) {
                if(((ix-0x3ff00000)|yisint)==0) {
                    z = (z-z)/(z-z); /* (-1)**non-int is NaN */
                } else if(yisint==1)
                    z = -z;        /* (x<0)**odd = -(|x|**odd) */
            }
            return z;
        }
    }

    n = (hx>>31)+1;

    /* (x<0)**(non-int) is NaN */
    if((n|yisint)==0) return (x-x)/(x-x);

    s = one; /* s (sign of result -ve**odd) = -1 else = 1 */
    if((n|(yisint-1))==0) s = -one;/* (-ve)**(odd int) */

    /* |y| is huge */
    if(iy>0x41e00000) { /* if |y| > 2**31 */
        if(iy>0x43f00000){    /* if |y| > 2**64, must o/uflow */
            if(ix<=0x3fefffff) return (hy<0)? huge*huge:tiny*tiny;
            if(ix>=0x3ff00000) return (hy>0)? huge*huge:tiny*tiny;
        }
        /* over/underflow if x is not close to one */
        if(ix<0x3fefffff) return (hy<0)? s*huge*huge:s*tiny*tiny;
        if(ix>0x3ff00000) return (hy>0)? s*huge*huge:s*tiny*tiny;
        /* now |1-x| is tiny <= 2**-20, suffice to compute
         log(x) by x-x^2/2+x^3/3-x^4/4 */
        t = ax-one;        /* t has 20 trailing zeros */
        w = (t*t)*(0.5-t*(0.3333333333333333333333-t*0.25));
        u = ivln2_h*t;    /* ivln2_h has 21 sig. bits */
        v = t*ivln2_l-w*ivln2;
        t1 = u+v;
        __LO(t1) = 0;
        t2 = v-(t1-u);
    } else {
        double ss,s2,s_h,s_l,t_h,t_l;
        n = 0;
        /* take care subnormal number */
        if(ix<0x00100000)
        {ax *= two53; n -= 53; ix = __HI(ax); }
        n  += ((ix)>>20)-0x3ff;
        j  = ix&0x000fffff;
        /* determine interval */
        ix = j|0x3ff00000;        /* normalize ix */
        if(j<=0x3988E) k=0;        /* |x|<sqrt(3/2) */
        else if(j<0xBB67A) k=1;    /* |x|<sqrt(3)   */
        else {k=0;n+=1;ix -= 0x00100000;}
        __HI(ax) = ix;

        /* compute ss = s_h+s_l = (x-1)/(x+1) or (x-1.5)/(x+1.5) */
        u = ax-bp[k];        /* bp[0]=1.0, bp[1]=1.5 */
        v = one/(ax+bp[k]);
        ss = u*v;
        s_h = ss;
        __LO(s_h) = 0;
        /* t_h=ax+bp[k] High */
        t_h = zero;
        __HI(t_h)=((ix>>1)|0x20000000)+0x00080000+(k<<18);
        t_l = ax - (t_h-bp[k]);
        s_l = v*((u-s_h*t_h)-s_h*t_l);
        /* compute log(ax) */
        s2 = ss*ss;
        r = s2*s2*(L1+s2*(L2+s2*(L3+s2*(L4+s2*(L5+s2*L6)))));
        r += s_l*(s_h+ss);
        s2  = s_h*s_h;
        t_h = 3.0+s2+r;
        __LO(t_h) = 0;
        t_l = r-((t_h-3.0)-s2);
        /* u+v = ss*(1+...) */
        u = s_h*t_h;
        v = s_l*t_h+t_l*ss;
        /* 2/(3log2)*(ss+...) */
        p_h = u+v;
        __LO(p_h) = 0;
        p_l = v-(p_h-u);
        z_h = cp_h*p_h;        /* cp_h+cp_l = 2/(3*log2) */
        z_l = cp_l*p_h+p_l*cp+dp_l[k];
        /* log2(ax) = (ss+..)*2/(3*log2) = n + dp_h + z_h + z_l */
        t = (double)n;
        t1 = (((z_h+z_l)+dp_h[k])+t);
        __LO(t1) = 0;
        t2 = z_l-(((t1-t)-dp_h[k])-z_h);
    }

    /* split up y into y1+y2 and compute (y1+y2)*(t1+t2) */
    y1  = y;
    __LO(y1) = 0;
    p_l = (y-y1)*t1+y*t2;
    p_h = y1*t1;
    z = p_l+p_h;
    j = __HI(z);
    i = __LO(z);
    if (j>=0x40900000) {                /* z >= 1024 */
        if(((j-0x40900000)|i)!=0)            /* if z > 1024 */
            return s*huge*huge;            /* overflow */
        else {
            if(p_l+ovt>z-p_h) return s*huge*huge;    /* overflow */
        }
    } else if((j&0x7fffffff)>=0x4090cc00 ) {    /* z <= -1075 */
        if(((j-0xc090cc00)|i)!=0)         /* z < -1075 */
            return s*tiny*tiny;        /* underflow */
        else {
            if(p_l<=z-p_h) return s*tiny*tiny;    /* underflow */
        }
    }
    /*
     * compute 2**(p_h+p_l)
     */
    i = j&0x7fffffff;
    k = (i>>20)-0x3ff;
    n = 0;
    if(i>0x3fe00000) {        /* if |z| > 0.5, set n = [z+0.5] */
        n = j+(0x00100000>>(k+1));
        k = ((n&0x7fffffff)>>20)-0x3ff;    /* new k for n */
        t = zero;
        __HI(t) = (n&~(0x000fffff>>k));
        n = ((n&0x000fffff)|0x00100000)>>(20-k);
        if(j<0) n = -n;
        p_h -= t;
    }
    t = p_l+p_h;
    __LO(t) = 0;
    u = t*lg2_h;
    v = (p_l-(t-p_h))*lg2+t*lg2_l;
    z = u+v;
    w = v-(z-u);
    t  = z*z;
    t1  = z - t*(P1+t*(P2+t*(P3+t*(P4+t*P5))));
    r  = (z*t1)/(t1-two)-(w+z*w);
    z  = one-(r-z);
    j  = __HI(z);
    j += (n<<20);
    if((j>>20)<=0) z = fdlibmScalbn(z,n);    /* subnormal output */
    else __HI(z) += (n<<20);
    return s*z;
}

static ALWAYS_INLINE bool isDenormal(double x)
{
    static const uint64_t signbit = 0x8000000000000000ULL;
    static const uint64_t minNormal = 0x0001000000000000ULL;
    return (bitwise_cast<uint64_t>(x) & ~signbit) - 1 < minNormal - 1;
}

static ALWAYS_INLINE bool isEdgeCase(double x)
{
    static const uint64_t signbit = 0x8000000000000000ULL;
    static const uint64_t infinity = 0x7fffffffffffffffULL;
    return (bitwise_cast<uint64_t>(x) & ~signbit) - 1 >= infinity - 1;
}

static ALWAYS_INLINE double mathPowInternal(double x, double y)
{
    if (!isDenormal(x) && !isDenormal(y)) {
        double libmResult = std::pow(x, y);
        if (libmResult || isEdgeCase(x) || isEdgeCase(y))
            return libmResult;
    }
    return fdlibmPow(x, y);
}

#else

ALWAYS_INLINE double mathPowInternal(double x, double y)
{
    return pow(x, y);
}

#endif

double JIT_OPERATION operationMathPow(double x, double y)
{
    if (std::isnan(y))
        return PNaN;
    double absoluteBase = fabs(x);
    if (absoluteBase == 1 && std::isinf(y))
        return PNaN;

    if (y == 0.5) {
        if (!absoluteBase)
            return 0;
        if (absoluteBase == std::numeric_limits<double>::infinity())
            return std::numeric_limits<double>::infinity();
        return sqrt(x);
    }

    if (y == -0.5) {
        if (!absoluteBase)
            return std::numeric_limits<double>::infinity();
        if (absoluteBase == std::numeric_limits<double>::infinity())
            return 0.;
        return 1. / sqrt(x);
    }

    int32_t yAsInt = y;
    if (static_cast<double>(yAsInt) == y && yAsInt >= 0 && yAsInt <= maxExponentForIntegerMathPow) {
        // If the exponent is a small positive int32 integer, we do a fast exponentiation

        // Do not use x87 values for accumulation. x87 values has 80bit precision.
        // The result produced by x87's 80bit double precision differs from the one calculated with SSE2 in DFG.
        // Using volatile double is workaround for this problem. By specifying volatile, we expect that `result` and `xd`
        // are stored in the stack. And at that time, we expect that they are rounded by fst/fstp[1, 2].
        // [1]: https://gcc.gnu.org/wiki/x87note
        // [2]: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=323
#if !CPU(X86) || (defined(__SSE2_MATH__) && defined(__SSE2__))
        typedef double DoubleValue;
#else
        typedef volatile double DoubleValue;
#endif
        DoubleValue result = 1;
        DoubleValue xd = x;
        while (yAsInt) {
            if (yAsInt & 1)
                result *= xd;
            xd *= xd;
            yAsInt >>= 1;
        }
        return result;
    }
    return mathPowInternal(x, y);
}

int32_t JIT_OPERATION operationToInt32(double value)
{
    return JSC::toInt32(value);
}

int32_t JIT_OPERATION operationToInt32SensibleSlow(double number)
{
    // This function is specialized `operationToInt32` for the slow case of
    // the sensible double-to-int32 operation. It is available in x86.
    //
    // In the sensible double-to-int32, first we attempt to truncate the
    // double value to int32 by using cvttsd2si_rr.
    // According to the Intel's manual, cvttsd2si perform the following truncate
    // operation.
    //
    // If src = NaN, +-Inf, or |(src)rz| > 0x7fffffff and (src)rz != 0x80000000,
    // the result becomes 0x80000000. Otherwise, the operation succeeds.
    // Note that ()rz is rouding towards zero.
    //
    // We call this slow case function when the above cvttsd2si fails. We check
    // this condition by performing `result == 0x80000000`. So this function only
    // accepts the following numbers.
    //
    //     NaN, +-Inf, |(src)rz| > 0x7fffffff.
    //
    // As a result, the exp of the double is always >= 31.
    // This condition simplifies and speeds up the toInt32 implementation.
    int64_t bits = WTF::bitwise_cast<int64_t>(number);
    int32_t exp = (static_cast<int32_t>(bits >> 52) & 0x7ff) - 0x3ff;

    // If exponent < 0 there will be no bits to the left of the decimal point
    // after rounding; if the exponent is > 83 then no bits of precision can be
    // left in the low 32-bit range of the result (IEEE-754 doubles have 52 bits
    // of fractional precision).
    // Note this case handles 0, -0, and all infinite, NaN, & denormal value.

    // If exp < 0, truncate operation succeeds. So this function does not
    // encounter that case. If exp > 83, it means exp >= 84. In that case,
    // the following operation produces 0 for the result.
    ASSERT(exp >= 0);

    // Select the appropriate 32-bits from the floating point mantissa. If the
    // exponent is 52 then the bits we need to select are already aligned to the
    // lowest bits of the 64-bit integer representation of the number, no need
    // to shift. If the exponent is greater than 52 we need to shift the value
    // left by (exp - 52), if the value is less than 52 we need to shift right
    // accordingly.
    int32_t result = (exp > 52)
        ? static_cast<int32_t>(bits << (exp - 52))
        : static_cast<int32_t>(bits >> (52 - exp));

    // IEEE-754 double precision values are stored omitting an implicit 1 before
    // the decimal point; we need to reinsert this now. We may also the shifted
    // invalid bits into the result that are not a part of the mantissa (the sign
    // and exponent bits from the floatingpoint representation); mask these out.
    //
    // The important observation is that exp is always >= 31. So the above case
    // is needed to be cared only when the exp == 31.
    ASSERT(exp >= 31);
    if (exp == 31) {
        int32_t missingOne = 1 << exp;
        result &= (missingOne - 1);
        result += missingOne;
    }

    // If the input value was negative (we could test either 'number' or 'bits',
    // but testing 'bits' is likely faster) invert the result appropriately.
    return bits < 0 ? -result : result;
}

#if HAVE(ARM_IDIV_INSTRUCTIONS)
static inline bool isStrictInt32(double value)
{
    int32_t valueAsInt32 = static_cast<int32_t>(value);
    if (value != valueAsInt32)
        return false;

    if (!valueAsInt32) {
        if (std::signbit(value))
            return false;
    }
    return true;
}
#endif

extern "C" {
double jsRound(double value)
{
    double integer = ceil(value);
    return integer - (integer - value > 0.5);
}

#if CALLING_CONVENTION_IS_STDCALL || CPU(ARM_THUMB2)
double jsMod(double x, double y)
{
#if HAVE(ARM_IDIV_INSTRUCTIONS)
    // fmod() does not have exact results for integer on ARMv7.
    // When DFG/FTL use IDIV, the result of op_mod can change if we use fmod().
    //
    // We implement here the same algorithm and conditions as the upper tier to keep
    // a stable result when tiering up.
    if (y) {
        if (isStrictInt32(x) && isStrictInt32(y)) {
            int32_t xAsInt32 = static_cast<int32_t>(x);
            int32_t yAsInt32 = static_cast<int32_t>(y);
            int32_t quotient = xAsInt32 / yAsInt32;
            if (!productOverflows<int32_t>(quotient, yAsInt32)) {
                int32_t remainder = xAsInt32 - (quotient * yAsInt32);
                if (remainder || xAsInt32 >= 0)
                    return remainder;
            }
        }
    }
#endif
    return fmod(x, y);
}
#endif
} // extern "C"

} // namespace JSC
