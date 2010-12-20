/* =====
 * Fetch
 * =====
 *
 */

// Glew must be included before OpenGL
#ifdef _WIN32
#include <GL/glew.h>
#define INIT_EXTENSIONS \
  assert(glewInit()==GLEW_OK)
#else
#define INIT_EXTENSIONS
#endif

#include <QtGui>
#include <QtOpenGL>
#include <assert.h>
#include "util/util-gl.h"
#include "util/util-qt.h"

#include "common.h"
#include "devices/Microscope.h"
#include "tasks/microscope-interaction.h"

#include "ui/MainWindow.h"
#include <stdio.h>
#include <google/protobuf/text_format.h>
/*
 * Global state
 */

fetch::device::Microscope *gp_microscope;
fetch::task::microscope::Interaction g_microscope_default_task;

/*
 * Shutdown callbacks
 */

unsigned int QtShutdownCallback(void)
{ QCoreApplication::exit(0); // Pass success (0), since I don't know what else to do
  return 0;
}

unsigned int KillMicroscopeCallback(void)
{
  if(gp_microscope) { delete gp_microscope; gp_microscope=NULL;}
  return 0;
}

void Init(void)
{
  gp_microscope = NULL;

  //Shutdown
  Register_New_Shutdown_Callback(KillMicroscopeCallback);
  Register_New_Shutdown_Callback(QtShutdownCallback);

  //Logging
  Reporting_Setup_Log_To_VSDebugger_Console();
  Reporting_Setup_Log_To_Filename("lastrun.log");
  Reporting_Setup_Log_To_Qt();

  //Microscope devices
  QFile cfgfile(":/config/microscope");
  Guarded_Assert(cfgfile.open(QIODevice::ReadOnly));
  Guarded_Assert(cfgfile.isReadable());
  //cfgfile.setTextModeEnabled(true);
  fetch::cfg::device::Microscope config;
  Guarded_Assert(google::protobuf::TextFormat::ParseFromString(cfgfile.readAll().constData(),&config));
  gp_microscope = new fetch::device::Microscope(config);


  // Connect video display
  // TODO
}

int main(int argc, char *argv[])
{ QGL::setPreferredPaintEngine(QPaintEngine::OpenGL);

  QApplication app(argc,argv);
  Init();
  fetch::ui::MainWindow mainwindow(gp_microscope);
  mainwindow.show();

  unsigned int eflag = app.exec();
  eflag |= Shutdown_Soft();
  //debug("Press <Enter> to exit.\r\n");
  //getchar();
  return eflag;
}
