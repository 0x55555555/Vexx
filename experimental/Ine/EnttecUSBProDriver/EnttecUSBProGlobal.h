#ifndef ENTTECUSBPROGLOBAL_H
#define ENTTECUSBPROGLOBAL_H

#include "IneGlobal.h"

#ifdef ENTTEXUSBPRO_BUILD
# define ENTTEXUSBPRO_EXPORT X_DECL_EXPORT
#else
# define ENTTEXUSBPRO_EXPORT X_DECL_IMPORT
#endif

#endif // ENTTECUSBPROGLOBAL_H
