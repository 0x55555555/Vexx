#ifndef SPROPERTY_H
#define SPROPERTY_H

#include "sglobal.h"
#include "XObject"
#include "schange.h"
#include "spropertyinformation.h"

class SEntity;
class SProperty;
class SPropertyContainer;
class SPropertyMetaData;
class SDatabase;

#define REGISTER_TYPE_FUNCTION(typeId, myName, createFn, saveFn, loadFn, assignFn, parentInfo, childOffset, childCount, childData) \
  static const SPropertyInformation * staticTypeInformation() { \
  static SPropertyInformation info = { myName::createFn, \
  saveFn, \
  loadFn, \
  assignFn, \
  0, \
  #myName, \
  typeId, \
  parentInfo, \
  childOffset, \
  childCount, \
  childData, \
  sizeof(myName), \
  0, \
  false }; return &info;}

#define S_PROPERTY_ROOT(myName, saveFn, loadFn, assignFn) \
  private: \
  static SProperty *create##myName(void *ptr) { return new(ptr) myName(); } \
  public: \
  enum { Type = Types::myName }; \
  REGISTER_TYPE_FUNCTION(Type, myName, create##myName, saveFn, loadFn, assignFn, 0, 0, 0, 0 )

#define S_PROPERTY(myName, superName, saveFn, loadFn, assignFn) \
  private: \
  static SProperty *create##myName(void *ptr) { return new(ptr)  myName(); } \
  public: \
  enum { Type = Types::myName }; \
  REGISTER_TYPE_FUNCTION(Type, myName, create##myName, saveFn, loadFn, assignFn, superName::staticTypeInformation(), 0, 0, 0 ) \

class SHIFT_EXPORT SProperty
  {
  S_PROPERTY_ROOT(SProperty, save, load, blankAssign)

public:
  SProperty();
  virtual ~SProperty() { }

  void assign(const SProperty *propToAssign);

  // get the parent entity for this attribute
  // or if this attribute is an entity, get it.
  SEntity *entity() const;

  SPropertyContainer *parent() const {return _parent;}

  SProperty *input() const {preGet(); return _input;}
  SProperty *output() const {preGet(); return _output;}
  SProperty *nextOutput() const {preGet(); return _nextOutput;}

  // connect this property (driver) to the passed property (driven)
  void connect(SProperty *) const;
  void disconnect(SProperty *) const;
  void disconnect() const;

  bool hasInput() const { preGet(); return _input; }
  bool hasOutputs() const { preGet(); return _output; }

  template <typename T> T *nextSibling()
    {
    SProperty *prop = nextSibling();
    while(prop)
      {
      T *t = prop->castTo<T>();
      if(t)
        {
        return t;
        }
      prop = prop->nextSibling();
      }
    return 0;
    }

  SProperty *nextSibling() const { preGet(); return _nextSibling; }

  SDatabase *database() { return _database; }
  const SDatabase *database() const { return _database; }

  bool inheritsFromType(const QString &type) const;
  bool inheritsFromType(SPropertyType type) const;

  const SPropertyInformation *typeInformation() const { return _info; }
  const SPropertyInformation::Child *instanceInformation() const;
  SPropertyType type() const;

  void postSet();
  void preGet() const;

  bool isDynamic() const;
  xsize index() const;

  QString path() const;
  QString path(const SProperty *from) const;

  bool isDescendedFrom(const SProperty *ent) const;
  SProperty *resolvePath(const QString &path);
  const SProperty *resolvePath(const QString &path) const;

  // set only works for dynamic properties
  void setName(const QString &);
  const QString &name() const;

  template <typename T>T *uncheckedCastTo()
    {
    xAssert(dynamic_cast<T *>(this));
    return static_cast<T *>(this);
    }
  template <typename T>const T *uncheckedCastTo() const
    {
    xAssert(dynamic_cast<const T *>(this));
    return static_cast<const T *>(this);
    }

  template <typename T>T *castTo()
    {
    if(inheritsFromType(T::Type))
      {
      return static_cast<T *>(this);
      }
    return 0;
    }
  template <typename T>const T *castTo() const
    {
    if(inheritsFromType(T::Type))
      {
      return static_cast<const T *>(this);
      }
    return 0;
    }

  class DataChange : public SChange
    {
    S_CHANGE(DataChange, SChange, 53)
  public:
    DataChange(SProperty *p) : _property(p) { }
    SProperty *property() {return _property;}
    const SProperty *property() const {return _property;}
  private:
    SProperty *_property;
    };

  class NameChange : public SChange
    {
    S_CHANGE(NameChange, SChange, 50)
  public:
    NameChange(const QString &b, const QString &a, SProperty *ent)
      : _before(b), _after(a), _property(ent)
      { }
    const QString &before() const {return _before;}
    const QString &after() const {return _after;}
    SProperty *property() {return _property;}
    const SProperty *property() const {return _property;}
  private:
    QString _before;
    QString _after;
    SProperty *_property;
    bool apply(int mode, SObservers &obs);
    };

  class ConnectionChange : public SChange
    {
    S_CHANGE(ConnectionChange, SChange, 51)
  public:
    enum Mode
      {
      Connect,
      Disconnect
      };

    ConnectionChange(Mode m, SProperty *driver, SProperty *driven)
      : _driver(driver), _driven(driven), _mode(m)
      { }
    SProperty *driver() { return _driver; }
    SProperty *driven() { return _driven; }
    const SProperty *driver() const { return _driver; }
    const SProperty *driven() const { return _driven; }
    Mode mode() const { return _mode; }
  private:
    SProperty *_driver;
    SProperty *_driven;
    Mode _mode;
    bool apply(int mode, SObservers &obs);
    };

protected:
  template <typename T> T *getChange() const
    {
    return reinterpret_cast<T*>(getChangeMemory(sizeof(T)));
    }

  static void blankAssign(const SProperty *, SProperty *);
  static void save(const SProperty *, SPropertyData &, SPropertyData::Mode);
  static void load(SProperty *, const SPropertyData &, xuint32, SPropertyData::Mode, SLoader &);

private:
  void setDirty();
  friend void setDependantsDirty(SProperty* prop);

  void connectInternal(SProperty *) const;
  void disconnectInternal(SProperty *) const;
  void *getChangeMemory(size_t size) const;
  SProperty *_nextSibling;
  SProperty *_input;
  SProperty *_output;
  SProperty *_nextOutput;
  SDatabase *_database;
  SPropertyContainer *_parent;
  const SPropertyInformation *_info;
  mutable SEntity *_entity;

  // could be replaced by a flag system if required.
  xuint8 _dirty;

  // only used for dynamic properties
  QString _name;

  friend class SEntity;
  friend class SDatabase;
  friend class SPropertyContainer;
  };

#endif // SPROPERTY_H