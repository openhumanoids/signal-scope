#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "plotwidget.h"
#include "signalhandler.h"
#include "signaldata.h"
#include "selectsignaldialog.h"
#include "signaldescription.h"
#include "lcmthread.h"

#if USE_BUILTIN_LCMTYPES
  #include "builtinmessages.h"
#endif

#include <QLabel>
#include <QLayout>
#include <QApplication>
#include <QComboBox>
#include <QLabel>
#include <QSpinBox>
#include <QDebug>
#include <QScrollArea>
#include <QPushButton>
#include <QShortcut>
#include <QFileDialog>
#include <QInputDialog>
#include <QTimer>

#include "qjson.h"

#include "ctkPythonConsole.h"
#include "ctkAbstractPythonManager.h"
#include "pythonsignalhandler.h"
#include "pythonmessageinspector.h"
#include "pythonchannelsubscribercollection.h"


#include <cstdio>
#include <limits>


class MainWindow::Internal : public Ui::MainWindow
{
public:

};


MainWindow::MainWindow(QWidget* parent): QMainWindow(parent)
{
  mInternal = new Internal;
  mInternal->setupUi(this);

  mPlaying = false;
  this->setWindowTitle("Signal Scope");

  mLCMThread = new LCMThread;

  this->initPython();

#if USE_BUILTIN_LCMTYPES
  BuiltinMessages::registerBuiltinHandlers(SignalHandlerFactory::instance());
  BuiltinMessages::registerBuiltinChannels(SignalHandlerFactory::instance());
#endif

  mScrollArea = new QScrollArea;
  mPlotArea = new QWidget;
  mPlotLayout = new QVBoxLayout(mPlotArea);

  mScrollArea->setWidget(mPlotArea);
  mScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  mScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  mScrollArea->setWidgetResizable(true);
  this->setCentralWidget(mScrollArea);

  mInternal->ActionOpen->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogOpenButton));
  mInternal->ActionOpenPython->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogOpenButton));
  mInternal->ActionSave->setIcon(qApp->style()->standardIcon(QStyle::SP_DialogSaveButton));
  mInternal->ActionPause->setIcon(qApp->style()->standardIcon(QStyle::SP_MediaPlay));
  mInternal->ActionResize->setIcon(qApp->style()->standardIcon(QStyle::SP_ArrowUp));
  mInternal->ActionAutomaticResize->setIcon(qApp->style()->standardIcon(QStyle::SP_BrowserReload));
  mInternal->ActionClearHistory->setIcon(qApp->style()->standardIcon(QStyle::SP_TrashIcon));
  mInternal->ActionAddPlot->setIcon(qApp->style()->standardIcon(QStyle::SP_TrashIcon));
  //QStyle::SP_DialogDiscardButton

  this->connect(mInternal->ActionQuit, SIGNAL(triggered()), SLOT(close()));
  this->connect(mInternal->ActionOpen, SIGNAL(triggered()), SLOT(onOpenSettings()));
  this->connect(mInternal->ActionOpenPython, SIGNAL(triggered()), SLOT(onOpenPythonScript()));
  this->connect(mInternal->ActionSave, SIGNAL(triggered()), SLOT(onSaveSettings()));
  this->connect(mInternal->ActionPause, SIGNAL(triggered()), SLOT(onTogglePause()));
  this->connect(mInternal->ActionResize, SIGNAL(triggered()), SLOT(onResize()));
  this->connect(mInternal->ActionAutomaticResize, SIGNAL(triggered()), SLOT(onAutomaticResize()));
  this->connect(mInternal->ActionAddPlot, SIGNAL(triggered()), SLOT(onNewPlotClicked()));
  this->connect(mInternal->ActionClearHistory, SIGNAL(triggered()), SLOT(onClearHistory()));
  this->connect(mInternal->ActionResetTimeZero, SIGNAL(triggered()), SLOT(onResetTimeZero()));

  this->connect(mInternal->ActionBackgroundColor, SIGNAL(triggered()), SLOT(onChooseBackgroundColor()));
  this->connect(mInternal->ActionSetHistoryLength, SIGNAL(triggered()), SLOT(onChooseHistoryLength()));


  mInternal->toolBar->addSeparator();
  mInternal->toolBar->addWidget(new QLabel("    Style: "));

  QComboBox* curveStyleCombo = new QComboBox(this);
  curveStyleCombo->addItem("points");
  curveStyleCombo->addItem("lines");
  mInternal->toolBar->addWidget(curveStyleCombo);

  mInternal->toolBar->addWidget(new QLabel("    Point size: "));

  QSpinBox* pointSizeSpin = new QSpinBox(this);
  pointSizeSpin->setMinimum(1);
  pointSizeSpin->setMaximum(20);
  pointSizeSpin->setSingleStep(1);
  pointSizeSpin->setValue(1);
  mInternal->toolBar->addWidget(pointSizeSpin);


  mInternal->toolBar->addWidget(new QLabel("    Align: "));
  QComboBox* alignCombo = new QComboBox(this);
  alignCombo->addItem("right");
  alignCombo->addItem("center");
  mInternal->toolBar->addWidget(alignCombo);


  this->connect(curveStyleCombo, SIGNAL(currentIndexChanged(const QString&)), SLOT(onCurveStyleChanged(QString)));
  this->connect(pointSizeSpin, SIGNAL(valueChanged(int)), SLOT(onPointSizeChanged(int)));
  this->connect(alignCombo, SIGNAL(currentIndexChanged(const QString&)), SLOT(onAlignModeChanged(QString)));

  mRedrawTimer = new QTimer(this);
  //mRedrawTimer->setSingleShot(true);
  this->connect(mRedrawTimer, SIGNAL(timeout()), this, SLOT(onRedrawPlots()));

  this->resize(1024,800);
  this->handleCommandLineArgs();

  this->onTogglePause();

  mLCMThread->start();
}

