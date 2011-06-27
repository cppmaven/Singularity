
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
//  int value = 3;
//
//  Usage as a Factory:
//
//  typedef singularity<Horizon, single_threaded, local_access> HorizonSingularityType;
//  Horizon & horizonA = HorizonSingularityType::create();
//                       HorizonSingularityType::destroy();
//
//  Horizon & horizonB = HorizonSingularityType::create(value, &event, event);
//                       HorizonSingularityType::destroy();
//
//  Usage as a Base Class:
//
//  class Horizon : public singularity<Horizon, multi_threaded, global_access>
//  Horizon & horizonC = Horizon::create();
//                       Horizon::destroy(0);
//
//  Horizon & horizonD = Horizon::create(value, &event, event);
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
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/facilities/expand.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/arithmetic/div.hpp>
#include <boost/preprocessor/arithmetic/mod.hpp>
#include <pow2.hpp>

// The user can choose a different arbitrary upper limit to the
// number of constructor arguments.  The Boost Preprocessor library
// is used to generate the appropriate code.
#ifndef BOOST_SINGULARITY_LIMIT_ARGS
#define BOOST_SINGULARITY_LIMIT_ARGS 6
#endif

namespace boost {

// The threading model for Singularity is policy based, and
// a single threaded, and multi threaded policy are provided,
// when thread safety is required.
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
// model, or access policy, only one singularity of type T can be created.
template <class T> struct singularity_state
{
    static bool created;
};

template <class T> bool singularity_state<T>::created = false;

// The instance pointer must depend on both type T and the threading model M,
// in order to control whether or not the pointer is volatile.  It does not
// need to depend on the access policy G.
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

// And now, presenting the singularity class itself.
template
<
    class T,
    template <class T> class M = single_threaded,
    class G = local_access
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
        singularity_instance<T, M>::instance_ptr = new T(BOOST_PP_ENUM_PARAMS(na, arg)); \
        singularity_state<T>::created = true; \
        return *singularity_instance<T, M>::instance_ptr; \
    }

#define SINGULARITY_CREATE_OVERLOADS(z, na, text) BOOST_PP_REPEAT(BOOST_PP_EXPAND(BOOST_PP_POW2(na)), SINGULARITY_CREATE_BODY, na)

BOOST_PP_REPEAT(BOOST_PP_INC(BOOST_SINGULARITY_LIMIT_ARGS), SINGULARITY_CREATE_OVERLOADS, _)

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

// Convenience macro which generates the required friend statement
// for use inside classes which are created by singularity.
#define FRIEND_CLASS_SINGULARITY \
    template <class T, template <class T> class M, class G> friend class singularity

} // boost namespace

#endif // SINGULARITY_HPP
