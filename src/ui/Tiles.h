#pragma once

#include <QtGui>
#include <QtOpenGL>

namespace mylib
{
#include <array.h>
}

#include "ui/StageController.h"

#define GOLDENRATIO 1.61803399

namespace fetch {
namespace ui {


class TilesView : public QGraphicsObject
{
  Q_OBJECT

public:
  TilesView(TilingController *tc, QGraphicsItem* parent = 0);


  ~TilesView();

  QRectF boundingRect() const;
  void   paint         (QPainter                       *painter,
                        const QStyleOptionGraphicsItem *option,
                        QWidget                        *widget = 0);

public slots:
  void addSelection(const QPainterPath& path);
  void removeSelection(const QPainterPath& path);

  void update_tiling();
  void show(bool tf);

protected:
  virtual void hoverMoveEvent (QGraphicsSceneHoverEvent *event);
  virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event); 

private:
  typedef TilingController::TTransform TTransform;

  TilingController *tc_;
  QGLBuffer vbo_,ibo_,cbo_;                    //vertex, index, and color  buffer objects for big lattice
  //QGLBUffer cursorColorVBO_,cursorIndexVBO_; //colors and indeces for highlighting tiles near the cursor
  //bool      cursorActive_;                   //
  //int       cursorIndexLength_;
  int       icursor_;
  QRectF    bbox_;
  bool      is_active_;

  QImage    *latticeImage_;

  void draw_grid_();
  void draw_cursor_();
  void paint_lattice_(const QPainterPath& path, const QColor& pc, const QColor &bc);
  void init_cursor_(const QPointF& pos);
  void initIBO();
  void updateIBO();
  void initVBO();
  void updateVBO();
  void initCBO();
  void updateCBO();
};

}} // end fetch::ui namespace
