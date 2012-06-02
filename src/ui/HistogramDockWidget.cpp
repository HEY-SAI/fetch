/** \file
    Histogram Dock Widget
*/
#include "HistogramDockWidget.h"
#include "qcustomplot.h"
#include "common.h"

#ifdef _MSC_VER
#define restrict __restrict
#else
#define restrict __restrict__ 
#endif

#define ENDL "\r\n"
#define PANIC(e) do{if(!(e)){qFatal("%s(%d)"ENDL "\tExpression evalatuated as false."ENDL "\t%s"ENDL,__FILE__,__LINE__,#e);           }}while(0)
#define FAIL     do{         qFatal("%s(%d)"ENDL "\tNot supposed to get here."ENDL,__FILE__,__LINE__);                                 }while(0)
#define TRY(e)   do{if(!(e)){qDebug("%s(%d)"ENDL "\tExpression evalatuated as false."ENDL "\t%s"ENDL,__FILE__,__LINE__,#e);goto Error;}}while(0)
#define HERE     qDebug("%s(%d). HERE."ENDL,__FILE__,__LINE__)

#define HINT_NBINS (1<<12)

namespace fetch{
namespace ui {

  HistogramDockWidget::HistogramDockWidget(QWidget *parent)
    : QDockWidget("Histogram",parent)
    , plot_(0)
    , ichan_(0)
    , last_(0)
    , x_(HINT_NBINS)
    , pdf_(HINT_NBINS)
    , cdf_(HINT_NBINS)
  { QBoxLayout *layout = new QVBoxLayout;
    { QWidget *w = new QWidget(this);    
      w->setLayout(layout);
      setWidget(w);
    }
    // plot
    plot_=new QCustomPlot;
    //plot_->addGraph();
    //plot_->graph(0)->setPen(QPen(Qt::blue));
    //plot_->addGraph();
    //plot_->graph(1)->setPen(QPen(Qt::red));
    //plot_->graph(0)->setData(x_,pdf_);
    //plot_->graph(1)->setData(x_,cdf_);
    //plot_->graph(0)->rescaleAxes();
    //plot_->graph(1)->rescaleAxes(true);
    layout->addWidget(plot_);
    //layout->addStretch(0);

    QFormLayout *form = new QFormLayout;
    // channel selector
    { QComboBox *c=new QComboBox;
      for(int i=0;i<4;++i) // initially add 4 channels to select
        c->addItem(QString("%1").arg(i),QVariant());
      PANIC(connect(   c,SIGNAL(currentIndexChanged(int)),
                    this,SLOT(set_ichan(int))));
      PANIC(connect(this,SIGNAL(change_chan(int)),
                       c,SLOT(setCurrentIndex(int))));
      form->addRow(tr("Channel:"),c);
    }
    // Live update button
    { QPushButton *b=new QPushButton("Live update");
      b->setCheckable(true);      
      PANIC(connect(   b,SIGNAL(toggled(bool)),
                    this,SLOT(set_live(bool))));
      b->setChecked(true);
      form->addRow(b);
    }
    layout->addLayout(form);
  }
  
// histogram utilities START  
#define TYPECASES(ARRAYARG) do {\
    switch(ARRAYARG->type)  \
    { CASE( UINT8_TYPE ,u8); \
      CASE( UINT16_TYPE,u16);\
      CASE( UINT32_TYPE,u32);\
      CASE( UINT64_TYPE,u64);\
      CASE(  INT8_TYPE ,i8); \
      CASE(  INT16_TYPE,i16);\
      CASE(  INT32_TYPE,i32);\
      CASE(  INT64_TYPE,i64);\
      CASE(FLOAT32_TYPE,f32);\
      CASE(FLOAT64_TYPE,f64);\
      default: \
        FAIL;  \
    }}while(0)

  static double amin(mylib::Array *a)
  { double out;
#define CASE(ID,T) case mylib::ID: {const T *d=(T*)a->data; T m=d[0]; for(size_t i=1;i<a->size;++i) m=(d[i]<m)?d[i]:m; out=(double)m;} break;
    TYPECASES(a);
#undef CASE
    return out;
  }

