
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
//! ::create_global(), in which case the object is accessible with ::get().
//----------------------------------------------------------------------------
//  Event event;
//
//  Usage as a Factory:
//
//  Horizon & horizonF = singularity<Horizon, single_threaded>::create(1, &event, event);
//                       singularity<Horizon, single_threaded>::destroy();
//
//  Usage as a Base Class:
//
//  class Horizon : public singularity<Horizon, multi_threaded>
//  Horizon & horizonB = Horizon::create_global(1, &event, event);
//  Horizon & horizonC = Horizon::get_global();
//                       Horizon::destroy();
//----------------------------------------------------------------------------

#ifndef SINGULARITY_HPP
#define SINGULARITY_HPP

#include <exception>
#include <boost/scoped_ptr.hpp>
#include <boost/mpl/assert.hpp>

#include <singularity_policies.hpp>

namespace boost {

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

namespace detail {

// This pointer only depends on type T, so regardless of the threading
// model, only one singularity of type T can be created.
template <class T> struct singularity_instance
{
    typedef ::boost::scoped_ptr<T> ptrtype;
    static bool get_enabled;
    static ptrtype ptr;
};

template <class T> bool singularity_instance<T>::get_enabled = false;
template <class T> typename singularity_instance<T>::ptrtype singularity_instance<T>::ptr(0);

} // detail namespace

// And now, presenting the singularity class itself.
template <class T, template <class T> class M = single_threaded>
class singularity
{
public:
    // Forwards to the default constructor
    static inline T& create()
    {
        M<T> guard;
        (void)guard;

        verify_not_created();

        detail::singularity_instance<T>::get_enabled = false;
        detail::singularity_instance<T>::ptr.reset(new T());
        return *detail::singularity_instance<T>::ptr;
    }

    // Forwards to constructors with one argument
    template <class A0>
    static inline T& create(A0 const & a0)
    {
        M<T> guard;
        (void)guard;

        verify_not_created();

        detail::singularity_instance<T>::get_enabled = false;
        detail::singularity_instance<T>::ptr.reset(new T(a0));
        return *detail::singularity_instance<T>::ptr;
    }
    template <class A0>
    static inline T& create(A0 & a0)
    {
        M<T> guard;
        (void)guard;

        verify_not_created();

        detail::singularity_instance<T>::get_enabled = false;
        detail::singularity_instance<T>::ptr.reset(new T(a0));
        return *detail::singularity_instance<T>::ptr;
    }

    // Forwards to constructors with two arguments
    template <class A0, class A1>
    static inline T& create(A0 const & a0, A1 & a1)
    {
        M<T> guard;
        (void)guard;

        verify_not_created();

        detail::singularity_instance<T>::get_enabled = false;
        detail::singularity_instance<T>::ptr.reset(new T(a0, a1));
        return *detail::singularity_instance<T>::ptr;
    }
    template <class A0, class A1>
    static inline T& create(A0 & a0, A1 const & a1)
    {
        M<T> guard;
        (void)guard;

        verify_not_created();

        detail::singularity_instance<T>::get_enabled = false;
        detail::singularity_instance<T>::ptr.reset(new T(a0, a1));
        return *detail::singularity_instance<T>::ptr;
    }
    template <class A0, class A1>
    static inline T& create(A0 const & a0, A1 const & a1)
    {
        M<T> guard;
        (void)guard;

        verify_not_created();

        detail::singularity_instance<T>::get_enabled = false;
        detail::singularity_instance<T>::ptr.reset(new T(a0, a1));
        return *detail::singularity_instance<T>::ptr;
    }
    template <class A0, class A1>
    static inline T& create(A0 & a0, A1 & a1)
    {
        M<T> guard;
        (void)guard;

        verify_not_created();

        detail::singularity_instance<T>::get_enabled = false;
        detail::singularity_instance<T>::ptr.reset(new T(a0, a1));
        return *detail::singularity_instance<T>::ptr;
    }

    //------------------------------------------------------------------
    // After BOOST_SINGULARITY_PERFECT_FORWARD_ARG_SIZE number of arguments, only
    // the non-const l-value reference arguments are generated, similiar to Boost Bind.
    //------------------------------------------------------------------

    // Forwards non-const l-value arguments to constructors with five arguments
    template <class A0, class A1, class A2, class A3, class A4>
    static inline T& create(A0 & a0, A1 & a1, A2 & a2, A3 & a3, A4 & a4)
    {
        M<T> guard;
        (void)guard;

        verify_not_created();

        detail::singularity_instance<T>::get_enabled = false;
        detail::singularity_instance<T>::ptr.reset(new T(a0, a1, a2, a3, a4));
        return *detail::singularity_instance<T>::ptr;
    }

