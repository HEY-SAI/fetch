#include "StageController.h"

#include <QtGui>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <functional>

#include <iostream>

using namespace std;
using namespace Eigen;

#undef HERE
#define HERE "[STAGECONTROLLER] At " << __FILE__ << "("<<__LINE__<<")\n"
#define DBG(e) if(!(e)) qDebug() << HERE << "(!)\tCheck failed for Expression "#e << "\n" 
#define SHOW(e) qDebug() << HERE << "\t"#e << " is " << e <<"\n" 

//////////////////////////////////////////////////////////////////////////
//  TilingController  ///// //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

fetch::ui::TilingController::TilingController( device::StageTiling *tiling, QObject* parent/*=0*/ )
  : QObject(parent)
  , tiling_(tiling)
{ 
  connect(
    &listener_,SIGNAL(sig_tile_done(unsigned,unsigned char)),
    this,      SIGNAL(tileDone(unsigned,unsigned char)),
    Qt::QueuedConnection);
  connect(
    &listener_,SIGNAL(sig_tiling_changed(device::StageTiling*)),
    this,        SLOT(update(device::StageTiling*)),
    Qt::QueuedConnection);
  connect(
    &listener_,SIGNAL(sig_tile_next(unsigned)),
    this,      SIGNAL(nextTileRequest(unsigned)),
    Qt::QueuedConnection);

}


bool fetch::ui::TilingController::fovGeometry( TRectVerts *out )
{ 
  if(tiling_)
  {  
    device::FieldOfViewGeometry fov(tiling_->fov());
    TRectVerts rect;
    rect << 
      -1.0f, 1.0f, 1.0f, -1.0f, // x - hopefully, counter clockwise
      -1.0f,-1.0f, 1.0f,  1.0f, // y
      0.0f, 0.0f, 0.0f,  0.0f; // z
    TTransform t = TTransform::Identity();
    //std::cout << "---" << std::endl << t.matrix() << std::endl << "---" << std::endl;
    t.rotate(AngleAxisf(fov.rotation_radians_,Vector3f::UnitZ()));
    //std::cout << "---" << std::endl << t.matrix() << std::endl << "---" << std::endl;    
    t.scale(fov.field_size_um_);
    //std::cout << "---" << std::endl << t.matrix() << std::endl << "---" << std::endl;    
    *out = t*rect;
    return true;
  }
  return false;
}

bool fetch::ui::TilingController::latticeTransform( TTransform *out )
{ 
  if(tiling_)
  { 
    *out = tiling_->latticeToStageTransform();
    return true;
  }
  return false;
}

bool fetch::ui::TilingController::latticeTransform( QTransform *out )
{ 
  if(tiling_)
  {     
    TTransform latticeToStage;
    latticeTransform(&latticeToStage);    
    Matrix2f m  = latticeToStage.linear().topLeftCorner<2,2>();
    Vector2f dr = latticeToStage.translation().head<2>();
    QTransform t(
      m(0,0), m(0,1), 0.0,
      m(1,0), m(1,1), 0.0,
      dr(0) ,  dr(1), 1.0);
    *out=t;
    return true;
  }
  return false;
}

bool fetch::ui::TilingController::latticeShape( unsigned *width, unsigned *height )
{ 
  if(tiling_)
  {      
    mylib::Array *lattice = tiling_->mask();
    *width  = lattice->dims[0];
    *height = lattice->dims[1]; 
    return true;
  }
  return false; 
}

bool fetch::ui::TilingController::latticeShape(QRectF *out)
{ 
  unsigned w,h;
  if(latticeShape(&w,&h))
  {      
    out->setTopLeft(QPointF(0.0f,0.0f));
    out->setSize(QSizeF(w,h));
    return true;
  }
  return false; 
}

bool fetch::ui::TilingController::stageAlignedBBox(QRectF *out)
{ QTransform l2s;
  if(!latticeShape(out))
    return false;
  latticeTransform(&l2s);    
  *out = l2s.mapRect(*out);
  //SHOW(l2s);
  //SHOW(*out);
  return true;
}

typedef mylib::uint8 uint8;
bool fetch::ui::TilingController::mark( const QPainterPath& path, uint8 attr, QPainter::CompositionMode mode )
{    
  if(tiling_)
  {           
    // 1. Get access to the attribute data
    unsigned w,h;
    latticeShape(&w,&h);
    mylib::Array *lattice = tiling_->mask(),
      plane = *lattice;
    mylib::Get_Array_Plane(&plane,tiling_->plane());  
    QImage im(AUINT8(&plane),w,h,w/*stride*/,QImage::Format_Indexed8);
    QPainter painter(&im);    

    // 2. Path is in scene coords.  transform to lattice coords
    //    Getting the transform is a bit of a pain bc we have to go from 
    //    Eigen to Qt :(
    QTransform l2s, s2l;
    latticeTransform(&l2s);
    s2l = l2s.inverted();        

    // 3. Fill in the path
    im.setColorCount(256);
    for(int i=0;i<256;++i) 
      im.setColor(i,qRgb(i,i,i));    
    painter.setTransform(s2l);
    painter.setCompositionMode(mode);
    painter.fillPath(path,QBrush(qRgb(attr,attr,attr)));
    return true;
  }
  return false;
}

bool fetch::ui::TilingController::markActive( const QPainterPath& path )
{   
  return mark(
    path,
    (uint8)device::StageTiling::Active,
    QPainter::RasterOp_SourceOrDestination);
}     

bool fetch::ui::TilingController::markInactive( const QPainterPath& path )
{   
  return mark(
    path,
    (uint8)device::StageTiling::Active,
    QPainter::RasterOp_NotSourceAndDestination);
}

// returns false if tiling is invalid or if stage_coord is oob
float roundf(float x) {return floor(0.5f+x);}
bool fetch::ui::TilingController::mapToIndex(const Vector3f & stage_coord, unsigned *index)
{
  TTransform t;  
  if(latticeTransform(&t))
  { unsigned w,h;
    Vector3f lr = t.inverse()*stage_coord;
    lr.unaryExpr(ptr_fun(roundf));
    latticeShape(&w,&h);
    QRectF bounds(0,0,w,h);
    if( bounds.contains(QPointF(lr(0),lr(1))) )
    {
      *index = lr(1)*w+lr(0);
      return true;
    }
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////
//  PlanarStageController   //////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
fetch::ui::PlanarStageController::PlanarStageController( device::Stage *stage, QObject *parent/*=0*/ )
   : QObject(parent)
   , stage_(stage)
   , agent_controller_(stage->_agent)
   , tiling_controller_(NULL)
{
  connect(
    &agent_controller_, SIGNAL(onAttach()),
    this,SLOT(updateTiling()) );

  connect(
    &agent_controller_, SIGNAL(onDetach()),
    this,SLOT(invalidateTiling()) );

  agent_controller_.createPollingTimer()->start(50 /*msec*/);
}
