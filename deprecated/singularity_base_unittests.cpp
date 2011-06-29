
//               Copyright Ben Robinson 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

//#define BOOST_TEST_MAIN defined
#include <boost/test/unit_test.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <singularity_base.hpp>

namespace {

using ::boost::singularity_base;
using ::boost::single_threaded;
using ::boost::multi_threaded;
using ::boost::global_access;
using ::boost::noncopyable;

// Some generic, non POD class.
class Event : private noncopyable {
public:
    Event() {}
private:
};

// This class demonstrates making itself a Singularity,
// by inheriting from singularity_base, and initializing the base class.
class Horizon : public singularity_base<Horizon, single_threaded>, private noncopyable {
public:
    Horizon()                 : singularity_base(), mInt(0),    mEvent(),          mEventPtr(&mEvent),   mEventRef(mEvent)    {}
    Horizon(int xInt)         : singularity_base(), mInt(xInt), mEvent(),          mEventPtr(&mEvent),   mEventRef(mEvent)    {}
    Horizon(Event& xEventRef) : singularity_base(), mInt(0),    mEvent(),          mEventPtr(&mEvent),   mEventRef(xEventRef) {}
    Horizon(Event* xEventPtr) : singularity_base(), mInt(0),    mEvent(),          mEventPtr(xEventPtr), mEventRef(mEvent)    {}
    Horizon(int xInt, Event* xEventPtr, Event& xEventRef)
                              : singularity_base(), mInt(xInt), mEvent(),          mEventPtr(xEventPtr), mEventRef(xEventRef) {}

private:
    int    mInt;
    Event  mEvent;
    Event* mEventPtr;
    Event& mEventRef;
};

BOOST_AUTO_TEST_CASE(passOneArgumentByValue) {
    int value = 3;
    Horizon * horizon = new Horizon(value);
    (void)horizon;
    delete horizon;
}

BOOST_AUTO_TEST_CASE(passOneArgumentByAddress) {
    Event event;
    Horizon * horizon = new Horizon(&event);
    (void)horizon;
    delete horizon;
}

BOOST_AUTO_TEST_CASE(passOneArgumentByReference) {
    Event event;
    Horizon * horizon = new Horizon(event);
    (void)horizon;
    delete horizon;
}

BOOST_AUTO_TEST_CASE(passThreeArguments) {
    Event event;
    int value = 3;
    Horizon * horizon = new Horizon(value, &event, event);
    (void)horizon;
    delete horizon;
}

BOOST_AUTO_TEST_CASE(shouldThrowOnDoubleCalls) {
    Horizon * horizon = new Horizon();
    (void)horizon;
    BOOST_CHECK_THROW( // Call create() twice in a row
        Horizon * horizon2 = new Horizon(),
        boost::singularity_already_created
    );

    delete horizon;
}

BOOST_AUTO_TEST_CASE(shouldThrowOnDoubleCallsWithDifferentArguments) {
    Horizon * horizon = new Horizon();
    (void)horizon;
    int value = 5;
    BOOST_CHECK_THROW( // Call create() twice in a row
        Horizon * horizon2 = (new Horizon(value)),
        boost::singularity_already_created
    );

    delete horizon;
}

BOOST_AUTO_TEST_CASE(shouldCreateDestroyCreateDestroy) {
    Horizon * horizon = new Horizon();
    (void)horizon;
    delete horizon;
    Horizon * different_horizon = new Horizon();
    (void)different_horizon;
    delete different_horizon;
}

} // namespace anonymous
