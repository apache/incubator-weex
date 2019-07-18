//
// Copyright (c) 2013-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

/*
   Portions from http://www.adp-gmbh.ch/cpp/common/base64.html
   Copyright notice:

   base64.cpp and base64.h

   Copyright (C) 2004-2008 Ren� Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   Ren� Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/

/**
 * boost source
 * url : https://raw.githubusercontent.com/boostorg/beast/3777cb8b9c78748d0e64401316f00382d37b83be/include/beast/core/detail/base64.hpp
 */


#ifndef BEAST_DETAIL_BASE64_HPP
#define BEAST_DETAIL_BASE64_HPP

#include <cctype>
#include <string>
#include <utility>

namespace beast {
namespace detail {

namespace base64 {

inline
char const*
get_alphabet()
{
    static char constexpr tab[] = {
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
    };
    return &tab[0];
}

inline
signed char const*
get_inverse()
{
    static signed char constexpr tab[] = {
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, //   0-15
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, //  16-31
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, //  32-47
         52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, //  48-63
         -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, //  64-79
         15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, //  80-95
         -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, //  96-111
         41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, // 112-127
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 128-143
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 144-159
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 160-175
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 176-191
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 192-207
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 208-223
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 224-239
         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  // 240-255
    };
    return &tab[0];
}


/// Returns max chars needed to encode a base64 string
inline
std::size_t constexpr
encoded_size(std::size_t n)
{
    return 4 * ((n + 2) / 3);
}

/// Returns max bytes needed to decode a base64 string
inline
std::size_t constexpr
decoded_size(std::size_t n)
{
    return n / 4 * 3; // requires n&3==0, smaller
    //return 3 * n / 4;
}

/** Encode a series of octets as a padded, base64 string.

    The resulting string will not be null terminated.

    @par Requires

    The memory pointed to by `out` points to valid memory
    of at least `encoded_size(len)` bytes.

    @return The number of characters written to `out`. This
    will exclude any null termination.
*/
template<class = void>
std::size_t
encode(void* dest, void const* src, std::size_t len)
{
    char*      out = static_cast<char*>(dest);
    char const* in = static_cast<char const*>(src);
    auto const tab = base64::get_alphabet();

    for(auto n = len / 3; n--;)
    {
        *out++ = tab[ (in[0] & 0xfc) >> 2];
        *out++ = tab[((in[0] & 0x03) << 4) + ((in[1] & 0xf0) >> 4)];
        *out++ = tab[((in[2] & 0xc0) >> 6) + ((in[1] & 0x0f) << 2)];
        *out++ = tab[  in[2] & 0x3f];
        in += 3;
    }

    switch(len % 3)
    {
    case 2:
        *out++ = tab[ (in[0] & 0xfc) >> 2];
        *out++ = tab[((in[0] & 0x03) << 4) + ((in[1] & 0xf0) >> 4)];
        *out++ = tab[                         (in[1] & 0x0f) << 2];
        *out++ = '=';
        break;

    case 1:
        *out++ = tab[ (in[0] & 0xfc) >> 2];
        *out++ = tab[((in[0] & 0x03) << 4)];
        *out++ = '=';
        *out++ = '=';
        break;

    case 0:
        break;
    }

    return out - static_cast<char*>(dest);
}

/** Decode a padded base64 string into a series of octets.

    @par Requires

    The memory pointed to by `out` points to valid memory
    of at least `decoded_size(len)` bytes.

    @return The number of octets written to `out`, and
    the number of characters read from the input string,
    expressed as a pair.
*/
template<class = void>
std::pair<std::size_t, std::size_t>
decode(void* dest, char const* src, std::size_t len)
{
    char* out = static_cast<char*>(dest);
    auto in = reinterpret_cast<unsigned char const*>(src);
    unsigned char c3[3], c4[4];
    int i = 0;
    int j = 0;

    auto const inverse = base64::get_inverse();

    while(len-- && *in != '=')
    {
        auto const v = inverse[*in];
        if(v == -1)
            break;
        ++in;
        c4[i] = v;
        if(++i == 4)
        {
            c3[0] =  (c4[0]        << 2) + ((c4[1] & 0x30) >> 4);
            c3[1] = ((c4[1] & 0xf) << 4) + ((c4[2] & 0x3c) >> 2);
            c3[2] = ((c4[2] & 0x3) << 6) +   c4[3];

            for(i = 0; i < 3; i++)
                *out++ = c3[i];
            i = 0;
        }
    }

    if(i)
    {
        c3[0] = ( c4[0]        << 2) + ((c4[1] & 0x30) >> 4);
        c3[1] = ((c4[1] & 0xf) << 4) + ((c4[2] & 0x3c) >> 2);
        c3[2] = ((c4[2] & 0x3) << 6) +   c4[3];

        for(j = 0; j < i - 1; j++)
            *out++ = c3[j];
    }

    return {out - static_cast<char*>(dest),
        in - reinterpret_cast<unsigned char const*>(src)};
}

} // base64

template<class = void>
std::string
base64_encode (std::uint8_t const* data,
    std::size_t len)
{
    std::string dest;
    dest.resize(base64::encoded_size(len));
    dest.resize(base64::encode(&dest[0], data, len));
    return dest;
}

inline
std::string
base64_encode(std::string const& s)
{
    return base64_encode (reinterpret_cast <
        std::uint8_t const*> (s.data()), s.size());
}

template<class = void>
std::string
base64_decode(std::string const& data)
{
    std::string dest;
    dest.resize(base64::decoded_size(data.size()));
    auto const result = base64::decode(
        &dest[0], data.data(), data.size());
    dest.resize(result.first);
    return dest;
}

} // detail
} // beast

#endif