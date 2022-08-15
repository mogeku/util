#pragma once

#include <string>
#include <algorithm>

namespace pkcs5 {

// Password-Based Key Derivation Function 2 (PBKDF2)
// see RFC 2898 PKCS#5 version 2.0
template<class PRF>
std::string
pbkdf2 (std::string const& password, std::string const& salt, std::size_t const rounds, std::size_t keylen)
{
    PRF prf (password);
    std::string key;
    std::uint32_t i = 0;
    while (keylen > 0) {
        ++i;
        std::string block_number;
        block_number.push_back ((i >> 24) & 0xff);
        block_number.push_back ((i >> 16) & 0xff);
        block_number.push_back ((i >>  8) & 0xff);
        block_number.push_back (i & 0xff);
        std::string u = prf.add (salt).add (block_number).digest ();
        std::string t = u;
        for (std::size_t j = 1; j < rounds; ++j) {
            u = prf.add (u).digest ();
            for (std::size_t k = 0; k < u.size (); ++k)
                t[k] ^= u[k];
        }
        std::size_t n = std::min (keylen, t.size ());
        key.append (t.begin (), t.begin () + n);
        keylen -= n;
    }
    return key;
}

}//namespace pkcs5

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
