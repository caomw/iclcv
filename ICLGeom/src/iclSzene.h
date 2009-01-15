#ifndef ICL_SZENE_H
#define ICL_SZENE_H

#include "iclObject.h"
#include "iclCamera.h"
#include <vector>
#include <iclSize.h>
#include <iclUncopyable.h>

// the icl namespace
namespace icl{
  
  /** \cond */
  class ICLDrawWidget;
  /** \endcond*/
  
  /// class for a 3D Szene containing objects and a camera
  /** The szene class is top level class for 3D geometry handling
      using the ICLGeom package.
      
      A Szene has a virtual camera and a list of all currently available
      Objects. After Objects have been added, the szene cam be updated.
      This must be done each time, when 
      - Objects have been changed
      - The camera has been changed

      E.g.
      \code
      Size size(640,480);
      Szene s(size);
      s.add(new CubeObject(...));
      s.cam.transform(...));
      s.update();
      Img32s dst(size,formatRGB);
      s.render(&dst);
      \endcode
      
      */
  class Szene : public Uncopyable{
    public:
    /// Create a new Szene with give view-port size
    /** The view-port-size is given to the internally 
        created camera
        @param viewPort view-port size, which must be equal
                         to the view-port (image-size) of the
                         given ICLDrawWidget if the corresponding
                         render-function is called, resp. the 
                         size of the Img32f for the other render-
                         function.
        @param cam TODO
    */
    Szene(const Rect &viewPort=Rect(0,0,320,240),const Camera &cam=Camera() );
    
    /// Copy constructor (contained objects are not copied)
    Szene(const Szene &other){
      *this = other;
    }

    /// Destructor
    ~Szene();
    
    /// Assignment operator (contained objects are not copied)
    /** internal objects are deleted */
    Szene &operator=(const Szene &other);

    /// returns the szenes camera
    Camera &getCam(){ return m_oCam; }

    /// returns the szenes camera
    const Camera &getCam() const{ return m_oCam; }
    
    /// renders the szene into the given draw-widget
    void render(ICLDrawWidget *w) const;

    /// renders the szene into the given image
    void render(Img32f *image) const;
    
    /// adds a new Object to the szene (objects ownership is passed)
    void add(Object *obj);
    
    /// removes this Object from the szene (object is released internally)
    void remove(Object *obj);
    
    /// updates all objects in the szene
    /** This function has to be called after 
        -# the camera has been changed or
        -# objects have been changed or added */
    void update();
    
    /// passes the given matrix to all objects transform(.)-function
    void transformAllObjs(const Mat &m);

    /// transforms the szene transformation matrix
    void transform(const Mat &m){
      m_oTransMat *= m;
    }
    
    /// resets the current szene transformation matrix
    void resetTransformation() {
      m_oTransMat = Mat::id();
    }

    /// sets the current view port
    void setViewPort(const Rect &viewPort){
      m_oViewPort = viewPort;
    }
    /// returns the current view port
    const Rect &getViewPort() const{
      return m_oViewPort;
    }
    /// returns the current szene transformation matrix
    const Mat &getTransformation() const {
      return m_oTransMat;
    }

    /// returns the current viewport matrix
    Mat getViewPortMatrix() const;
    
    /// shows the current transformation matrices to std::out
    void showMatrices(const std::string &title="") const;
    private:

    /// current view port
    Rect m_oViewPort;
    
    /// szene camera
    Camera m_oCam;
    
    /// list of currently available szene objects
    std::vector<Object*> m_vecObjs;
    
    /// current szene trans formation (id by default)
    Mat m_oTransMat;
  };
  
}

#endif