    // Forwards to the default constructor
    static inline T& create_global()
    {
        M<T> guard;
        (void)guard;

        verify_not_created();

        detail::singularity_instance<T>::get_enabled = true;
        detail::singularity_instance<T>::ptr.reset(new T());
        return *detail::singularity_instance<T>::ptr;
    }

    // Forwards to constructors with one argument
    template <class A0>
    static inline T& create_global(A0 const & a0)
    {
        M<T> guard;
        (void)guard;

        verify_not_created();

        detail::singularity_instance<T>::get_enabled = true;
        detail::singularity_instance<T>::ptr.reset(new T(a0));
        return *detail::singularity_instance<T>::ptr;
    }
    template <class A0>
    static inline T& create_global(A0 & a0)
    {
        M<T> guard;
        (void)guard;

        verify_not_created();

        detail::singularity_instance<T>::get_enabled = true;
        detail::singularity_instance<T>::ptr.reset(new T(a0));
        return *detail::singularity_instance<T>::ptr;
    }

    // Forwards to constructors with two arguments
    template <class A0, class A1>
    static inline T& create_global(A0 const & a0, A1 & a1)
    {
        M<T> guard;
        (void)guard;

        verify_not_created();

        detail::singularity_instance<T>::get_enabled = true;
        detail::singularity_instance<T>::ptr.reset(new T(a0, a1));
        return *detail::singularity_instance<T>::ptr;
    }
    template <class A0, class A1>
    static inline T& create_global(A0 & a0, A1 const & a1)
    {
        M<T> guard;
        (void)guard;

        verify_not_created();

        detail::singularity_instance<T>::get_enabled = true;
        detail::singularity_instance<T>::ptr.reset(new T(a0, a1));
        return *detail::singularity_instance<T>::ptr;
    }
    template <class A0, class A1>
    static inline T& create_global(A0 const & a0, A1 const & a1)
    {
        M<T> guard;
        (void)guard;

        verify_not_created();

        detail::singularity_instance<T>::get_enabled = true;
        detail::singularity_instance<T>::ptr.reset(new T(a0, a1));
        return *detail::singularity_instance<T>::ptr;
    }
    template <class A0, class A1>
    static inline T& create_global(A0 & a0, A1 & a1)
    {
        M<T> guard;
        (void)guard;

        verify_not_created();

        detail::singularity_instance<T>::get_enabled = true;
        detail::singularity_instance<T>::ptr.reset(new T(a0, a1));
        return *detail::singularity_instance<T>::ptr;
    }

    //------------------------------------------------------------------
    // After BOOST_SINGULARITY_PERFECT_FORWARD_ARG_SIZE number of arguments, only
    // the non-const l-value reference arguments are generated, similiar to Boost Bind.
    //------------------------------------------------------------------

    // Forwards non-const l-value arguments to constructors with five arguments
    template <class A0, class A1, class A2, class A3, class A4>
    static inline T& create_global(A0 & a0, A1 & a1, A2 & a2, A3 & a3, A4 & a4)
    {
        M<T> guard;
        (void)guard;

        verify_not_created();

        detail::singularity_instance<T>::get_enabled = true;
        detail::singularity_instance<T>::ptr.reset(new T(a0, a1, a2, a3, a4));
        return *detail::singularity_instance<T>::ptr;
    }

    static inline void destroy()
    {
        M<T> guard;
        (void)guard;

        if (detail::singularity_instance<T>::ptr.get() == 0)
        {
            BOOST_THROW_EXCEPTION(singularity_already_destroyed());
        }

        detail::singularity_instance<T>::ptr.reset();
    }

    static inline T& get_global()
    {
        M<T> guard;
        (void)guard;

        if (detail::singularity_instance<T>::get_enabled == false) {
            BOOST_THROW_EXCEPTION(singularity_no_global_access());
        }

        if (detail::singularity_instance<T>::ptr.get() == 0)
        {
            BOOST_THROW_EXCEPTION(singularity_not_created());
        }

        return *detail::singularity_instance<T>::ptr;
    }
private:
    static inline void verify_not_created()
    {
        if (detail::singularity_instance<T>::ptr.get() != 0)
        {
            BOOST_THROW_EXCEPTION(singularity_already_created());
        }
    }
};

// Convenience macro which generates the required friend statement
// for use inside classes which use singularity as a factory.
#define FRIEND_CLASS_SINGULARITY \
    template <class T, template <class T> class M> friend class singularity

} // boost namespace

#endif // SINGULARITY_HPP
