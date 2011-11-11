/*
 * Microscope.cpp
 *
 * Author: Nathan Clack <clackn@janelia.hhmi.org>
 *   Date: Apr 28, 2010
 */
/*
 * Copyright 2010 Howard Hughes Medical Institute.
 * All rights reserved.
 * Use is subject to Janelia Farm Research Campus Software Copyright 1.1
 * license terms (http://license.janelia.org/license/jfrc_copyright_1_1.html).
 */
 
 
#include "Microscope.h"
#include <time.h>

#include <iostream>
#include <fstream>
#include "stack.pb.h"
#include "microscope.pb.h"
#include "google\protobuf\text_format.h"

#define CHKJMP(expr,lbl) \
  if(!(expr)) \
  { warning("[MICROSCOPE] %s(%d): "ENDL"\tExpression: %s"ENDL"\tevaluated false."ENDL,__FILE__,__LINE__,#expr); \
    goto lbl; \
  }
 
 namespace fetch
{ namespace device {

    Microscope::Microscope()
      :IConfigurableDevice<Config>(&__self_agent)
      ,__self_agent("Microscope",NULL)
      ,__scan_agent("Scanner",&scanner)
      ,__io_agent("IO",&disk)
      ,__vibratome_agent("Vibratome",&vibratome_)
      ,scanner(&__scan_agent)    
      ,stage_(&__self_agent)  
      ,vibratome_(&__vibratome_agent)
      ,fov_(_config->fov())
      ,disk(&__io_agent)
      ,frame_averager("FrameAverager")
      ,pixel_averager("PixelAverager")
      ,cast_to_i16("i16Cast")
      ,inverter("inverter")
      ,wrap()
      ,unwarp()
      ,frame_formatter("FrameFormatter")
      ,trash("Trash")
    {      
      set_config(_config);
      unwarp.setDuty(_config->scanner3d().scanner2d().line_duty_cycle());
      __common_setup();
    }  
    /*
    worker::FrameAverageAgent 	   frame_averager;
    worker::HorizontalDownsampleAgent pixel_averager;
    worker::FrameCastAgent_i16     cast_to_i16;
    worker::FrameInvertAgent       inverter;
    worker::ResonantWrapAgent      wrap;

    worker::TerminalAgent		       trash;    
    */
    Microscope::Microscope( const Config &cfg )
      :IConfigurableDevice<Config>(&__self_agent)
      ,__self_agent("Microscope",NULL)
      ,__scan_agent("Scanner",&scanner)
      ,__io_agent("IO",&disk) 
      ,__vibratome_agent("Vibratome",&vibratome_) 
      ,scanner(&__scan_agent)       
      ,stage_(&__self_agent)
      ,vibratome_(&__vibratome_agent)
      ,fov_(cfg.fov())
      ,disk(&__io_agent)
      ,frame_averager("FrameAverager")
      ,pixel_averager("PixelAverager")
      ,cast_to_i16("i16Cast")
      ,inverter("inverter")
      ,wrap()  
      ,unwarp()   
      ,frame_formatter("FrameFormatter")
      ,trash("Trash")
      ,file_series()
    {
      set_config(cfg);
      unwarp.setDuty(cfg.scanner3d().scanner2d().line_duty_cycle());
      __common_setup();
    }

    Microscope::Microscope(Config *cfg )
      :IConfigurableDevice<Config>(&__self_agent,cfg)
      ,__self_agent("Microscope",NULL)
      ,__scan_agent("Scanner",&scanner)
      ,__io_agent("IO",&disk)  
      ,__vibratome_agent("Vibratome",&vibratome_) 
      ,scanner(&__scan_agent,cfg->mutable_scanner3d())       
      ,stage_(&__self_agent,cfg->mutable_stage())
      ,vibratome_(&__vibratome_agent,cfg->mutable_vibratome())
      ,fov_(cfg->fov())
      ,disk(&__io_agent)
      ,frame_averager(cfg->mutable_frame_average(),"FrameAverager")
      ,pixel_averager(cfg->mutable_horizontal_downsample(),"PixelAverager")
      ,cast_to_i16("i16Cast") 
      ,inverter("Inverter")
      ,wrap(cfg->mutable_resonant_wrap())  
      ,unwarp(cfg->mutable_resonant_unwarp()) 
      ,frame_formatter("FrameFormatter")
      ,trash("Trash")
      ,file_series(cfg->mutable_file_series())
    {
      unwarp.setDuty(cfg->scanner3d().scanner2d().line_duty_cycle());
      __common_setup();
    }


    Microscope::~Microscope(void)
    { 
      if(__scan_agent.detach()) warning("Microscope __scan_agent did not detach cleanly\r\n");
      if(__self_agent.detach()) warning("Microscope __self_agent did not detach cleanly\r\n");
      if(  __io_agent.detach()) warning("Microscope __io_agent did not detach cleanly\r\n");
      if(  __vibratome_agent.detach()) warning("Microscope __vibratome_agent did not detach cleanly\r\n");
    } 

    unsigned int
    Microscope::on_attach(void)
    { 
      // argh this is a confusing way to do things.  which attach to call when.
      //
      // on_attach/on_detach only gets called for the owner, so attach/detach events have to be forwarded
      // to devices that share the agent.  This seems awkward :C
      std::string stackname;

      CHKJMP(     __scan_agent.attach()==0,ESCAN);
      CHKJMP(        stage_.on_attach()==0,ESTAGE);
      CHKJMP(__vibratome_agent.attach()==0,EVIBRATOME);

      stackname = _config->file_prefix()+_config->stack_extension();
      //file_series.ensurePathExists();   

      return 0;   // success
EVIBRATOME:
      stage_.on_detach();
ESTAGE:
      __scan_agent.detach();
ESCAN:
      return 1;


    }
    
    unsigned int
    Microscope::on_detach(void)
    { 
      int eflag = 0; // 0 success, 1 failure
      eflag |= scanner._agent->detach(); //scanner.detach();
      eflag |= frame_averager._agent->detach();
      eflag |= pixel_averager._agent->detach();
      eflag |= inverter._agent->detach();
      eflag |= cast_to_i16._agent->detach();
      eflag |= wrap._agent->detach();
      eflag |= frame_formatter._agent->detach(); 
      eflag |= unwarp._agent->detach();
      eflag |= trash._agent->detach();
      eflag |= disk._agent->detach();

      eflag |= stage_.on_detach();
      eflag |= vibratome_._agent->detach();
      return eflag;  
    }
    
    unsigned int Microscope::on_disarm()
    {
      unsigned int sts = 1; // success      
      sts &= scanner._agent->disarm();
      sts &= frame_averager._agent->disarm();
      sts &= pixel_averager._agent->disarm();
      sts &= inverter._agent->disarm();
      sts &= cast_to_i16._agent->disarm();
      sts &= wrap._agent->disarm();  
      sts &= frame_formatter._agent->disarm(); 
      sts &= unwarp._agent->disarm(); 
      sts &= trash._agent->disarm();
      sts &= disk._agent->disarm();
      sts &= vibratome_._agent->disarm();
      return sts;
    }
    
    const std::string Microscope::stack_filename()
    {     
      return file_series.getFullPath(_config->file_prefix(),_config->stack_extension());
    }  
    
    const std::string Microscope::config_filename()
    {     
      return file_series.getFullPath(_config->file_prefix(),_config->config_extension());
    }        

    const std::string Microscope::metadata_filename()
    {
      return file_series.getFullPath(_config->file_prefix(),_config->metadata_extension());
    }

    void Microscope::write_stack_metadata()
    {      
      //{ std::ofstream fout(config_filename(),std::ios::out|std::ios::trunc|std::ios::binary);
      //  get_config().SerializePartialToOstream(&fout);
      //}      
      { std::ofstream fout(config_filename().c_str(),std::ios::out|std::ios::trunc);
        std::string s;
        Config c = get_config();
        google::protobuf::TextFormat::PrintToString(c,&s);
        fout << s;
        //get_config().SerializePartialToOstream(&fout);
      }
      { float x,y,z;
        std::ofstream fout(metadata_filename().c_str(),std::ios::out|std::ios::trunc);
        fetch::cfg::data::Acquisition data;
        stage_.getPos(&x,&y,&z);
        data.set_x_mm(x);
        data.set_y_mm(y);
        data.set_z_mm(z);        
        std::string s;
        google::protobuf::TextFormat::PrintToString(data,&s);
        fout << s;        
        //get_config().SerializePartialToOstream(&fout);
      }
    }

    void Microscope::_set_config( Config IN *cfg )
    {      
      scanner._set_config(cfg->mutable_scanner3d());
      pixel_averager._set_config(cfg->mutable_horizontal_downsample());
      frame_averager._set_config(cfg->mutable_frame_average());
      wrap._set_config(cfg->mutable_resonant_wrap()); 
      unwarp._set_config(cfg->mutable_resonant_unwarp());       
      vibratome_._set_config(cfg->mutable_vibratome());

      fov_.update(_config->fov());
      stage_._set_config(cfg->mutable_stage());      
    }

    void Microscope::_set_config( const Config& cfg )
    {
      *_config=cfg;         // Copy
      _set_config(_config); // Update
    }

    void Microscope::onUpdate()
    {
      scanner.onUpdate();
      vibratome_.onUpdate();
      fov_.update(_config->fov());
      stage_.setFOV(&fov_);
      unwarp.setDuty(_config->scanner3d().scanner2d().line_duty_cycle());

      file_series.updateDesc(_config->mutable_file_series());

      // update microscope's run state based on sub-agents
      // require scan agent and vibratome agent to be attached
      if( __self_agent.is_attached() && !(__scan_agent.is_attached() && __vibratome_agent.is_attached()))
        __self_agent.detach();
    }

    IDevice* Microscope::configPipeline()
    {
      //Assemble pipeline here
      IDevice *cur;
      cur = &scanner;
      cur =  pixel_averager.apply(cur);
      cur =  frame_averager.apply(cur);
      cur =  inverter.apply(cur);
      cur =  cast_to_i16.apply(cur);
      cur =  wrap.apply(cur);      
      cur =  frame_formatter.apply(cur);
      cur =  unwarp.apply(cur);
      return cur;
    }

    void Microscope::__common_setup()
    {
      __self_agent._owner = this;
      stage_.setFOV(&fov_);
      //configPipeline();
      CHKJMP(_agent->attach()==0,Error);
      CHKJMP(_agent->arm(&interaction_task,this,INFINITE)==0,Error);
    Error:
      return;
    }

    unsigned int Microscope::runPipeline()
    { int sts = 1;                        
      transaction_lock();
      sts &= unwarp._agent->run();
      sts &= frame_formatter._agent->run();
      sts &= wrap._agent->run();
      sts &= cast_to_i16._agent->run();
      sts &= inverter._agent->run();
      sts &= frame_averager._agent->run();
      sts &= pixel_averager._agent->run();
      transaction_unlock();
      return (sts!=1); // returns 1 on fail and 0 on success
    }
    
    unsigned int Microscope::stopPipeline()
    { int sts = 1;
      transaction_lock();
      // These should block till channel's empty 
      sts &= unwarp._agent->stop(2000);
      sts &= frame_formatter._agent->stop();
      sts &= wrap._agent->stop();
      sts &= cast_to_i16._agent->stop();
      sts &= inverter._agent->stop();
      sts &= frame_averager._agent->stop();
      sts &= pixel_averager._agent->stop();      
      transaction_unlock();
      return (sts!=1); // returns 1 on fail and 0 on success
    }

#define TRY(e) \
    if(!(e))                                         \
    do { warning("Expression evalutated as false"ENDL \
              "\t%s(%d)"ENDL                         \
              "\t%s"ENDL,                            \
              __FILE__,__LINE__,#e);                 \
      goto Error;                                    \
    } while(0)

    int Microscope::updateFovFromStackDepth(int nowait)
    { Config c = get_config();
      float s,t;
      f64 ummin,ummax,umstep;
      scanner._zpiezo.getScanRange(&ummin,&ummax,&umstep);
      s = scanner.zpiezo()->getMax() - scanner.zpiezo()->getMin();
      t = vibratome()->thickness_um();
      c.mutable_fov()->set_z_size_um( s );
      c.mutable_fov()->set_z_overlap_um( s-t );
      if(nowait)
        TRY( set_config_nowait(c) );
      else
        set_config(c);

      return (s-t)>0.0;
Error:
      return 0;
    }

    int Microscope::updateStackDepthFromFov(int nowait)
    { Config c = get_config();
      float s,t,o;      
      o = c.fov().z_overlap_um();
      t = vibratome()->thickness_um(); 
      s = t+o;      

      // Awkward:
      // - Ideally, the ZPiezo class would be designed better so we wouldn't 
      //   have to do this.  But we don't have to do this too often, so 
      //   I'm not refactoring ZPiezo yet.
      // - Really, I want one big atomic update of the microscope state.
      //   and I'll get it this way.
      cfg::device::ZPiezo *z = c.mutable_scanner3d()->mutable_zpiezo();
      switch(z->kind())
      { case cfg::device::ZPiezo_ZPiezoType_Simulated:
          z->mutable_simulated()->set_um_min(0.0);
          z->mutable_simulated()->set_um_max(s);
          break;
        case cfg::device::ZPiezo_ZPiezoType_NIDAQ:
          z->mutable_nidaq()->set_um_min(0.0);
          z->mutable_nidaq()->set_um_max(s);
          break;
        default:
          UNREACHABLE;
      }            
      c.mutable_fov()->set_z_size_um( s );
      if(nowait)
        TRY( set_config_nowait(c) );
      else
        set_config(c);

      return (s-t)>0.0;
Error:
      return 0;
    }

    ///////////////////////////////////////////////////////////////////////
    // FileSeries
    ///////////////////////////////////////////////////////////////////////
    
#if 0
#define VALIDATE if(!_is_valid) {warning("(%s:%d) - Invalid location for file series."ENDL,__FILE__,__LINE__);}
#else
#define VALIDATE
#endif

    
    FileSeries& FileSeries::inc( void )
    {
      VALIDATE;
      int n = _desc->seriesno();   
      
      // reset series number when series path changes
      updateDate();                // get the current date
      std::string seriespath = _desc->root() + _desc->pathsep() + _desc->date();
      if(seriespath.compare(_lastpath)!=0)
      { _desc->set_seriesno(0);
        _lastpath = seriespath;
      } else
      { _desc->set_seriesno(n+1);
      }      
      _prev.set_seriesno(_desc->seriesno());
      notify();
      return *this;
    }

    const std::string FileSeries::getFullPath(const std::string& prefix, const std::string& ext)
    {
      VALIDATE;
      char strSeriesNo[32];      
      renderSeriesNo(strSeriesNo,sizeof(strSeriesNo));
      std::string seriespath = _desc->root() + _desc->pathsep() + _desc->date();

      std::string part2 = prefix;
      if(!part2.empty())
        part2 = "-" + prefix;

      return seriespath
        + _desc->pathsep()
        + strSeriesNo
        + _desc->pathsep()
        + strSeriesNo + part2 + ext;
    }

    const std::string FileSeries::getPath()
    {
      VALIDATE;
      char strSeriesNo[32];      
      renderSeriesNo(strSeriesNo,sizeof(strSeriesNo));
      std::string seriespath = _desc->root() + _desc->pathsep() + _desc->date();
    
      return seriespath
        + _desc->pathsep()
        + strSeriesNo
        + _desc->pathsep();
    }

    void FileSeries::updateDate( void )
    {
      time_t clock = time(NULL);
      struct tm *t = localtime(&clock);
      char datestr[] = "0000-00-00";      
      sprintf_s(datestr,sizeof(datestr),"%04d-%02d-%02d",t->tm_year+1900,t->tm_mon+1,t->tm_mday);
      _desc->set_date(datestr);      
      _prev.set_date(_desc->date());
    }
   
    bool FileSeries::updateDesc(cfg::FileSeries *desc)
    { 
      if(_desc)
        desc->set_seriesno(_prev.seriesno()); // keep old series no      
      _desc = desc;
      _prev.CopyFrom(*_desc);
      updateDate();
      //ensurePathExists();
      notify();
      return is_valid();
    }


    bool FileSeries::ensurePathExists()
    {
      std::string s,t;
      char strSeriesNo[32];

      renderSeriesNo(strSeriesNo,sizeof(strSeriesNo));
      updateDate();
      
      tryCreateDirectory(_desc->root().c_str(), "root path", "");

      s = _desc->root()+_desc->pathsep()+_desc->date();
      tryCreateDirectory(s.c_str(), "date path", _desc->root().c_str());

      t = s + _desc->pathsep()+strSeriesNo;
      tryCreateDirectory(t.c_str(), "series path", s.c_str());

      return is_valid();
    }

    void FileSeries::renderSeriesNo( char * strSeriesNo,int maxbytes )
    {
      int n = _desc->seriesno();
      if(n>99999)
        warning("File series number is greater than the supported number of digits.\r\n");
      memset(strSeriesNo,0,maxbytes);
      sprintf_s(strSeriesNo,maxbytes,"%05d",n); //limited to 5 digits
    }

    void FileSeries::tryCreateDirectory( LPCTSTR path, const char* description, LPCTSTR root )
    {
      _is_valid = true;
      if(!CreateDirectory(path,NULL/*default security*/))
      { DWORD err;
        switch(err=GetLastError())
        {
        case ERROR_ALREADY_EXISTS: /*ignore*/ 
          break;
        case ERROR_PATH_NOT_FOUND:
        case ERROR_NOT_READY:
          warning("[FileSeries] %s(%d)"ENDL"\tCould not create %s:"ENDL"\t%s"ENDL,__FILE__,__LINE__,description,path); // [ ] TODO chage this to a warning
          _is_valid = false;
          break;        
        default:
          _is_valid = false;
          warning("[FileSeries] %s(%d)"ENDL"\tUnexpected error returned after call to CreateDirectory()"ENDL"\tFor Path: %s"ENDL,__FILE__,__LINE__,path);
          ReportLastWindowsError();
        }
      }
    }

    void 
      FileSeries::
      notify()
    { TListeners::iterator i;
      std::string path = getPath();
      for(i=_listeners.begin();i!=_listeners.end();++i)
        (*i)->update(path);      
    }

    //notes
    //-----
    // - I can see no reason right now to use transactions
    //   It would prevent against renaming/deleting dependent directories during the transaction
    //   Might be interesting in other places to lock files during acquisition.  However, I think a little
    //   discipline from the user(me) can go a long way.
    //
    // - approach
    //
    //   1. assert <root> exists
    //   2. assert creation or existence of <root>/<date>
    //   3. assert creation of <root>/<data>/<seriesno>
    //   4. return true on success, fail otherwise.
    //see     
    //---
    //
    //MSDN CreateDirectory http://msdn.microsoft.com/en-us/library/aa363855(VS.85).aspx
    //MSDN CreateDirectoryTransacted http://msdn.microsoft.com/en-us/library/aa363855(VS.85).aspx
    //MSDN CreateTransaction http://msdn.microsoft.com/en-us/library/aa366011(v=VS.85).aspx
    //     handle = CreateTransaction(NULL/*default security*/,0,0,0,0,0/*timeout,0==inf*/,"description");


  } // end namespace device  
} // end namespace fetch
