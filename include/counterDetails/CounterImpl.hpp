#ifndef __COUNTER_IMPL_HPP__
#define __COUNTER_IMPL_HPP__

// The following include is added for two reasons even though it should not
// be needed because this file itself is included from Counter.h.
// Reason 1: form - as this serves as the implementation file, it makes sense
// to point from here to the header declaring the typename.
// Reason 2: EDE and Semantic autocompletion needs this to help with member
// variables and STL typenamees included from the header.
#include "Counter.h"

namespace Counters
{
  const static char* const MAPPING_DELIMITER = "=>";

  //------------- Constructors, Destructor, Assignment -------------------------

  template <typename V>
  Counter<V>::Counter() 
    : coreMap_(),
      cachedTotal_( new CountCache(0, CACHE_POLICY_RELAXED, true) )
  {}

  template <typename V>
  Counter<V>::Counter( Counter const& other )
    : coreMap_( other.coreMap_ ),
      cachedTotal_( new CountCache( *other.cachedTotal_ ) )
  {
  }

  template <typename V>
  Counter<V>::Counter( Counter && other )
    : coreMap_(), cachedTotal_(NULL)
  {
    swap( other );
  }

  template <typename V>
  Counter<V>::Counter( CoreMap_t coreMap )
    : coreMap_(std::move(coreMap)),
      cachedTotal_( new CountCache(0, CACHE_POLICY_RELAXED, false) )
  {}

  template <typename V>
  template <typename InputIterator>
  Counter<V>::Counter( InputIterator first, InputIterator last, Count_t count )
    : coreMap_(),
      cachedTotal_( new CountCache(0, CACHE_POLICY_RELAXED, true) )
  {
    incrementAll( first, last, count );
  }

  template <typename V>
  Counter<V>& Counter<V>::operator=( Counter const & rhs )
  {
    if( this != &rhs ) {
      coreMap_ = rhs.coreMap_;
      *cachedTotal_ = *rhs.cachedTotal_;
    }
    return *this;
  }

  template <typename V>
  Counter<V>& Counter<V>::operator=( Counter<V> && rhs )
  {
    swap( rhs );
    return *this;
  }

  template <typename V>
  void Counter<V>::swap( Counter<V>& other )
  {
    coreMap_.swap( other.coreMap_ );
    { 
      using std::swap;
      swap( cachedTotal_, other.cachedTotal_ );
    }
  }

  template <typename V>
  Counter<V>::~Counter()
  {
    delete cachedTotal_;
  }

  //--------------------------- Modifiers --------------------------------------

  template <typename V>
  void Counter<V>::incrementCount( V const& val, Count_t count )
  {
    coreMap_[val] += count;
    *cachedTotal_ += count;
  }

  template <typename V>
  void Counter<V>::incrementCount( V && val, Count_t count )
  {
    coreMap_[std::move(val)] += count;
    *cachedTotal_ += count;
  }

  template <typename V>
  template <typename InputIterator>
  void Counter<V>::incrementAll( InputIterator first, InputIterator last,
				 Count_t count )
  {
    for( ; first != last; ++first )
      incrementCount(*first, count);
  }

  template <typename V>
  void Counter<V>::setCount( V const& val, Count_t count )
  {
    Count_t& storedCount = coreMap_[val];
    *cachedTotal_ += (count - storedCount);
    storedCount = count;
  }

  template <typename V>
  void Counter<V>::setCount( V && val, Count_t count )
  {
    Count_t& storedCount = coreMap_[val];
    *cachedTotal_ += (count - storedCount);
    storedCount = count;
  }

  template <typename V>
  void Counter<V>::normalize(void)
  {
    Count_t total( totalCount() );
    if( total != 0 )
      {
	(*this) /= total;
	cachedTotal_->set( 1.0 );
      }
    else
      {
	(*this) *= 0;
	cachedTotal_->set( 0 );
      }
  }

  template <typename V>
  void Counter<V>::remove( V const& val )
  {
    typename CoreMap_t::const_iterator i(coreMap_.find(val));
    if( i != coreMap_.end() )
      {
	*cachedTotal_ -= i->second;
	coreMap_.erase(i->first);
      }
  }

  template <typename V>
  bool Counter<V>::empty(void) const
  {
    return coreMap_.empty();
  }

  template <typename V>
  typename Counter<V>::Size_t Counter<V>::size(void) const
  {
    return coreMap_.size();
  }

  template <typename V>
  typename Counter<V>::Size_t Counter<V>::maxSize(void) const
  {
    return coreMap_.max_size();
  }

  template <typename V>
  bool Counter<V>::contains( V const& val ) const
  {
    return coreMap_.find(val) != coreMap_.end();
  }

  template <typename V>
  typename Counter<V>::Count_t Counter<V>::getCount( V const& val ) const
  {
    typename CoreMap_t::const_iterator i(coreMap_.find(val));
    return i == coreMap_.end() ? 0 : i->second;
  }

  template <typename V>
  typename Counter<V>::Count_t Counter<V>::totalCount(void) const
  {
    if( ! cachedTotal_->isSynched() )
      {
	Count_t sum(0);
	for( typename CoreMap_t::const_iterator i(coreMap_.begin()); i != coreMap_.end(); ++i )
	  sum += i->second;
	cachedTotal_->set( sum );
      }
    return cachedTotal_->get();
  }