MainWindow::~MainWindow()
{

  QString settingsFile = QDir::homePath() + "/.signal_scope.json";
  this->saveSettings(settingsFile);

  mLCMThread->stop();
  mLCMThread->wait(250);
  delete mLCMThread;

  delete mInternal;
}

void MainWindow::handleCommandLineArgs()
{
  QStringList args = QApplication::instance()->arguments();

  if (args.length() > 1)
  {
    QString filename = args[1];
    if (filename.endsWith(".py"))
    {
      this->loadPythonScript(filename);
    }
    else
    {
      this->loadSettings(filename);
    }
  }
  else
  {
    QString settingsFile = QDir::homePath() + "/.signal_scope.json";
    if (QFileInfo(settingsFile).exists())
    {
      this->loadSettings(settingsFile);
    }
  }
}

bool MainWindow::eventFilter(QObject* watched, QEvent* e)
{
  if(watched == this->mConsole && (e->type() == QEvent::Hide || e->type() == QEvent::Show))
  {
      if (e->type() == QEvent::Show)
      {
        mLCMThread->pause();
      }
      else
      {
        mLCMThread->resume();
      }
  }

  return QObject::eventFilter(watched, e);
}

void MainWindow::initPython()
{
  this->mPythonManager = new ctkAbstractPythonManager(this);
  this->mConsole = new ctkPythonConsole(this);
  this->mConsole->setWindowFlags(Qt::Dialog);
  this->mConsole->initialize(this->mPythonManager);
  this->mConsole->setAttribute(Qt::WA_QuitOnClose, true);
  this->mConsole->resize(600, 280);
  this->mConsole->setProperty("isInteractive", true);
  this->connect(new QShortcut(QKeySequence("F8"), this), SIGNAL(activated()), this->mConsole, SLOT(show()));
  this->connect(new QShortcut(QKeySequence("Ctrl+W"), this->mConsole), SIGNAL(activated()), this->mConsole, SLOT(close()));
  this->mConsole->installEventFilter(this);

  QString closeShortcut = "Ctrl+D";
  #ifdef Q_OS_DARWIN
  closeShortcut = "Meta+D";
  #endif
  this->connect(new QShortcut(QKeySequence(closeShortcut), this->mConsole), SIGNAL(activated()), this->mConsole, SLOT(close()));


  PythonQt::self()->registerClass(&PlotWidget::staticMetaObject, "signal_scope");
  PythonQt::self()->registerClass(&PythonSignalHandler::staticMetaObject, "signal_scope");

  this->mPythonManager->addObjectToPythonMain("_console", this->mConsole);
  this->mPythonManager->addObjectToPythonMain("_mainWindow", this);

  QString startupFile = QString(":/signalScopeSetup.py");

  QFile f(startupFile);
  if (!f.open(QFile::ReadOnly | QFile::Text))
  {
    printf("error opening startup file: %s\n", qPrintable(startupFile));
  }
  else
  {
    QTextStream in(&f);
    this->mPythonManager->executeString(in.readAll());
  }

  PythonQtObjectPtr mainContext = PythonQt::self()->getMainModule();
  PythonQtObjectPtr decodeCallback = PythonQt::self()->getVariable(mainContext, "decodeMessageFunction");

  this->mSubscribers = new PythonChannelSubscriberCollection(mLCMThread, decodeCallback, this);
  this->mMessageInspector = new PythonMessageInspector(decodeCallback);
  //this->mLCMThread->addSubscriber(this->mMessageInspector);
}

