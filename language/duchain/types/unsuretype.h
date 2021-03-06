/*
   Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPLATFORM_UNSURETYPE_H
#define KDEVPLATFORM_UNSURETYPE_H

#include "abstracttype.h"
#include "typesystemdata.h"
#include "../appendedlist.h"
#include <language/languageexport.h>


namespace KDevelop {

KDEVPLATFORMLANGUAGE_EXPORT DECLARE_LIST_MEMBER_HASH(UnsureTypeData, m_types, IndexedType)

struct KDEVPLATFORMLANGUAGE_EXPORT UnsureTypeData : public AbstractTypeData {
  UnsureTypeData() {
    initializeAppendedLists(m_dynamic);
  }
  
   ~UnsureTypeData() {
    freeAppendedLists();
  }
  
  UnsureTypeData(const UnsureTypeData& rhs) : AbstractTypeData(rhs){
    initializeAppendedLists(m_dynamic);
    copyListsFrom(rhs);
  }
  
  START_APPENDED_LISTS_BASE(UnsureTypeData, AbstractTypeData)
  APPENDED_LIST_FIRST(UnsureTypeData, IndexedType, m_types)
  END_APPENDED_LISTS(UnsureTypeData, m_types)
};

class KDEVPLATFORMLANGUAGE_EXPORT UnsureType : public AbstractType
{
public:
  typedef TypePtr<UnsureType> Ptr;

  UnsureType(const UnsureType& rhs);
  UnsureType();
  explicit UnsureType(UnsureTypeData& data);
  
  virtual KDevelop::AbstractType* clone() const override;
  virtual QString toString() const override;
  virtual bool equals(const KDevelop::AbstractType* rhs) const override;
  virtual uint hash() const override;
  virtual KDevelop::AbstractType::WhichType whichType() const override;
  virtual void exchangeTypes(KDevelop::TypeExchanger* exchanger) override;
  
  virtual void addType(IndexedType type);
  virtual void removeType(IndexedType type);
  
  ///Array of represented types. You can conveniently iterate it using the FOREACH_FUNCTION macro,
  ///or just access them using indices
  const IndexedType* types() const;
  ///Count of types accessible through types()
  uint typesSize() const;
  
  enum {
    Identity = 39
  };
  
  typedef KDevelop::AbstractType BaseType;
  typedef UnsureTypeData Data;
  
protected:
  TYPE_DECLARE_DATA(UnsureType)
  virtual void accept0(KDevelop::TypeVisitor* v) const override;
};

}

#endif // KDEVPLATFORM_UNSURETYPE_H
