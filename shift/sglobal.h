#ifndef SGLOBAL_H
#define SGLOBAL_H

#include <QtCore/qglobal.h>
#include "XGlobal"
#include "XList"

#if defined(SHIFT_BUILD)
#  define SHIFT_EXPORT Q_DECL_EXPORT
#else
#  define SHIFT_EXPORT Q_DECL_IMPORT
#endif

typedef xuint32 SPropertyType;

class SEntity;
class SProperty;
class SObserver;
class STreeObserver;
class SDataObserver;
class SConnectionObserver;

typedef XList<SObserver*> SObservers;
typedef XList<STreeObserver*> STreeObservers;
typedef XList<SDataObserver*> SDataObservers;
typedef XList<SConnectionObserver*> SConnectionObservers;

typedef XList<SEntity*> SEntities;
typedef XList<const SEntity*> SConstEntities;

#define S_PROPERTY_TYPE(name, ns, id) namespace Types { static const SPropertyType name = ns + id; }

#define S_CORE_TYPE(name, id) S_PROPERTY_TYPE(name, 0, id)

S_CORE_TYPE(SProperty, 1);
S_CORE_TYPE(SPropertyContainer, 2);
S_CORE_TYPE(SPropertyArray, 3);
S_CORE_TYPE(STypedPropertyArray, 4);
S_CORE_TYPE(SEntity, 5);

S_CORE_TYPE(BoolProperty, 20);
S_CORE_TYPE(IntProperty, 21);
S_CORE_TYPE(LongIntProperty, 22);
S_CORE_TYPE(UnsignedIntProperty, 23);
S_CORE_TYPE(LongUnsignedIntProperty, 24);
S_CORE_TYPE(FloatProperty, 25);
S_CORE_TYPE(DoubleProperty, 26);
S_CORE_TYPE(Vector2DProperty, 27);
S_CORE_TYPE(Vector3DProperty, 28);
S_CORE_TYPE(Vector4DProperty, 29);
S_CORE_TYPE(ColourProperty, 30);
S_CORE_TYPE(QuaternionProperty, 31);
S_CORE_TYPE(StringProperty, 32);
S_CORE_TYPE(LongStringProperty, 33);
S_CORE_TYPE(Pointer, 34);
S_CORE_TYPE(PointerArray, 35);
S_CORE_TYPE(SDatabase, 36);

S_CORE_TYPE(SArrayProperty, 40);
S_CORE_TYPE(SFloatArrayProperty, 41);

S_CORE_TYPE(SReferenceEntity, 50);

#endif // SHIFT_GLOBAL_H
