/*!@file Counter.h
 * @brief A counter with templated key types intended for use as a probability 
 * distribution.
 * 
 * @author Yuriy Skobov
 */

#ifndef __COUNTER_H__
#define __COUNTER_H__

#include <ostream>
#include <limits>
#include <cmath>
#include <utility>
#include "MapTypeErasure/AnyMap.hpp"
#include "Util/NumCache.h"


namespace Counters
{
#ifndef  COUNT_T_DEF
#define  COUNT_T_DEF 1
  typedef double CountersCount_t;
#endif //COUNT_T_DEF
  
  template <typename V>
  class Counter;

  template <typename V>
  std::ostream& operator<<(std::ostream&, const Counter<V>&);

  /*!
   * @brief A counter with templated key types intended for use as a probability 
   * distribution.
   *
   * The Counter is essentially a map (this implementation uses AnyMap as the
   * underlying map). The key type of this map is the Counter template's V type
   * parameter. The mapped type of this map is the Counter::Count_t type (currently,
   * this is 'double').
   *
   * Additionally to keeping the counts associated with each value, the Counter
   * keeps a cache. This cache stores the total count (the sum of all counts 
   * stored in the Counter). By default, the caching policy is set to "relaxed"
   * meaning that any modification to the counts will cause the total to be
   * desynchronized. It will be synchronized again next time total count is
   * computed. It is possible to the the caching policy to "persistent" (see
   * methods in "Caching Policy" group). With persistent caching, the Counter is
   * able to maintain the total throughout setCount and incrementCount operations
   * as well as through some of the arithmetic operations (e.g. scale all by a 
   * value or add a value to all counts). However, due to the round off errors, 
   * it is recommended to avoid persistent caching if large amounts of count 
   * modification are expected. Alternatively, one can manually desynchronized
   * the cache by calling resetCache() before getting totalCount(). Note that 
   * copy and assignment operations include copying the cache, although equality
   * does not take the cache into consideration.
   *
   * For cache details, see NumCache.
   *
   * @param V Value type whose counts are stored.
   */
  template<typename V>
  class Counter
  {
  public:

    /*! @brief Type parameter V a.k.a. type of values whose counts are stored. */
    typedef V Value_t;
    /*! @brief Type used for stored counts (currently double). */
    typedef CountersCount_t Count_t;
    /*! @brief Type of map used to maintain value-count associations. */
    typedef MapTypeErasure::AnyMap<V, Count_t> CoreMap_t;
    /*! @brief The type to which the iterators dereference (should be 
     *  std::pair<V, double>). */
    typedef typename CoreMap_t::value_type IteratorValue_t;
    /*! @brief A constant iterator for the underlying map. */
    typedef typename CoreMap_t::const_iterator ConstIterator;
    /*! @brief Unsigned integer type that can represent any non-negative value.*/
    typedef typename CoreMap_t::size_type Size_t;

    /*! @name Constructors, Destructor, Assignment, and Swap
     *  @{
     */
    /*! @brief Default constructor. */
    Counter();
    /*! @brief Standard copy constructor. */
    Counter( const Counter & other );
    /*! @brief Move copy constructor. */
    Counter( Counter && other );

    /*! @brief Constructs the Counter with the given map, copying or moving it.*/
    explicit Counter( CoreMap_t coreMap );
    /*!
     * @brief Increments all the elements in the range by the given count.
     *
     * Iterates through the elements starting at i1 and ending at but not
     * including i2. Each element is incremented by count. Repeats of values
     * are incremented multiple times. InputIterator type must dereference to
     * type V.
     *
     * @param i1 Iterator pointing to the first value to be incremented.
     * @param i2 Iterator pointing past the last value to be incremented.
     * @param count The count by which each occurence of a value found in the
     * range is incremented.
     */
    template <typename InputIterator>
    Counter( InputIterator i1, InputIterator i2, Count_t count=1.0 );

    /*!
     * @brief Standard Assignment. Copies the values and their associated counts as 
     * well as the cache.
     * @param rhs Original counter to be copied.
     * @return This counter.
     */
    Counter& operator=( Counter const& rhs );

    /*!
     * @brief Move Assignment. Steals the resources from a temporary Counter rhs 
     * equating this Counter to it.
     * @param rhs Oirignal counter to be equated to.
     * @return This counter.
     */
    Counter& operator=( Counter && rhs );

    /*!
     * @brief Swaps contents with another Counter in constant time (assuming
     * constant time swap of the underlying maps.
     * @param other Counter to swap contents with.
     */
    void swap( Counter& other );

    ~Counter();
    /*! @} */

    /*!  @name Modifiers
     *   @{  */
    /*!
     * @brief Increments the count associated with the given key by the given
     * value.  If the key is not in the counter, it will be added and its
     * associated count will be set to 'val'.
     * @param val Value whose count is incremented.
     * @param count Count by which the count under val is incremented.
     */
    void incrementCount( V const& val, Count_t count );

