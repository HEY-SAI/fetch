
#include "types.h"
#include "frame.h"

namespace mylib
{ 
  #include <array.h>
  #include <MY_TIFF\tiff.image.h>
}

namespace mylib
{
  mylib::Value_Type fetchTypeToArrayType(Basic_Type_ID id);
  int fetchTypeToArrayScale(Basic_Type_ID id);
  void castFetchFrameToDummyArray(mylib::Array* dest, fetch::Frame* src, size_t dims[3]);
} //end namespace mylib

namespace mytiff
{
  int islsm(const char* fname);
  Basic_Type_ID pixel_type(mylib::Tiff_Image *tim);
}
