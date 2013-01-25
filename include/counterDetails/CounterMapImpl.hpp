#ifndef __COUNTER_MAP_IMPL_HPP__
#define __COUNTER_MAP_IMPL_HPP__

// The following include is added for two reasons even though it should not
// be needed because this file itself is included from CounterMap.h.
// Reason 1: form - as this serves as the implementation file, it makes sense
// to point from here to the header declaring the typename.
// Reason 2: EDE and Semantic autocompletion needs this to help with member
// variables and STL typenamees included from the header.
#include "CounterMap.h"

namespace Counters
{
  template <typename K, typename V>
  CounterMap<K, V>::CounterMap( CounterMap<K, V> const & other )
    : coreMap_(other.coreMap_),
      counterFactory_(other.counterFactory_->clone()),
      cachedTotal_(new CountCache(*other.cachedTotal_))
  {}

  template <typename K, typename V>
  CounterMap<K, V>::CounterMap( CounterMap<K, V> && other )
    : coreMap_(),
      counterFactory_(NULL),
      cachedTotal_(NULL)
  {
    swap(other);
  }

  template <typename K, typename V>
  CounterMap<K, V>::CounterMap( CoreMap_t coreMap, 
				CounterFactory<V> const & counterFactory )
    : coreMap_(std::move(coreMap)),
      counterFactory_(counterFactory.clone()),
      cachedTotal_(new CountCache(0, CACHE_POLICY_RELAXED, false))
  {}
  
  template <typename K, typename V>
  CounterMap<K, V>::~CounterMap()
  {
    delete counterFactory_;
    delete cachedTotal_;
  }

  template <typename K, typename V>
  CounterMap<K, V>& CounterMap<K, V>::operator=(CounterMap<K, V> const & other)
  {
    coreMap_ = other.coreMap_;
    counterFactory_ = other.counterFactory_->clone();
    *cachedTotal_ = *other.cachedTotal_;
    return *this;
  }

  template <typename K, typename V>
  CounterMap<K, V>& CounterMap<K, V>::operator=(CounterMap<K, V> && other)
  {
    swap( other );
    return *this;
  }

  template <typename K, typename V>
  void CounterMap<K, V>::swap(CounterMap<K, V>& other )
  {
    coreMap_.swap(other.coreMap_);
    std::swap(counterFactory_, other.counterFactory_);
    std::swap(cachedTotal_, other.cachedTotal_);
  }

  //--------------------------- Modifiers ---------------------------------------

  template <typename K, typename V>
  void CounterMap<K, V>::incrementCount(K const& key, V const& val, Count_t count)
  {
    ensureCounter(key).incrementCount(val, count);
    cachedTotal_->reset();
  }

  template <typename K, typename V>
  void CounterMap<K, V>::incrementCount(K && key, V const& val, Count_t count)
  {
    ensureCounter(std::move(key)).incrementCount(val, count);
    cachedTotal_->reset();
  }

  template <typename K, typename V>
  void CounterMap<K, V>::incrementCount(K && key, V && val, Count_t count)
  {
    ensureCounter(std::move(key)).incrementCount(std::move(val), count);
    cachedTotal_->reset();
  }

  template <typename K, typename V>
  void CounterMap<K, V>::incrementCount(K const& key, V && val, Count_t count)
  {
    ensureCounter(key).incrementCount(std::move(val), count);
    cachedTotal_->reset();
  }
  
  template <typename K, typename V>
  void CounterMap<K, V>::setCount(K const& key, V const& val, Count_t count)
  {
    ensureCounter(key).setCount(val, count);
    cachedTotal_->reset();
  }

  template <typename K, typename V>
  void CounterMap<K, V>::setCount(K && key, V const& val, Count_t count)
  {
    ensureCounter(std::move(key)).setCount(val, count);
    cachedTotal_->reset();
  }

  template <typename K, typename V>
  void CounterMap<K, V>::setCount(K && key, V && val, Count_t count)
  {
    ensureCounter(std::move(key)).setCount(std::move(val), count);
    cachedTotal_->reset();
  }
  
  template <typename K, typename V>
  void CounterMap<K, V>::setCount(K const& key, V && val, Count_t count)
  {
    ensureCounter(key).setCount(std::move(val), count);
    cachedTotal_->reset();
  }

