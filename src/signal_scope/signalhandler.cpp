#include "signalhandler.h"

#include "signaldata.h"
#include "signaldescription.h"

#include <lcm/lcm-cpp.hpp>


SignalHandler::SignalHandler(const SignalDescription* signalDescription, QObject* parent) : LCMSubscriber(parent)
{
  mDescription = *signalDescription;
  mSignalData = new SignalData();
}

SignalHandler::~SignalHandler()
{
  delete mSignalData;
}

void SignalHandler::handleMessage(const lcm::ReceiveBuffer* rbuf, const std::string& channel)
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

void SignalHandler::subscribe(lcm::LCM* lcmInstance)
{
  if (mSubscription)
  {
    printf("error: SignalHandler::subscribe() called without first calling unsubscribe.\n");
    return;
  }
  mSubscription = lcmInstance->subscribe(this->channel().toAscii().data(), &SignalHandler::handleMessage, this);
}

SignalHandlerFactory& SignalHandlerFactory::instance()
{
  static SignalHandlerFactory factory;
  return factory;
}

SignalHandler* SignalHandlerFactory::createHandler(const SignalDescription* desc) const
{
  Constructor constructor = mConstructors.value(desc->mMessageType).value(desc->mFieldName);
  if (constructor == NULL)
  {
    return NULL;
  }
  return (*constructor)(desc);
}

double SignalHandlerFactory::getOffsetTime(int64_t messageTime)
{
  if (mTimeOffset == 0)
  {
    mTimeOffset = messageTime;
  }
  return (messageTime - mTimeOffset) * 1e-6;
}
