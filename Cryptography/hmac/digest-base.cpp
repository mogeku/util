#include <string>
#include <cstdint>
#include <algorithm>
#include "digest.hpp"

namespace digest {

base&
base::reset ()
{
    mstate = ADD;
    mbuf.clear ();
    mlen = 0;
    init_sum ();
    return *this;
}

base&
base::add (std::string::const_iterator s, std::string::const_iterator e)
{
    if (ADD != mstate)
        reset ();
    if (s >= e)
        return *this;
    std::size_t const blksize = blocksize ();
    if (mbuf.size () > 0 && mbuf.size () < blksize) {
        std::size_t const datasize = e - s;
        std::size_t const n = std::min (datasize, blksize - mbuf.size ());
        mbuf.append (s, s + n);
        s += n;
        mlen += n;
    }
    if (mbuf.size () == blksize) {
        std::string::const_iterator t = mbuf.cbegin ();
        update_sum (t);
        mbuf.clear ();
    }
    if (s == e)
        return *this;
    for (; e-s > blksize ; s += blksize) {
        std::string::const_iterator t = s;
        update_sum (t);
        mbuf.clear ();
        mlen += blksize;
    }
    mbuf.assign (s, e);
    mlen += mbuf.size ();
    return *this;
}

base&
base::add (std::string const& data)
{
    return add (data.cbegin (), data.cend ());
}

base&
base::finish ()
{
    if (INIT == mstate)
        reset ();
    if (FINISH == mstate)
        return *this;
    mstate = FINISH;
    last_sum ();
    return *this;
}

std::string
base::hexdigest ()
{
    std::string const octets = digest ();
    std::string hex;
    for (int x: octets) {
        unsigned int const hi = (x >> 4) & 0x0f;
        unsigned int const lo = x & 0x0f;
        hex.push_back (hi < 10 ? hi + '0' : hi + 'a' - 10);
        hex.push_back (lo < 10 ? lo + '0' : lo + 'a' - 10);
    }
    return hex;
}

}//namespace digest

/* Copyright (c) 2015, MIZUTANI Tociyuki  
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