  template <typename K, typename V>
  void CounterMap<K, V>::remove(K const& key)
  {
    if( coreMap_.erase(key) > 0 )
      cachedTotal_->reset();
  }

  template <typename K, typename V>
  void CounterMap<K, V>::remove(K const& key, V const& val)
  {
    typename CounterMap<K, V>::CoreMap_t::iterator i( coreMap_.find(key) );
    if( i != coreMap_.end() )
      i->second.remove(val);
  }


  template <typename K, typename V>
  void CounterMap<K, V>::conditionalNormalize(void)
  {
    for( typename CounterMap<K, V>::CoreMap_t::iterator i( coreMap_.begin() );
	 i != coreMap_.end(); ++i )
      {
	i->second.normalize();
      }
    cachedTotal_->reset();
  }

  //------------------- Lookup ---------------------

  template <typename K, typename V>
  bool CounterMap<K, V>::contains(K const& key) const
  {
    return coreMap_.count(key) > 0;
  }

  template <typename K, typename V>
  bool CounterMap<K, V>::contains(K const& key, V const& val) const
  {
    typename CounterMap<K, V>::CoreMap_t::const_iterator i( coreMap_.find(key) );
    return i == coreMap_.end() ? false : i->second.contains(val);
  }

  template <typename K, typename V>
  typename CounterMap<K, V>::Size_t CounterMap<K, V>::size(void) const
  {
    return coreMap_.size();
  }

  template <typename K, typename V>
  typename CounterMap<K, V>::Size_t CounterMap<K, V>::size(K const& key) const
  {
    typename CounterMap<K, V>::Counter_t const *counter = getCounter(key);
    return counter == NULL ? 0 : counter->size();
  }

  template <typename K, typename V>
  bool CounterMap<K, V>::empty(void) const
  {
    return coreMap_.empty();
  }

  template <typename K, typename V>
  typename CounterMap<K, V>::Count_t CounterMap<K, V>::getCount(K const& key, V const& val) const
  {
    typename CounterMap<K, V>::Counter_t const *counter = getCounter(key);
    return counter == NULL ? 0 : counter->getCount(val);
  }

  template <typename K, typename V>
  typename CounterMap<K, V>::Count_t CounterMap<K, V>::totalCount(void) const
  {
    if( !cachedTotal_->isSynched() ) {
      Count_t total(0);
      for( typename CounterMap<K, V>::CoreMap_t::const_iterator i( coreMap_.begin() );
	   i != coreMap_.end(); ++i )
	{
	  total += i->second.totalCount();
	}
      cachedTotal_->set(total);
    }
    return cachedTotal_->get();
  }
  
  template <typename K, typename V>
  typename CounterMap<K, V>::Count_t CounterMap<K, V>::totalCount(K const& key) const
  {
    typename CounterMap<K, V>::Counter_t const *counter = getCounter(key);
    return counter == NULL ? 0 : counter->totalCount();
  }

  //--------------------- Counters ---------------------------------
  
  template <typename K, typename V>
  Counter<V> const * const CounterMap<K, V>::getCounter(K const& key) const
  {
    typename CounterMap<K, V>::CoreMap_t::const_iterator i( coreMap_.find(key) );
    return i == coreMap_.end() ? NULL : &i->second;
  }
  
  template <typename K, typename V>
  Counter<V> const * const CounterMap<K, V>::getCounter(K && key) const
  {
    typename CounterMap<K, V>::CoreMap_t::const_iterator i( coreMap_.find(std::move(key)) );
    return i == coreMap_.end() ? NULL : &i->second;
  }

  template <typename K, typename V>
  Counter<V>& CounterMap<K, V>::ensureCounter(K const& key)
  {
    typename CounterMap<K, V>::CoreMap_t::iterator i( coreMap_.find(key) );
    if( i != coreMap_.end() )
      return i->second;
    return 
      coreMap_.insert(typename CounterMap<K, V>::CoreMap_t::value_type(key, 
								       counterFactory_->createCounter())).first->second;
  }
  
  template <typename K, typename V>
  Counter<V>& CounterMap<K, V>::ensureCounter(K && key)
  {
    typename CounterMap<K, V>::CoreMap_t::iterator i( coreMap_.find(key) );
    if( i != coreMap_.end() )
      return i->second;
    return
      coreMap_.insert(typename CounterMap<K, V>::CoreMap_t::value_type(std::move(key),
								       counterFactory_->createCounter())).first->second;
  }
  
