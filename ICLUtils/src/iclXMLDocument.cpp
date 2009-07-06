#include <iclXMLDocument.h>
#include <iclXMLNode.h>
#include <iclStringUtils.h>
#include <sstream>
#include <fstream>
#include <list>
#include <string.h>

namespace icl{
# if 0
  static std::string get_next_tag(std::istream &is, std::ostringstream &rest){
    char c;
    do{ 
      if(!is.good()) throw ParseException("unexpected stream end!");
      c = is.get();
      if(c != '<'){
        rest << c; 
      }
    } while(c != '<');
    
    std::ostringstream os;
    os << c;
    try{
      do{ 
        if(!is.good()) throw ParseException("unexpected stream end!");
        c = is.get(); 
        os << c;  
        if(os.str().size()==4 && os.str() == "<!--"){
          throw 1;
        }
      } while(c != '>');
    }catch(int){
      // searching for the corresponding comment end "-->"
      std::string cs = "!--"; // always the last 3
      while(cs != "-->"){
        if(!is.good()) throw ParseException("unexpected stream end (within comment node: " +os.str() +")");
        c = is.get();
        cs = cs.substr(1)+c;
        os << c;
      }
    }
#else
  static std::string get_next_tag(std::istream &is, std::ostringstream &rest){
    char c;
    do{ 
      if(!is.good()) throw ParseException(__FUNCTION__,str(__LINE__),"(document structure not complete,"
                                          " input stream ended while still tags open)");
      c = is.get();
      if(c != '<'){
        rest << c; 
      }
    } while(c != '<');
    
    std::ostringstream os;
    os << c;
    
    bool inAttribValue = false;
    try{
      do{ 
        if(!is.good()) throw ParseException(__FUNCTION__,str(__LINE__),"(stream ended within a tag! already read this: '"+
                                            os.str() + "')");
        c = is.get(); 
        os << c;  
        if(os.str().size()==4 && os.str() == "<!--"){
          throw 1;
        }
        if(c == '"'){
          inAttribValue = !inAttribValue;
        }
      } while(inAttribValue || c != '>');
    }catch(int){
      // searching for the corresponding comment end "-->"
      std::string cs = "!--"; // always the last 3
      while(cs != "-->"){
        if(!is.good()) throw ParseException(__FUNCTION__,str(__LINE__),"(stream ended within comment! already read this: '"+
                                            os.str() + "')");
        c = is.get();
        cs = cs.substr(1)+c;
        os << c;
      }
    }
#endif
    
    
    return os.str();
  }
  
  /// XXXX
  struct SimpleNode{
    enum Type {UnknownTag, XMLVersionTag, CommentTag, OpenTag, CloseTag, SingleTag, Text};
    SimpleNode(){}
    SimpleNode(const std::string &text,Type type=UnknownTag, XMLAttMapPtr att=0):
      type(type),text(text),att(att){}

    Type type;
    std::string text;
    XMLAttMapPtr att;

    friend std::ostream &operator<<(std::ostream &s, const SimpleNode &node){
      static std::string names[]={"UnknownTag", "XMLVersionTag", "CommentTag", "OpenTag", "CloseTag", "SingleTag", "Text"};
      s << "Node["<<names[node.type]<<"]{"<<node.text<<'}';
      if(node.att && node.att->size()){
        s << "(Att:";
        for(XMLNode::const_attrib_iterator it = node.att->begin(); it != node.att->end();){
          s << it->first << "=" << it->second;
          ++it;
          if(it != node.att->end()){
            s << ',';
          }else{
            s << ")";
          }
        }
      }
      return s;
    }
  };
  
  static const char &front(const std::string &s){
    return s.at(0);
  }
  static const char &back(const std::string &s){
    return s.at(s.length()-1);
  }

  
  SimpleNode::Type remove_tag_braces_and_get_type(std::string &tag_text){
    if(front(tag_text) != '<') throw ParseException(__FUNCTION__,str(__LINE__),"(missing '<' at beginning of tag '" + tag_text + "')"); 
    if(back(tag_text) != '>') throw ParseException(__FUNCTION__,str(__LINE__),"(missing '>' at end of tag '" + tag_text + "')"); 
    if(tag_text.length() < 3) throw ParseException(__FUNCTION__,str(__LINE__),"(tag empty '<>')");
    tag_text = tag_text.substr(1,tag_text.length()-2);
    if(tag_text[0] == '/'){
      tag_text = tag_text.substr(1);
      return SimpleNode::CloseTag;
    }else if(tag_text[0] == '?'){
      if(back(tag_text) != '?') throw ParseException(__FUNCTION__,str(__LINE__),"(xml description tag must end with '?>')");
      tag_text = tag_text.substr(1,tag_text.length()-2);
      return SimpleNode::XMLVersionTag;
    }
    else if(back(tag_text) == '/'){
      tag_text = tag_text.substr(0,tag_text.length()-1);
      return SimpleNode::SingleTag;
    }
    else if(tag_text[0] == '!'){
      int len = (int)tag_text.length();
      if(len < 5) throw ParseException(__FUNCTION__,str(__LINE__),"(error in comment node: syntax is '<!-- comment -->')");
      if(tag_text[1] != '-' || tag_text[2] != '-' ||
         tag_text[len-1] != '-' || tag_text[len-2] != '-'){
        throw ParseException(__FUNCTION__,str(__LINE__),"(error in comment node: syntax is '<!-- comment -->')");
      }
      tag_text = tag_text.substr(3,tag_text.length()-5);
      return SimpleNode::CommentTag;
    }
    return SimpleNode::OpenTag;
  }
#if 0  
  static std::string cut_quotes(const std::string &s){
    if(!s.size()) return "";
    bool f = s[0] == '"';
    bool b = s[s.length()-1] == '"';
    if(f){
      if(b)return s.substr(1,s.length()-2);
      else return s.substr(1);
    }
    if (b) return s.substr(0,s.length()-1);
    return s;
  }


  static XMLAttMapPtr split_tag_name_and_attribs(const std::string &tag_text, std::string &tag_name){
    XMLAttMapPtr att = new std::map<std::string,std::string>;

    std::istringstream is(tag_text);

    is >> tag_name;
    try{
      while(true){
        if(!is.good()) throw std::exception();
        std::string a;
        is >> a;
        std::string::size_type eqpos = a.find('=',0);
        if(eqpos != std::string::npos){
          (*att.get())[a.substr(0,eqpos)] = cut_quotes(a.substr(eqpos+1));
          continue;
        }
        std::string n;
        is >> n;
        if(!n.size()){
          // no more tokens available (except some whitespaces)
          // if(is.good()) throw 100;
          // }else{
          throw std::exception();
        }
        if(n[0] != '=') throw 101;
        if(n.size() > 1){
          (*att.get())[a] = cut_quotes(n.substr(1));
          continue;
        }
        is >> n;
        (*att.get())[a] = n;
      }
    }catch(std::exception &ex){
      return att;
    }catch(int i){
      throw ParseException(str("Camera [Code ")+ str(i) +"]");
    }
  }
#else
  
  static inline bool is_letter(char c){
    return isalnum(c);
  }
  
  static inline bool is_whitespace(char c){
    return isspace(c);
  }

  
  

  
  static XMLAttMapPtr split_tag_name_and_attribs(const std::string &tag_text, std::string &tag_name){
    XMLAttMapPtr att = new std::map<std::string,std::string>;

    std::istringstream is(tag_text);
    //#define XXX std::cout << "["<<c << "]int(" << (int)c << "){"<< __LINE__ <<"}"<< std::endl;
#define XXX
    is >> tag_name;
    try{
      while(true){
        char c = is.get();
        XXX;
        if(c == -1) return att;
        while(is_whitespace(c)){
          c = is.get();
          XXX;
          if(c == -1) return att;
        }
        if(c=='=') throw int(__LINE__);
        std::string attrib;
        while(is_letter(c) && c != '='){
          attrib += c;
          c = is.get();
          XXX;
          if(c == -1) throw int(__LINE__);
        }
        
        while(c != '='){
          c = is.get();
          XXX;
          if(c == -1) throw int(__LINE__);
          if(c != '=' && !is_whitespace(c)){
            throw int(__LINE__);
          }
        }
        
        while(c != '"'){
          c = is.get();
          XXX;
          if(c == -1) throw int(__LINE__);
          if(!is_whitespace(c) && (c != '"')) throw int(__LINE__);
        }
        std::string value;
        bool first = true;
        while(first || c != '"'){
          first = false;
          c = is.get();
          XXX;
          if(c == -1) throw int(__LINE__);
          if(c != '"') value += c;
        }
        (*att.get())[attrib] = value;
      }
    }catch(int line){
      throw ParseException(__FUNCTION__,str(line),"(tag-text:" + tag_text + ")");
    }
  }

#endif
  static void add_intermediate_node(std::list<SimpleNode> &L,std::string tag_text, SimpleNode::Type t){
    switch(t){
      case SimpleNode::CommentTag:
      case SimpleNode::CloseTag:
      case SimpleNode::XMLVersionTag:
      case SimpleNode::Text:
        L.push_back(SimpleNode(tag_text,t));
        break;
      case SimpleNode::OpenTag:
      case SimpleNode::SingleTag:{
        std::string tag_name;
        XMLAttMapPtr att=split_tag_name_and_attribs(tag_text,tag_name);
        L.push_back(SimpleNode(tag_name,t,att));
        break;
      }
      default:
        throw ParseException("found unknown node type!");
    }
  }
  
  static void parse_intermediate_rek(std::istream &is, std::list<SimpleNode> &L, const std::string &root_tag_text, int &openCnt){
    std::ostringstream rest;
    std::string tag_text;
    try{
      tag_text = get_next_tag(is,rest);
    }catch(ParseException &ex){
      if(L.size()){
        throw ParseException("unexpected end of stream (last node was " + str(L.back()) + ")");
      }
    }
    SimpleNode::Type t = remove_tag_braces_and_get_type(tag_text);
    if(t == SimpleNode::CloseTag){
      if(L.back().type == SimpleNode::OpenTag){
        add_intermediate_node(L,rest.str(),SimpleNode::Text);
      }
    }
    add_intermediate_node(L,tag_text,t);
    switch(t){
      case SimpleNode::XMLVersionTag:
        throw ParseException("XMLVersion tag must not occur within root tag");
        break;
      case SimpleNode::OpenTag:{
        if(tag_text == root_tag_text) openCnt++;
        parse_intermediate_rek(is, L, root_tag_text, openCnt);
        break;
      }
      case SimpleNode::CloseTag:
        if(tag_text == root_tag_text) openCnt--;
        if(openCnt){
          parse_intermediate_rek(is, L, root_tag_text, openCnt);
        }
        break;
      case SimpleNode::SingleTag:
      case SimpleNode::CommentTag:
        parse_intermediate_rek(is, L, root_tag_text, openCnt);
        break;
      default:
        throw ParseException("Somehow the parse extracted an unknown or text node type");
    }
  }
  
  static void parse_intermediate(std::istream &is, std::list<SimpleNode> &L){
    std::ostringstream rest;
    std::string root_tag_text = get_next_tag(is,rest);
    SimpleNode::Type t = remove_tag_braces_and_get_type(root_tag_text);
    add_intermediate_node(L,root_tag_text,t);
    switch(t){
      case SimpleNode::XMLVersionTag:
        if(L.size()>1) throw ParseException("XMLVersion Tag is only allowed as first Node");
        parse_intermediate(is,L);
        break;
      case SimpleNode::CommentTag:
        parse_intermediate(is,L);
        break;
      case SimpleNode::OpenTag:{
        int openCnt = 1;
        parse_intermediate_rek(is, L, root_tag_text, openCnt);
        break;
      }
      case SimpleNode::CloseTag:
        throw ParseException("Found close tag at beginning");
        break;
      case SimpleNode::SingleTag:
        break;
      default:
        throw ParseException("Found unknown or text tag at beginning");
    }
  }
  
 
 
  
  void XMLDocument::parse_it(XMLNode *root, std::istream &src, void *state){
    std::list<SimpleNode> &L = *reinterpret_cast<std::list<SimpleNode>*>(state); 

    XMLNode *parent = root;

    while(L.size() && parent){
      SimpleNode n = L.front();      
      L.pop_front();

      switch(n.type){
        case SimpleNode::OpenTag:
          parent->m_subnodes.push_back(new XMLNode(parent,parent->m_document));
          parent->m_type = XMLNode::CONTAINER;
          parent = parent->m_subnodes.back().get();
          parent->m_attribs = n.att;
          parent->m_tag = n.text;
          break;
        case SimpleNode::SingleTag:
          parent->m_subnodes.push_back(new XMLNode(parent,parent->m_document));
          parent->m_subnodes.back()->m_attribs = n.att;
          parent->m_subnodes.back()->m_tag = n.text;
          parent->m_subnodes.back()->m_type = XMLNode::SINGLE;
          break;
        case SimpleNode::CommentTag:
          parent->m_subnodes.push_back(new XMLNode(parent,parent->m_document));
          parent->m_subnodes.back()->m_comment = n.text;
          parent->m_subnodes.back()->m_type = XMLNode::COMMENT;
          break;
        case SimpleNode::Text:
          if(parent->m_text != "")  throw ParseException("Found 2nd text content for node: " + parent->m_tag);
          parent->m_text = n.text;
          parent->m_type = XMLNode::TEXT;

          break;
        case SimpleNode::CloseTag:
          if(parent->m_tag != n.text) throw ParseException(str("found closing tag ") + 
                                                           str(n.text) + " but open tag was " + parent->m_tag);
          parent = parent->m_parent;
          break;
        case SimpleNode::XMLVersionTag:
          throw ParseException("XML-Description tag is only allowes as document header");
        case SimpleNode::UnknownTag:
          throw ParseException("found unknown tag type");
      }
    }
    if(!parent) throw ParseException("parent got NULL ???");
    if(parent != root) throw ParseException("document tree structure incomplete");
  }
  //    enum Type {UnknownTag, XMLVersionTag, CommentTag, OpenTag, CloseTag, SingleTag, Text};

  void XMLDocument::parse(XMLNode *instance, std::istream &src,
                          std::vector<std::string> &headerComments,
                          std::string &xmlVersionDescription){
    xmlVersionDescription = "";
    std::list<SimpleNode> L;
    icl::parse_intermediate(src,L);

    if(!L.size()) throw ParseException("Document is empty");

    if(L.front().type == SimpleNode::XMLVersionTag){
      xmlVersionDescription = L.front().text;
      L.pop_front();
    }
    while(L.front().type == SimpleNode::CommentTag){
      headerComments.push_back(L.front().text);
      L.pop_front();
    }
    if(!L.size()) throw ParseException("only comment tags found");

    SimpleNode n = L.front();    
    L.pop_front();
    switch(n.type){
      case SimpleNode::OpenTag:
        instance->m_attribs = n.att;
        instance->m_tag = n.text;
        if(L.back().type != SimpleNode::CloseTag) throw ParseException("Root node's closing tag not found (found:" + str(L.back().text) +')');
        if(L.back().text != n.text) throw ParseException("Root node's closing tag does not match");
        L.pop_back();
        
        if(L.front().type == SimpleNode::Text){
          if(L.size() != 1) throw ParseException("Text and container subnodes must not be mixed (for root nodes as well)");
          instance->m_text = L.front().text;
          instance->m_type = XMLNode::TEXT;
          L.pop_front();
        }else{
          instance->m_type = XMLNode::CONTAINER;
          parse_it(instance,src,&L);
        }
        break;
      case SimpleNode::SingleTag:
        instance->m_attribs = L.front().att;
        instance->m_tag = L.front().text;
        instance->m_type = XMLNode::SINGLE;
        break;
      case SimpleNode::Text:
        throw ParseException("Root node must not be of type 'text'");
        break;
      case SimpleNode::CloseTag:
        throw ParseException("found close tag as root node");
        break;
      case SimpleNode::CommentTag:
        throw ParseException("found comment tag as root node");
        break;
      case SimpleNode::XMLVersionTag:
        throw ParseException("XML description is only allowed as document header!");
      case SimpleNode::UnknownTag:
        throw ParseException("Found unknow root node tag type");

    }
  }

  
  void XMLDocument::init(std::istream &is){
    m_root = new XMLNode(0,this);
    parse(m_root.get(),is,m_headerComments,m_xmlVersion);
  }
  XMLDocument::XMLDocument(){}
  
  XMLDocument::XMLDocument(std::istream &is) throw (ParseException){
    init(is);
  }
  XMLDocument::XMLDocument(const std::string &text) throw (ParseException){
    std::istringstream is(text);
    init(is);
  }

  XMLNodePtr XMLDocument::getRootNode(){
    return m_root;
  }

  const XMLNodePtr XMLDocument::getRootNode() const{
    return m_root;
  }

  XMLNode* XMLDocument::operator->() throw (NullDocumentException){
    if(isNull()) throw NullDocumentException(__FUNCTION__);
    return m_root.get();
  }
  
  /// provides direct access to the documents root node (const)
  const XMLNode* XMLDocument::operator->() const throw (NullDocumentException){
    if(isNull()) throw NullDocumentException(__FUNCTION__);
    return m_root.get();
  }


  std::ostream &operator<<(std::ostream &os, const XMLDocument &doc) throw (NullDocumentException){
    if(doc.m_xmlVersion != "") os << "<?" << doc.m_xmlVersion << "?>" << std::endl;
    for(unsigned int i=0;i<doc.m_headerComments.size();++i){
      os << "<!--" << doc.m_headerComments[i] << "-->" << std::endl;
    }
    doc.getRootNode()->serialize(os,0); 
    return os;
  }
  
  bool XMLDocument::isNull() const {
    return !m_root;
  }

  XMLNode &XMLDocument::operator[](const std::string &tag) throw (NullDocumentException, ChildNotFoundException){
    if(isNull()) throw NullDocumentException("document was null");
    if(m_root->getTag() != tag) throw ChildNotFoundException("root nodes tag is " + m_root->m_tag + " and not " + tag);
    return *m_root;
  }
  const XMLNode &XMLDocument::operator[](const std::string &tag) const throw (NullDocumentException, ChildNotFoundException){
    return const_cast<XMLDocument*>(this)->operator[](tag);
  }
  
  XMLDocument::XMLDocument(const XMLDocument &other):m_root(0){
    
    *this = other;
  }
  
  XMLDocument &XMLDocument::operator=(const XMLDocument &other){
    if(other.isNull()){ 
      m_root = XMLNodePtr(0);
      m_headerComments.clear();
      m_xmlVersion="";
    }else{
      m_root = other.m_root->deepCopy(0,this);
      m_headerComments=other.m_headerComments;
      m_xmlVersion = other.m_xmlVersion;
    }
    return *this;
  }

  void XMLDocument::removeAllComments(){
    m_headerComments.clear();
    m_root->removeAllComments();
  }
  
  XMLDocument XMLDocument::load(const std::string &fileName) throw (ParseException,FileNotFoundException){
    std::ifstream f(fileName.c_str(),std::ifstream::binary);
    if(!f) throw FileNotFoundException("file " + fileName + " was not found");
    return XMLDocument(f);
  }
  
  void XMLDocument::save(const XMLDocument &doc, const std::string &fileName) throw (ICLException,NullDocumentException){
    if(doc.isNull()) throw NullDocumentException("unable to save null document");
    std::ofstream f(fileName.c_str());
    try{
      f << doc;
    }catch(const std::exception &ex){
      throw ICLException(ex.what());
    }
  }



}