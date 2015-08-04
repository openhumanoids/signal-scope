#ifndef _PYTHONMESSAGEINSPECTOR_H_
#define _PYTHONMESSAGEINSPECTOR_H_

#include "lcmsubscriber.h"
#include "lcmthread.h"

#include <PythonQt.h>

#include <QVariant>
#include <QMap>
#include <QString>

class PythonMessageInspector : public LCMSubscriber
{
  Q_OBJECT

public:

  PythonMessageInspector(PythonQtObjectPtr decodeCallback, QObject* parent=0) : LCMSubscriber(parent)
  {
    mDecodeCallback = decodeCallback;
  }

  void subscribe(lcm::LCM* lcmHandle)
  {
    if (mSubscription)
    {
      printf("error: subscribe() called without first calling unsubscribe.\n");
      return;
    }
    mSubscription = lcmHandle->subscribe(".*", &PythonMessageInspector::handleMessageOnChannel, this);
  }

  QVariant decodeMessage(const lcm::ReceiveBuffer* rbuf)
  {
    if (!mDecodeCallback)
    {
      return QVariant();
    }

    QVariantList args;
    args << QByteArray((char*)rbuf->data, rbuf->data_size);
    return PythonQt::self()->call(mDecodeCallback, args);
  }


  void handleMessageOnChannel(const lcm::ReceiveBuffer* rbuf, const std::string& channel)
  {
    QString channelStr = channel.c_str();

    if (this->mChannelMap.contains(channelStr))
    {
      return;
    }

    PythonQtObjectPtr decodedMessage = this->decodeMessage(rbuf);

    this->mChannelMap[channelStr] = decodedMessage;

    if (decodedMessage.isNull())
    {
      printf("failed to decode: %s\n", qPrintable(channelStr));
      return;
    }

    QList<QString> fields = decodedMessage.getVariable("__slots__").toStringList();
    QString className = PythonQtObjectPtr(decodedMessage.getVariable("__class__")).getVariable("__name__").toString();

    foreach (QString field, fields)
    {
      printf("%s %s %s\n", qPrintable(channelStr), qPrintable(className), qPrintable(field));
    }
  }


  QMap<QString, PythonQtObjectPtr> mChannelMap;
  PythonQtObjectPtr mDecodeCallback;
};

#endif
