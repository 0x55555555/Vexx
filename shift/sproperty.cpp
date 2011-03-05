#include "sproperty.h"
#include "sentity.h"
#include "sdatabase.h"
#include "schange.h"
#include "QString"

bool SProperty::NameChange::apply(int mode, SObservers& obs)
  {
  if(mode&Forward)
    {
    property()->_name = after();
    }
  else if(mode&Backward)
    {
    property()->_name = before();
    }
  if(mode&Inform)
    {
    xAssert(property()->entity());
    obs.clear();
    property()->entity()->informTreeObservers(mode, this, obs);
    }
  return true;
  }

SProperty::SProperty() : _nextSibling(0), _input(0), _output(0), _nextOutput(0),
    _database(0), _parent(0), _info(0), _entity(0), _dirty(true)
  {
  }

bool SProperty::isDynamic() const
  {
  if(_parent)
    {
    return _parent->isChildDynamic(this);
    }
  return true;
  }

xsize SProperty::index() const
  {
  preGet();
  if(_parent)
    {
    return _parent->indexOfChild(this);
    }
  return -1;
  }

void SProperty::setName(const QString &in)
  {
  xAssert(isDynamic());
  xAssert(parent());

  if(name() == in)
    {
    return;
    }

  // ensure the name is unique
  QString realName = in;
  xsize num = 1;
  while(parent()->findChild(realName))
    {
    realName = in + QString::number(num++);
    }

  void *changeMemory = getChange< NameChange >();
  NameChange *change = new(changeMemory) NameChange(name(), realName, this);
  database()->submitChange(change);
  }

void SProperty::blankAssign(const SProperty *, SProperty *)
  {
  }

void SProperty::save(const SProperty *p, SPropertyData &d, SPropertyData::Mode m)
  {
  if(p->input())
    {
    d.appendAttribute("input", p->input()->path(p).toUtf8());
    }
  }

void SProperty::load(SProperty *p, const SPropertyData &d, xuint32, SPropertyData::Mode m, SLoader &loader)
  {
  if(d.hasAttribute("input"))
    {
    QString path();
    loader.resolveInputAfterLoad(p, QString::fromUtf8(d.attribute("input")));
    }
  }

bool SProperty::inheritsFromType(SPropertyType type) const
  {
  const SPropertyInformation *current = typeInformation();
  xAssert(current);
  while(current)
    {
    if(current->typeId == type)
      {
      return true;
      }
    current = current->parentTypeInformation;
    }
  return false;
  }

bool SProperty::inheritsFromType(const QString &type) const
  {
  const SPropertyInformation *current = typeInformation();
  while(current)
    {
    if(current->typeName == type)
      {
      return true;
      }
    current = current->parentTypeInformation;
    }
  return false;
  }

const SPropertyInformation::Child *SProperty::instanceInformation() const
  {
  if(!isDynamic() && _parent)
    {
    return _parent->typeInformation()->child(index());
    }
  return 0;
  }

const QString &SProperty::name() const
  {
  const SPropertyInformation::Child *propInfo = instanceInformation();
  if(propInfo)
    {
    return propInfo->name;
    }
  return _name;
  }

void SProperty::assign(const SProperty *propToAssign)
  {
  const SPropertyInformation *info = typeInformation();
  xAssert(info);

  info->assign(propToAssign, this);
  }

SEntity *SProperty::entity() const
  {
  if(_entity)
    {
    return _entity;
    }
  _entity = (SEntity *)castTo<SEntity>();
  if(_entity)
    {
    return _entity;
    }
  if(_parent)
    {
    _entity = _parent->entity();
    }
  return _entity;
  }

void *SProperty::getChangeMemory(size_t size) const
  {
  return parent()->database()->allocateChangeMemory(size);
  }

SPropertyType SProperty::type() const
  {
  return typeInformation()->typeId;
  }

void SProperty::connect(SProperty *prop) const
  {
  void *changeMemory = getChange< ConnectionChange >();
  ConnectionChange *change = new(changeMemory) ConnectionChange(ConnectionChange::Connect, (SProperty*)this, prop);
  ((SDatabase*)database())->submitChange(change);
  prop->postSet();
  }

void SProperty::disconnect(SProperty *prop) const
  {
  void *changeMemory = getChange< ConnectionChange >();
  ConnectionChange *change = new(changeMemory) ConnectionChange(ConnectionChange::Disconnect, (SProperty*)this, prop);
  ((SDatabase*)database())->submitChange(change);
  prop->postSet();
  }

void SProperty::disconnect() const
  {
  if(_input)
    {
    ((SProperty*)_input)->disconnect((SProperty*)this);
    }

  while(_output)
    {
    disconnect(_output);
    }
  }

