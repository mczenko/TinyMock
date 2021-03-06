#include <iostream>
using namespace std;
#include <memory>

#include "yaffut.h"
#include "TinyMock.h"
using namespace TinyMock;

#include "TestMock.h"
#include "TestNotifier.h"
#include "ComplexArgument.h"
#include "ConcreteNotifier.h"

class LoggerInterface
{
public:
	virtual void log()=0;
};

class MockLogger : public LoggerInterface, public TinyMock::Mock
{
public:
	MockLogger() {}
        MockLogger(const std::string& className) : TinyMock::Mock(className) {}
	void log()
	{
                TinyMock::Method<void,void,void,void,void> actual("log");
                TinyMock::BaseMethod* expected = m_expectations.GetFirstExpectationFor(actual.Signature());
	
                Handle(expected,(TinyMock::BaseMethod*)&actual);
	}
};


class OurTestClass
{
public:
	OurTestClass(LoggerInterface* logger) : _logger(logger) {}
	void methodA()
	{
		_logger->log();
	}
private:
	LoggerInterface* _logger ;
};


class StandardNotifier : public TinyMock::TinyNotifier
{
public:
	class MockFailureException {};
	StandardNotifier() {}
	void Send(bool status=true)
	{
		throw MockFailureException();
	}
	void ResetNotificationFlag()
	{
		sendWasCalled = false ;
	}
	bool sendWasCalled ;
};

class YaffutFailureNotifier : public TinyMock::TinyNotifier
{
public:	
	YaffutFailureNotifier() {}
	void Send(bool status=true)
	{
		FAIL("");
	}	
};

class ExceptionFailureNotifier : public TinyMock::TinyNotifier
{
public:	
	ExceptionFailureNotifier() {}
	void Send(bool status=true)
	{
		throw exception();
	}	
};


struct TestTinyMock
{
    TestTinyMock()
    {        
    }
	
    ~TestTinyMock()
    {        
    }
};

TEST(TestTinyMock,TestIfANotifierRegisteredWithAnExpectationIsCalled)
{
	ConcreteNotifier notifier ;

	MockRepository<YaffutFailureNotifier> mockRepository ;

        TestMock* testMock = mockRepository.CreateMock<TestMock,ConcreteNotifier>("TestMock");

        testMock->RegisterExpectation(new TinyMock::Method<void,void,void,void,void>("TestMethod")).AddNotifier<ExceptionFailureNotifier>();

	try
	{
		testMock->TestMethod();
	}
	catch (exception)
	{
		mockRepository.verifyAll();
		return;
	}
	FAIL("");
}

TEST(TestTinyMock,TestReturningArguments)
{	
	MockRepository<YaffutFailureNotifier> mockRepository ;

        TestMock* testMock = mockRepository.CreateMock<TestMock,YaffutFailureNotifier>("TestMock");

        testMock->RegisterExpectation(new TinyMock::Method<void,void,void,void,int>("TestMethodWithReturnValue",125));

	EQUAL(125,testMock->TestMethodWithReturnValue());

	mockRepository.verifyAll();
}

TEST(TestTinyMock,TestFailureNotifier)
{
	const int argValue = 25 ;

	MockRepository<YaffutFailureNotifier> mockRepository;

        TestMock* testMock = mockRepository.CreateMock<TestMock, ExceptionFailureNotifier>("TestMock");

        testMock->RegisterExpectation(new TinyMock::Method<int,void,void,void,void>("TestMethodWithAnArgument",argValue));

	try
	{
	   testMock->TestMethodWithAnArgument(argValue-1);
	}
	catch(exception)
	{
		mockRepository.verifyAll();
		return;
	}
	FAIL("");
}

TEST(TestTinyMock,TestIfFailureNotifierIsCalledOnlyWithTheFirstFailingExpectation)
{
	const int argValue = 25 ;

	MockRepository<YaffutFailureNotifier> mockRepository;

        TestMock* testMock = mockRepository.CreateMock<TestMock, ExceptionFailureNotifier>("TestMock");

        testMock->RegisterExpectation(new TinyMock::Method<int,void,void,void,void>("TestMethodWithAnArgument",argValue));
        testMock->RegisterExpectation(new TinyMock::Method<int,void,void,void,void>("TestMethodWithAnArgument",argValue));

	try
	{
	   testMock->TestMethodWithAnArgument(argValue-1);
	}
	catch(exception)
	{
		try
		{
			testMock->TestMethodWithAnArgument(argValue-1);
		}
		catch(exception)
		{
			FAIL("PIES");
		}
        mockRepository.verifyAll();
		return;
	}
	FAIL("");
}

