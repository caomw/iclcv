#include <string>
#include <map>
#include <iclFileGrabber.h>
#include <iclFileList.h>
#include <iclFilenameGenerator.h>
#include <iclException.h>
#include <iclIOUtils.h>
#include <iclStringUtils.h>
// plugins
#include <iclFileGrabberPluginPNM.h>
#include <iclFileGrabberPluginJPEG.h>
#include <iclFileGrabberPluginCSV.h>


using namespace std;

namespace icl{

  map<string,FileGrabberPlugin*> FileGrabber::s_mapPlugins;

  struct FileGrabberPluginMapInitializer{
    // {{{ open PLUGINS ARE INCLUDED HERE    FileGrabberPluginMapInitializer(){      FileGrabber::s_mapPlugins[".ppm"] = new FileGrabberPluginPNM;        FileGrabber::s_mapPlugins[".pgm"] = new FileGrabberPluginPNM;       FileGrabber::s_mapPlugins[".pnm"] = new FileGrabberPluginPNM;       FileGrabber::s_mapPlugins[".icl"] = new FileGrabberPluginPNM;       FileGrabber::s_mapPlugins[".jpg"] = new FileGrabberPluginJPEG;       FileGrabber::s_mapPlugins[".jpeg"] = new FileGrabberPluginJPEG;       FileGrabber::s_mapPlugins[".csv"] = new FileGrabberPluginCSV; #ifdef WITH_ZLIB_SUPPORT      FileGrabber::s_mapPlugins[".ppm.gz"] = new FileGrabberPluginPNM;       FileGrabber::s_mapPlugins[".pgm.gz"] = new FileGrabberPluginPNM;       FileGrabber::s_mapPlugins[".pnm.gz"] = new FileGrabberPluginPNM;       FileGrabber::s_mapPlugins[".icl.gz"] = new FileGrabberPluginPNM;       FileGrabber::s_mapPlugins[".csv.gz"] = new FileGrabberPluginCSV;       #endif      // add additional plugins to the map    }    ~FileGrabberPluginMapInitializer(){      for(map<string,FileGrabberPlugin*>::iterator it = FileGrabber::s_mapPlugins.begin();           it!= FileGrabber::s_mapPlugins.end(); ++it){        delete it->second;      }    }  };  // }}}

  static FileGrabberPluginMapInitializer ___filegrabber_plugin_map_initializer__;
  
  
  FileGrabber::FileGrabber():m_iCurrIdx(0){}
  
  FileGrabber::FileGrabber(const std::string &pattern, bool buffer, bool ignoreDesired):
    // {{{ open

    m_oFileList(pattern),
    m_iCurrIdx(0),
    m_bBufferImages(false),
    m_bIgnoreDesiredParams(ignoreDesired),
    m_poBufferImage(0){
    
    ICLASSERT_RETURN(!m_oFileList.isNull());
    
    if(buffer){
      bufferImages(false);
    }    
  }

  // }}}

  FileGrabber::~FileGrabber(){  
    // {{{ open    ICL_DELETE(m_poBufferImage);    for(unsigned int i=0;i<m_vecImageBuffer.size();i++){      ICL_DELETE(m_vecImageBuffer[i]);    }  }  // }}}

  const FileList &FileGrabber::bufferImages(bool omitExceptions){
    // {{{ open

    if(!m_vecImageBuffer.size()){
      vector<string> correctNames;
      m_vecImageBuffer.resize(m_oFileList.size());
      std::fill(m_vecImageBuffer.begin(),m_vecImageBuffer.end(),(ImgBase*)0);
      for(int i=0;i<m_oFileList.size();i++){
        if(omitExceptions){
          try{
            grab(&m_vecImageBuffer[i]);
            correctNames.push_back(m_oFileList[i]);
          }catch(ICLException &ex){
            (void)ex;
          }
        }else{
          grab(&m_vecImageBuffer[i]);
          correctNames.push_back(m_oFileList[i]);
        }
      }
      vector<ImgBase*> buf;
      for(unsigned int i=0;i<m_vecImageBuffer.size();++i){
        if(m_vecImageBuffer[i]){
          buf.push_back(m_vecImageBuffer[i]);
        }
      }
      m_vecImageBuffer = buf;
      m_oFileList = FileList(correctNames);
      if(!buf.size()){
        throw ICLException("FileGrabber got a file list without any valid entry");
      }
    }
    m_bBufferImages = true;
    return m_oFileList;
  }

  // }}}

  void FileGrabber::next(){
    // {{{ open    ICLASSERT_RETURN(m_oFileList.size());    m_iCurrIdx++;    if(m_iCurrIdx >= m_oFileList.size()) m_iCurrIdx = 0;  }  // }}}
  void FileGrabber::prev(){
    // {{{ open    ICLASSERT_RETURN(m_oFileList.size());    m_iCurrIdx--;    if(m_iCurrIdx <= 0) m_iCurrIdx = m_oFileList.size()-1;  }  // }}}
  
  unsigned int FileGrabber::getFileCount() const{
    // {{{ open    return m_oFileList.size();  }  // }}}
  
  const std::string &FileGrabber::getNextFileName() const{
    // {{{ open    return m_oFileList[m_iCurrIdx];  }  // }}}
  
  void FileGrabber::setIgnoreDesiredParams(bool flag){
    // {{{ open    m_bIgnoreDesiredParams = flag;  }  // }}}
  
  const ImgBase *FileGrabber::grab(ImgBase **ppoDst){
    // {{{ open

    if(m_bBufferImages){
      ICLASSERT_RETURN_VAL(m_vecImageBuffer.size(),NULL);
      if(!m_bIgnoreDesiredParams){
        ImgBase* useDestImage = prepareOutput(ppoDst);
        m_oConverter.apply(m_vecImageBuffer[m_iCurrIdx++],useDestImage);
        if(m_iCurrIdx >= (int)m_vecImageBuffer.size()) m_iCurrIdx = 0;
        return useDestImage;
      }else{
        ImgBase *p = m_vecImageBuffer[m_iCurrIdx++];
        if(m_iCurrIdx >= (int)m_vecImageBuffer.size()) m_iCurrIdx = 0;
        return p;
      }
    }
    ICLASSERT_RETURN_VAL(!m_oFileList.isNull(),NULL);
    
    File f(m_oFileList[m_iCurrIdx++]);
    if(!f.exists()) throw FileNotFoundException(f.getName());
    if(m_iCurrIdx >= m_oFileList.size()) m_iCurrIdx = 0;
    
    std::map<string,FileGrabberPlugin*>::iterator it = s_mapPlugins.find(toLower(f.getSuffix()));
    if(it == s_mapPlugins.end()){     
      InvalidFileException(string("file type \"*")+f.getSuffix()+"\"");
      return 0;
    }
    if(m_bIgnoreDesiredParams){
      ImgBase** useDestImage = ppoDst ? ppoDst : &m_poBufferImage;
      it->second->grab(f,useDestImage);
      if(f.isOpen()) f.close();
      if(!useDestImage){
        throw InvalidFileException(f.getName());
        return 0;
      }
      return *useDestImage;
    }else{
      it->second->grab(f,&m_poBufferImage);
      if(f.isOpen()) f.close();
      
      if(!m_poBufferImage){
        throw InvalidFileException(f.getName());
        return 0;
      }
      ImgBase* useDestImage = prepareOutput(ppoDst);
      m_oConverter.apply(m_poBufferImage,useDestImage);    
      return useDestImage;        
    }
    

  }

  // }}}
  
}

