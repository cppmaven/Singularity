
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
//! object is simply defined between ::create() and ::destroy().
//! An object created with singularity must be passed into objects which depend
//! on them, just like any other object.  Unless created with
//! ::create_enable_get(), in which case the object is accessible with ::get().
//----------------------------------------------------------------------------
//  Event event;
//
//  Usage as a Factory:
//
//  typedef singularity<Horizon, single_threaded> HorizonSingularityType;
//  Horizon & horizonF = HorizonSingularityType::create(1, &event, event);
//                       HorizonSingularityType::destroy();
//
//  Usage as a Base Class:
//
//  class Horizon : public singularity<Horizon, multi_threaded>
//  Horizon & horizonB = Horizon::create_enable_get(1, &event, event);
//  Horizon & horizonC = Horizon::get();
//                       Horizon::destroy();
//----------------------------------------------------------------------------

#ifndef SINGULARITY_CPP11_HPP_
#define SINGULARITY_CPP11_HPP_

// Certain developers cannot use exceptions, therefore this class
// can be defined to use assertions instead.
#ifndef BOOST_NO_EXCEPTIONS
#include <exception>
#else
#include <boost/assert.hpp>
#endif

#include <singularity_cpp11_policies.hpp>

namespace boost {

#ifndef BOOST_NO_EXCEPTIONS
struct singularity_already_created : virtual std::exception
{
    virtual char const *what() const throw()
    {
        return "boost::singularity_already_created";
    }
};

struct singularity_already_destroyed : virtual std::exception
{
    virtual char const *what() const throw()
    {
        return "boost::singularity_already_destroyed";
    }
};

struct singularity_not_created : virtual std::exception
{
    virtual char const *what() const throw()
    {
        return "boost::singularity_not_created";
    }
};

struct singularity_no_global_access : virtual std::exception
{
    virtual char const *what() const throw()
    {
        return "boost::singularity_no_global_access";
    }
};
#endif

namespace detail {

// This pointer only depends on type T, so regardless of the threading
// model, or access policy, only one singularity of type T can be created.
template <class T> struct singularity_instance
{
    static bool get_enabled;
    static std::unique_ptr<T> ptr;
};

template <class T> bool singularity_instance<T>::get_enabled = false;
template <class T> std::unique_ptr<T> singularity_instance<T>::ptr(0);

} // detail namespace

template <class T, template <class T> class M = single_threaded>
class singularity
{
public:
    template <class ...A>
    static inline T& create(A && ...args)
    {
        M<T> guard;
        (void)guard;

        verify_not_created();

        detail::singularity_instance<T>::get_enabled = false;
        detail::singularity_instance<T>::ptr.reset(new T(std::forward<A>(args)...));
        return *detail::singularity_instance<T>::ptr;
    }

    template <class ...A>
    static inline T& create_enable_get(A && ...args)
    {
        M<T> guard;
        (void)guard;

        verify_not_created();

        detail::singularity_instance<T>::get_enabled = true;
        detail::singularity_instance<T>::ptr.reset(new T(std::forward<A>(args)...));
        return *detail::singularity_instance<T>::ptr;
    }

    static inline void destroy()
    {
        M<T> guard;
        (void)guard;

        #ifndef BOOST_NO_EXCEPTIONS
        if (detail::singularity_instance<T>::ptr.get() == 0)
        {
            throw singularity_already_destroyed();
        }
        #else
        BOOST_ASSERT((detail::singularity_instance<T>::ptr.get() != 0));
        #endif

        delete detail::singularity_instance<T>::ptr.get();
        detail::singularity_instance<T>::ptr.reset();
    }

    static inline T& get()
    {
        M<T> guard;
        (void)guard;

        #ifndef BOOST_NO_EXCEPTIONS
        if (detail::singularity_instance<T>::get_enabled == false) {
            throw singularity_no_global_access();
        }
        #else
        BOOST_ASSERT(detail::singularity_instance<T>::get_enabled != false);
        #endif

        #ifndef BOOST_NO_EXCEPTIONS
        if (detail::singularity_instance<T>::ptr.get() == 0)
        {
            throw singularity_not_created();
        }
        #else
        BOOST_ASSERT(detail::singularity_instance<T>::ptr.get() != 0);
        #endif

        return *detail::singularity_instance<T>::ptr;
    }
private:
    static inline void verify_not_created()
    {
        #ifndef BOOST_NO_EXCEPTIONS
        if (detail::singularity_instance<T>::ptr.get() != 0)
        {
            throw singularity_already_created();
        }
        #else
        BOOST_ASSERT(detail::singularity_instance<T>::ptr.get() == 0);
        #endif
    }
};

// Convenience macro which generates the required friend statement
// for use inside classes which are created by singularity.
#define FRIEND_CLASS_SINGULARITY \
    template <class T, template <class T> class M> friend class singularity

} // boost namespace

#endif // SINGULARITY_CPP11_HPP_
