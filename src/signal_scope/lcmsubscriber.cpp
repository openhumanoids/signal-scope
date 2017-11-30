#include "lcmsubscriber.h"


void LCMSubscriber::handleMessage(const lcm::ReceiveBuffer* rbuf, const std::string& channel)
{
  double timeNow;
  double signalValue;
  (void)channel;

  bool valid = this->extractSignalData(rbuf, timeNow, signalValue);
  if (valid)
  {
    mSignalData->appendSample(timeNow, signalValue);
  }
  else
  {
    mSignalData->flagMessageError();
  }
}

void LCMSubscriber::subscribe(lcm::LCM* lcmInstance)
{
  if (mSubscription)
  {
    printf("error: SignalHandler::subscribe() called without first calling unsubscribe.\n");
    return;
  }
#if QT_VERSION >= 0x050000
  mSubscription = lcmInstance->subscribe(this->channel().toLatin1().data(), &SignalHandler::handleMessage, this);
#else
  mSubscription = lcmInstance->subscribe(this->channel().toAscii().data(), &SignalHandler::handleMessage, this);
#endif
}
