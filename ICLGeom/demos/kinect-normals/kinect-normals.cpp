/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/demos/kinect-normals/kinect-normals.cpp        **
** Module : ICLGeom                                                **
** Authors: Andre Ueckermann                                       **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/


#include <ICLQt/Common.h>
#include <ICLGeom/PointCloudNormalEstimator.h>
#include <ICLUtils/Time.h>

HSplit gui;
GenericGrabber grabDepth, grabColor;

PointCloudNormalEstimator *normalEstimator;

ButtonGroupHandle usedFilterHandle;
ButtonGroupHandle usedSmoothingHandle;
ButtonGroupHandle usedAngleHandle;

Img8u edgeImage(Size(320,240), formatGray);
Img32f angleImage(Size(320,240), formatGray);
Img8u normalImage(Size(320,240), formatRGB);

Camera cam;

void init(){

  Size size = pa("-size");
  edgeImage.setSize(size);
  angleImage.setSize(size);
    
  normalEstimator = new PointCloudNormalEstimator(size);
  
  grabDepth.init("kinectd","kinectd=0");
  grabDepth.setPropertyValue("depth-image-unit","raw");
  grabColor.init("kinectc","kinectc=0");
  grabDepth.useDesired(depth32f, size, formatMatrix);
  grabColor.useDesired(depth8u, size, formatRGB);
  GUI controls = VBox().minSize(12,2);
  controls << Fps(10).handle("fps")
           << FSlider(0.7,1.0,0.89).out("threshold").label("threshold").handle("thresholdHandle")
           << ButtonGroup("unfiltered,median3x3,median5x5").handle("usedFilter")
           << Slider(1,15,2).out("normalrange").label("normal range").handle("normalrangeHandle")
           << Button("disable averaging","enable averaging").out("disableAveraging")
           << ButtonGroup("linear,gauss").handle("usedSmoothing")
           << Slider(1,15,1).out("avgrange").label("averaging range").handle("avgrangeHandle")
           << ButtonGroup("max,mean").handle("usedAngle")
           << Slider(1,15,3).out("neighbrange").label("neighborhood range").handle("neighbrangeHandle")
           << Button("disable CL","enable CL").out("disableCL");
  
  gui << Image().handle("depth").minSize(16,12)
      << Image().handle("color").minSize(16,12)
      << Image().handle("angle").minSize(16,12)
      << Image().handle("edge").minSize(16,12)
      << Image().handle("normal").minSize(16,12)
      << controls
      << Show();
      
  if(pa("-cam")){
    string camname = pa("-cam").as<std::string>();
    cam=Camera(camname);
    cam.setName("Depth Camera");
  }

  usedFilterHandle= gui.get<ButtonGroupHandle>("usedFilter");
  usedFilterHandle.select(1);
  usedSmoothingHandle= gui.get<ButtonGroupHandle>("usedSmoothing");
  usedSmoothingHandle.select(0);
  usedAngleHandle= gui.get<ButtonGroupHandle>("usedAngle");
  usedAngleHandle.select(0);
	
  gui.get<ImageHandle>("depth")->setRangeMode(ICLWidget::rmAuto);
  gui.get<ImageHandle>("angle")->setRangeMode(ICLWidget::rmAuto);
}

void run(){
  Size size = pa("-size");

  const Img8u &colorImage = *grabColor.grab()->asImg<icl8u>();
  const Img32f &depthImage = *grabDepth.grab()->asImg<icl32f>();
    
  if(gui["disableCL"]){
    normalEstimator->setUseCL(false);
  }
  else{
    normalEstimator->setUseCL(true);
  }
  
  int normalrange = gui["normalrange"];
  int neighbrange = gui["neighbrange"];
  float threshold = gui["threshold"];
  int avgrange = gui["avgrange"];
	
  Time start, end;
  start = Time::now();

  usedFilterHandle = gui.get<ButtonGroupHandle>("usedFilter");
  if(usedFilterHandle.getSelected()==1){ //median 3x3
    normalEstimator->setMedianFilterSize(3);
  }
  else if(usedFilterHandle.getSelected()==2){ //median 5x5
    normalEstimator->setMedianFilterSize(5);
  }

  normalEstimator->setNormalCalculationRange(normalrange);	
  normalEstimator->setNormalAveragingRange(avgrange);	
  
  usedSmoothingHandle = gui.get<ButtonGroupHandle>("usedSmoothing");
  usedAngleHandle = gui.get<ButtonGroupHandle>("usedAngle");
  if(usedAngleHandle.getSelected()==0){//max
    normalEstimator->setAngleNeighborhoodMode(0);
  }
  else if(usedAngleHandle.getSelected()==1){//mean
    normalEstimator->setAngleNeighborhoodMode(1);
  }
  else{
    std::cout<<"ANGLE CALCULATION METHOD NOT FOUND"<<std::endl;
  }

  normalEstimator->setAngleNeighborhoodRange(neighbrange);
  normalEstimator->setBinarizationThreshold(threshold);
    
  if(usedFilterHandle.getSelected()==0){//unfiltered
    if(gui["disableAveraging"]){
      edgeImage=normalEstimator->calculate(depthImage, false, false, false);
    }
    else{//normal averaging
      if(usedSmoothingHandle.getSelected()==0){//linear
        edgeImage=normalEstimator->calculate(depthImage, false, true, false);
      }
      else if(usedSmoothingHandle.getSelected()==1){//gauss
        edgeImage=normalEstimator->calculate(depthImage, false, true, true);
      }
    }
  }else{
    if(gui["disableAveraging"]){//filtered
      edgeImage=normalEstimator->calculate(depthImage, true, false, false);
    }else{//normal averaging
      if(usedSmoothingHandle.getSelected()==0){//linear
        edgeImage=normalEstimator->calculate(depthImage, true, true, false);
      }
      else if(usedSmoothingHandle.getSelected()==1){//gauss
        edgeImage=normalEstimator->calculate(depthImage, true, true, true);
      }
    }
  }
    
  //access interim result
  angleImage=normalEstimator->getAngleImage();
  
  if(pa("-cam")){
    normalEstimator->applyWorldNormalCalculation(cam);
    normalImage=normalEstimator->getRGBNormalImage();
  }
  end = Time::now();
  if(normalEstimator->isCLReady()==true && normalEstimator->isCLActive()==true){
    std::cout<<"Size: "<<size<<" ,Open CL, Runtime: ";
  }
  else{ 
    std::cout<<"Size: "<<size<<" ,CPU, Runtime: ";
  }
  std::cout <<(end-start).toMicroSeconds() <<" ms" << endl;

  gui["depth"] = depthImage;
  gui["color"] = colorImage;
  gui["angle"] = angleImage;
  gui["edge"] = edgeImage;
  gui["normal"] = normalImage;

  gui["fps"].render();
}

int main(int n, char **ppc){
  return ICLApp(n,ppc,"-size|-s(Size=QVGA) -cam|-c(file)",init,run).exec();
}