PythonSignalHandler* MainWindow::addPythonSignal(PlotWidget* plot, QVariant signalData)
{
  QList<QVariant> signalDataList = signalData.toList();
  if (signalDataList.length() != 4)
  {
    printf("incorrect args passed to addPythonSignal\n");
    return 0;
  }

  QString channel = signalDataList[0].toString();
  PythonQtObjectPtr callback = signalDataList[1].value<PythonQtObjectPtr>();
  QString fieldName = signalDataList[2].toString();

  SignalDescription signalDescription;
  signalDescription.mChannel = channel;
  signalDescription.mFieldName = fieldName;
  signalDescription.mColor = signalDataList[3].value<QColor>();

  PythonSignalHandler* signalHandler = new PythonSignalHandler(&signalDescription, callback);
  plot->addSignal(signalHandler);
  return signalHandler;
}

void MainWindow::loadPythonScript(const QString& filename)
{
  if (QFileInfo(filename).exists())
  {
    this->mLCMThread->pause();
    this->mLastPythonScript = filename;
    this->onRemoveAllPlots();
    this->mPythonManager->executeFile(filename);
    this->mLCMThread->resume();
  }
  else
  {
    printf("file does not exist: %s\n", qPrintable(filename));
  }
}

void MainWindow::onCurveStyleChanged(QString style)
{
  QwtPlotCurve::CurveStyle curveStyle = style == "lines" ? QwtPlotCurve::Lines : QwtPlotCurve::Dots;

  foreach (PlotWidget* plot, mPlots)
  {
    plot->setCurveStyle(curveStyle);
  }
}

void MainWindow::onAlignModeChanged(QString mode)
{
  foreach (PlotWidget* plot, mPlots)
  {
    plot->setAlignMode(mode);
  }
}


void MainWindow::onPointSizeChanged(int pointSize)
{
  foreach (PlotWidget* plot, mPlots)
  {
    plot->setPointSize(pointSize - 1);
  }
}

void MainWindow::onResize()
{
  foreach (PlotWidget* plot, mPlots)
  {
    plot->onResetYAxisScale();
  }
}

void MainWindow::onAutomaticResize()
{
  if (mInternal->ActionAutomaticResize->isChecked()){
    foreach (PlotWidget* plot, mPlots)
    {
      plot->onResetYAxisScale();
      plot->getScale();
      plot->rescalingTimer->start(100);
    }    
  }
  else
  {
    foreach (PlotWidget* plot, mPlots)
    {
      plot->rescalingTimer->stop();
    }
  }
}

void MainWindow::onClearHistory()
{
  foreach (PlotWidget* plot, mPlots)
  {
    plot->clearHistory();
  }
}

QString MainWindow::defaultSettingsDir()
{
  QString configDir = qgetenv("DRC_BASE") + "/software/config/signal_scope/json";
  if (QDir(configDir).exists())
  {
    return QDir(configDir).canonicalPath();
  }
  else
  {
    return QDir::currentPath();
  }
}

