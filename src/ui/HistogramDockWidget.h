#pragma once
#include <QtGui>
namespace mylib{
#include "array.h"
}

class QCustomPlot;

namespace fetch {
namespace ui {

class HistogramDockWidget: public QDockWidget
{ Q_OBJECT
  QCustomPlot      *plot_;
  size_t            ichan_;
  mylib::Array     *last_;
  bool              is_live_;
  QVector<double>   x_,pdf_,cdf_;
  public:
    HistogramDockWidget(QWidget *parent=NULL);

  signals:
    void change_chan(int ichan);
    
  public slots:
    void set(mylib::Array *im);
    void set_ichan(int ichan);
    void set_live(bool is_live);
  private:
    void swap(mylib::Array *im);
    int  check_chan(mylib::Array *im);
    void compute(mylib::Array *im);

};

}} // namespace fetch::ui
