#ifndef _BUILTINMESSAGES_H_
#define _BUILTINMESSAGES_H_

class SignalHandlerFactory;

class BuiltinMessages {

public:

  static void registerBuiltinHandlers(SignalHandlerFactory& factory);
  static void registerBuiltinChannels(SignalHandlerFactory& factory);

};

#endif