TEST(TestTinyMock,TestArgumentDereference)
{
        ComplexArgument complexArg(151) ;
	ComplexArgument complexArg2(151) ;		

	MockRepository<YaffutFailureNotifier> mockRepository;

        TestMock* testMock = mockRepository.CreateMock<TestMock, YaffutFailureNotifier>("TestMock");

        testMock->RegisterExpectation(new TinyMock::MethodWithDereferencedArguments<ComplexArgument*,void,void,void,void>("TestMethodWithAPointerArgument",&complexArg2));
	
	testMock->TestMethodWithAPointerArgument(&complexArg);

	mockRepository.verifyAll();
}

TEST(TestTinyMock,SequentialCheckingIfAMethodForWhichAllCallsShouldBeIgnoredIsIgnoredShouldReturnTrue)
{
        TinyMock::IgnoredMethodsContainer ignoredMethodContainer ;

	ignoredMethodContainer.ignoreAll("AMethod") ;

	// we use 4 calls - it should pass even if you add more calls - the number of calls should not matter
	CHECK(ignoredMethodContainer.isIgnored("AMethod")) ;
	CHECK(ignoredMethodContainer.isIgnored("AMethod")) ;
	CHECK(ignoredMethodContainer.isIgnored("AMethod")) ;
	CHECK(ignoredMethodContainer.isIgnored("AMethod")) ;
}

TEST(TestTinyMock,TestIfAMethodWhichWasNotMarkedAsIgnoredWillBeTreatedAsSuch)
{
        TinyMock::IgnoredMethodsContainer ignoredMethodContainer ;
	
	CHECK(!ignoredMethodContainer.isIgnored("AMethod")) ;
}

TEST(TestTinyMock,AsADevelopperIWantToBeAbleToIgnoreAllCallsToACertainMethod)
{
	MockRepository<YaffutFailureNotifier> mockRepository;

        TestMock* testMock = mockRepository.CreateMock<TestMock, YaffutFailureNotifier>("TestMock");

	testMock->IgnoreAll("TestMethod");

        testMock->RegisterExpectation(new TinyMock::Method<void,void,void,void,void>("TestMethod")).AddNotifier<YaffutFailureNotifier>();

	testMock->TestMethod();
	testMock->TestMethod();
	
	mockRepository.verifyAll();
}

TEST(TestTinyMock,WhenIgnoringAllCallsToAMethodYouDoNotNeedToRegisterExpectationForThisMethodCall)
{
	MockRepository<YaffutFailureNotifier> mockRepository;

        TestMock* testMock = mockRepository.CreateMock<TestMock, YaffutFailureNotifier>("TestMock");

	testMock->IgnoreAll("TestMethod");	

	testMock->TestMethod();
	testMock->TestMethod();
	
	mockRepository.verifyAll();
}

TEST(TestTinyMock,IgnoringArgumentsInACallToASpecificMethod)
{
	MockRepository<YaffutFailureNotifier> mockRepository;

        TestMock* testMock = mockRepository.CreateMock<TestMock, YaffutFailureNotifier>("TestMock");

	const int doesNotMatter = 0 ;	
	const int actualArgument = 255 ;

        testMock->RegisterExpectation(new TinyMock::Method<int,void,void,void,void>("TestMethodWithAnArgument",doesNotMatter)).ignoreArguments();
	
	testMock->TestMethodWithAnArgument(actualArgument);

	mockRepository.verifyAll();
}

TEST(TestTinyMock,IgnoringArgumentsInACallToASpecificMethodButNotIgnoringThemForTheSubsequentCalls)
{
	MockRepository<YaffutFailureNotifier> mockRepository;

        TestMock* testMock = mockRepository.CreateMock<TestMock, ExceptionFailureNotifier>("TestMock");

	const int doesNotMatter = 0 ;
	const int expectedArgument = 1 ;
	const int actualArgument = 255 ;

        testMock->RegisterExpectation(new TinyMock::Method<int,void,void,void,void>("TestMethodWithAnArgument",doesNotMatter)).ignoreArguments();
        testMock->RegisterExpectation(new TinyMock::Method<int,void,void,void,void>("TestMethodWithAnArgument",expectedArgument));

	try
	{
		testMock->TestMethodWithAnArgument(actualArgument);
	}
	catch(std::exception)
	{
		FAIL("");
	}

	try
	{
		testMock->TestMethodWithAnArgument(actualArgument);
	}
	catch(std::exception)
	{
		mockRepository.verifyAll();
		return ;
	}
	FAIL("");	
}

TEST(TestTinyMock,TestIfAMethodIsCalled)
{
	MockRepository<YaffutFailureNotifier> mockRepository;
        MockLogger* logger = mockRepository.CreateMock<MockLogger, ExceptionFailureNotifier>("Logger") ;
        logger->RegisterExpectation(new TinyMock::Method<void,void,void,void,void>("log"));

	OurTestClass testClass(logger) ;

	testClass.methodA() ;

	mockRepository.verifyAll();
}
