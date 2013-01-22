/*! @file CounterMap.h
  @brief A map of counters suitable for use as a conditional distribution.

  @author Yuriy Skobov
 */

#ifndef __COUNTER_MAP_H__
#define __COUNTER_MAP_H__

#include <ostream>
#include <utility>
#include "Counter.h"
#include "CounterFactories.h"
#include "Util/NumCache.h"
#include "MapTypeErasure/AnyMap.hpp"

namespace Counters
{
  
  template <typename K, typename V>
  class CounterMap
  {
    typedef K Key_t;
    typedef V Value_t;
    typedef Counter<V> Counter_t;
    typedef typename Counter_t::Count_t Count_t;
    typedef MapTypeErasure::AnyMap<K, Counter_t> CoreMap_t;
    typedef typename CoreMap_t::value_type IteratorValue_t;
    typedef typename CoreMap_t::const_iterator ConstIterator;
    typedef typename CoreMap_t::size_type Size_t;

    
    
    CounterMap( CounterMap const & other );
    CounterMap( CounterMap && other );
    explicit CounterMap( CoreMap_t coreMap
			 = CoreMap_t(),
			 CounterFactory<V> const & counterFactory
			 = DefaultCounterFactory<V>() );
    ~CounterMap();
    
    CounterMap& operator=(CounterMap const & rhs);
    CounterMap& operator=(CounterMap && rhs);

    void swap( CounterMap& other );
    
    void incrementCount(K const& key, V const& val, Count_t count);
    void incrementCount(K     && key, V const& val, Count_t count);
    void incrementCount(K     && key, V     && val, Count_t count);
    void incrementCount(K const& key, V     && val, Count_t count);

    void setCount(K const& key, V const& val, Count_t count);
    void setCount(K     && key, V const& val, Count_t count);
    void setCount(K     && key, V     && val, Count_t count);
    void setCount(K const& key, V     && val, Count_t count);

    void conditionalNormalize(void);

    //------------------- Lookup ---------------------

    bool contains(K const& key) const;
    bool contains(K const& key, V const& val) const;
    Size_t size(void) const;
    Size_t size(K const& key) const;
    bool empty(void) const;
    CounterMap::Count_t getCount(K const& key, V const& val) const;
    CounterMap::Count_t totalCount(void) const;
    CounterMap::Count_t totalCount(K const& key) const;

    //--------------------- Counters ---------------------------------
    
    Counter<V> const * const getCounter(K const& key) const;
    Counter<V> const * const getCounter(K     && key) const;

    //--------------------- Traversal --------------------------------

    ConstIterator begin(void) const;
    ConstIterator end(void) const;

    //--------------------- Equality ---------------------------------

    bool operator==(CounterMap const& other) const;
    bool operator!=(CounterMap const& other) const;
    bool equals(const CounterMap& other, 
		Count_t precision=std::numeric_limits<Count_t>::epsilon()) const;

    //------------------ Arithmetic Operators ------------------------

    CounterMap& operator+=(CounterMap const& rhs);
    CounterMap& operator-=(CounterMap const& rhs);
    CounterMap& operator+(CounterMap rhs) const;
    CounterMap& operator-(CounterMap rhs) const;

  protected:
    Counter<V>& ensureCounter(K const& key);
    Counter<V>& ensureCounter(K     && key);
    
  private:
    CoreMap_t coreMap_;

    CounterFactory<V> *counterFactory_;
    
    // total cache:
    typedef NumCache<Count_t> CountCache;
    CountCache *cachedTotal_;
  };

};

#include "counterDetails/CounterMapImpl.hpp"

#endif //__COUNTER_MAP_H__
