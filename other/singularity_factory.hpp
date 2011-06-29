
//               Copyright Ben Robinson 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

//----------------------------------------------------------------------------
//! \file
//! \brief singularity design pattern enforces a single instance of a class.
//!
//! Unlike Singleton, singularity does not force global access, nor does it
//! require that the class have a default constructor.  The lifetime of the
//! object is simply defined between create() and destroy().
//! An object created with singularity must be passed into objects which depend
//! on them, just like any other object.  Unless using the "global_access"
//! policy, in which case global access to the object is provided.
//----------------------------------------------------------------------------
//  Event event;
//
//  Usage as a Factory:
//
//  Horizon & horizonA = singularity_factory<Horizon>::create();
//                       singularity_factory<Horizon>::destroy();
//  Horizon & horizonB = singularity_factory<Horizon, single_threaded, no_global_access, int>::create(3);
//                       singularity_factory<Horizon>::destroy();
//  Horizon & horizonC = singularity_factory<Horizon, single_threaded, no_global_access, Event*>::create(&event);
//                       singularity_factory<Horizon>::destroy();
//  Horizon & horizonD = singularity_factory<Horizon, single_threaded, global_access, Event&>::create(event);
//                       singularity_factory<Horizon>::destroy();
//  Horizon & horizonE = singularity_factory<Horizon, multi_threaded, global_access, int, Event*, Event&>::create(3, &event, event);
//  Horizon & horizonE2 = singularity_factory<Horizon>::get();
//                       singularity_factory<Horizon>::destroy();
//----------------------------------------------------------------------------

#ifndef SINGULARITY_FACTORY_HPP_
#define SINGULARITY_FACTORY_HPP_

// Certain developers cannot use exceptions, therefore this class
// can be defined to use assertions instead.
#ifndef BOOST_NO_EXCEPTIONS
#include <exception>
#else
#include <boost/assert.hpp>
#endif
#include <boost/noncopyable.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>
#include <boost/scoped_ptr.hpp>

#include <../singularity_policies.hpp>

// The user can choose a different arbitrary upper limit to the
// number of constructor arguments.  The Boost Preprocessor library
// is used to generate the appropriate code.
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

class optional {};

// This pointer only depends on type T, so regardless of the threading
// model, or access policy, only one singularity of type T can be created.
template <class T> struct singularity_instance
{
    static ::boost::scoped_ptr<T> ptr;
};

template <class T> ::boost::scoped_ptr<T> singularity_instance<T>::ptr(0);

} // detail namespace

// BOOST_STATIC_ASSERT is not usable inside BOOST_PP_REPEAT because
// the BOOST_STATIC_ASSERT counter does not increment with each
// invocation.
template <bool condition> struct static_assert;
template <> struct static_assert<true> {};

// Generates: class A0 = optional, class A1 = optional, class A2 = optional
#define BOOST_PP_DEF_CLASS_TYPE_DEFAULT(z, n, text) BOOST_PP_COMMA_IF(n) class A##n = ::boost::detail::optional*

template
<
    class T,
    template <class T> class M = single_threaded,
    class G = no_global_access,
    BOOST_PP_REPEAT(BOOST_SINGULARITY_CONSTRUCTOR_ARG_SIZE, BOOST_PP_DEF_CLASS_TYPE_DEFAULT, _)
>
class singularity_factory
{
    #define SINGULARITY_ASSERT(z, nn, text) static_assert<is_pointer< A##nn >::value || is_reference< A##nn >::value> TYPE_A##nn##_IS_NEITHER_A_POINTER_OR_REFERENCE;
    BOOST_PP_REPEAT(BOOST_SINGULARITY_CONSTRUCTOR_ARG_SIZE, SINGULARITY_ASSERT, _)
    #undef SINGULARITY_ASSERT

public:
    // Generates: Family of create(...) functions
    #define BOOST_PP_DEF_TYPE_ARG(z, n, text) BOOST_PP_COMMA_IF(n) A##n arg##n
    #define BOOST_PP_DEF_ARG(z, n, text) BOOST_PP_COMMA_IF(n) arg##n
    #define BOOST_PP_DEF_CREATE_FUNCTION(z, n, text) \
        static inline T& create(BOOST_PP_REPEAT(n, BOOST_PP_DEF_TYPE_ARG, _)) \
        { \
            M<T> guard; \
            (void)guard; \
            \
            detect_already_created(); \
            \
            detail::singularity_instance<T>::ptr.reset(new T(BOOST_PP_REPEAT(n, BOOST_PP_DEF_ARG, _))); \
            return *detail::singularity_instance<T>::ptr; \
        }

    BOOST_PP_REPEAT(BOOST_PP_INC(BOOST_SINGULARITY_CONSTRUCTOR_ARG_SIZE), BOOST_PP_DEF_CREATE_FUNCTION, _)
    #undef BOOST_PP_DEF_TYPE_ARG
    #undef BOOST_PP_DEF_ARG
    #undef BOOST_PP_DEF_CREATE_FUNCTION

    static inline void destroy() {
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

#undef BOOST_PP_DEF_CLASS_TYPE_DEFAULT

// Convenience macro which generates the required friend statement
// for use inside classes which are created by singularity.
#define FRIEND_CLASS_SINGULARITY_FACTORY \
    template < class T, template <class T> class M, class G, \
        BOOST_PP_ENUM_PARAMS(BOOST_SINGULARITY_CONSTRUCTOR_ARG_SIZE, class A) \
    > friend class singularity_factory

} // boost namespace

#endif // SINGULARITY_FACTORY_HPP_
