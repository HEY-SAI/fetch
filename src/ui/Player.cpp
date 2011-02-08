#include <QtGui>
#include <assert.h>
#include "Figure.h"
#include "Player.h"
namespace mylib {
#include <utilities.h>
#include <image.h>
}
#include <frame.h>
#include <util/util-mylib.h>
#include "chan.h"

using namespace ::mylib;

namespace fetch{
namespace ui {

/************************************************************************/
/* IPlayerThread                                                        */
/************************************************************************/
IPlayerThread::IPlayerThread( Figure *w/*=0*/ )
  :w_(NULL)
  ,running_(0)
{
  setFigure(w);
}

void IPlayerThread::setFigure( Figure *w )
{ QMutexLocker lock(&lock_);
  if(w_)
  {
    disconnect(
      this,SIGNAL(imageReady(mylib::Array*)),
      w_ ,SLOT  (imshow(mylib::Array*)));
    w_ = NULL;
  } 
  w_ = w;
  if(w)
  {
    connect(
      this,SIGNAL(imageReady(mylib::Array*)),
      w_ ,SLOT  (imshow(mylib::Array*)),
      Qt::BlockingQueuedConnection);
  }
}

/************************************************************************/
/* ArrayPlayer                                                          */
/************************************************************************/
ArrayPlayer::ArrayPlayer(const char* filename, Figure *w/*=0*/)
  :IPlayerThread(w)
{
  assert(im_=Read_Image(const_cast<char*>(filename),0));
  if(w_==NULL)
  {
    if(im_->ndims>2)
      w_ = imshow(Get_Array_Plane(im_,0));
    else
      w_ = imshow(im_);
    setFigure(w);
  }

  Print_Inuse_List(stderr,1);
}

ArrayPlayer::~ArrayPlayer()
{
  Free_Array(im_);
  Print_Inuse_List(stderr,1);
}

void ArrayPlayer::run()
{ 
  size_t count =0 ;
  int frame = 0;
  int d;
  running_ = 1;
  if(im_->ndims>2)
    d = im_->dims[2];
  else
    return; // nothing to do
  while(running())
  {
    emit imageReady(Get_Array_Plane(im_,frame));
    frame = ++frame%d; //loop
    ++count;
    //qDebug() << count << "\tFrame: " << frame << "\tArray Usage:"<<Array_Usage();
  }
}

/************************************************************************/
/* AsynqPlayer                                                          */
/************************************************************************/

AsynqPlayer::AsynqPlayer( asynq *in, Figure *w/*=0*/ )
  :IPlayerThread(w)
  ,in_(0)  
  ,peek_timeout_ms_(10)
{
  in_ = Asynq_Ref(in);
}

AsynqPlayer::~AsynqPlayer()
{
  Asynq_Unref(in_);
  in_ = NULL;
}

void AsynqPlayer::run()
{
  Frame *buf =  (Frame*)Chan_Token_Buffer_Alloc(in_);
  size_t nbytes  = in_->q->buffer_size_bytes;
  mylib::Array im;
  size_t dims[3];
  running_ = 1;
  // Notes: o Peek copies from current data into frame buffer.
  //        o Peek might realloc the input frame buffer.
  while(running())
  {
    if( Asynq_Peek_Timed(in_,(void**)&buf,nbytes,peek_timeout_ms_) )
    { nbytes = buf->size_bytes();
      castFetchFrameToDummyArray(&im,buf,dims);
      emit imageReady(&im); //blocks until receiver returns
    }
  }
  Chan_Token_Buffer_Free(buf);
}


}} //end fetch::ui