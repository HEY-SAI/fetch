#pragma once
#include <QtGui>
namespace mylib {
#include <array.h>
}
#include "StageScene.h"
#include "imitem.h"
#include "ScaleBar.h"
#include "TilesView.h"
#include "StageView.h"

namespace fetch {
namespace ui {

class ZoomableView:public QGraphicsView
{
	Q_OBJECT
public:
	ZoomableView(QGraphicsScene *scene, QWidget *parent=0);
	
  //inline double metersToPixels()                                           {return _scalebar.unit2px();}
  //inline void   setMetersToPixels(double unit2px)                          {_scalebar.setUnit(unit2px);}
	virtual void	wheelEvent(QWheelEvent *event);

  inline void notifyZoomChanged()                                            {emit zoomChanged(transform().m11());}
signals:
	void zoomChanged(double zoom);
	
protected:
	virtual void drawForeground(QPainter* painter, const QRectF& rect);
	
private:
	ScaleBar _scalebar;
};


class Figure:public QWidget
{
  Q_OBJECT
public:
	Figure(PlanarStageController *stageController, QWidget *parent=0);
  Figure(double unit2px, QWidget *parent=0);
  virtual ~Figure();
	
public slots:
	inline void push(mylib::Array *im)                                       {_item->push(im); updatePos(); emit pushed(); maybeFit();}
	inline void imshow(mylib::Array *im)                                     {_item->push(im); updatePos(); _item->flip(); maybeFit(); }
  inline void fit(void)                                                    {_view->fitInView(_item->mapRectToScene(_item->boundingRect()),Qt::KeepAspectRatio);_view->notifyZoomChanged();}
  inline void fitNext(void)                                                {/*_isFitOnNext=true;*/}
  inline void updatePos(void)                                              {_item->setPos(units::cvt<units::PIXEL_SCALE,PlanarStageController::Unit>(_sc->pos()));}
  inline void updatePos(QPointF r)                                         {_item->setPos(units::cvt<units::PIXEL_SCALE,PlanarStageController::Unit>(r));}

  inline void setPixelSizeMicrons(double w, double h)                      {_item->setPixelSizeMicrons(w,h);}
  inline void setPixelGeometry(double w, double h, double angle)           {_item->setPixelGeometry(w,h,angle);}
         void setDragModeToNoDrag();
         void setDragModeToSelect();
         void setDragModeToPan();
         //void setAddTilesMode();
         //void setDelTilesMode();

signals:
	void lastFigureClosed();
  void pushed();

protected:
  void readSettings();
  void writeSettings();

private:
  inline void maybeFit()                                                   {if(_isFitOnNext) {fit(); _isFitOnNext=false;}}
         void createActions();
         void createMenus();
private:
  PlanarStageController *_sc;
	ZoomableView*      _view;
	StageScene         _scene;
	ImItem*            _item;
  StageView*         _stage;
  TilesView*         _tv;

  bool               _isFitOnNext;

  QMenu             *_toolMenu;
  QAction           *_noDragModeAct;
  QAction           *_scrollDragModeAct;
  //QAction           *_addTilesModeAct;
  //QAction           *_delTilesModeAct;
  QAction           *_rubberBandModeAct;
};


//Figure* imshow (mylib::Array *im);
//void    imclose(Figure *w=0);

}} //end namespace fetch::ui
