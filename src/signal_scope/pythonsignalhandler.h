#ifndef _PYTHONSIGNALHANDLER_H_
#define _PYTHONSIGNALHANDLER_H_

#include "signalhandler.h"
#include "signaldescription.h"
#include "signaldata.h"

#include <PythonQt.h>

class SignalData;
class SignalDescription;


class PythonSignalHandler : public SignalHandler
{
  Q_OBJECT

public:

  PythonSignalHandler(SignalDescription* signalDescription, PythonQtObjectPtr callback) : SignalHandler(signalDescription)
  {
    mCallback = callback;
  }

  virtual QString description()
  {
    return mDescription.mFieldName;
  }

  void onNewMessage(const QVariant& message)
  {
    double timeNow;
    double signalValue;

    bool valid = this->extractSignalData(message, timeNow, signalValue);
    if (valid)
    {
      mSignalData->appendSample(timeNow, signalValue);
    }
    else
    {
      mSignalData->flagMessageError();
    }
  }

  virtual bool extractSignalData(const QVariant& message, double& timeNow, double& signalValue)
  {
    QVariantList args;
    args << message;

    QVariant result = PythonQt::self()->call(mCallback, args);
    QList<QVariant> values = result.toList();
    if (values.size() == 2)
    {
      timeNow = SignalHandlerFactory::instance().getOffsetTime(static_cast<int64_t>(values[0].toDouble()*1e6));
      signalValue = values[1].value<double>();
      //std::cout << timeNow << ", " << signalValue << std::endl;
      return true;
    }
    else
    {
      return false;
    }

    return true;
  }

  virtual bool extractSignalData(const lcm::ReceiveBuffer* rbuf, double& timeNow, double& signalValue)
  {
    return false;
  }

 protected:

  PythonQtObjectPtr mCallback;
};


#endif
