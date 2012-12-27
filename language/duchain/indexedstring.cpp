/*
 * This file is part of KDevelop
 * Copyright 2012 Milian Wolff <mail@milianw.de>
 * Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "indexedstring.h"

#include <language/duchain/referencecounting.h>

#include <language/duchain/repositories/itemrepository.h>

#include <KUrl>

namespace KDevelop {
namespace {
struct IndexedStringData
{
  int length;
  uint refCount;
  inline uint itemSize() const
  {
    return sizeof(IndexedStringData) + length * sizeof(QChar);
  }
  inline uint hash() const
  {
    return qHash(QString::fromRawData(reinterpret_cast<const QChar*>(this+1), length));
  }
  inline QString string() const
  {
    return QString(reinterpret_cast<const QChar*>(this+1), length);
  }
};

inline void increase(uint& val)
{
  ++val;
}

inline void decrease(uint& val)
{
  --val;
}

struct IndexedStringRepositoryItemRequest
{
  IndexedStringRepositoryItemRequest(const QStringRef& string)
  : m_hash(qHash(string))
  , m_str(string)
  {
  }

  enum {
    AverageSize = 10 //This should be the approximate average size of an Item
  };

  typedef uint HashType;

  //Should return the hash-value associated with this request(For example the hash of a string)
  HashType hash() const
  {
    return m_hash;
  }

  //Should return the size of an item created with createItem
  uint itemSize() const
  {
    return sizeof(IndexedStringData) + m_str.length() * sizeof(QChar);
  }
  //Should create an item where the information of the requested item is permanently stored. The pointer
  //@param item equals an allocated range with the size of itemSize().
  void createItem(IndexedStringData* item) const
  {
    item->length = m_str.length();
    item->refCount = 0;
    memcpy(item+1, m_str.constData(), m_str.length() * sizeof(QChar));
  }

  static void destroy(IndexedStringData* item, KDevelop::AbstractItemRepository&)
  {
    Q_UNUSED(item);
    //Nothing to do here (The object is not intelligent)
  }

  static bool persistent(const IndexedStringData* item)
  {
    return (bool)item->refCount;
  }

  //Should return whether the here requested item equals the given item
  bool equals(const IndexedStringData* item) const
  {
    return item->length == m_str.length() && (memcmp(++item, m_str.constData(), m_str.length() * sizeof(QChar)) == 0);
  }
  uint m_hash;
  QStringRef m_str;
};

typedef ItemRepository<IndexedStringData, IndexedStringRepositoryItemRequest, false, false> IndexedStringRepository;

RepositoryManager<IndexedStringRepository>& getGlobalIndexedStringRepository()
{
  static RepositoryManager<IndexedStringRepository> globalIndexedStringRepository("String Index ",
                                                                                      new QMutex(QMutex::NonRecursive));
  return globalIndexedStringRepository;
}

uint indexString(const QStringRef& string, IndexedString* item)
{
  if (string.isEmpty()) {
    return 0;
  } else if (string.length() == 1) {
    return IndexedString::charToIndex(string.at(0));
  } else {
    QMutexLocker lock(getGlobalIndexedStringRepository()->mutex());
    uint index = getGlobalIndexedStringRepository()->index(IndexedStringRepositoryItemRequest(string));

    //TODO: this could be merged with the above if the ItemRepository API got a
    //      Action argument for the index member function.
    if (shouldDoDUChainReferenceCounting(item)) {
      increase(getGlobalIndexedStringRepository()->dynamicItemFromIndexSimple(index)->refCount);
    }
    return index;
  }
}

}

IndexedString::IndexedString(const QString& string)
: m_index(indexString(QStringRef(&string), this))
{
}

IndexedString::IndexedString(const QStringRef& string)
: m_index(indexString(string, this))
{
}
IndexedString::IndexedString(const KUrl& url)
{
  const QString str = url.pathOrUrl(KUrl::RemoveTrailingSlash);
  m_index = indexString(QStringRef(&str), this);
}

IndexedString::IndexedString(const char* string)
{
  const QString str = QString::fromUtf8(string);
  m_index = indexString(QStringRef(&str), this);
}

IndexedString::~IndexedString()
{
  if (isNonTrivial()) {
    if (shouldDoDUChainReferenceCounting(this)) {
      QMutexLocker lock(getGlobalIndexedStringRepository()->mutex());
      decrease(getGlobalIndexedStringRepository()->dynamicItemFromIndexSimple(m_index)->refCount);
    }
  }
}

IndexedString::IndexedString(const IndexedString& rhs)
: m_index(rhs.m_index)
{
  if (isNonTrivial()) {
    if (shouldDoDUChainReferenceCounting(this)) {
      QMutexLocker lock(getGlobalIndexedStringRepository()->mutex());
      increase(getGlobalIndexedStringRepository()->dynamicItemFromIndexSimple(m_index)->refCount);
    }
  }
}

IndexedString& IndexedString::operator=(const IndexedString& rhs)
{
  if (m_index == rhs.m_index) {
    return *this;
  }

  // decrease refcount of current/old index
  if (isNonTrivial()) {
    if (shouldDoDUChainReferenceCounting(this)) {
      QMutexLocker lock(getGlobalIndexedStringRepository()->mutex());
      decrease(getGlobalIndexedStringRepository()->dynamicItemFromIndexSimple(m_index)->refCount);
    }
  }

  m_index = rhs.m_index;

  // increase refcount for new index
  if (isNonTrivial()) {
    if(shouldDoDUChainReferenceCounting(this)) {
      QMutexLocker lock(getGlobalIndexedStringRepository()->mutex());
      increase(getGlobalIndexedStringRepository()->dynamicItemFromIndexSimple(m_index)->refCount);
    }
  }

  return *this;
}

QString IndexedString::toString() const
{
  if (isEmpty()) {
    return QString();
  } else if(isChar()) {
    return QString(toChar());
  } else {
    QMutexLocker lock(getGlobalIndexedStringRepository()->mutex());
    return getGlobalIndexedStringRepository()->itemFromIndex(m_index)->string();
  }
}

int IndexedString::length() const
{
  if(isEmpty()) {
    return 0;
  } else if (isChar()) {
    return 1;
  } else {
    QMutexLocker lock(getGlobalIndexedStringRepository()->mutex());
    return getGlobalIndexedStringRepository()->itemFromIndex(m_index)->length;
  }
}

}

QDebug operator<<(QDebug s, const KDevelop::IndexedString& string)
{
  s.nospace() << string.toString();
  return s.space();
}