  //--------------------- Traversal --------------------------------

  template <typename K, typename V>
  typename CounterMap<K, V>::ConstIterator CounterMap<K, V>::begin(void) const
  {
    return coreMap_.begin();
  }

  template <typename K, typename V>
  typename CounterMap<K, V>::ConstIterator CounterMap<K, V>::end(void) const
  {
    return coreMap_.end();
  }

  //--------------------- Equality ---------------------------------

  template <typename K, typename V>
  bool CounterMap<K, V>::operator==(CounterMap const& other) const
  {
    return coreMap_ == other.coreMap_;
  }

  template <typename K, typename V>
  bool CounterMap<K, V>::operator!=(CounterMap const& other) const
  {
    return !(operator==(other));
  }

  template <typename K, typename V>
  bool CounterMap<K, V>::equals(const CounterMap& other, Count_t precision) const
  {
    if( this == &other )           return true;
    if( size() != other.size() )   return false;
    for( typename CounterMap<K, V>::CoreMap_t::const_iterator i(coreMap_.begin());
	 i != coreMap_.end(); ++i )
      {
	typename CounterMap<K, V>::CoreMap_t::const_iterator 
	  oi( other.coreMap_.find( i->first ) );
	
	if( oi == other.coreMap_.end() || !i->second.equals( oi->second, precision ) )
	  return false;
      }
    return true;
  }

  template <typename K, typename V>
  CounterMap<K, V>& CounterMap<K, V>::operator+=(CounterMap const& rhs)
  {
    for( typename CounterMap<K, V>::ConstIterator i( rhs.begin() );
	 i != rhs.end(); ++i )
      {
	ensureCounter(i->first) += i->second;
      }

    return *this;
  }

  template <typename K, typename V>
  CounterMap<K, V>& CounterMap<K, V>::operator-=(CounterMap const& rhs)
  {
    for( typename CounterMap<K, V>::ConstIterator i( rhs.begin() );
	 i != rhs.end(); ++i )
      {
	ensureCounter(i->first) -= i->second;
      }

    return *this;
  }

  template <typename K, typename V>
  CounterMap<K, V> operator+(CounterMap<K, V> const& lhs, CounterMap<K, V> const& rhs)
  { CounterMap<K, V> tmp = lhs;  tmp += rhs; return tmp; }

  template <typename K, typename V>
  CounterMap<K, V> operator+(CounterMap<K, V> && tmp, CounterMap<K, V> const& rhs)
  { tmp += rhs; return std::move(tmp); }

  template <typename K, typename V>
  CounterMap<K, V> operator+(CounterMap<K, V> const& lhs, CounterMap<K, V> && tmp)
  { tmp += lhs; return std::move(tmp); }

  template <typename K, typename V>
  CounterMap<K, V> operator+(CounterMap<K, V> && tmp, CounterMap<K, V> && rhs)
  { tmp += rhs; return std::move(tmp); }

  template <typename K, typename V>
  CounterMap<K, V> operator-(CounterMap<K, V> const& lhs, CounterMap<K, V> const& rhs)
  { CounterMap<K, V> tmp = lhs; tmp -= rhs; return tmp; }

  template <typename K, typename V>
  CounterMap<K, V> operator-(CounterMap<K, V> && tmp, CounterMap<K, V> const& rhs)
  { tmp -= rhs; return std::move(tmp); }

  template <typename K, typename V>
  CounterMap<K, V> operator-(CounterMap<K, V> && tmp, CounterMap<K, V> && rhs)
  { tmp -= rhs; return std::move(tmp); }

  //----------------- Output Operator ------------------------

  template <typename K, typename V>
  std::ostream& operator<<(std::ostream& os, CounterMap<K, V> const& counterMap)
  {
    os << "[\n";
    if( counterMap.size() > 0 )
      {
	typename CounterMap<K, V>::ConstIterator i(counterMap.begin());
	os << " " << i->first << MAPPING_DELIMITER << i->second << "\n";
	for( ; ++i != counterMap.end(); )
	  os << " " <<  i->first << MAPPING_DELIMITER << i->second << "\n";
      }
    return os << "]";
  }


};

#endif // __COUNTER_MAP_IMPL_HPP__
