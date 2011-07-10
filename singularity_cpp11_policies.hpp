
//               Copyright Ben Robinson 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef SINGULARITY_CPP11_POLICIES_HPP
#define SINGULARITY_CPP11_POLICIES_HPP

// Certain developers cannot use exceptions, therefore this class
// can be defined to use assertions instead.
#ifndef BOOST_NO_EXCEPTIONS
#include <exception>
#include <boost/thread/mutex.hpp>
#else
#include <boost/assert.hpp>
#endif

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
// and constraints.  The policy provided here is implemented
// using ::boost::mutex, and therefore requires exceptions on
// certain platforms.
#ifndef BOOST_NO_EXCEPTIONS
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
#endif

} // boost namespace

#endif // SINGULARITY_CPP11_POLICIES_HPP