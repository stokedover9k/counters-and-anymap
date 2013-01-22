/*! @file CounterFactories.h
  @brief A set of Factory classes for Counter types.

  @author Yuriy Skobov
*/

#ifndef __COUNTER_FACTORIES_H__
#define __COUNTER_FACTORIES_H__

#include "Counter.h"
#include "MapTypeErasure/AnyMap.hpp"

namespace Counters
{

  /*! @brief A pure virtual type for creating Counters. */
  template <typename V>
  struct CounterFactory
  {
    /*! @brief Constructs and returns a Counter object. */
    virtual Counter<V> createCounter(void) const = 0;
    /*! @brief Clones the factory. Caller responsible for deletion. */
    virtual CounterFactory<V> *clone(void) const = 0;
  };

  /*! @brief A factory type which creates default Counter objects. */
  template <typename V>
  struct DefaultCounterFactory : public CounterFactory<V>
  {
    Counter<V> createCounter(void) const {
      return Counter<V>(); }
    DefaultCounterFactory<V> *clone(void) const {
      return new DefaultCounterFactory<V>(*this); }
  };

  /*! @brief A factory type which creates copies of the specified Counter object. */
  template <typename V>
  struct CopyCounterFactory : public CounterFactory<V>
  {
    CopyCounterFactory( Counter<V> counter ) : source_(std::move(counter)) {}
    
    Counter<V> createCounter(void) const {
      return Counter<V>(source_); }
    CopyCounterFactory<V> *clone(void) const {
      return new CopyCounterFactory<V>(*this); }
    
  private:
    Counter<V> source_;
  };

  /*! @brief A factory type which creates Counter objects which have a map of type
   *  CoreMap at their core. 
   */
  template <typename V, typename CoreMap>
  struct MapTypeCounterFactory : public CounterFactory<V>
  {
    Counter<V> createCounter(void) const
    {
      return Counter<V>( typename MapTypeErasure::AnyMap<V, typename Counter<V>::Count_t>( CoreMap() ) );
    }

    MapTypeCounterFactory<V, CoreMap> *clone(void) const {
      return new MapTypeCounterFactory<V, CoreMap>(*this); }
  };

};


#endif // __COUNTER_FACTORIES_H__
