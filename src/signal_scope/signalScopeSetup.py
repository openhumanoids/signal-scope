import os
import sys
import glob
import traceback
from PythonQt import QtGui


_messageTypes = {}

def loadMessageTypes(typesModule):

    typesDict = _messageTypes
    originalSize = len(typesDict)
    for name, value in typesModule.__dict__.iteritems():
        if hasattr(value, '_get_packed_fingerprint'):
            typesDict[value._get_packed_fingerprint()] = value

    print 'Loaded %d lcm message types from: %s' % (len(typesDict) - originalSize, typesModule.__name__)


def findLCMModules(searchDir):

    initFiles = glob.glob(os.path.join(searchDir, '*/__init__.py'))
    for initFile in initFiles:
        if open(initFile, 'r').readline() == '"""LCM package __init__.py file\n':
            moduleDir = os.path.dirname(initFile)
            moduleName = os.path.basename(moduleDir)
            #print 'loading module:', moduleName
            #if moduleName == 'bot_procman':
            #    continue

            sys.path.insert(0, os.path.dirname(moduleDir))
            try:
                module = __import__(moduleName)
            except ImportError:
                print traceback.format_exc()

            sys.path.pop(0)
            loadMessageTypes(module)


def findLCMModulesInSysPath():
    try:
        for searchDir in sys.path:
            findLCMModules(searchDir)
    except:
        print traceback.format_exc()

findLCMModulesInSysPath()


class LookupHelper(object):


  def __init__(self, lookups=()):
      self._lookups = lookups

  def __getitem__(self, i):
      return LookupHelper(self._lookups + ([i],))

  def __getattr__(self, attr):
      return LookupHelper(self._lookups + (attr,))

  def _getResolverFunction(self):

    lookups = []
    name = ''
    for a in self._lookups:
        if isinstance(a, str):
            if len(name): name += '.'
            name += '%s' % a
            f = lambda msg, field, a=a: getattr(field, a)
        else:
          a = a[0]
          if isinstance(a, int):
              name += '[%d]' % a
              f = lambda msg, field, a=a: field[a]
          if isinstance(a, str):
              name += "['%s']" % a

              def f(msg, field, a=a, data=[]):
                  if not data:
                      data.append(field.index(a))
                  return data[0]

          elif isinstance(a, LookupHelper):
              key = a._lookups[-1][0]
              subFunc = a._getResolverFunction()
              name += '[%s]' % key #subFunc.__doc__

              f = lambda msg, field, a=a: field[subFunc(msg)]

        lookups.append(f)


    def func(msg):
        value = msg
        for x in lookups:
            value = x(msg, value)
        return value

    func.__doc__ = name
    return func


def createSignalFunction(timeLookup, valueLookup):
    t = timeLookup._getResolverFunction()
    v = valueLookup._getResolverFunction()

    def func(msg):
        return t(msg)*1e-6, v(msg)
    func.__doc__ = v.__doc__
    return func



def decodeMessageFunction(messageBytes):
    s = str(messageBytes)
    message = _messageTypes[s[:8]].decode(s)
    return message


msg = LookupHelper()


def addPlot(timeWindow=None, yLimits=None):
    plot = _mainWindow.addPlot()
    if timeWindow is not None:
        plot.setTimeWindow(timeWindow)
    if yLimits is not None:
        plot.setYAxisScale(yLimits[0], yLimits[1])
    return plot


def getPlot():
    plots = getPlots()
    return plots[-1] if plots else None


def getPlots():
    return _mainWindow.getPlots()


def removePlots():
    _mainWindow.onRemoveAllPlots()


def addSignalFunction(channel, signalFunction, plot=None, color=None):

    if plot is None:
        plot = getPlot()
        if plot is None:
            plot = addPlot()

    if color is not None:
        color = QtGui.QColor(*[int(x*255) for c in color])
    else:
        color = QtGui.QColor()

    _mainWindow.addPythonSignal(plot, [channel, signalFunction, signalFunction.__doc__, color])


def addSignal(channel, timeLookup, valueLookup, plot=None, color=None):

    signalFunction = createSignalFunction(timeLookup, valueLookup)
    addSignalFunction(channel, signalFunction, plot=plot, color=color)


def addSignals(channel, timeLookup, valueLookup, keys, keyLookup=None, plot=None, colors=None):

    if colors is None:
        colors = [None] * len(keys)

    for key, color in zip(keys, colors):

        if keyLookup is not None:
            key = keyLookup[key]

        addSignal(channel, timeLookup, valueLookup[key], plot=plot, color=color)
