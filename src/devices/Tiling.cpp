#include "Tiling.h"

namespace fetch {

  //////////////////////////////////////////////////////////////////////
  //  StageTiling  /////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  //  Constructors  ////////////////////////////////////////////////////
  StageTiling::StageTiling(const StageTravel         &travel,     
                           const FieldOfViewGeometry &fov,        
                           const Mode                 alignment)
    :
      mask_(NULL),
      latticeToStage_()
  {
    computeLatticeToStageTransform_(fov,alignment);
    initMask_(computeLatticeExtents_(travel));
  }

  //  Destructors  /////////////////////////////////////////////////////
  StageTiling::~StageTiling()
  { if(mask_)           Free_Array(mask_);
  }

  //  computeLatticeToStageTransform_  /////////////////////////////////
  //
  //  For an index vector i, compute T, such that T*i are the nodes of 
  //  the lattice in stage space.
  //
  //  FOV angle should be between 0 and pi/2 (180 degrees).
  void StageTiling::computeLatticeToStageTransform_
                          (const FieldOfViewGeometry &fov,
                           const Mode                 alignment)
  { latticeToStage_ = TTransform::Identity();
    Vector3f sc = Map<Vector3f>(fov.field_size_um_);
    switch(alignment)
    { case PixelAligned:
        // Rotate the lattice
        latticeToStage_
          .rotate(fov.rotation_radians)
          .scale(sc);
        break;
      case StageTravel:
        // Shear the lattice
        float th = fov.rotation_radians;
        latticeToStage_.linear() = 
          (Matrix3f() << 
            1.0f/cos(th), -sin(th), 0,
                       0,  cos(th), 0,
                       0,        0, 1).finished()
          .scale(sc);
        break;
    }
  }
  
  //  computeLatticeExtents_  //////////////////////////////////////////
  //
  //  Find the range of indexes that cover the stage.
  //  This is done by applying the inverse latticeToStage_ transform to the
  //    rectangle described by the stage extents, and then finding extreema.
  //  The latticeToStage_ transfrom is adjusted so the minimal extrema is the
  //    origin.  That is, [min extremal] = T(0,0,0).
  mylib::Coordinate* StageTiling::computeLatticeExtents_(const StageTravel  &travel)
  {
    Matrix<float,8,3> sabox; // vertices of the cube, stage aligned
    sabox 
      << travel.x.min << travel.y.min << travel.z.min
      << travel.x.min << travel.y.max << travel.z.min
      << travel.x.max << travel.y.max << travel.z.min
      << travel.x.max << travel.y.min << travel.z.min

      << travel.x.min << travel.y.min << travel.z.max
      << travel.x.min << travel.y.max << travel.z.max
      << travel.x.max << travel.y.max << travel.z.max
      << travel.x.max << travel.y.min << travel.z.max;

    Matrix<float,3,8> labox; // vertices of the cube, lattice aligned
    labox = latticeToStage_.inverse() * sabox.transpose();

    Vector<float,3> maxs,mins;
    maxs = labox.rowwise().maxCoeff();
    mins = labox.rowwise().minCoeff();

    latticeToStage_.translate(-mins);

  }
} // end namespace fetch
