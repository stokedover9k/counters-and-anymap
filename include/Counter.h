/*! @file Counter.h
  @brief A counter with templated key types intended for use as a probability 
  distribution.
  
  @author Yuriy Skobov
*/

#ifndef __COUNTER_H__
#define __COUNTER_H__

#include <boost/assert.hpp>

#include <typeinfo>

#include <ostream>
#include <boost/unordered_map.hpp>
#include <boost/any.hpp>

#ifndef  COUNT_T_DEF
#define  COUNT_T_DEF 1
typedef double count_t;
#endif //COUNT_T_DEF

template<typename V>
class Counter
{
public:

  typedef V Value_t;
  typedef count_t Count_t;

  Counter( const boost::any& anyMap = boost::unordered_map<V, Count_t>() )
    : anyMap_(anyMap) 
  {}

  Counter( const Counter& other )
    : anyMap_(other.anyMap_)
  {}

  Counter( const Counter& other, const boost::any& anyMap )
    : anyMap_(anyMap)
  {
    //BOOST_ASSERT( (*anyMap).size() == 0 );
    //(*anyMap_).insert( (*other.anyMap_).begin(), (*other.anyMap_).end() );
  }


  void doStuff() const { std::cout << "doing stuff..." << std::endl; }

private:

  boost::any anyMap_;
};

#endif //__COUNTER_H__
