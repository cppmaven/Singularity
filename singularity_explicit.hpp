
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
//  Horizon & horizonA = singularity<Horizon>::create();
//                       singularity<Horizon>::destroy();
//  Horizon & horizonB = singularity<Horizon, int>::create(3);
//                       singularity<Horizon, int>::destroy();
//  Horizon & horizonC = singularity<Horizon, Event*>::create(&event);
//                       singularity<Horizon, Event*>::destroy();
//  Horizon & horizonD = singularity<Horizon, Event&>::create(event);
//                       singularity<Horizon, Event&>::destroy();
//  Horizon & horizonE = singularity<Horizon, int, Event*, Event&>::create(3, &event, event);
//                       singularity<Horizon, int, Event*, Event&>::destroy();
//----------------------------------------------------------------------------

#ifndef SINGULARITY_HPP_
#define SINGULARITY_HPP_

// Certain developers cannot use exceptions, therefore this class
// can be defined to use assertions instead.
#ifndef BOOST_NO_EXCEPTIONS
#include <exception>
#else
#include <boost/assert.hpp>
#endif
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>

// The user can choose a different arbitrary upper limit to the
// number of constructor arguments.  The Boost Preprocessor library
// is used to generate the appropriate code.
#ifndef BOOST_SINGULARITY_LIMIT_ARGS
#define BOOST_SINGULARITY_LIMIT_ARGS 6
#endif

namespace boost {

namespace { class optional {}; }

// The threading model for Singularity is policy based, and
// a single threaded, and multi threaded policy are provided,
// when thread safety is required.
template <class T> class single_threaded
{
public:
    typedef T * ptr_type;
};

// boost::mutex requires exceptions on certain platforms.
// The user of singularity can create their own policy implemented
// with a mutex object supported on their platform if needed.
//
// In addition, if the user desires to use an auto_ptr<T>,
// shared_ptr<T>, unique_ptr<T>, etc... instead of T *,
// a custom policy can be provided to implement that
// functionality.
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
    typedef T * volatile ptr_type;
private:
    static boost::mutex lockable;
};

template <class T> boost::mutex multi_threaded<T>::lockable;
#endif

// This template argument policy disables instantiation of
// the singularity::get() member function.
struct local_access {};

// This template argument policy enables instantiation of
// the singularity::get() member function.
struct global_access {};

#ifndef BOOST_NO_EXCEPTIONS
struct singularity_already_created : virtual std::exception {};
struct singularity_already_destroyed : virtual std::exception {};
struct singularity_not_created : virtual std::exception {};
struct singularity_destroy_on_incorrect_threading : virtual std::exception {};
#endif

// This boolean only depends on type T, so regardless of the threading
// model, or constructor agrument types, only one singularity of type
// T can be created.
template <class T> struct singularity_state
{
    static bool created;
};

template <class T> bool singularity_state<T>::created = false;

// The instance pointer must depend on type T and the threading model,
// in order to control whether or not the pointer is volatile.
template <class T, template <class T> class M> struct singularity_instance
{
    typedef typename M<T>::ptr_type instance_ptr_type;
    static instance_ptr_type instance_ptr;
};

template <class T, template <class T> class M>
typename singularity_instance<T, M>::instance_ptr_type
singularity_instance<T, M>::instance_ptr = NULL;

// This class provides the implementation for the singularity::get() member
// function.  A partial specialization is only provided if the access
// policy "global_access" is provided.  This produces a compile time error
// if the user attempts to call get() when using the "local_access" default
// policy.
template <class T, template <class T> class M, class G> class singularity_global_access;

template <class T, template <class T> class M>
class singularity_global_access<T, M, global_access>
{
public:
    static inline T& get()
    {
        #ifndef BOOST_NO_EXCEPTIONS
        if (singularity_instance<T, M>::instance_ptr == NULL)
        {
            throw singularity_not_created();
        }
        #else
        BOOST_ASSERT(singularity_instance<T, M>::instance_ptr != NULL);
        #endif
        return *singularity_instance<T, M>::instance_ptr;
    }
};

// BOOST_STATIC_ASSERT is not usable inside BOOST_PP_REPEAT because
// the BOOST_STATIC_ASSERT counter does not increment with each
// invocation.
template <bool condition> struct static_assert;
template <> struct static_assert<true> {};

// Generates: class A0 = optional, class A1 = optional, class A2 = optional
#define BOOST_PP_DEF_CLASS_TYPE_DEFAULT(z, n, text) BOOST_PP_COMMA_IF(n) class A##n = optional*

template
<
    class T,
    template <class T> class M = single_threaded,
    class G = local_access,
    BOOST_PP_REPEAT(BOOST_SINGULARITY_LIMIT_ARGS, BOOST_PP_DEF_CLASS_TYPE_DEFAULT, _)
>
class singularity : private boost::noncopyable
{
    #define SINGULARITY_ASSERT(z, nn, text) static_assert<is_pointer< A##nn >::value || is_reference< A##nn >::value> TYPE_A##nn##_IS_NEITHER_A_POINTER_OR_REFERENCE;
    BOOST_PP_REPEAT(BOOST_SINGULARITY_LIMIT_ARGS, SINGULARITY_ASSERT, _)
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
            singularity_instance<T, M>::instance_ptr = new T(BOOST_PP_REPEAT(n, BOOST_PP_DEF_ARG, _)); \
            singularity_state<T>::created = true; \
            return *singularity_instance<T, M>::instance_ptr; \
        }

    BOOST_PP_REPEAT(BOOST_PP_INC(BOOST_SINGULARITY_LIMIT_ARGS), BOOST_PP_DEF_CREATE_FUNCTION, _)
    #undef BOOST_PP_DEF_TYPE_ARG
    #undef BOOST_PP_DEF_ARG
    #undef BOOST_PP_DEF_CREATE_FUNCTION

    static inline void destroy() {
        M<T> guard;
        (void)guard;

        #ifndef BOOST_NO_EXCEPTIONS
        if (singularity_state<T>::created != true)
        {
            throw singularity_already_destroyed();
        }
        if (singularity_instance<T, M>::instance_ptr == NULL)
        {
            throw singularity_destroy_on_incorrect_threading();
        }
        #else
        BOOST_ASSERT(singularity_state<T>::created == true);
        BOOST_ASSERT((singularity_instance<T, M>::instance_ptr != NULL));
        #endif

        delete singularity_instance<T, M>::instance_ptr;
        singularity_instance<T, M>::instance_ptr = NULL;
        singularity_state<T>::created = false;
    }

    static inline T& get()
    {
        M<T> guard;
        (void)guard;

        return singularity_global_access<T, M, G>::get();
    }
private:
    static inline void detect_already_created()
    {
        #ifndef BOOST_NO_EXCEPTIONS
        if (singularity_state<T>::created != false)
        {
            throw singularity_already_created();
        }
        #else
        BOOST_ASSERT(singularity_state<T>::created == false);
        #endif
    }
};

#undef BOOST_PP_DEF_CLASS_TYPE_DEFAULT

// Convenience macro which generates the required friend statement
// for use inside classes which are created by singularity.
#define FRIEND_CLASS_SINGULARITY \
    template < class T, template <class T> class M, class G, \
        BOOST_PP_ENUM_PARAMS(BOOST_SINGULARITY_LIMIT_ARGS, class A) \
    > friend class singularity

} // boost namespace

#endif // SINGULARITY_HPP_
