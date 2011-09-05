#include "VibratomeDockWidget.h"
#include "devices\microscope.h"
#include "AgentController.h"
#include "google\protobuf\descriptor.h"

namespace fetch {
namespace ui {


  //
  // VibratomeDockWidget
  //

  VibratomeDockWidget::VibratomeDockWidget(device::Microscope *dc, MainWindow *parent)
    :QDockWidget("Vibratome",parent)
    ,dc_(dc)
    ,cmb_feed_axis_(NULL)
  {
    QWidget *formwidget = new QWidget(this);
    QFormLayout *form = new QFormLayout;
    formwidget->setLayout(form);
    setWidget(formwidget);

    parent->_vibratome_amp_controller->createLineEditAndAddToLayout(form);
    parent->_vibratome_feed_distance_controller->createLineEditAndAddToLayout(form);
    parent->_vibratome_feed_velocity_controller->createLineEditAndAddToLayout(form);
    parent->_vibratome_feed_axis_controller->createComboBoxAndAddToLayout(form);

    qDebug() << dc->vibratome()->_config->VibratomeFeedAxis_descriptor()->value_count();
    qDebug() << dc->vibratome()->_config->VibratomeFeedAxis_descriptor()->value(0)->name().c_str();
    qDebug() << dc->vibratome()->_config->VibratomeFeedAxis_descriptor()->value(1)->name().c_str();
    
    QHBoxLayout *row = new QHBoxLayout();
    
    QPushButton *b = new QPushButton("&Start",this);
    connect(b,SIGNAL(clicked()),this,SLOT(start()));
    row->addWidget(b);

    b = new QPushButton("Sto&p",this);
    connect(b,SIGNAL(clicked()),this,SLOT(stop()));
    row->addWidget(b);

    form->addRow(row);

    AgentControllerButtonPanel *btns = new AgentControllerButtonPanel(&parent->_scope_state_controller,&dc->cut_task);
    form->addRow(btns);    
  }

  //
  //  VibratomeBoundsModel
  //  - QAbstractDataModel for interacting with a particular protobuf RepeatedField 
  //

    VibratomeBoundsModel::VibratomeBoundsModel(device::Microscope *dc, QObject *parent)
      : QAbstractTableModel(parent)
      , dc_(dc) 
    {}

    Qt::ItemFlags VibratomeBoundsModel::flags       (const QModelIndex &index ) const { return Qt::ItemIsEditable 
                                                                                             | Qt::ItemIsEnabled
                                                                                           //| Qt::ItemIsDragEnabled
                                                                                           //| Qt::ItemIsDropEnabled
                                                                                           //| Qt::ItemIsSelectable
                                                                                             ; }
    int           VibratomeBoundsModel::rowCount    (const QModelIndex &parent) const { return dc_->vibratome()->_config->geometry().bounds_mm().size(); }
    int           VibratomeBoundsModel::columnCount (const QModelIndex &parent) const { return 2; }

    QVariant 
      VibratomeBoundsModel::
      headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const
    { switch(role)
      { case Qt::DisplayRole:
          switch(orientation)
          { case Qt::Horizontal: return section?"Y (mm)":"X(mm)";            
            case Qt::Vertical:   return QString("%1").arg(section);            
            default: break;
          }
          break;
        default: 
          break;
      }
      return QVariant();
    }

    QVariant 
      VibratomeBoundsModel::
      data(const QModelIndex &index, int role /*= Qt::DisplayRole*/ ) const 
    { cfg::device::Point2d p;
      float v;

      // Guard: prefilter roles that don't require data access
      switch(role)
      { case Qt::DisplayRole:
        case Qt::EditRole:
          break;
        case Qt::TextAlignmentRole: return Qt::AlignRight;
        default: goto Error;
      }

      // now access the data
      if(index.row() < rowCount(index.parent()))
        p = dc_->vibratome()->_config->geometry().bounds_mm().Get(index.row());
      else goto Error;

      switch(index.column())
      { case 0: v=p.x(); break;
        case 1: v=p.y(); break;
        default: goto Error;
      }
      
      switch(role)
      { case Qt::DisplayRole: return v;
        case Qt::EditRole: return QString("%1").arg(v);
        default: break;
      }
    Error:
      return QVariant();
    }