bool SProperty::ConnectionChange::apply(int mode, SObservers &obs)
  {
  if(mode&Forward)
    {
    if(_mode == Connect)
      {
      _driver->connectInternal(_driven);
      }
    else if(_mode == Disconnect)
      {
      _driver->disconnectInternal(_driven);
      }
    }
  else if(mode&Backward)
    {
    if(_mode == Connect)
      {
      _driver->disconnectInternal(_driven);
      }
    else if(_mode == Disconnect)
      {
      _driver->connectInternal(_driven);
      }
    }
  if(mode&Inform)
    {
    xAssert(_driver->entity());
    xAssert(_driven->entity());
    obs.clear();
    _driver->entity()->informConnectionObservers(mode, this, obs);
    _driven->entity()->informConnectionObservers(mode, this, obs);
    }
  return true;
  }

void SProperty::connectInternal(SProperty *prop) const
  {
  // prop can't already have an output
  if(prop->hasInput())
    {
    return;
    }
  prop->_input = (SProperty*)this;

  SProperty **output = (SProperty**)&_output;
  while(*output)
    {
    SProperty **nextOutput =  &((*output)->_nextOutput);
    if(!*nextOutput)
      {
      break;
      }
    output = nextOutput;
    }
  *output = prop;
  }

void SProperty::disconnectInternal(SProperty *prop) const
  {
  xAssert(prop->_input == this);

  prop->_input = 0;

  SProperty **output = (SProperty**)&_output;
  while(*output)
    {
    if((*output) == prop)
      {
      SProperty **nextOutput = &((*output)->_nextSibling);
      (*output) = 0;
      output = nextOutput;
      }
    else
      {
      output = &((*output)->_nextOutput);
      }
    }
  }

QString SProperty::path() const
  {
  if(parent() == 0)
    {
    return SDatabase::pathSeparator() + name();
    }
  return parent()->path() + SDatabase::pathSeparator() + name();
  }

QString SProperty::path(const SProperty *from) const
  {
  if(from == this)
    {
    return "";
    }
  if(isDescendedFrom(from))
    {
    QString ret;
    const SProperty *p = parent();
    while(p && p != from)
      {
      xAssert(p->name() != "");
      ret = p->name() + SDatabase::pathSeparator() + ret;

      p = p->parent();
      }
    return ret + name();
    }
  else if(from->parent())
    {
    return ".." + SDatabase::pathSeparator() + path(from->parent());
    }
  xAssert(0);
  return "";
  }

bool SProperty::isDescendedFrom(const SProperty *in) const
  {
  if(this == in)
    {
    return true;
    }
  else if(parent() == 0)
    {
    return false;
    }
  return parent()->isDescendedFrom(in);
  }

SProperty *SProperty::resolvePath(const QString &path)
  {
  preGet();
  QStringList splitPath(path.split(SDatabase::pathSeparator()));
  SProperty *cur = this;
  foreach(const QString &name, splitPath)
    {
    if(!cur)
      {
      return 0;
      }
    if(name == "..")
      {
      cur = cur->parent();
      }

    SPropertyContainer* container = cur->castTo<SPropertyContainer>();
    if(!container)
      {
      return 0;
      }
    SProperty *child = container->firstChild();
    while(child)
      {
      if(child->name() == name)
        {
        cur = child;
        break;
        }
      child = child->nextSibling();
      }
    }
  return cur;
  }

const SProperty *SProperty::resolvePath(const QString &path) const
  {
  preGet();
  QStringList splitPath(path.split(SDatabase::pathSeparator()));
  const SProperty *cur = this;
  foreach(const QString &name, splitPath)
    {
    if(!cur)
      {
      return 0;
      }
    if(name == "..")
      {
      cur = cur->parent();
      }

    const SPropertyContainer* container = cur->castTo<SPropertyContainer>();
    if(!container)
      {
      return 0;
      }
    const SProperty *child = container->firstChild();
    while(child)
      {
      if(child->name() == name)
        {
        cur = child;
        break;
        }
      child = child->nextSibling();
      }
    }
  return cur;
  }

inline void setDependantsDirty(SProperty* prop)
  {
  for(SProperty *o=prop->output(); o; o = o->nextOutput())
    {
    o->setDirty();
    }

  const SPropertyInformation::Child *child = prop->instanceInformation();

  if(child && child->affects)
    {
    xsize i=0;
    while(child->affects[i])
      {
      const SProperty SPropertyContainer::* affectsPtr(child->affects[i]);
      SProperty *affectsProp = (SProperty*)&(prop->parent()->*affectsPtr);

      xAssert(affectsProp);
      affectsProp->setDirty();
      i++;
      }
    }
  }

void SProperty::postSet()
  {
  _dirty = false;

  setDependantsDirty(this);
  }

void SProperty::setDirty()
  {
  if(!_dirty)
    {
    _dirty = true;

    setDependantsDirty(this);
    }
  }

void SProperty::preGet() const
  {
  if(_dirty)
    {
    // this is a const function, but because we delay computation we may need to assign here
    SProperty *prop = ((SProperty*)this);

    prop->_dirty = false;

    if(input())
      {
      prop->assign(input());
      }

    const SPropertyInformation::Child *child = instanceInformation();
    if(child && child->compute)
      {
      xAssert(parent());
      child->compute(parent());
      }
    }
  }