#pragma once

#include <QtGui>
#include <ui/StageController.h>

namespace fetch {
namespace ui {

class StageView: public QGraphicsItem
{
public:
  StageView(PlanarStageController *stageControl,QGraphicsItem *parent=0);
  virtual ~StageView();
	
  QRectF boundingRect  () const;                                           // in meters
  void   paint         (QPainter                       *painter,
                        const QStyleOptionGraphicsItem *option,
                        QWidget                        *widget = 0);

private:
  QRectF  bbox_meters_;
  QPointF pos_meters_;

  QPen    pen_;
  QBrush  brush_;

  PlanarStageController *control_;
};


}}//end fetch::ui


