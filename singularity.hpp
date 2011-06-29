
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
//
//  Usage as a Factory:
//
//  typedef singularity<Horizon, single_threaded, local_access> HorizonSingularityType;
//  Horizon & horizonF = HorizonSingularityType::create(value, &event, event);
//                       HorizonSingularityType::destroy();
//
//  Usage as a Base Class:
//
//  class Horizon : public singularity<Horizon, multi_threaded, global_access>
//  Horizon & horizonB = Horizon::create(value, &event, event);
//  Horizon & horizonC = Horizon::get();
//                       Horizon::destroy();
//----------------------------------------------------------------------------

#ifndef SINGULARITY_HPP
#define SINGULARITY_HPP

// Certain developers cannot use exceptions, therefore this class
// can be defined to use assertions instead.
#ifndef BOOST_NO_EXCEPTIONS
#include <exception>
#else
#include <boost/assert.hpp>
#endif
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/arithmetic/div.hpp>
#include <boost/preprocessor/arithmetic/mod.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/type_traits/is_same.hpp>
#include <detail/pow2.hpp>
#include <boost/scoped_ptr.hpp>

#include <singularity_policies.hpp>

// The user can choose a different arbitrary upper limit to the
// maximum number of constructor arguments.
#ifndef BOOST_SINGULARITY_CONSTRUCTOR_ARG_SIZE
#define BOOST_SINGULARITY_CONSTRUCTOR_ARG_SIZE 5
#endif

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
#endif

namespace detail {

// This pointer only depends on type T, so regardless of the threading
// model, or access policy, only one singularity of type T can be created.
template <class T> struct singularity_instance
{
    static ::boost::scoped_ptr<T> ptr;
};

template <class T> ::boost::scoped_ptr<T> singularity_instance<T>::ptr(0);

} // detail namespace

// And now, presenting the singularity class itself.
template
<
    class T,
    template <class T> class M = single_threaded,
    class G = no_global_access
>
class singularity
{
public:
// Generate the 2^(n+1)-1 create(...) function overloads
// where n is the maximum number of arguments.  With variadic
// templates, it should be possible to create only n+1 overloads.
//
// na = Number of arguments
// ai = Argument Index
// fi = Function Index
#define SINGULARITY_CREATE_ARGUMENTS(z, ai, fi) BOOST_PP_COMMA_IF(ai) A##ai BOOST_PP_IF(BOOST_PP_MOD(BOOST_PP_IF(ai,BOOST_PP_DIV(fi,BOOST_PP_POW2(ai)),fi),2),*,&) arg##ai

#define SINGULARITY_CREATE_BODY(z, fi, na) \
    BOOST_PP_IF(na,template <,) BOOST_PP_ENUM_PARAMS(na, class A) BOOST_PP_IF(na,>,) \
    static inline T& create( BOOST_PP_REPEAT(na, SINGULARITY_CREATE_ARGUMENTS, fi) ) \
    { \
        M<T> guard; \
        (void)guard; \
        \
        detect_already_created(); \
        \
        detail::singularity_instance<T>::ptr.reset(new T(BOOST_PP_ENUM_PARAMS(na, arg))); \
        return *detail::singularity_instance<T>::ptr; \
    }

#define SINGULARITY_CREATE_OVERLOADS(z, na, text) BOOST_PP_REPEAT(BOOST_PP_POW2(na), SINGULARITY_CREATE_BODY, na)

BOOST_PP_REPEAT(BOOST_PP_INC(BOOST_SINGULARITY_CONSTRUCTOR_ARG_SIZE), SINGULARITY_CREATE_OVERLOADS, _)

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
        BOOST_MPL_ASSERT(( is_same<G, global_access> ));

        M<T> guard;
        (void)guard;

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
    static inline void detect_already_created()
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
    template <class T, template <class T> class M, class G> friend class singularity

} // boost namespace

#endif // SINGULARITY_HPP
