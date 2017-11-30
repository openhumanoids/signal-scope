#ifndef _SIGNALHANDLER_H_
#define _SIGNALHANDLER_H_

#include "signaldescription.h"

#include <QHash>
#include <string>


class SignalData;
class SignalDescription;


class SignalHandler : public QObject
{
  Q_OBJECT

public:

  SignalHandler(const SignalDescription* signalDescription, QObject* parent=0);
  virtual ~SignalHandler();

  SignalData* signalData()
  {
    return mSignalData;
  }

public slots:

  virtual QString description()
  {
    return mDescription.mFieldName;
  }

  QString channel() { return mDescription.mChannel; }
  SignalDescription* signalDescription() { return &mDescription; }

  void appendSample(double timeNow, double signalValue);

 protected:

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
