
//               Copyright Ben Robinson 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

//#define BOOST_TEST_MAIN defined
#include <boost/test/unit_test.hpp>
#include <boost/noncopyable.hpp>
#include <singularity_cpp11.hpp>

namespace {

using ::boost::singularity;
using ::boost::single_threaded;
using ::boost::multi_threaded;
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
class Horizon : public singularity<Horizon, single_threaded>, private noncopyable {
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

    friend class singularity<Horizon, single_threaded>;
};

BOOST_AUTO_TEST_CASE(passOneArgumentByValue) {
    int value = 3;
    Horizon::enable_global_access(true);

    Horizon & horizon = Horizon::create(value);

    (void)horizon;
    Horizon & horizonGlobal = Horizon::get();
    (void)horizonGlobal;
    Horizon::destroy();

    Horizon::enable_global_access(false);
}

BOOST_AUTO_TEST_CASE(passOneArgumentByAddress) {
    Event event;
    Horizon & horizon = Horizon::create(&event);
    (void)horizon;
    Horizon::destroy();
}

BOOST_AUTO_TEST_CASE(passOneArgumentByReference) {
    Event event;
    Horizon & horizon = Horizon::create(event);
    (void)horizon;
    Horizon::destroy();
}

BOOST_AUTO_TEST_CASE(passThreeArguments) {
    int value = 3;
    Event event;
    typedef Horizon singularityType;
    Horizon & horizon = singularityType::create(value, &event, event);
    (void)horizon;
    singularityType::destroy();
}

BOOST_AUTO_TEST_CASE(shouldThrowOnDoubleCalls) {
    Horizon & horizon = Horizon::create();
    (void)horizon;
    BOOST_CHECK_THROW( // Call create() twice in a row
        Horizon & horizon2 = Horizon::create(),
        boost::singularity_already_created
    );

    Horizon::destroy();
    BOOST_CHECK_THROW( // Call destroy() twice in a row
        Horizon::destroy(),
        boost::singularity_already_destroyed
    );
}

BOOST_AUTO_TEST_CASE(shouldThrowOnDoubleCallsWithDifferentArguments) {
    Horizon & horizon = Horizon::create();
    (void)horizon;
    int value = 5;
    BOOST_CHECK_THROW( // Call create() twice in a row
        Horizon & horizon2 = (Horizon::create(value)),
        boost::singularity_already_created
    );

    Horizon::destroy();
    BOOST_CHECK_THROW( // Call destroy() twice in a row
        (Horizon::destroy()),
        boost::singularity_already_destroyed
    );
}

BOOST_AUTO_TEST_CASE(shouldCreateDestroyCreateDestroy) {
    Horizon & horizon = Horizon::create();
    (void)horizon;
    Horizon::destroy();
    Horizon & new_horizon = Horizon::create();
    (void)new_horizon;
    Horizon::destroy();
}

BOOST_AUTO_TEST_CASE(shouldThrowOnGetBeforeCreate) {
    Horizon::enable_global_access(true);

    BOOST_CHECK_THROW( // Call get() before create()
        Horizon & horizon1 = Horizon::get(),
        boost::singularity_not_created
    );

    Horizon & horizon2 = Horizon::create();
    (void)horizon2;

    Horizon & horizon3 = Horizon::get();
    (void)horizon3;

    Horizon::destroy();
    BOOST_CHECK_THROW( // Call get() after destroy()
        Horizon & horizon4 = Horizon::get(),
        boost::singularity_not_created
    );
}

} // namespace anonymous
