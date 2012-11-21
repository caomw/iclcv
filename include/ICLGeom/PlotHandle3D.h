/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/PlotHandle3D.h                         **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#pragma once

#include <ICLQt/GUIHandle.h>
#include <ICLQt/GUIWidget.h>
#include <ICLGeom/PlotWidget3D.h>

namespace icl{
  namespace geom{
    
    /// Handle class for image components \ingroup HANDLES
    class PlotHandle3D : public qt::GUIHandle<PlotWidget3D>{
      public:
      /// Create an empty handle
      PlotHandle3D(){}
      
      /// create a new ImageHandel
      PlotHandle3D(PlotWidget3D *w, qt::GUIWidget *guiw):qt::GUIHandle<PlotWidget3D>(w,guiw){}
      
      /// re-renders the widget
      inline void render(){
        (***this).render();
      }
  
      // todo: implement several set data method for more convenience
  
      /// callback registration is not supported for this compoment
      virtual void registerCallback(const qt::GUI::Callback &cb, const std::string &events="all"){
        throw utils::ICLException("PlotHandle3D::registerCallback: you cannot register" 
                           " Callbacks to this component");
      }
  
      /// complex callbacks are not allowed for image-components (this method will throw an exception)
      virtual void registerCallback(const qt::GUI::ComplexCallback&, const std::string &){
        throw utils::ICLException("PlotHandle3D::registerCallback: you cannot register "
                           "GUI::ComplexCallback instances to a plot3D GUI component");
      }
    };
  } // namespace qt
}