#include "scplugin.h"
#include "scsurface.h"
#include "QApplication"
#include "QScriptEngine"
#include "UIPlugin.h"
#include "acore.h"
#include "QScriptEngineDebugger"
#include "QDir"
#include "aplugin.h"
#include "QDebug"
#include "QMainWindow"

ALTER_PLUGIN(ScPlugin);

ScPlugin::ScPlugin() : _engine(0), _debugger(0), _surface(0)
  {
  }

ScPlugin::~ScPlugin()
  {
  delete _debugger;
  delete _engine;
  delete _surface;

  _debugger = 0;
  _engine = 0;
  _surface = 0;
  }

void ScPlugin::load()
  {
  _engine = new QScriptEngine(this);
  _debugger = new QScriptEngineDebugger(this);
  _debugger->setAutoShowStandardWindow(true);


  registerScriptGlobal(this);

  APlugin<UIPlugin> ui(this, "ui");
  if(ui.isValid())
    {
    _surface = new ScSurface(this);
    ui->addSurface(_surface);

    connect(ui.plugin(), SIGNAL(aboutToClose()), this, SLOT(hideDebugger()));
    }

  includePath(":/Sc/CoreUtils.js");
  }

void ScPlugin::enableDebugging(bool enable)
  {
  emit debuggingStateChanged(enable);
  if(enable)
    {
    _debugger->attachTo(_engine);
    }
  else
    {
    _debugger->detach();
    }
  }

void ScPlugin::showDebugger()
  {
  _debugger->standardWindow()->show();
  }

void ScPlugin::hideDebugger()
  {
  _debugger->standardWindow()->hide();
  }

bool ScPlugin::loadPlugin(const QString &plugin)
  {
  return core()->load(plugin);
  }

void ScPlugin::includePath(const QString &filename)
  {
  bool result = executeFile(filename);
  if(result)
    {
    qDebug() << " Include File" << filename << " ... Success";
    }
  else
    {
    qWarning() << " Include File" << filename << " ... Failure";
    }
  }

void ScPlugin::include(const QString &filename)
  {
  qDebug() << "Include File" << filename;
  foreach( const QFileInfo &fileInfo, core()->directories() )
    {
    includePath(fileInfo.filePath());
    }
  }

void ScPlugin::includeFolder(const QString &folder)
  {
  qDebug() << "Include Folder" << folder;
  QDir dir(folder);
  foreach(const QFileInfo &filename, dir.entryInfoList(QStringList() << "*.js"))
    {    
    includePath(filename.filePath());
    }
  qDebug() << "... Success";
  }

void ScPlugin::registerScriptGlobal(QObject *in)
  {
  QScriptValue objectValue = _engine->newQObject(in);
  _engine->globalObject().setProperty(in->objectName(), objectValue);
  }

bool ScPlugin::executeFile(const QString &filename)
  {
  QFile file(filename);
  if(file.exists() && file.open( QIODevice::ReadOnly))
    {
    QString data(QString::fromUtf8(file.readAll()));
    return execute(data);
    }
  return false;
  }

bool ScPlugin::isDebuggingEnabled()
  {
  return _engine->agent() != 0;
  }

bool ScPlugin::execute(const QString &code)
  {
  QScriptValue ret = _engine->evaluate(code);

  if(_engine->hasUncaughtException())
    {
    qWarning() << "Error in script at line " << _engine->uncaughtExceptionLineNumber() << ": " << endl << ret.toString() << endl;
    return false;
    }
  else if(!ret.isUndefined())
    {
    qDebug() << "Script Returned: " << ret.toString() << endl;
    }
  return true;
  }