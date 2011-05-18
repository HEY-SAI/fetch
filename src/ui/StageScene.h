#pragma once
#include <QtGui>

class SelectionRectGraphicsWidget : public QGraphicsWidget
{
  Q_OBJECT

public:
  enum RectOp {Add,Remove};

  SelectionRectGraphicsWidget(QGraphicsItem *parent=0);

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0 );

public slots:
  void setOpAdd()     {lastop_=op_=Add; update();}
  void setOpRemove()  {lastop_=op_=Remove; update();}
  void commit();
  void cancel();

signals:
  void addSelectedArea(const QPainterPath& path);
  void removeSelectedArea(const QPainterPath& path);

protected:
  virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

  virtual void focusInEvent(QFocusEvent *event);
  virtual void focusOutEvent(QFocusEvent *event);

private:
  RectOp op_;
  static RectOp lastop_;

  void createActions();

  QColor getPenColor();
  QColor getBaseColor();
};


class StageScene : public QGraphicsScene
{
  Q_OBJECT

public:
  enum Mode {DrawRect, Pick, DoNothing};  

  StageScene(QObject *parent=0);

  virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
  virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
  virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

public slots:
  void setMode(Mode mode);

signals:    
  void addSelectedArea(const QPainterPath& path);
  void removeSelectedArea(const QPainterPath& path);

private:
  QGraphicsWidget *current_item_;
  Mode mode_;

};