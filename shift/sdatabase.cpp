#include "sdatabase.h"
#include "sentity.h"
#include "schange.h"
#include "sentity.h"
#include "spropertydata.h"
#include "sreferenceentity.h"
#include "QFile"
#include "QRegExp"
#include "QDebug"

S_ENTITY_DEFINITION(SDatabase, SEntity)
  S_PROPERTY_DEFINITION(UnsignedIntProperty, majorVersion)
  S_PROPERTY_DEFINITION(UnsignedIntProperty, minorVersion)
  S_PROPERTY_DEFINITION(UnsignedIntProperty, revision)
S_ENTITY_END_DEFINITION(SDatabase, SEntity)

SDatabase::SDatabase() : majorVersion(0), minorVersion(0), revision(0), _blockLevel(0), _inSubmitChange(0), _readLevel(0)
  {
  _database = this;
  _info = staticTypeInformation();

  initiate();
  initiatePropertyFromMetaData(this, staticTypeInformation());
  }

SDatabase::SDatabase(const SPropertyInformation *info) : majorVersion(0), minorVersion(0), revision(0), _blockLevel(0), _inSubmitChange(0), _readLevel(0)
  {
  _database = this;
  _info = info;

  initiate();
  initiatePropertyFromMetaData(this, info);
  }

void SDatabase::initiate()
  {
  addType<SProperty>();
  addType<SPropertyContainer>();
  addType<SPropertyArray>();

  addType<SEntity>();
  addType<SDatabase>();
  addType<SReferenceEntity>();

  addType<BoolProperty>();
  addType<IntProperty>();
  addType<LongIntProperty>();
  addType<UnsignedIntProperty>();
  addType<LongUnsignedIntProperty>();
  addType<FloatProperty>();
  addType<DoubleProperty>();
  addType<Vector2DProperty>();
  addType<Vector3DProperty>();
  addType<Vector4DProperty>();
  addType<QuaternionProperty>();
  addType<StringProperty>();
  addType<ColourProperty>();
  addType<LongStringProperty>();

  addType<Pointer>();
  addType<PointerArray>();
  }

SDatabase::~SDatabase()
  {
  // clear out our child elements before the allocator is destroyed.
  xsize propIndex = 0;
  SProperty *prop = _child;
  while(prop)
    {
    SProperty *next = prop->_nextSibling;
    prop->~SProperty();
    if(propIndex >= _containedProperties)
      {
      database()->deleteProperty(prop);
      }
    propIndex++;
    prop = next;
    }
  _child = 0;

  foreach(SChange *ch, _done)
    {
    delete ch;
    }

  xAssert(_properties.empty());
  }

SProperty *SDatabase::createProperty(xuint32 t)
  {
  xAssert(_types.contains(t));
  const SPropertyInformation *type = _types[t];
  void *ptr = _properties.alloc(type->size);
  SProperty *prop = type->create(ptr);
  prop->_database = this;
  prop->_info = type;

  initiateProperty(prop);
  return prop;
  }

void SDatabase::initiatePropertyFromMetaData(SPropertyContainer *container, const SPropertyInformation *mD)
  {
  xAssert(mD);

  if(mD->parentTypeInformation)
    {
    initiatePropertyFromMetaData(container, mD->parentTypeInformation);
    }

  for(xsize i=0; i<mD->propertyCount; ++i)
    {
    // no contained properties with duplicated names...
    const SPropertyInformation::Child &child = mD->childMetaData[i];

    // extract the properties location from the meta data.
    const SProperty SPropertyContainer::* prop(child.location);
    SProperty *thisProp = (SProperty*)&(container->*prop);

    container->internalInsertProperty(true, thisProp, X_SIZE_SENTINEL);
    thisProp->_info = child.childInformation;
    initiateProperty(thisProp);
    }
  }

void SDatabase::uninitiatePropertyFromMetaData(SPropertyContainer *container, const SPropertyInformation *mD)
  {
  xAssert(mD);

  if(mD->parentTypeInformation)
    {
    uninitiatePropertyFromMetaData(container, mD->parentTypeInformation);
    }

  for(xsize i=0; i<mD->propertyCount; ++i)
    {
    // no contained properties with duplicated names...
    const SPropertyInformation::Child &child = mD->childMetaData[i];

    // extract the properties location from the meta data.
    const SProperty SPropertyContainer::* prop(child.location);
    SProperty *thisProp = (SProperty*)&(container->*prop);

    uninitiateProperty(thisProp);
    }
  }

void SDatabase::initiateProperty(SProperty *prop)
  {
  ++((SPropertyInformation*)prop->typeInformation())->instances;

  SPropertyContainer *container = prop->castTo<SPropertyContainer>();
  if(container)
    {
    const SPropertyInformation *metaData = container->typeInformation();
    xAssert(metaData);

    initiatePropertyFromMetaData(container, metaData);
    }
  xAssert(prop->database());
  }

