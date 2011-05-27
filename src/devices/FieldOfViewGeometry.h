#pragma once
#include <Eigen/Core>
#include "microscope.pb.h"

using namespace Eigen;


namespace fetch {
namespace device {

//////////////////////////////////////////////////////////////////////
//  FieldOfViewGeometry  /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

struct FieldOfViewGeometry
{ 
  Vector3f field_size_um_;
  Vector3f overlap_um_;
  float    rotation_radians_;

  FieldOfViewGeometry(float x, float y, float z, float radians) :
        field_size_um_(x,y,z), rotation_radians_(radians) {}
  FieldOfViewGeometry(const cfg::device::FieldOfViewGeometry &cfg) {update(cfg);}

  void update(const cfg::device::FieldOfViewGeometry &cfg)
       { 
         field_size_um_ << cfg.x_size_um(),cfg.y_size_um(),cfg.z_size_um();
         overlap_um_ << cfg.x_overlap_um(),cfg.y_overlap_um(),cfg.z_overlap_um();
         rotation_radians_ = cfg.rotation_radians();           
       }
    
};

}} // end namespace fetch::device