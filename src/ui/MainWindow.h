#pragma once
#include <QtGui>
#include "ui/AgentController.h"
#include "ui/StageController.h"
#include "DevicePropController.h"

namespace fetch {
namespace device {class Microscope;}
namespace ui {

class VideoAcquisitionDockWidget;
class StackAcquisitionDockWidget;
class MicroscopeStateDockWidget;
class Figure;
class IPlayerThread;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(device::Microscope *dc);
  virtual ~MainWindow();

protected:
  void closeEvent(QCloseEvent *event);

public: // semi-private
  void createStateMachines();
  void createMenus();
  void createActions();
  void createDockWidgets();
  void createViews();
  device::Microscope *_dc;

  QStateMachine fullscreenStateMachine;
  QMenu   *viewMenu;
  QMenu   *fileMenu;

  QAction *quitAct;
  QAction *openAct;
  QAction *saveToAct;

  QAction *fullscreenAct;
  QState  *fullscreenStateOn;
  QState  *fullscreenStateOff;

  VideoAcquisitionDockWidget *_videoAcquisitionDockWidget;
  StackAcquisitionDockWidget *_stackAcquisitionDockWidget;
  MicroscopeStateDockWidget  *_microscopesStateDockWidget;
  Figure *_display;
  IPlayerThread *_player;
  
  QTimer _poller;
  AgentController             _scope_state_controller;
  
  PlanarStageController*      _stageController;  
  
  // Property controllers
  ResonantTurnController*     _resonant_turn_controller;
  LinesController*            _vlines_controller;
  LSMVerticalRangeController* _lsm_vert_range_controller;
  PockelsController*          _pockels_controller;

  ZPiezoMaxController*        _zpiezo_max_control;
  ZPiezoMinController*        _zpiezo_min_control;
  ZPiezoStepController*       _zpiezo_step_control;

private:  
  void load_settings_();
  void save_settings_();
  static const int version__ = 0;
};

//namespace ends
} //ui
} //fetch