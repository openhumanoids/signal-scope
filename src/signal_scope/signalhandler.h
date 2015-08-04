#ifndef _SIGNALHANDLER_H_
#define _SIGNALHANDLER_H_

#include "lcmsubscriber.h"
#include "signaldescription.h"

#include <QHash>
#include <string>


namespace lcm {
  class LCM;
  class ReceiveBuffer;
  class Subscription;
}

class SignalData;
class SignalDescription;


class SignalHandler : public LCMSubscriber
{
  Q_OBJECT

public:

  SignalHandler(const SignalDescription* signalDescription, QObject* parent=0);
  virtual ~SignalHandler();

  SignalData* signalData()
  {
    return mSignalData;
  }

  virtual QString description() = 0;
  QString channel() { return mDescription.mChannel; }
  SignalDescription* signalDescription() { return &mDescription; }

  virtual bool extractSignalData(const lcm::ReceiveBuffer* rbuf, double& timeNow, double& signalValue) = 0;

  virtual void subscribe(lcm::LCM* lcmInstance);

 protected:

  void handleMessage(const lcm::ReceiveBuffer* rbuf, const std::string& channel);

  SignalData* mSignalData;
  SignalDescription mDescription;
};


class SignalHandlerFactory
{
public:
    template<typename T>
    void registerClass()
    {
      mConstructors[T::messageType()][T::fieldName()] = &constructorHelper<T>;
      mValidArrayKeys[T::messageType()][T::fieldName()] = T::validArrayKeys();
    }

    SignalHandler* createHandler(const SignalDescription* desc) const;

    static SignalHandlerFactory& instance();

    QList<QString> messageTypes() const { return mConstructors.keys(); }
    QList<QString> channels() const { return mChannels; }
    QList<QString> fieldNames(const QString& messageType) const { return mConstructors.value(messageType).keys(); }
    QList<QList<QString> > validArrayKeys(const QString& messageType, const QString& fieldName) const { return mValidArrayKeys[messageType][fieldName]; }

    void addChannel(const QString& channel) { mChannels.append(channel); };
    void addChannels(const QList<QString>& channels) { mChannels += channels; };

    void setTimeZero(int64_t offset) { mTimeOffset = offset; }
    int64_t getTimeZero() const { return mTimeOffset; }
    double getOffsetTime(int64_t messageTime);

private:
  typedef SignalHandler* (*Constructor)(const SignalDescription* desc);

  template<typename T>
  static SignalHandler* constructorHelper(const SignalDescription* desc)
  {
    return new T(desc);
  }

  int64_t mTimeOffset;
  QStringList mChannels;
  QHash<QString, QHash<QString, Constructor> > mConstructors;
  QHash<QString, QHash<QString, QList<QList<QString> >  > > mValidArrayKeys;
};


#endif
