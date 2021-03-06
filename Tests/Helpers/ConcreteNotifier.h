#ifndef CONCRETENOTIFIER_H
#define CONCRETENOTIFIER_H

class ConcreteNotifier : public TinyMock::TinyNotifier
{
public:
	ConcreteNotifier() : sendWasCalled(false) {}
	void Send(bool status=true)
	{
		sendWasCalled = true ;		
	}
	void ResetNotificationFlag()
	{
		sendWasCalled = false ;		
	}
	bool sendWasCalled ;	
};

#endif
