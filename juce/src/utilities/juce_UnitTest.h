/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_UNITTEST_JUCEHEADER__
#define __JUCE_UNITTEST_JUCEHEADER__

#include "../text/juce_StringArray.h"
#include "../containers/juce_OwnedArray.h"
class UnitTestRunner;


//==============================================================================
/**
    This is a base class for classes that perform a unit test.

    To write a test using this class, your code should look something like this:

    @code
    class MyTest  : public UnitTest
    {
    public:
        MyTest()  : UnitTest ("Foobar testing") {}

        void runTest()
        {
            beginTest ("Part 1");

            expect (myFoobar.doesSomething());
            expect (myFoobar.doesSomethingElse());

            beginTest ("Part 2");

            expect (myOtherFoobar.doesSomething());
            expect (myOtherFoobar.doesSomethingElse());

            ...etc..
        }
    };

    // Creating a static instance will automatically add the instance to the array
    // returned by UnitTest::getAllTests(), so the test will be included when you call
    // UnitTestRunner::runAllTests()
    static MyTest test;
    @endcode

    To run a test, use the UnitTestRunner class.

    @see UnitTestRunner
*/
class JUCE_API  UnitTest
{
public:
    //==============================================================================
    /** Creates a test with the given name. */
    explicit UnitTest (const String& name);

    /** Destructor. */
    virtual ~UnitTest();

    /** Returns the name of the test. */
    const String& getName() const noexcept       { return name; }

    /** Runs the test, using the specified UnitTestRunner.
        You shouldn't need to call this method directly - use
        UnitTestRunner::runTests() instead.
    */
    void performTest (UnitTestRunner* runner);

    /** Returns the set of all UnitTest objects that currently exist. */
    static Array<UnitTest*>& getAllTests();

    //==============================================================================
    /** You can optionally implement this method to set up your test.
        This method will be called before runTest().
    */
    virtual void initialise();

    /** You can optionally implement this method to clear up after your test has been run.
        This method will be called after runTest() has returned.
    */
    virtual void shutdown();

    /** Implement this method in your subclass to actually run your tests.

        The content of your implementation should call beginTest() and expect()
        to perform the tests.
    */
    virtual void runTest() = 0;

    //==============================================================================
    /** Tells the system that a new subsection of tests is beginning.
        This should be called from your runTest() method, and may be called
        as many times as you like, to demarcate different sets of tests.
    */
    void beginTest (const String& testName);

    //==============================================================================
    /** Checks that the result of a test is true, and logs this result.

        In your runTest() method, you should call this method for each condition that
        you want to check, e.g.

        @code
        void runTest()
        {
            beginTest ("basic tests");
            expect (x + y == 2);
            expect (getThing() == someThing);
            ...etc...
        }
        @endcode

        If testResult is true, a pass is logged; if it's false, a failure is logged.
        If the failure message is specified, it will be written to the log if the test fails.
    */
    void expect (bool testResult, const String& failureMessage = String::empty);

    /** Compares two values, and if they don't match, prints out a message containing the
        expected and actual result values.
    */
    template <class ValueType>
    void expectEquals (ValueType actual, ValueType expected, String failureMessage = String::empty)
    {
        const bool result = (actual == expected);

        if (! result)
        {
            if (failureMessage.isNotEmpty())
                failureMessage << " -- ";

            failureMessage << "Expected value: " << expected << ", Actual value: " << actual;
        }

        expect (result, failureMessage);
    }

    //==============================================================================
    /** Writes a message to the test log.
        This can only be called from within your runTest() method.
    */
    void logMessage (const String& message);

private:
    //==============================================================================
    const String name;
    UnitTestRunner* runner;

    JUCE_DECLARE_NON_COPYABLE (UnitTest);
};


//==============================================================================
/**
    Runs a set of unit tests.

    You can instantiate one of these objects and use it to invoke tests on a set of
    UnitTest objects.

    By using a subclass of UnitTestRunner, you can intercept logging messages and
    perform custom behaviour when each test completes.

    @see UnitTest
*/
class JUCE_API  UnitTestRunner
{
public:
    //==============================================================================
    /** */
    UnitTestRunner();

    /** Destructor. */
    virtual ~UnitTestRunner();

    /** Runs a set of tests.

        The tests are performed in order, and the results are logged. To run all the
        registered UnitTest objects that exist, use runAllTests().
    */
    void runTests (const Array<UnitTest*>& tests, bool assertOnFailure);

    /** Runs all the UnitTest objects that currently exist.
        This calls runTests() for all the objects listed in UnitTest::getAllTests().
    */
    void runAllTests (bool assertOnFailure);

    //==============================================================================
    /** Contains the results of a test.

        One of these objects is instantiated each time UnitTest::beginTest() is called, and
        it contains details of the number of subsequent UnitTest::expect() calls that are
        made.
    */
    struct TestResult
    {
        /** The main name of this test (i.e. the name of the UnitTest object being run). */
        String unitTestName;
        /** The name of the current subcategory (i.e. the name that was set when UnitTest::beginTest() was called). */
        String subcategoryName;

        /** The number of UnitTest::expect() calls that succeeded. */
        int passes;
        /** The number of UnitTest::expect() calls that failed. */
        int failures;

        /** A list of messages describing the failed tests. */
        StringArray messages;
    };

    /** Returns the number of TestResult objects that have been performed.
        @see getResult
    */
    int getNumResults() const noexcept;

    /** Returns one of the TestResult objects that describes a test that has been run.
        @see getNumResults
    */
    const TestResult* getResult (int index) const noexcept;

protected:
    /** Called when the list of results changes.
        You can override this to perform some sort of behaviour when results are added.
    */
    virtual void resultsUpdated();

    /** Logs a message about the current test progress.
        By default this just writes the message to the Logger class, but you could override
        this to do something else with the data.
    */
    virtual void logMessage (const String& message);

private:
    //==============================================================================
    friend class UnitTest;

    UnitTest* currentTest;
    String currentSubCategory;
    OwnedArray <TestResult, CriticalSection> results;
    bool assertOnFailure;

    void beginNewTest (UnitTest* test, const String& subCategory);
    void endTest();

    void addPass();
    void addFail (const String& failureMessage);

    JUCE_DECLARE_NON_COPYABLE (UnitTestRunner);
};


#endif   // __JUCE_UNITTEST_JUCEHEADER__