    bool  
      VibratomeBoundsModel::
      setData ( const QModelIndex & index, const QVariant & value, int role /*= Qt::EditRole*/ )
    { bool ok = false;
      switch(role)
      { case Qt::EditRole:        
          if(index.row()<rowCount(index.parent()))
          { cfg::device::Point2d *p = dc_->vibratome()->_config->mutable_geometry()->mutable_bounds_mm(index.row());
            switch(index.column())
            { case 0: p->set_x(value.toFloat(&ok)); break;
              case 1: p->set_y(value.toFloat(&ok)); break;
              default:
                goto Error;
            }
          }
          break;
        default:
          break;
      }
      return ok;
    Error:
      return false;
    }

    bool  
      VibratomeBoundsModel::
      insertRows( int row, int count, const QModelIndex & parent /*= QModelIndex()*/ )
    { Eigen::Vector3f v = dc_->stage()->getPos();
      int sz = rowCount(parent)+count;
      cfg::device::VibratomeGeometry *g = dc_->vibratome()->_config->mutable_geometry(); 
      cfg::device::Point2d *o;
      beginInsertRows(parent,row,row+count-1);      
      for(int i=0   ;i<count       ;++i)  g->add_bounds_mm();                              // 1. Append elements        
      for(int i=sz-1;i>=(row+count);--i)  g->mutable_bounds_mm()->SwapElements(i,i-count); // 2. Copy down elements              
      for(int i=0   ;i<count       ;++i)                                                   // 3. Set the first elements to current stage pos (x,y)
      { o = g->mutable_bounds_mm()->Mutable(row+i); 
        o->set_x(v.x());
        o->set_y(v.y());
      }
      endInsertRows();
      return true;
    }

    static inline int mod(int x, int m) {
      return (x%m + m)%m;
    }
    bool  
      VibratomeBoundsModel::
      removeRows ( int row, int count, const QModelIndex & parent /*= QModelIndex()*/ )
    { ::google::protobuf::RepeatedPtrField< cfg::device::Point2d >* r = dc_->vibratome()->_config->mutable_geometry()->mutable_bounds_mm();
      int sz = rowCount(parent);
      beginRemoveRows(parent,row,row+count-1);
      qDebug() << "Remove: " << (row) << "\tCount: " << count;
      for(int i=0;i<count;++i)                    // 1. Circularly permute dudes from the end into interval.
        r->SwapElements(row+i,row+mod(i-1,sz-row));                                                       
      for(int i=0;i<count;++i)  r->RemoveLast();  // 2. Remove the end
      sz-=count;                                  
      for(int i=count-1;i>=0;--i)                 // 3. Reverse circularly permute.
        r->SwapElements(row+i,row+mod(i-1,sz-row));       
      endRemoveRows();
      return true;
    }    

    //
    // VibratomeGeometryDockWidget
    //
    
    VibratomeGeometryDockWidget::VibratomeGeometryDockWidget(device::Microscope *dc, MainWindow* parent)
      : QDockWidget("Vibratome Geometry",parent)
      , dc_(dc)
    { QWidget *formwidget = new QWidget(this);
      QFormLayout *form = new QFormLayout;
      formwidget->setLayout(form);
      setWidget(formwidget);

      t_ = new QTableView(this);
      VibratomeBoundsModel *m = new VibratomeBoundsModel(dc,this);
      t_->setModel(m);
      t_->setContextMenuPolicy(Qt::CustomContextMenu);
      connect(t_, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(geometryCtxMenu(const QPoint &)));

      //connect(parent,SIGNAL(configUpdated()),this,/***/);

      form->addRow(t_);
    }

    void
    VibratomeGeometryDockWidget::geometryCtxMenu(const QPoint &p)
    { QMenu *menu = new QMenu;
      lastCtxMenuPos_ = t_->mapToGlobal(p);
      menu->addAction("Insert Above: Current Position",this,SLOT(insert()));
      menu->addAction("Remove",this,SLOT(remove()));      
      menu->exec(QCursor::pos());
    }

    void VibratomeGeometryDockWidget::insert()
    { QPoint p = t_->mapFromGlobal(lastCtxMenuPos_);
      int row = t_->rowAt(p.y());      
      if(row>-1)
        t_->model()->insertRow(row);
    }

    void VibratomeGeometryDockWidget::remove()
    { QPoint p = t_->mapFromGlobal(lastCtxMenuPos_);
      int row = t_->rowAt(p.y());
      if(row>-1)
        t_->model()->removeRow(row);
    }
}} //end namespace fetch::ui