
//               Copyright Ben Robinson 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

//#define BOOST_TEST_MAIN defined
#include <boost/test/unit_test.hpp>
#include <boost/noncopyable.hpp>
#include <singularity_factory.hpp>

namespace {

using ::boost::singularity_factory;
using ::boost::single_threaded;
using ::boost::multi_threaded;
using ::boost::no_global_access;
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

    FRIEND_CLASS_SINGULARITY_FACTORY;
};

BOOST_AUTO_TEST_CASE(passOneArgumentByValue) {
    int value = 3;
    Horizon & horizon = singularity_factory<Horizon, single_threaded, no_global_access, int&>::create(value);
    (void)horizon;
    singularity_factory<Horizon, single_threaded, int&>::destroy();
}

BOOST_AUTO_TEST_CASE(passOneArgumentByAddress) {
    Event event;
    Horizon & horizon = singularity_factory<Horizon, single_threaded, no_global_access, Event*>::create(&event);
    (void)horizon;
    singularity_factory<Horizon, single_threaded, Event*>::destroy();
}

BOOST_AUTO_TEST_CASE(passOneArgumentByReference) {
    Event event;
    Horizon & horizon = singularity_factory<Horizon, single_threaded, no_global_access, Event&>::create(event);
    (void)horizon;
    singularity_factory<Horizon, single_threaded, Event&>::destroy();
}

BOOST_AUTO_TEST_CASE(passThreeArguments) {
    int value = 3;
    Event event;
    typedef singularity_factory<Horizon, single_threaded, no_global_access, int&, Event*, Event&> singularityType;
    Horizon & horizon = singularityType::create(value, &event, event);
    (void)horizon;
    singularityType::destroy();
}

BOOST_AUTO_TEST_CASE(shouldThrowOnDoubleCalls) {
    Horizon & horizon = singularity_factory<Horizon>::create();
    (void)horizon;
    BOOST_CHECK_THROW( // Call create() twice in a row
        Horizon & horizon2 = singularity_factory<Horizon>::create(),
        boost::singularity_already_created
    );

    singularity_factory<Horizon>::destroy();
    BOOST_CHECK_THROW( // Call destroy() twice in a row
        singularity_factory<Horizon>::destroy(),
        boost::singularity_already_destroyed
    );
}

BOOST_AUTO_TEST_CASE(shouldThrowOnDoubleCallsWithDifferentArguments) {
    Horizon & horizon = singularity_factory<Horizon>::create();
    (void)horizon;
    int value = 5;
    BOOST_CHECK_THROW( // Call create() twice in a row
        Horizon & horizon2 = (singularity_factory<Horizon, single_threaded, no_global_access, int&>::create(value)),
        boost::singularity_already_created
    );

    singularity_factory<Horizon>::destroy();
    BOOST_CHECK_THROW( // Call destroy() twice in a row
        (singularity_factory<Horizon, single_threaded, no_global_access, int&>::destroy()),
        boost::singularity_already_destroyed
    );
}

BOOST_AUTO_TEST_CASE(shouldCreateDestroyCreateDestroy) {
    Horizon & horizon = singularity_factory<Horizon>::create();
    (void)horizon;
    singularity_factory<Horizon>::destroy();
    Horizon & new_horizon = singularity_factory<Horizon>::create();
    (void)new_horizon;
    singularity_factory<Horizon>::destroy();
}

BOOST_AUTO_TEST_CASE(useMultiThreadedPolicy) {
    Horizon & horizon = singularity_factory<Horizon, multi_threaded>::create();
    (void)horizon;
    singularity_factory<Horizon, multi_threaded>::destroy();
}

BOOST_AUTO_TEST_CASE(shouldThrowOnGetBeforeCreate) {
    typedef singularity_factory<Horizon, single_threaded, global_access> HorizonSingularityType;
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