  template <typename V>
  V Counter<V>::maxValue(void) const
  {
    V const* maxVal(NULL);
    Count_t max = std::numeric_limits<Count_t>::min();
    for( typename CoreMap_t::const_iterator i(coreMap_.begin()); i != coreMap_.end(); ++i )
      {
	if( i->second > max ) {
	  max = i->second;
	  maxVal = &i->first;
	}
      }
    return maxVal == NULL ? V() : *maxVal;
  }

  template <typename V>
  bool Counter<V>::isTotalSynched() const
  {
    return cachedTotal_->isSynched();
  }

  template <typename V>
  typename Counter<V>::ConstIterator Counter<V>::begin(void) const
  {
    return coreMap_.begin();
  }

  template <typename V>
  typename Counter<V>::ConstIterator Counter<V>::end(void) const
  {
    return coreMap_.end();
  }

  template <typename V>
  bool Counter<V>::operator==(const Counter<V>& o) const
  {
    return coreMap_ == o.coreMap_;
  }

  template <typename V>
  bool Counter<V>::operator!=(const Counter<V>& o) const
  {
    return !(operator==(o));
  }

  //----------------------- Caching Policy ------------------------------------

  template <typename V>
  void Counter<V>::setCachePolicy(NumCachePolicy cachePolicy) const
  {
    cachedTotal_->setCachePolicy( cachePolicy );
  }

  template <typename V>
  NumCachePolicy Counter<V>::getCachePolicy(void) const
  {
    return cachedTotal_->getCachePolicy();
  }

  template <typename V>
  void Counter<V>::resetCache(void) const
  {
    cachedTotal_->reset();
  }

  //----------------------- Arithmetic Operators ------------------------------

  template <typename V>
  Counter<V>& Counter<V>::operator+=(const Counter& o)
  {
    for( typename Counter<V>::ConstIterator i(o.begin()); i != o.end(); ++i )
      incrementCount( i->first, i->second );
    return *this;
  }

  template <typename V>
  Counter<V>& Counter<V>::operator-=(const Counter& o)
  {
    for( typename Counter<V>::ConstIterator i(o.begin()); i != o.end(); ++i )
      incrementCount( i->first, -i->second );
    return *this;
  }

  template <typename V>
  Counter<V>& Counter<V>::operator+=(Count_t count)
  {
    for( typename Counter<V>::CoreMap_t::iterator i(coreMap_.begin());
	 i != coreMap_.end(); ++i )
      i->second += count;
    *cachedTotal_ += (count * size());
    return *this;
  }

  template <typename V>
  Counter<V>& Counter<V>::operator-=(Count_t count)
  { return operator+=(-count); }

  template <typename V>
  Counter<V>& Counter<V>::operator*=(Count_t count)
  {
    for( typename Counter<V>::CoreMap_t::iterator i(coreMap_.begin());
	 i != coreMap_.end(); ++i )
      i->second *= count;
    *cachedTotal_ *= count;
    return *this;
  }

  template <typename V>
  Counter<V>& Counter<V>::operator/=(Count_t count)
  { return operator*=(1.0/count); }

  template <typename V>
  Counter<V> Counter<V>::operator+(const Counter<V>& o) const
  {
    Counter<V> tmp(*this);
    tmp += o;
    return tmp;
  }

  template <typename V>
  Counter<V> Counter<V>::operator-(const Counter<V>& o) const
  {
    Counter<V> tmp(*this);
    tmp -= o;
    return tmp;
  }

  template <typename V>
  Counter<V> Counter<V>::operator+(Count_t count) const
  {
    Counter<V> tmp(*this);
    tmp += count;
    return tmp;
  }

  template <typename V>
  Counter<V> Counter<V>::operator-(Count_t count) const
  { return operator+(-count); }

  template <typename V>
  Counter<V> Counter<V>::operator*(Count_t count) const
  {
    Counter<V> tmp(*this);
    tmp *= count;
    return tmp;
  }

  template <typename V>
  Counter<V> Counter<V>::operator/(Count_t count) const
  { return operator*(1.0/count); }

  //-------------------- Counts Equal ------------------------------------------

  template <typename V>
  bool Counter<V>::equals( const Counter<V>& o, Count_t precision ) const
  {
    if( this == &o ) return true;
    if( size() != o.size() ) return false;
    for( typename Counter<V>::CoreMap_t::const_iterator i(coreMap_.begin());
	 i != coreMap_.end(); ++i)
      {
	typename Counter<V>::CoreMap_t::const_iterator 
	  oi( o.coreMap_.find( i->first ) );

	if( oi == o.coreMap_.end() || 
	    std::fabs(i->second - oi->second) >= precision )
	  {
	    return false;
	  }
      }
    return true;
  }

  //-------------------- Output Operator ---------------------------------------

  template <typename V>
  std::ostream& operator<<(std::ostream& os, const Counter<V>& counter)
  {
    os << "[";
    if( counter.size() > 0 )
      {
	typename Counter<V>::ConstIterator i(counter.begin());
	os << i->first << MAPPING_DELIMITER << i->second;
	for( ; ++i != counter.end(); )
	  os << ", " << i->first << MAPPING_DELIMITER << i->second;
      }
    os << "]";
    return os;
  }

};

#endif // __COUNTER_IMPL_HPP__
