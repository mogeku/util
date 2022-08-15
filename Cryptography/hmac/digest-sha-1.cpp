#include <string>
#include <cstdint>
#include "digest.hpp"

// SHA-1 implementation

namespace digest {

static inline void
unpack_big_endian (std::string& t, std::size_t const i, std::uint32_t const x)
{
    t[i + 0] = (x >> 24) & 0xff;    
    t[i + 1] = (x >> 16) & 0xff;
    t[i + 2] = (x >>  8) & 0xff;
    t[i + 3] = x & 0xff;
}

static inline std::uint32_t
rotate_left (std::uint32_t const x, std::size_t const n)
{
    return (x << n) | (x >> (32 - n));
}

void
SHA1::init_sum ()
{
    sum[0] = 0x67452301; sum[1] = 0xefcdab89; sum[2] = 0x98badcfe;
    sum[3] = 0x10325476; sum[4] = 0xc3d2e1f0;
}

static inline void
round (
    std::uint32_t const a, std::uint32_t& b,
    std::uint32_t& e, std::uint32_t const f,
    std::uint32_t const k, std::uint32_t const w)
{
    b = rotate_left (b, 30);
    e += rotate_left (a, 5) + f + k + w;
}

static inline void
round0 (
    std::uint32_t const a, std::uint32_t& b, std::uint32_t const c,
    std::uint32_t const d, std::uint32_t& e,
    std::uint32_t const k, std::uint32_t const w)
{
    round (a, b, e, (b & c) | (~b & d), k, w);
}

static inline void
round1 (
    std::uint32_t const a, std::uint32_t& b, std::uint32_t const c,
    std::uint32_t const d, std::uint32_t& e,
    std::uint32_t const k, std::uint32_t const w)
{
    round (a, b, e, b ^ c ^ d, k, w);
}

static inline void
round2 (
    std::uint32_t const a, std::uint32_t& b, std::uint32_t const c,
    std::uint32_t const d, std::uint32_t& e,
    std::uint32_t const k, std::uint32_t const w)
{
    round (a, b, e, (b & c) | (b & d) | (c & d), k, w);
}

void
SHA1::update_sum (std::string::const_iterator s)
{
    std::uint32_t w[80];
    std::uint32_t a = sum[0], b = sum[1], c = sum[2], d = sum[3], e = sum[4];
    for (std::size_t i = 0; i < 16U; i++)
        w[i] = (static_cast<std::uint8_t>(*s++) << 24)
             | (static_cast<std::uint8_t>(*s++) << 16)
             | (static_cast<std::uint8_t>(*s++) <<  8)
             |  static_cast<std::uint8_t>(*s++);
    for (std::size_t i = 16U; i < 80U; i++)
        w[i] = rotate_left (w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    for (std::size_t i = 0; i < 20; i += 5) {
        round0 (a, b, c, d, e, 0x5a827999, w[i + 0]);
        round0 (e, a, b, c, d, 0x5a827999, w[i + 1]);
        round0 (d, e, a, b, c, 0x5a827999, w[i + 2]);
        round0 (c, d, e, a, b, 0x5a827999, w[i + 3]);
        round0 (b, c, d, e, a, 0x5a827999, w[i + 4]);
    }
    for (std::size_t i = 20; i < 40; i += 5) {
        round1 (a, b, c, d, e, 0x6ed9eba1, w[i + 0]);
        round1 (e, a, b, c, d, 0x6ed9eba1, w[i + 1]);
        round1 (d, e, a, b, c, 0x6ed9eba1, w[i + 2]);
        round1 (c, d, e, a, b, 0x6ed9eba1, w[i + 3]);
        round1 (b, c, d, e, a, 0x6ed9eba1, w[i + 4]);
    }
    for (std::size_t i = 40; i < 60; i += 5) {
        round2 (a, b, c, d, e, 0x8f1bbcdc, w[i + 0]);
        round2 (e, a, b, c, d, 0x8f1bbcdc, w[i + 1]);
        round2 (d, e, a, b, c, 0x8f1bbcdc, w[i + 2]);
        round2 (c, d, e, a, b, 0x8f1bbcdc, w[i + 3]);
        round2 (b, c, d, e, a, 0x8f1bbcdc, w[i + 4]);
    }
    for (std::size_t i = 60; i < 80; i += 5) {
        round1 (a, b, c, d, e, 0xca62c1d6, w[i + 0]);
        round1 (e, a, b, c, d, 0xca62c1d6, w[i + 1]);
        round1 (d, e, a, b, c, 0xca62c1d6, w[i + 2]);
        round1 (c, d, e, a, b, 0xca62c1d6, w[i + 3]);
        round1 (b, c, d, e, a, 0xca62c1d6, w[i + 4]);
    }
    sum[0] += a; sum[1] += b; sum[2] += c; sum[3] += d; sum[4] += e;
}

std::string
SHA1::digest ()
{
    std::string octets (20, 0);
    finish ();
    std::uint32_t *p = sum;
    for (std::size_t i = 0; i < octets.size (); i += 4)
        unpack_big_endian (octets, i, *p++);
    return octets;
}

}//namespace digest

/* Copyright (c) 2016, MIZUTANI Tociyuki  
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
