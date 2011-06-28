
//               Copyright Ben Robinson 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef SINGULARITY_POLICIES_HPP
#define SINGULARITY_POLICIES_HPP

// Certain developers cannot use exceptions, therefore this class
// can be defined to use assertions instead.
#ifndef BOOST_NO_EXCEPTIONS
#include <exception>
#include <boost/thread/mutex.hpp>
#else
#include <boost/assert.hpp>
#endif

namespace boost {

// The threading model for Singularity is policy based, and
// a single threaded, and multi threaded policy are provided,
// when thread safety is required.

// The single_threaded policy uses a non-volatile pointer,
// and a POD struct with no mutex to maximize compiler
// optimizations.
template <class T> class single_threaded
{
public:
    typedef T * ptr_type;
};

// Users of singularity are encouraged to develop their own
// thread-safe policies which meet their unique requirements
// and constraints.  The policy provided here is implemented
// using boost::mutex, and therefore requires exceptions on
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
    typedef T * volatile ptr_type; // CV qualifiers are best read right to left.
private:
    static ::boost::mutex lockable;
};

template <class T> boost::mutex multi_threaded<T>::lockable;
#endif

// This template argument policy disables instantiation of
// the singularity::get() member function.  This policy is
// supplied by default.
struct no_global_access {};

// This template argument policy enables instantiation of
// the singularity::get() member function.
struct global_access {};

} // boost namespace

#endif // SINGULARITY_POLICIES_HPP
