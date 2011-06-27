
//               Copyright Ben Robinson 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_MAIN defined
#include <boost/test/unit_test.hpp>
#include <boost/noncopyable.hpp>
#include <singularity_explicit.hpp>

namespace {

using ::boost::singularity;
using ::boost::single_threaded;
using ::boost::multi_threaded;
using ::boost::local_access;
using ::boost::global_access;
using ::boost::noncopyable;

// Some generic, non POD class.
class Event : private noncopyable {
public:
    Event() {}
private:
};

// This class demonstrates making itself a Singularity,
// by making its constructors private, and friending
// the Singulariy using the convenience macro.
class Horizon : private noncopyable {
private:
    Horizon()                 : mInt(0),    mEvent(),          mEventPtr(&mEvent),   mEventRef(mEvent)    {}
    Horizon(int xInt)         : mInt(xInt), mEvent(),          mEventPtr(&mEvent),   mEventRef(mEvent)    {}
    Horizon(Event& xEventRef) : mInt(0),    mEvent(),          mEventPtr(&mEvent),   mEventRef(xEventRef) {}
    Horizon(Event* xEventPtr) : mInt(0),    mEvent(),          mEventPtr(xEventPtr), mEventRef(mEvent)    {}
    Horizon(int xInt, Event* xEventPtr, Event& xEventRef)
                              : mInt(xInt), mEvent(),          mEventPtr(xEventPtr), mEventRef(xEventRef) {}
    int    mInt;
    Event  mEvent;
    Event* mEventPtr;
    Event& mEventRef;

    FRIEND_CLASS_SINGULARITY;
};

BOOST_AUTO_TEST_CASE(passOneArgumentByValue) {
    int value = 3;
    Horizon & horizon = singularity<Horizon, single_threaded, local_access, int&>::create(value);
    (void)horizon;
    singularity<Horizon, single_threaded, int&>::destroy();
}

BOOST_AUTO_TEST_CASE(passOneArgumentByAddress) {
    Event event;
    Horizon & horizon = singularity<Horizon, single_threaded, local_access, Event*>::create(&event);
    (void)horizon;
    singularity<Horizon, single_threaded, Event*>::destroy();
}

BOOST_AUTO_TEST_CASE(passOneArgumentByReference) {
    Event event;
    Horizon & horizon = singularity<Horizon, single_threaded, local_access, Event&>::create(event);
    (void)horizon;
    singularity<Horizon, single_threaded, Event&>::destroy();
}

BOOST_AUTO_TEST_CASE(passThreeArguments) {
    int value = 3;
    Event event;
    typedef singularity<Horizon, single_threaded, local_access, int&, Event*, Event&> singularityType;
    Horizon & horizon = singularityType::create(value, &event, event);
    (void)horizon;
    singularityType::destroy();
}

BOOST_AUTO_TEST_CASE(shouldThrowOnDoubleCalls) {
    Horizon & horizon = singularity<Horizon>::create();
    (void)horizon;
    BOOST_CHECK_THROW( // Call create() twice in a row
        Horizon & horizon2 = singularity<Horizon>::create(),
        boost::singularity_already_created
    );

    singularity<Horizon>::destroy();
    BOOST_CHECK_THROW( // Call destroy() twice in a row
        singularity<Horizon>::destroy(),
        boost::singularity_already_destroyed
    );
}

BOOST_AUTO_TEST_CASE(shouldThrowOnDoubleCallsWithDifferentArguments) {
    Horizon & horizon = singularity<Horizon>::create();
    (void)horizon;
    int value = 5;
    BOOST_CHECK_THROW( // Call create() twice in a row
        Horizon & horizon2 = (singularity<Horizon, single_threaded, local_access, int&>::create(value)),
        boost::singularity_already_created
    );

    singularity<Horizon>::destroy();
    BOOST_CHECK_THROW( // Call destroy() twice in a row
        (singularity<Horizon, single_threaded, local_access, int&>::destroy()),
        boost::singularity_already_destroyed
    );
}

BOOST_AUTO_TEST_CASE(shouldThrowOnDestroyWithWrongThreading) {
    Horizon & horizon = singularity<Horizon, single_threaded>::create();
    (void)horizon;

    BOOST_CHECK_THROW( // Call destroy() with wrong threading
        (singularity<Horizon, multi_threaded>::destroy()),
        boost::singularity_destroy_on_incorrect_threading
    );
    singularity<Horizon, single_threaded>::destroy();
}

BOOST_AUTO_TEST_CASE(shouldCreateDestroyCreateDestroy) {
    Horizon & horizon = singularity<Horizon>::create();
    (void)horizon;
    singularity<Horizon>::destroy();
    Horizon & new_horizon = singularity<Horizon>::create();
    (void)new_horizon;
    singularity<Horizon>::destroy();
}

BOOST_AUTO_TEST_CASE(useMultiThreadedPolicy) {
    Horizon & horizon = singularity<Horizon, multi_threaded>::create();
    (void)horizon;
    singularity<Horizon, multi_threaded>::destroy();
}

BOOST_AUTO_TEST_CASE(shouldThrowOnGetBeforeCreate) {
    typedef singularity<Horizon, single_threaded, global_access> HorizonSingularityType;
    BOOST_CHECK_THROW( // Call get() before create()
        Horizon & horizon1 = HorizonSingularityType::get(),
        boost::singularity_not_created
    );

    Horizon & horizon2 = HorizonSingularityType::create();
    (void)horizon2;

    Horizon & horizon3 = HorizonSingularityType::get();
    (void)horizon3;

    HorizonSingularityType::destroy();
    BOOST_CHECK_THROW( // Call get() after destroy()
        Horizon & horizon4 = HorizonSingularityType::get(),
        boost::singularity_not_created
    );
}

} // namespace anonymous