  static double amax(mylib::Array *a)
  { double out;
#define CASE(ID,T) case mylib::ID: {const T *d=(T*)a->data; T m=d[0]; for(size_t i=1;i<a->size;++i) m=(d[i]>m)?d[i]:m; out=(double)m;} break;    
    TYPECASES(a);
#undef CASE
    return out;    
  }
  
  static unsigned nbins(mylib::Array *a,double min, double max)
  { unsigned n,lim = 1<<12; // max number of bins
    switch(a->type)
    { case mylib::UINT8_TYPE:  lim=1<<8;
      case mylib::INT8_TYPE:   lim=1<<8;
      n=max-min+1;
      return (n<lim)?n:lim;
      
      case mylib::FLOAT32_TYPE:
      case mylib::FLOAT64_TYPE:
        return lim;
      default:
        FAIL;
    }
  }
  
  static double binsize(unsigned n, double min, double max)
  { return (n==0)?0:(((double)n)-1)/(max-min);
  }
  
  template<class T>
  static void count(double *restrict pdf, size_t nbins, T *restrict data, size_t nelem, T min, double dy)
  { for(size_t i=0;i<nelem;++i) pdf[ (size_t)((data[i]-min)*dy) ]++;      
    for(size_t i=0;i<nbins;++i) pdf[i]/=(double)nelem;
  }
  static void scan(double *restrict cdf, double *restrict pdf, size_t n)
  { memcpy(cdf,pdf,n*sizeof(double));
    for(size_t i=1;i<n;++i) cdf[i]+=cdf[i-1];
  }
  static void setbins(double *x, size_t n, double min, double dy)
  { for(size_t i=0;i<n;++i) x[i] = i*dy+min;
  }

  static void histogram(QVector<double> &x, QVector<double> &pdf, QVector<double> &cdf, mylib::Array *a)
  { double min,max,dy;
    unsigned n;
    min=amin(a);
    max=amax(a);
    n=nbins(a,min,max);
    dy=binsize(n,min,max); // value transform is  (data[i]-min)*dy --> for max this is (max-min)*(nbins-1)/(max-min)

    x.resize(n);
    pdf.resize(n);
    cdf.resize(n);
    
#define CASE(ID,T) case mylib::ID: count<T>(pdf.data(),n,(T*)a->data,a->size,min,dy);
    TYPECASES(a);
#undef CASE
    scan(cdf.data(),pdf.data(),n);
    setbins(x.data(),n,min,dy);
  }
 // histogram utilities END

  /** Presumes channels are on different planes */
  void HistogramDockWidget::set(mylib::Array *im)
  { HERE;
    mylib::Array t,*ch;
    TRY(check_chan(im));
    swap(im);    
    TRY(ch=mylib::Get_Array_Plane(&(t=*im),ichan_)); //select channel    
    histogram(x_,pdf_,cdf_,ch);
    plot_->graph(0)->setData(x_,pdf_);
    plot_->graph(1)->setData(x_,cdf_);
    plot_->graph(0)->rescaleAxes();
    plot_->graph(1)->rescaleAxes(true);
    plot_->replot();
 Error:
    ; // bad input, ignore      
  }
  void HistogramDockWidget::set_ichan(int ichan)
  { ichan_=ichan;
    if(last_)
      set(last_);
  }
  void HistogramDockWidget::set_live(bool is_live)
  { is_live_=is_live;
  }
  
  /** updates the last_ array with a new one. */
  void HistogramDockWidget::swap(mylib::Array *im)
  { 
    mylib::Array *t=last_;
    TRY(last_=mylib::Copy_Array(im));
    if(t) mylib::Free_Array(t);
  Error:
    ; // presumably a memory error...not sure what to do, should be rare    
  }

  int HistogramDockWidget::check_chan(mylib::Array *im)
  { int oc=ichan_;
    TRY(im->ndims<=3);
    if( (im->ndims==2) && (ichan_!=0) )
      ichan_=0;
    if( (im->ndims==3) && (ichan_>=im->dims[2]) )
      ichan_=0;
    if(ichan_!=oc)
      emit change_chan(ichan_);
    return 1;
Error:
    return 0;      
  }

}} //namspace fetch::ui