void SDatabase::uninitiateProperty(SProperty *prop)
  {
  --((SPropertyInformation*)prop->typeInformation())->instances;

  SPropertyContainer *container = prop->castTo<SPropertyContainer>();
  if(container)
    {
    const SPropertyInformation *metaData = container->typeInformation();
    xAssert(metaData);

    uninitiatePropertyFromMetaData(container, metaData);
    }
  }

void SDatabase::deleteProperty(SProperty *prop)
  {
  uninitiateProperty(prop);

  SEntity *ent = prop->castTo<SEntity>();
  if(ent)
    {
    if(ent->parentEntity())
      {
      ent->parentEntity()->children.internalRemoveProperty(ent);
      }
    }
  prop->~SProperty();
  _properties.free(prop);
  }

void SDatabase::beginBlock()
  {
  _blockLevel++;
  }

void SDatabase::endBlock()
  {
  xAssert(_blockLevel > 0);
  _blockLevel--;
  if(_blockLevel == 0)
    {
    inform();
    }
  }

QString SDatabase::pathSeparator()
  {
  return "/";
  }

QString SDatabase::propertySeparator()
  {
  return ":";
  }

const SPropertyInformation *SDatabase::findType(xuint32 i) const
  {
  if(_types.contains(i))
    {
    return ((XMap <SPropertyType, SPropertyInformation*>&)_types)[i];
    }
  return 0;
  }

const SPropertyInformation *SDatabase::findType(const QString &in) const
  {
  QList <xuint32> keys(_types.keys());
  for(int i=0, s=keys.size(); i<s; ++i)
    {
    if(_types[keys[i]]->typeName == in)
      {
      return ((XMap <SPropertyType, SPropertyInformation*>&)_types)[keys[i]];
      }
    }
  return 0;
  }

void SDatabase::write(const SProperty *prop, SPropertyData &data, SPropertyData::Mode mode) const
  {
  if(!prop)
    {
    prop = this;
    }

  xAssert(_types.contains(prop->type()));

  if(!_types.contains(prop->type()))
    {
    return;
    }

  const SPropertyInformation *type = prop->typeInformation();

  data.setName(type->typeName);
  data.appendAttribute("name", prop->name().toUtf8());
  data.appendAttribute("version", QString::number(type-> version).toUtf8());
  data.appendAttribute("dynamic", QString::number(prop->isDynamic()).toUtf8());
  type->save(prop, data, mode);
  }

SProperty *SDatabase::read(const SPropertyData &data, SPropertyContainer *parent, SPropertyData::Mode mode)
  {
  if(_readLevel == 0)
    {
    _resolveAfterLoad.clear();
    }
  _readLevel++;

  const SPropertyInformation *type = findType(data.name());
  xAssert(type);
  xAssert(type->load);

  if(!type)
    {
    return 0;
    }

  SProperty *prop = 0;

  if(parent)
    {
    QString name(QString::fromUtf8(data.attribute("name")));
    bool dynamic(QString::fromUtf8(data.attribute("dynamic")).toUInt());
    if(dynamic)
      {
      prop = parent->addProperty(type->typeId);
      prop->setName(name);
      }
    else
      {
      prop = parent->findChild(name);
      xAssert(prop);
      }
    }
  else
    {
    prop = this;
    }

  xuint32 version = QString::fromUtf8(data.attribute("version")).toUInt();
  type->load(prop, data, version, mode, *this);
  _readLevel--;

  if(_readLevel == 0)
    {
    foreach(SProperty *prop, _resolveAfterLoad.keys())
      {
      SProperty* input = prop->resolvePath(_resolveAfterLoad.value(prop));

      xAssert(input);
      if(input)
        {
        input->connect(prop);
        }
      }
    }

  return prop;
  }

SBlock::SBlock(SDatabase *db) : _db(db)
  {
  _db->beginBlock();
  }

SBlock::~SBlock()
  {
  _db->endBlock();
  }

void *SDatabase::allocateChangeMemory(xsize s)
  {
  return new char[s];
  }

void SDatabase::destoryChangeMemory(xsize s, void *ptr)
  {
  delete [] (char*)ptr;
  }

void SDatabase::submitChange(SChange *ch)
  {
  _inSubmitChange = true;
  try
    {
    _currentObservers.clear();
    bool result = ch->apply(SChange::Forward|SChange::Inform, _currentObservers);
    _blockObservers << _currentObservers;

    if(result)
      {
      _done << ch;

      if(_blockLevel == 0)
        {
        inform();
        }
      }
    else
      {
      qDebug() << "Failure in change";
      delete ch;
      }
    }
  catch(...)
    {
    xAssert(0 && "Unhandled exception");
    }
  _inSubmitChange = false;
  }

void SDatabase::inform()
  {
  foreach(SObserver *obs, _blockObservers)
    {
    obs->actOnChanges();
    }
  _blockObservers.clear();
  }

void SDatabase::resolveInputAfterLoad(SProperty *prop, QString inputPath)
  {
  xAssert(_readLevel > 0);
  if(_readLevel > 0)
    {
    _resolveAfterLoad.insert(prop, inputPath);
    }
  }