    /*!
     * @brief Increments the count associated with the given key by the given
     * value.  If the key is not in the counter, it will be added and its
     * associated count will be set to 'val'.
     * @param val Value whose count is incremented.
     * @param count Count by which the count under val is incremented.
     */
    void incrementCount( V && val, Count_t count );
    
    /*!
     * @brief Increments all values in the range by the given count.
     *
     * Each value starting at the one pointed to by 'first' and ending at but not
     * including one pointed to by 'last' is incremented by 'count'. The repeated
     * values in the range are incremented multiple times. Iterators must be
     * dereferenced (via operator*()) to the Value type.
     * @param first The iterator pointing to the first value to be incremented.
     * @param last The iterator pointing past the last value to be incremented.
     * @param count The count by which each value is incremented.
     */
    template <typename InputIterator>
    void incrementAll( InputIterator first, InputIterator last, Count_t count );

    /*!
     * @brief Set the count associated with the given key to the given value.  If
     * the key is not in the counter, it is added.
     * @param val Value whose count is set.
     * @param count Value to which the count under 'val' is set.
     */
    void setCount( V const& val, Count_t count );

    /*!
     * @brief Set the count associated with the given key to the given value.  If
     * the key is not in the counter, it is added.
     * @param val Value whose count is set.
     * @param count Value to which the count under 'val' is set.
     */
    void setCount( V && val, Count_t count );

    /*!
     * @brief Scales the counts stored in the counter so that they add up to 1.
     *
     * This is nearly equivalent to calling *this /= this->totalCount().
     * If totalCount() returns 0.0, all values are set to 0. Otherwise, they are
     * set to add up to 1.0. Synchronizes the cache.
     */
    void normalize(void);

    /*!
     * @brief Removes the key and its associated count from the counter.
     * @param val Value to be removed along with its count.
     */
    void remove( V const& val );
    /*! @} */
    
    /*!  @name Size and Capacity
     *   @{ */
    /*!
     * @brief Checks whether the counter is empty. Equivalent to size() == 0.
     * @return TRUE if there are no values stored in the counter, FALSE 
     * otherwise.
     */
    bool empty(void) const;
    /*!
     * @brief Returns the number of values in the counter.
     * @return The number of values in the counter.
     */
    Size_t size(void) const;
    /*!
     * @brief Returns the maximum number of values the underlying map can store.
     * @return The maxiumum number of values the underlying map can store.
     */
    Size_t maxSize(void) const;
    /*! @} */
    
    /*!  @name Lookup
     *   @{ */
    /*!
     * @brief Checks whether the value is recorded in the counter.
     * @param val Value whose existence in the counter is checked.
     * @return TRUE if value is already in the counter, FALSE otherwise.
     */
    bool contains( V const& val ) const;

    /*!
     * @brief Returns the count associated with the given value. If the value is 
     * not contained in the counter, returns 0.
     * @param val Value whose associated count is requested.
     * @return The count associated with 'val' or 0.
     */
    Count_t getCount( V const& val ) const;

    /*!
     * @brief Returns the sum of all the counts stored in the counter.
     *
     * The counter attempts to cache the total count so that it does not need to
     * calculate it every time this method is called.  Some non-constant 
     * methods will cause this synchronization to be broken. Though currently 
     * many of the methods will keep the total synchronized, this behavior is not
     * guaranteed and is subject to change.
     * @return The sum of all the counts stored in the counter.
     */
    Count_t totalCount(void) const;

    /*!
     * @brief Returns the value associated with the greatest count in the
     * counter.
     * @return The key whose associated count is the greatest in the counter.
     */
    V maxValue(void) const;
    /*! @} */
    
    /*!  @name Traversal
     *   @{ */
    /*!
     * @brief Returns the iterator over the counter which points to the first 
     * value in this counter (or an iterator equaling that returned by 'end()' 
     * if the counter is empty.
     * @return An iterator over the values of the counter.
     */
    ConstIterator begin(void) const;
    /*! 
     * @brief Returns an iterator just past the last value in the counter.
     * @return An iterator just past the last value in the counter.
     */
    ConstIterator end(void) const;
    /*! @} */

    /*!  @name Equality
     *   @{ */
    /*!
     * @brief Compares counters based on the values and their associated counts.
     * Uses equality operator to compare counts - see equals() for approximate
     * equality.
     * @param o Other counter to be compared.
     * @return TRUE if the counters contain the same set of values with the same
     * associated counts, FALSE otherwise.
     */
    bool operator==( Counter const& o ) const;

    /*!
     * @brief Equivalent to !(this->operator==(o)).
     * @param o Other counter to be compared.
     * @return TRUE if the counters differ by at least one value or a value's 
     * associated value, FALSE otherwise.
     */
    bool operator!=( Counter const& o ) const;

