#ifndef ICL_TAB_HANDLE_H
#define ICL_TAB_HANDLE_H

#include <QTabWidget>
#include <QLayout>
#include <ICLQt/GUIHandle.h>
#include <ICLQt/ContainerHandle.h>
namespace icl{
  
  /// A Handle for TabWidget container GUI components  \ingroup HANDLES
  class TabHandle : public GUIHandle<QTabWidget>, public ContainerHandle{
    public:
    /// create an empty handle
    TabHandle(): GUIHandle<QTabWidget>(){}
    
    /// create a difined handle
    TabHandle(QTabWidget *w, GUIWidget *guiw):GUIHandle<QTabWidget>(w,guiw){}
    
    /// adds an external compnent to the tab widget
    virtual void add(QWidget *comp, const QString &tabName){ 
      (**this)->addTab(comp,tabName); 
    }

    /// inserts a widget at givel location
    virtual void insert(int idx, QWidget *comp, const QString &tabName){ 
      (**this)->insertTab(idx,comp,tabName);
    }
  };  
}

#endif