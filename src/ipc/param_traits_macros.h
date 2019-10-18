// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IPC_PARAM_TRAITS_MACROS_H_
#define IPC_PARAM_TRAITS_MACROS_H_

#include <string>

// Traits generation for structs.
#define IPC_STRUCT_TRAITS_BEGIN(struct_name) \
  namespace IPC { \
    template <> \
    struct IPC_MESSAGE_EXPORT ParamTraits<struct_name> { \
      typedef struct_name param_type; \
      static void Write(Message* m, const param_type& p); \
      static bool Read(const Message* m, PickleIterator* iter, param_type* p); \
      static void Log(const param_type& p, std::string* l); \
    }; \
  }

#define IPC_STRUCT_TRAITS_MEMBER(name)
#define IPC_STRUCT_TRAITS_PARENT(type)
#define IPC_STRUCT_TRAITS_END()

// Convenience macro for defining enumerated type traits for types which are
// not range-checked by the IPC system. The author of the message handlers
// is responsible for all validation. This macro should not need to be
// subsequently redefined.
#define IPC_ENUM_TRAITS(type) \
  IPC_ENUM_TRAITS_VALIDATE(type, true)

// Convenience macro for defining enumerated type traits for types which are
// range-checked by the IPC system to be in the range of 0..maxvalue inclusive.
// This macro should not need to be subsequently redefined.
#define IPC_ENUM_TRAITS_MAX_VALUE(type, maxvalue) \
  IPC_ENUM_TRAITS_MIN_MAX_VALUE(type, 0, maxvalue)

// Convenience macro for defining enumerated type traits for types which are
// range-checked by the IPC system to be in the range of minvalue..maxvalue
// inclusive. This macro should not need to be subsequently redefined.
#define IPC_ENUM_TRAITS_MIN_MAX_VALUE(type, minvalue, maxvalue)  \
  IPC_ENUM_TRAITS_VALIDATE(type, (value >= (minvalue) && value <= (maxvalue)))

// Traits generation for enums. This macro may be redefined later.
#define IPC_ENUM_TRAITS_VALIDATE(enum_name, validation_expression) \
  namespace IPC { \
    template <> \
    struct IPC_MESSAGE_EXPORT ParamTraits<enum_name> { \
      typedef enum_name param_type; \
      static void Write(Message* m, const param_type& p); \
      static bool Read(const Message* m, PickleIterator* iter, param_type* p); \
      static void Log(const param_type& p, std::string* l); \
    }; \
  }

#endif  // IPC_PARAM_TRAITS_MACROS_H_