void MainWindow::onChooseBackgroundColor()
{
  QStringList colors;
  colors << "Black" << "White";

  bool ok;
  QString color = QInputDialog::getItem(this, "Choose background color", "Color", colors, 0, false, &ok);
  if (ok)
  {
    this->setPlotBackgroundColor(color);
  }
}

void MainWindow::onChoosePointSize()
{
  bool ok;
  int pointSize = QInputDialog::getInt(this, "Choose point size", "Point size", 1, 1, 20, 1, &ok);
  if (ok)
  {
    foreach (PlotWidget* plot, mPlots)
    {
      plot->setPointSize(pointSize - 1);
    }
  }
}

void MainWindow::onChooseHistoryLength()
{
  bool ok;
  int defaultHistoryLength = SignalData::getHistoryLength();
  int historyLength = QInputDialog::getInt(this, "Choose history length", "History length (seconds)", defaultHistoryLength, 1, 60*60, 1, &ok);
  if (ok)
  {
    SignalData::setHistoryLength(historyLength);
  }
}

void MainWindow::setPlotBackgroundColor(QString color)
{
  foreach (PlotWidget* plot, mPlots)
  {
    plot->setBackgroundColor(color);
  }
}

void MainWindow::onSyncXAxis(double x0, double x1)
{
  if (mPlaying)
    return;

  foreach (PlotWidget* plot, mPlots)
  {
    if (plot == this->sender())
      continue;

    plot->setXAxisScale(x0, x1);
    plot->replot();
  }
}

void MainWindow::onRedrawPlots()
{
  mFPSCounter.update();
  //printf("redraw fps: %f\n", this->mFPSCounter.averageFPS());

  if (mPlots.isEmpty())
  {
    return;
  }

  QList<SignalData*> signalDataList;
  foreach (PlotWidget* plot, mPlots)
  {
    foreach (SignalHandler* signalHandler, plot->signalHandlers())
    {
      signalDataList.append(signalHandler->signalData());
    }
  }

  if (signalDataList.isEmpty())
  {
    return;
  }

  double maxTime = -std::numeric_limits<float>::max();

  foreach (SignalData* signalData, signalDataList)
  {
    signalData->updateValues();
    double signalMaxTime = signalData->lastSampleTime();

    if (signalMaxTime > maxTime)
    {
      maxTime = signalMaxTime;
    }
  }

  if (maxTime == -std::numeric_limits<float>::max())
  {
    return;
  }

  foreach (PlotWidget* plot, mPlots)
  {
    plot->setEndTime(maxTime);
    plot->replot();
  }
}


void MainWindow::onOpenPythonScript()
{
  QString filter = "Python (*.py)";
  QString filename = QFileDialog::getOpenFileName(this, "Open Python Script", QDir(this->mLastPythonScript).canonicalPath(), filter);
  if (filename.length())
  {
    this->loadPythonScript(filename);
  }
}


void MainWindow::onOpenSettings()
{
  QString filter = "JSON (*.json)";
  QString filename = QFileDialog::getOpenFileName(this, "Open Settings", this->defaultSettingsDir(), filter);
  if (filename.length())
  {
    this->onRemoveAllPlots();
    this->loadSettings(filename);
  }
}

void MainWindow::onSaveSettings()
{
  QString defaultFile = mLastOpenFile.isEmpty() ? this->defaultSettingsDir() : mLastOpenFile;
  QString filter = "JSON (*.json)";
  QString filename = QFileDialog::getSaveFileName(this, "Save Settings", defaultFile, filter);
  if (filename.length())
  {
    this->saveSettings(filename);
  }
}

void MainWindow::saveSettings(const QString& filename)
{
  QMap<QString, QVariant> settings;

  settings["windowWidth"] = this->width();
  settings["windowHeight"] = this->height();
  if (this->mLastPythonScript.length())
  {
    settings["script"] = QFileInfo(this->mLastPythonScript).canonicalFilePath();
  }

  QList<QVariant> plotSettings;
  foreach (PlotWidget* plot, mPlots)
  {
    plotSettings.append(plot->saveSettings());
  }

  settings["plots"] = plotSettings;

  Json::encodeFile(filename, settings);
}

