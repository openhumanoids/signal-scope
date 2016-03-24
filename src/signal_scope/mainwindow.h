#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <qmainwindow.h>
#include <qvariant.h>
#include "fpscounter.h"

class QScrollArea;
class QVBoxLayout;
class QTimer;
class PlotWidget;
class LCMThread;
class PythonChannelSubscriberCollection;
class SignalHandler;
class PythonSignalHandler;
class PythonMessageInspector;
class ctkPythonConsole;
class ctkAbstractPythonManager;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

  MainWindow(QWidget * = NULL);
  ~MainWindow();

  static QString defaultSettingsDir();

public slots:

  void onTogglePause();
  void onNewPlotClicked();
  void onSaveSettings();
  void onOpenSettings();
  void onOpenPythonScript();
  void onClearHistory();
  void onRemovePlot(PlotWidget* plot);
  void onAddSignalToPlot(PlotWidget* plot);
  void onRemoveAllPlots();
  void onResetTimeZero();
  void setPlotBackgroundColor(QString color);
  void onResize();
  void onAutomaticResize();
  void onChooseBackgroundColor();
  void onChooseHistoryLength();
  void onChoosePointSize();

  PlotWidget* addPlot();
  void loadPythonScript(const QString& filename);
  PythonSignalHandler* addPythonSignal(PlotWidget* plot, QVariant signalData);

  QList<PlotWidget*> getPlots() { return mPlots; }

protected slots:

  void onSyncXAxis(double x0, double x1);

  void onRedrawPlots();

  void onPointSizeChanged(int size);
  void onCurveStyleChanged(QString style);
  void onAlignModeChanged(QString mode);

protected:

  void handleCommandLineArgs();
  void saveSettings(const QString& filename);
  void loadSettings(const QString& filename);
  void loadSettings(const QMap<QString, QVariant>& settings);
  void loadPlot(const QMap<QString, QVariant>& plot);

  void testPythonSignals();
  void initPython();

  bool eventFilter(QObject* watched, QEvent* e);


  QList<SignalHandler*> getSignalSelectionFromUser();

  bool mPlaying;
  QString mLastOpenFile;
  QString mLastPythonScript;
  QScrollArea* mScrollArea;
  QWidget* mPlotArea;
  QVBoxLayout* mPlotLayout;
  QTimer *mRedrawTimer;

  FPSCounter mFPSCounter;

  QList<PlotWidget*> mPlots;

  LCMThread* mLCMThread;

  ctkPythonConsole* mConsole;
  ctkAbstractPythonManager* mPythonManager;
  PythonChannelSubscriberCollection* mSubscribers;
  PythonMessageInspector* mMessageInspector;

  class Internal;
  Internal* mInternal;

};

#endif
