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
	
  QRectF boundingRect  () const;
  void   paint         (QPainter                       *painter,
                        const QStyleOptionGraphicsItem *option,
                        QWidget                        *widget = 0);
	void push(mylib::Array *plane);
  void flip(int isupdate=1);

	inline const QRectF& bbox() {return _bbox;}
signals:
  void sizeChanged(const QRectF& bbox);
protected:
  void updateDisplayLists();
  void _common_setup();
	void _loadTex(mylib::Array *im);
  void _setupShader();

	float _fill;
	
  QRectF _bbox;
  QGraphicsSimpleTextItem _text;
  GLuint _hQuadDisplayList;
	GLuint _hTexture;
  GLuint _nchan;

  QGLShaderProgram _shader;
  GLuint _hShaderPlane;
  GLuint _hShaderCmap;
  GLuint _hTexCmap;

  int _loaded;
};


}}//end fetch::ui