void MainWindow::loadSettings(const QString& filename)
{
  QMap<QString, QVariant> settings = Json::decodeFile(filename);
  this->loadSettings(settings);
}

void MainWindow::loadSettings(const QMap<QString, QVariant>& settings)
{
  QString pythonScript = settings.value("script").toString();
  if (pythonScript.length())
  {
    this->loadPythonScript(pythonScript);
  }
  else
  {
    QList<QVariant> plots = settings.value("plots").toList();
    foreach (const QVariant& plot, plots)
    {
      PlotWidget* plotWidget = this->addPlot();
      QMap<QString, QVariant> plotSettings = plot.toMap();
      plotWidget->loadSettings(plotSettings);
    }
  }

  int windowWidth = settings.value("windowWidth", 1024).toInt();
  int windowHeight = settings.value("windowHeight", 800).toInt();
  this->resize(windowWidth, windowHeight);
}

void MainWindow::onTogglePause()
{
  mPlaying = !mPlaying;
  mInternal->ActionPause->setChecked(mPlaying);
  mInternal->ActionPause->setIcon(qApp->style()->standardIcon(mPlaying ? QStyle::SP_MediaPause : QStyle::SP_MediaPlay));
  mInternal->ActionPause->setText(mPlaying ? "Pause" : "Play");


  foreach (PlotWidget* plot, mPlots)
  {
    if (mPlaying)
      plot->start();
    else
      plot->stop();
  }

  if (mPlaying)
  {
    mRedrawTimer->start(33);
  }
  else
  {
    mRedrawTimer->stop();
  }
}

QList<SignalHandler*> MainWindow::getSignalSelectionFromUser()
{
  SelectSignalDialog dialog(this);
  int result = dialog.exec();
  if (result != QDialog::Accepted)
  {
    return QList<SignalHandler*>();
  }

  return dialog.createSignalHandlers();
}

void MainWindow::onNewPlotClicked()
{
  QList<SignalHandler*> signalHandlers = this->getSignalSelectionFromUser();
  if (signalHandlers.isEmpty())
  {
    return;
  }

  PlotWidget* plot = this->addPlot();
  foreach (SignalHandler* signalHandler, signalHandlers)
  {
    plot->addSignal(signalHandler);
  }
}

PlotWidget* MainWindow::addPlot()
{
  PlotWidget* plot = new PlotWidget(mSubscribers);
  mPlotLayout->addWidget(plot);
  this->connect(plot, SIGNAL(removePlotRequested(PlotWidget*)), SLOT(onRemovePlot(PlotWidget*)));
  this->connect(plot, SIGNAL(addSignalRequested(PlotWidget*)), SLOT(onAddSignalToPlot(PlotWidget*)));
  this->connect(plot, SIGNAL(syncXAxisScale(double, double)), SLOT(onSyncXAxis(double, double)));
  mPlots.append(plot);

  if (mPlaying)
    plot->start();
  else
    plot->stop();

  return plot;
}

void MainWindow::onAddSignalToPlot(PlotWidget* plot)
{
  if (!plot)
  {
    return;
  }

  QList<SignalHandler*> signalHandlers = this->getSignalSelectionFromUser();
  if (signalHandlers.isEmpty())
  {
    return;
  }

  foreach (SignalHandler* signalHandler, signalHandlers)
  {
    plot->addSignal(signalHandler);
  }
}

void MainWindow::onResetTimeZero()
{
  SignalHandlerFactory::instance().setTimeZero(0);
  this->onClearHistory();
}

void MainWindow::onRemoveAllPlots()
{
  QList<PlotWidget*> plots = mPlots;
  foreach(PlotWidget* plot, plots)
  {
    this->onRemovePlot(plot);
  }
}

void MainWindow::onRemovePlot(PlotWidget* plot)
{
  if (!plot)
  {
    return;
  }

  mPlotLayout->removeWidget(plot);
  mPlots.removeAll(plot);
  delete plot;
}
