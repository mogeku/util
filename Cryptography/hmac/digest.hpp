#pragma once

#include <string>
#include <cstdint>

namespace digest {

class base {
protected:
    enum { INIT, ADD, FINISH } mstate;
    std::string mbuf;
    std::size_t mlen;
public:
    base () : mstate (INIT), mbuf (), mlen (0) {}
    virtual ~base () {}
    virtual base& reset ();
    virtual base& add (std::string::const_iterator s, std::string::const_iterator e);
    virtual base& add (std::string const& data);
    virtual base& finish ();
    virtual std::string digest () = 0;
    virtual std::string hexdigest ();
    virtual std::size_t blocksize () const = 0;
protected:
    virtual void init_sum () = 0;
    virtual void update_sum (std::string::const_iterator s) = 0;
    virtual void last_sum () = 0;
};

class SHA2_32BIT : public base {
protected:
    std::uint32_t sum[8];
public:
    SHA2_32BIT () : base (), sum () {}
    std::size_t blocksize () const { return 64U; }
protected:
    void update_sum (std::string::const_iterator s);
    void last_sum ();
};

//class SHA2_64BIT : public base {
//protected:
//    std::uint64_t sum[8];
//public:
//    SHA2_64BIT () : base (), sum () {}
//    std::size_t blocksize () const { return 128U; }
//protected:
//    void update_sum (std::string::const_iterator s);
//    void last_sum ();
//};

class SHA1 : public SHA2_32BIT {
protected:
    std::uint32_t sum[5];
public:
    SHA1 () : SHA2_32BIT () {}
    std::string digest ();
protected:
    void init_sum ();
    void update_sum (std::string::const_iterator s);
};

class SHA224 : public SHA2_32BIT {
public:
    SHA224 () : SHA2_32BIT () {}
    std::string digest ();
protected:
    void init_sum ();
};

class SHA256 : public SHA2_32BIT {
public:
    SHA256 () : SHA2_32BIT () {}
    std::string digest ();
protected:
    void init_sum ();
};

//class SHA384 : public SHA2_64BIT {
//public:
//    SHA384 () : SHA2_64BIT () {}
//    std::string digest ();
//protected:
//    void init_sum ();
//};
//
//class SHA512 : public SHA2_64BIT {
//public:
//    SHA512 () : SHA2_64BIT () {}
//    std::string digest ();
//protected:
//    void init_sum ();
//};
//
//class SHA512_224 : public SHA512 {
//public:
//    SHA512_224 () : SHA512 () {}
//    std::string digest ();
//};
//
//class SHA512_256 : public SHA512 {
//public:
//    SHA512_256 () : SHA512 () {}
//    std::string digest ();
//};

template<class HASH>
class HMAC : public base {
    HASH ihash;
    HASH ohash;
    std::string mkey;
public:
    HMAC (std::string const& key) : base (), ihash (), ohash (), mkey (key) {}
    std::string digest () { finish (); return ohash.digest (); }
    std::string hexdigest () { finish (); return ohash.hexdigest (); }
    std::size_t blocksize () const { return ihash.blocksize (); }

    base&
    reset ()
    {
        std::size_t const blksize = blocksize ();
        if (mkey.size () > blksize) {
            HASH khash;
            mkey = khash.add(mkey).digest ();
        }
        if (mkey.size () < blksize)
            mkey.resize (blksize, 0);
        std::string kipad (mkey);
        for (std::size_t i = 0; i < blksize; ++i)
            kipad[i] ^= 0x36;
        ihash.reset ().add (kipad);
        mstate = ADD;
        return *this;
    }

    base&
    add (std::string::const_iterator s, std::string::const_iterator e)
    {
        if (ADD != mstate)
            reset ();
        ihash.add (s, e);
        return *this;
    }

    base&
    add (std::string const& data)
    {
        return add (data.cbegin (), data.cend ());
    }

    base&
    finish (void)
    {
        if (FINISH == mstate)
            return *this;
        if (ADD != mstate)
            reset ();
        mstate = FINISH;
        std::string kopad (mkey);
        std::size_t const blksize = blocksize ();
        for (std::size_t i = 0; i < blksize; ++i)
            kopad[i] ^= 0x5c;
        ohash.reset ().add (kopad).add (ihash.digest ());
        return *this;
    }

protected:
    void init_sum () {}
    void update_sum (std::string::const_iterator s) {}
    void last_sum () {}
};

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
