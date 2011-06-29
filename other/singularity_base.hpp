
//               Copyright Ben Robinson 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

//----------------------------------------------------------------------------
//! \file
//! \brief Singularity design pattern enforces a single instance of a class.
//!
//! Unlike Singleton, singularity does not force global access, nor does it
//! require that the class have a default constructor.  The lifetime of the
//! object is simply defined between create() and destroy().
//! An object created with singularity must be passed into objects which depend
//! on them, just like any other object.  Unless using the "global_access"
//! policy, in which case global access to the object is provided.
//----------------------------------------------------------------------------
//  Event event;
//  int value = 1;

//  Usage as a Base Class:
//
//  class Horizon : public singularity<Horizon, multi_threaded>
//  Horizon * horizonB = new Horizon(value, &event, event);
//                       horizonB->delete();
//----------------------------------------------------------------------------

#ifndef SINGULARITY_BASE_HPP
#define SINGULARITY_BASE_HPP

// Certain developers cannot use exceptions, therefore this class
// can be defined to use assertions instead.
#ifndef BOOST_NO_EXCEPTIONS
#include <exception>
#else
#include <boost/assert.hpp>
#endif

#include <../singularity_policies.hpp>

namespace boost {

#ifndef BOOST_NO_EXCEPTIONS
struct singularity_already_created : virtual std::exception
{
    virtual char const *what() const throw()
    {
        return "boost::singularity_already_created";
    }
};
#endif

// And now, presenting the singularity class itself.
template
<
    class T,
    template <class T> class M = single_threaded
>
class singularity_base
{
public:
    singularity_base()
    {
        M<T> guard;
        (void)guard;

        #ifndef BOOST_NO_EXCEPTIONS
        if (created != false)
        {
            throw singularity_already_created();
        }
        #else
        BOOST_ASSERT(created == false);
        #endif

        created = true;
    }

    ~singularity_base()
    {
        M<T> guard;
        (void)guard;

        created = false;
    }

private:
    static bool created;
};

template <class T, template <class T> class M>
bool singularity_base<T, M>::created = false;

} // boost namespace

#endif // SINGULARITY_BASE_HPP
