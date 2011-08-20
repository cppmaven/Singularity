
//               Copyright Ben Robinson 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef SINGULARITY_CPP11_POLICIES_HPP
#define SINGULARITY_CPP11_POLICIES_HPP

#include <boost/thread/mutex.hpp>

using boost::mutex;

namespace boost {

// The threading model for Singularity is policy based.  The
// single_threaded policy provides maximum performance, and
// the multi-threaded policy provides thread safety.

// The single_threaded policy is a POD struct with
// no mutex to maximize compiler optimizations.
template <class T> class single_threaded {};

// Users of singularity are encouraged to develop their own
// thread-safe policies which meet their unique requirements
// and constraints.
template <class T> class multi_threaded
{
public:
    inline multi_threaded()
    {
        lockable.lock();
    }
    inline ~multi_threaded()
    {
        lockable.unlock();
    }
private:
    // The mutex acquisition and release must provide
    // fencing in order to be thread-safe.
    static mutex lockable;
};

template <class T> mutex multi_threaded<T>::lockable;

} // boost namespace

#endif // SINGULARITY_CPP11_POLICIES_HPP
