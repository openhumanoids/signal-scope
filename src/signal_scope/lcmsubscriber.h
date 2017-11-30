#ifndef _LCMSUBSCRIBER_h
#define _LCMSUBSCRIBER_h

#include "signalhandler.h"

#include <QObject>

#include <lcm/lcm-cpp.hpp>

namespace lcm
{
  class LCM;
  class ReceiveBuffer;
  class Subscription;
}


class LCMSubscriber : public SignalHandler
{
  Q_OBJECT

public:

  LCMSubscriber(QObject* parent=0) : QObject(parent)
  {
    mSubscription = 0;
  }

  virtual ~LCMSubscriber()
  {

  }

  virtual void subscribe(lcm::LCM* lcmInstance) = 0;

  virtual void unsubscribe(lcm::LCM* lcmHandle)
  {
    lcmHandle->unsubscribe(mSubscription);
    mSubscription = 0;
  }


  virtual bool extractSignalData(const lcm::ReceiveBuffer* rbuf, double& timeNow, double& signalValue) = 0;

  virtual void subscribe(lcm::LCM* lcmInstance);

 protected:

  void handleMessage(const lcm::ReceiveBuffer* rbuf, const std::string& channel);


  lcm::Subscription* mSubscription;
};

#endif
