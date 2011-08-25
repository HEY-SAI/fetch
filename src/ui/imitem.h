#pragma once

#include <QtGui>
#include <QtOpenGL>

namespace mylib {
#include <array.h>
}

namespace fetch {
namespace ui {

class ImItem: public QGraphicsItem
{
public:
  ImItem();
  virtual ~ImItem();
  
  QRectF boundingRect  () const;                                           // in meters
  void   paint         (QPainter                       *painter,
                        const QStyleOptionGraphicsItem *option,
                        QWidget                        *widget = 0);
  void push(mylib::Array *plane);
  void flip(int isupdate=1);

  void autoscale(int chan)                                                {_selected_channel=chan; _autoscale_next=true;}
  void resetscale(int chan)                                               {_selected_channel=chan; _resetscale_next=true;}

         void   setRotation(double radians);
         void   setFOVGeometry(float w_um, float h_um, float rotation_radians);

  inline double rotationRadians()                                         {return _rotation_radians;}  
protected:
  void updateDisplayLists();
  void _common_setup();
  void _loadTex(mylib::Array *im);
  void _setupShader();
  void _updateCmapCtrlPoints();
  void _autoscale(mylib::Array *data, int ichannel, float percent);
  void _resetscale(int ichannel);

  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e);

  float _fill;
  
  QGraphicsSimpleTextItem _text;
  GLuint _hQuadDisplayList;
  GLuint _hTexture;
  GLuint _nchan;                   // updated when an image is pushed
  GLuint _show_mode;

  QGLShaderProgram _shader;
  GLuint _hShaderPlane;
  GLuint _hShaderCmap;
  GLuint _hTexCmapCtrlS;
  GLuint _hTexCmapCtrlT;
  GLuint _hTexCmap;

  GLuint _cmap_ctrl_count;
  GLuint _cmap_ctrl_last_size;
  float *_cmap_ctrl_s,
        *_cmap_ctrl_t;

  QRectF _bbox_um;
  //QSizeF _pixel_size_meters;
  double _rotation_radians;

  int  _loaded;
  bool _resetscale_next;
  bool _autoscale_next;
  int  _selected_channel;
};


}}//end fetch::ui

