#include <string>
#include <cstdint>
#include "digest.hpp"

// SHA-256 and SHA-224 implementation

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
rotate_right (std::uint32_t const x, std::size_t const n)
{
    return (x >> n) | (x << (32 - n));
}

static inline std::uint32_t
ch (std::uint32_t const x, std::uint32_t const y, std::uint32_t const z)
{
    return (x & y) ^ ((~x) & z);
}

static inline std::uint32_t
ma (std::uint32_t const x, std::uint32_t const y, std::uint32_t const z)
{
    return (x & y) ^ (x & z) ^ (y & z);
}

static inline std::uint32_t
gamma0 (std::uint32_t const x)
{
    return rotate_right (x, 7) ^ rotate_right (x, 18) ^ (x >> 3);
}

static inline std::uint32_t
gamma1 (std::uint32_t const x)
{
    return rotate_right (x, 17) ^ rotate_right (x, 19) ^ (x >> 10);
}

static inline std::uint32_t
sigma0 (std::uint32_t const x)
{
    return rotate_right (x, 2) ^ rotate_right (x, 13) ^ rotate_right (x, 22);
}

static inline std::uint32_t
sigma1 (std::uint32_t const x)
{
    return rotate_right (x, 6) ^ rotate_right (x, 11) ^ rotate_right (x, 25);
}

void
SHA256::init_sum ()
{
    sum[0] = 0x6a09e667; sum[1] = 0xbb67ae85; sum[2] = 0x3c6ef372;
    sum[3] = 0xa54ff53a; sum[4] = 0x510e527f; sum[5] = 0x9b05688c;
    sum[6] = 0x1f83d9ab; sum[7] = 0x5be0cd19;
}

void
SHA224::init_sum ()
{
    sum[0] = 0xc1059ed8; sum[1] = 0x367cd507; sum[2] = 0x3070dd17;
    sum[3] = 0xf70e5939; sum[4] = 0xffc00b31; sum[5] = 0x68581511;
    sum[6] = 0x64f98fa7; sum[7] = 0xbefa4fa4;
}

static inline void
round (
    std::uint32_t const a, std::uint32_t const b, std::uint32_t const c,
    std::uint32_t& d,
    std::uint32_t const e, std::uint32_t const f, std::uint32_t const g,
    std::uint32_t& h,
    std::uint32_t const k, std::uint32_t const w)
{
    std::uint32_t const t0 = h + sigma1 (e) + ch (e, f, g) + k + w;
    std::uint32_t const t1 = sigma0 (a) + ma (a, b, c);
    d += t0;
    h = t0 + t1;
}

void
SHA2_32BIT::update_sum (std::string::const_iterator s)
{
    static const std::uint32_t K[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
        0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
        0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
        0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
        0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
        0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };
    std::uint32_t w[64];
    std::uint32_t a = sum[0], b = sum[1], c = sum[2], d = sum[3];
    std::uint32_t e = sum[4], f = sum[5], g = sum[6], h = sum[7];
    for (std::size_t i = 0; i < 16U; i++)
        w[i] = (static_cast<std::uint8_t>(*s++) << 24)
             | (static_cast<std::uint8_t>(*s++) << 16)
             | (static_cast<std::uint8_t>(*s++) <<  8)
             |  static_cast<std::uint8_t>(*s++);
    for (std::size_t i = 16U; i < 64U; i++)
        w[i] = gamma1 (w[i - 2]) + w[i - 7] + gamma0 (w[i - 15]) + w[i - 16];
    for (std::size_t i = 0; i < 64; i += 8) {
        round (a, b, c, d, e, f, g, h, K[i + 0], w[i + 0]);
        round (h, a, b, c, d, e, f, g, K[i + 1], w[i + 1]);
        round (g, h, a, b, c, d, e, f, K[i + 2], w[i + 2]);
        round (f, g, h, a, b, c, d, e, K[i + 3], w[i + 3]);
        round (e, f, g, h, a, b, c, d, K[i + 4], w[i + 4]);
        round (d, e, f, g, h, a, b, c, K[i + 5], w[i + 5]);
        round (c, d, e, f, g, h, a, b, K[i + 6], w[i + 6]);
        round (b, c, d, e, f, g, h, a, K[i + 7], w[i + 7]);
    }
    sum[0] += a; sum[1] += b; sum[2] += c; sum[3] += d;
    sum[4] += e; sum[5] += f; sum[6] += g; sum[7] += h;
}

void
SHA2_32BIT::last_sum ()
{
    mbuf.push_back (0x80);
    std::size_t n = (mbuf.size () + 8U + 64U - 1U) / 64U * 64U;
    mbuf.resize (n, 0);
    unpack_big_endian (mbuf, n - 8, mlen >> 29);
    unpack_big_endian (mbuf, n - 4, mlen <<  3);
    std::string::const_iterator p = mbuf.cbegin ();
    for (std::size_t i = 0; i < n; i += 64U)
        update_sum (p);
}

std::string
SHA256::digest ()
{
    std::string octets (32, 0);
    finish ();
    std::uint32_t *p = sum;
    for (std::size_t i = 0; i < octets.size (); i += 4)
        unpack_big_endian (octets, i, *p++);
    return octets;
}

std::string
SHA224::digest ()
{
    std::string octets (28, 0);
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