    /*!
     * @brief Similar to operator==, but uses specified precision in comparing
     * the matching counts.
     *
     * Counts a and b are declared different if std::fabs(a-b) => precision.
     * @param other Other counter to be compared to this counter.
     * @param precision The difference allowed between the corresponding counts
     * before the couters are declared not equal.
     * @return TRUE if all corresponding counts exist and are within the specified
     * margin, FALSE 
     * otherwise.
     */
    bool equals(const Counter& other, Count_t precision =
		std::numeric_limits<Count_t>::epsilon() ) const;
    /*! @} */

    /*! @name Caching Policy
     *  @{ */
    /*!
     * @brief Sets caching policy.
     * @param cachePolicy Mode to which caching policy is set:
     * Counters::CACHE_POLICY_PERSISTENT or Counters::CACHE_POLICY_RELAXED.
     */
    void setCachePolicy(NumCachePolicy cachePolicy) const;
    /*!
     * @brief Reports current caching policy.
     * @return Counters::CACHE_POLICY_PERSISTENT or 
     * Counters::CACHE_POLICY_RELAXED.
     */
    NumCachePolicy getCachePolicy(void) const;
    /*!
     * @brief Sets the cache to "unsynchronized". Does not affect the policy.
     */
    void resetCache(void) const;
    /*!  @} */

    //---------------- Arithmetic Operators ---------------------
    
    /*! @name Arithmetic Operators
     *  @{ */
    /*!
     * @brief Adds each count of the other counter to the count under the same
     * key in this counter.
     * @param o Other counter whose counts are to be added.
     * @return This counter.
     */
    Counter& operator+=(const Counter& o);

    /*!
     * @brief Subtracts each count of the other counter from the count under the
     * same key in this counter.
     * @param o Other counter whose counts are to be subtracted.
     * @return This counter.
     */
    Counter& operator-=(const Counter& o);

    /*! @brief Adds 'val' to each count in the counter.
     * @param count Count to be added to each count in the counter.
     * @return This counter.
     */
    Counter& operator+=(Count_t count);

    /*!
     * @brief Subtracts 'val' from each count in the counter.
     * @param count Count to be subtracted from each count in the counter.
     * @return This counter.
     */
    Counter& operator-=(Count_t count);

    /*!
     * @brief Multiplies each count in the counter by 'val'.
     * @param count Count by which all the counts are multiplied.
     * @return This counter.
     */
    Counter& operator*=(Count_t count);

    /*!
     * @brief Divides each count in the counter by 'val'.
     * @param count Count by which all the counts are divided.
     * @return This counter.
     */
    Counter& operator/=(Count_t count);

    /*!
     * @brief Adds all counts of the two counters in a new counter.
     * @param o Other counter to be added with this counter.
     * @return New counter whose counts are the sums of the counts under the same
     * key in the two counters.
     */
    Counter operator+(const Counter& o) const;

    /*!
     * @brief Produces a new counter whose each count under a key 'k' is the 
     * difference (this->getCount(k) - o.getCount(k)).
     * @param o Other counter to be subtracted from this counter.
     * @return New counter whose counts are the differences of the counts under
     * the same keys in the two counters.
     */
    Counter operator-(const Counter& o) const;

    /*!
     * @brief Produces a new counter by increasing each count of this counter by
     * 'val'.
     * @param count The count added to each count.
     * @return The new counter whose each count is 'val' greater than the 
     * corresponding count in this counter.
     */
    Counter operator+(Count_t count) const;

    /*!
     * @brief Produces a new counter by decreasing each count of this counter by
     * 'val'.
     * @param count The count subtracted from each count.
     * @return The new counter whose each count is 'val' less than the 
     * corresponding count in this counter.
     */
    Counter operator-(Count_t count) const;

    /*!
     * @brief Produces a new counter by multiplying each count of this counter by
     * 'val'.
     * @param count The value by which each count is multiplied.
     * @return The new counter whose each count is 'val' times greater than the
     * corresponding count in this counter.
     */
    Counter operator*(Count_t count) const;

    /*!
     * @brief Produces a new counter by dividing each count of this counter by 
     * 'val'.
     * @param count The value by which each count is divided.
     * @return The new counter whose each count is 'val' times less than the
     * corresponding count in this counter.
     */
    Counter operator/(Count_t count) const;
    /*! @} */

  private:

    CoreMap_t coreMap_;

    // total cache:
    typedef NumCache<Count_t> CountCache;
    CountCache *cachedTotal_;

#ifdef __COUNTER_DEBUG__
  public:
#else
  private:
#endif
    bool isTotalSynched(void) const;

  private:

  };

};

#include "counterDetails/CounterImpl.hpp"

#endif //__COUNTER_H__
