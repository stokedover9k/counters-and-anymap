/*! @file CounterMap.h
  @brief A map of counters suitable for use as a conditional distribution.

  @author Yuriy Skobov
 */

#ifndef __COUNTER_MAP_H__
#define __COUNTER_MAP_H__

#include <ostream>
#include <utility>

#include "Counters/Counter.hpp"
#include "Counters/CounterFactories.hpp"
#include "Counters/NumCache.hpp"
#include "AnyMap/AnyMap.hpp"

namespace Counters
{
  template <typename K, typename V>
  class CounterMap;

  template <typename K, typename V> CounterMap<K, V> operator+(CounterMap<K, V> const&, CounterMap<K, V> const&);
  template <typename K, typename V> CounterMap<K, V> operator+(CounterMap<K, V>     &&, CounterMap<K, V> const&);
  template <typename K, typename V> CounterMap<K, V> operator+(CounterMap<K, V>     &&, CounterMap<K, V>     &&);
  template <typename K, typename V> CounterMap<K, V> operator+(CounterMap<K, V> const&, CounterMap<K, V>     &&);
  template <typename K, typename V> CounterMap<K, V> operator-(CounterMap<K, V> const&, CounterMap<K, V> const&);
  template <typename K, typename V> CounterMap<K, V> operator-(CounterMap<K, V>     &&, CounterMap<K, V> const&);
  template <typename K, typename V> CounterMap<K, V> operator-(CounterMap<K, V>     &&, CounterMap<K, V>     &&);
  
  template <typename K, typename V> CounterMap<K, V> operator*(CounterMap<K, V> const&, typename CounterMap<K, V>::Count_t);
  template <typename K, typename V> CounterMap<K, V> operator*(CounterMap<K, V>     &&, typename CounterMap<K, V>::Count_t);
  template <typename K, typename V> CounterMap<K, V> operator*(typename CounterMap<K, V>::Count_t, CounterMap<K, V> const&);
  template <typename K, typename V> CounterMap<K, V> operator*(typename CounterMap<K, V>::Count_t, CounterMap<K, V>     &&);

  template <typename K, typename V> CounterMap<K, V> operator/(CounterMap<K, V> const&, typename CounterMap<K, V>::Count_t);
  template <typename K, typename V> CounterMap<K, V> operator/(CounterMap<K, V>     &&, typename CounterMap<K, V>::Count_t);

  /*!
   * @brief Outputs the CounterMap in a human readable format.
   */
  template <typename K, typename V> std::ostream& operator<<(std::ostream&, CounterMap<K, V> const&);
  
  /*!
   * @brief A mapping of keys to Counter values intended for use as a conditional
   * distribution.
   *
   * The CounterMap is essentially a map (this implementation uses AnyMap as the
   * underlying map). The key type of this map is the CounterMap template's K type
   * parameter. The mapped type of the map is the Counter<V> type.
   *
   * Additionally to keeping the key-counter association, the CounterMap keeps a 
   * cache. This cache stores the total count (the sum of all the counts stored in
   * the mapped Counter objects). This cache is synchronized every time 
   * totalCount(void) method is called and persists until the next modification of
   * the stored counts. This means that the total count must only be computed once
   * between modifications.
   * 
   * For cache details, see NumCache (note, that CounterMap always uses 
   * Counters::CACHE_POLICY_RELAXED for the caching policy).
   *
   * @param K Key type for the Counter mapping.
   * @param V Value type for the mapped Counter objects.
   */
  template <typename K, typename V>
  class CounterMap
  {
  public:
    /*! @brief Type parameter K a.k.a. type of top-level keys in the mapping. */
    typedef K Key_t;
    /*! @brief Type parameter V a.k.a. type of values stored by the mapped counters.*/
    typedef V Value_t;
    /*! @brief Type of Counters associated with the top-level keys. */
    typedef Counter<V> Counter_t;
    /*! @brief Type used by the mapped Counter objects to store counts. */
    typedef typename Counter_t::Count_t Count_t;
    /*! @brief Type of map used to maintain the key-counter associations. */
    typedef MapTypeErasure::AnyMap<K, Counter_t> CoreMap_t;
    /*! @brief The type to which the iterators dereference (should be
     *  std::pair<K, Counters::Counter<V> >). */
    typedef typename CoreMap_t::value_type IteratorValue_t;
    /*! @brief A constant iterator for the underlying map. */
    typedef typename CoreMap_t::const_iterator ConstIterator;
    /*! @brief Unsigned integer type that can represent any non-negative value. */
    typedef typename CoreMap_t::size_type Size_t;

    /*!  @name Constructors, Destructor, Assignment, and Swap
     *   @{
     */
    /*! @brief Standard copy constructor. */
    CounterMap( CounterMap const & other );
    /*! @brief Move copy constructor. */
    CounterMap( CounterMap && other );
    /*! 
     * @brief Constructs the CounterMap with the specified underlying map and
     * counter factory.
     * @param coreMap The map used to hold the key-counter associations.
     * @param counterFactory The factory used to create new Counter objects. The 
     * provided factory is duplicated via CounterFactory::clone() method.
     */
    explicit CounterMap( CoreMap_t coreMap
			 = CoreMap_t(),
			 CounterFactory<V> const & counterFactory
			 = DefaultCounterFactory<V>() );
    ~CounterMap();
    
    /*!
     * @brief Standard Assignment. Copies the keys and their associated Counter 
     * objects as well as the cache and CounterFactory.
     * @param rhs Original CounterMap to be copied.
     * @return This CounterMap.
     */
    CounterMap& operator=(CounterMap const & rhs);

    /*!
     * @brief Move Assignment. Steals the resources from a temporary CounterMap rhs 
     * equating this CounterMap to it.
     * @param rhs Original CounterMap to swap contents with.
     */
    CounterMap& operator=(CounterMap && rhs);

    /*!
     * @brief Swaps contents with another CounterMap in constant time (assuming
     * constant time swap of the underlying maps.
     * @param other CounterMap to swap contents with.
     */
    void swap( CounterMap& other );
    /*!  @} */

    /*!  @name Modifiers
     *   @{
     */
    /*!
     * @brief Increments the count associated with the key-value pair by the given
     * amount.
     *
     * If the key is not in the CounterMap, it will be created with an
     * associated Counter. If the value is not in the assocated Counter, it will be
     * created and set to 'count'.
     * @param key Key whose Counter is querried and modified.
     * @param val Value in K's Counter which is incremented.
     * @param count Amount by which the count is incremented.
     */
    void incrementCount(K const& key, V const& val, Count_t count);
    /*! @overload incrementCount(K const& key, V const& val, Count_t count); */
    void incrementCount(K     && key, V const& val, Count_t count);
    /*! @overload incrementCount(K const& key, V const& val, Count_t count); */
    void incrementCount(K     && key, V     && val, Count_t count);
    /*! @overload incrementCount(K const& key, V const& val, Count_t count); */
    void incrementCount(K const& key, V     && val, Count_t count);

    /*!
     * @brief Sets the count associated with the key-value pair to the given count.
     *
     * If the key is not in the CounterMap, it will be created with an
     * associated Counter. If the value is not in the assocated Counter, it will be
     * created and set to 'count'.
     * @param key Key whose Counter is querried and modified.
     * @param val Value in K's Counter which is set.
     * @param count Amount to which the count is set.
     */
    void setCount(K const& key, V const& val, Count_t count);
    /*! @overload setCount(K const& key, V const& val, Count_t count); */
    void setCount(K     && key, V const& val, Count_t count);
    /*! @overload setCount(K const& key, V const& val, Count_t count); */
    void setCount(K     && key, V     && val, Count_t count);
    /*! @overload setCount(K const& key, V const& val, Count_t count); */
    void setCount(K const& key, V     && val, Count_t count);

    /*!
     * @brief Removes the key and its associated Counter.
     * @param key Key to be removed from the mapping.
     */
    void remove(K const& key);

    /*!
     * @brief Removes the count associated with the key-value pair if it exists.
     * @param key Key under which a count is removed.
     * @param val Value which is removed from the key's Counter.
     */
    void remove(K const& key, V const& val);

    /*!
     * @brief Calls normalize on each of the stored counters.
     */
    void conditionalNormalize(void);
    /*!  @} */

    /*!  @name Lookup
     *   @{  
     */
    /*!
     * @brief Checks whether the key exists in the mapping.
     * @param key Key to be checked.
     * @return TRUE if the key is in the map, FALSE otherwise.
     */
    bool contains(K const& key) const;

    /*!
     * @brief Checks whether the given key-value pair exists in the mapping.
     * @param key Key to be checked.
     * @param val Value to be checked under the key.
     * @return TRUE if the key is in the map and key's associated Counter contains
     * the given value, FALSE otherwise.
     */
    bool contains(K const& key, V const& val) const;

    /*!
     * @brief Reports the number of top level keys in the mapping.
     * @return Number of top level keys in the mapping.
     */
    Size_t size(void) const;

    /*!
     * @brief Reports the number of values in the Counter under key.
     * @param key Key whose counter is querried for size.
     * @return Number of values in the Counter under 'key' or 0 if no such counter
     * exists.
     */
    Size_t size(K const& key) const;

    /*!
     * @brief Checks if there are any keys in the mapping.
     * @return FALSE if any keys are stored in the mapping, TRUE otherwise.
     */
    bool empty(void) const;

    /*!
     * @brief Reports the count associated with the given key-value pair.
     * @param key Key whose Counter is querried for the count of 'val'.
     * @param val Values whose count is querried in the Counter under 'key'.
     * @return Count associated with 'val' in the Counter associated with 'key' or 0
     * if either such Counter does not exist or such Counter does not contain 'val'.
     */
    CounterMap::Count_t getCount(K const& key, V const& val) const;

    /*!
     * @brief Reports the sum of all counts stored in the CounterMap.
     * @return Sum of all counts in all the Counter objects stored in the mapping.
     */
    CounterMap::Count_t totalCount(void) const;

    /*!
     * @brief Reports the total count of the Counter associated with 'key'.
     * @return Sum of all counts stored in the Counter associated with 'key' or 0 if
     * no such Counter exists.
     */
    CounterMap::Count_t totalCount(K const& key) const;
    /*!  @} */

    /*!  @name Counters
     *   @{  
     */
    /*!
     * @brief Returns a pointer to a Counter associated with the given key or NULL.
     * @return A pointer to a Counter associated with the given key or the NULL 
     * pointer if no such Counter exists in the mapping.
     */
    Counter<V> const * const getCounter(K const& key) const;
    /*! @overload getCounter(K const& key) const */
    Counter<V> const * const getCounter(K     && key) const;
    /*!  @} */

    /*!  @name Traversal
     *   @{  
     */
    /*!
     * @brief Returns a constant iterator pointing to the first value stored in the
     * mapping.
     * @return A constant iterator to the first element in the mapping or an iterator
     * to past-end if the map is empty.
     */
    ConstIterator begin(void) const;

    /*!
     * @brief Returns a constant iterator pointing past the end of the container.
     * @return A constant iterator pointing past the last element stored in the
     * mapping.
     */
    ConstIterator end(void) const;
    /*!  @} */

    /*!  @name Equality
     *   @{  
     */
    /*!
     * @brief Compares this CounterMap with another based on their keys and assocaited
     * Counter objects. 
     *
     * Uses the equality operator on the Counter objects. See 
     * equals() for approximate equality.
     * @param other Other CounterMap to be compared.
     * @return TRUE if the counter maps contain the same sets of keys and equal
     * associated counters, FALSE otherwise.
     */
    bool operator==(CounterMap const& other) const;

    /*!
     * @brief Equivalte to !(this->operator==(other)).
     * @param other Other counter map to be compared.
     * @return TRUE if the counter maps differ by at least one key or a key's 
     * associated Counter, FALSE otherwise.
     */
    bool operator!=(CounterMap const& other) const;

    /*!
     * @brief Similar to operator==, but uses specified precision in comparing the
     * matching counts. Also see Counter::equals().
     * @param other Other CounterMap to be compared to this CounterMap.
     * @param precision The difference allowed between the corresponding counts
     * before the corresponding counters are declared not equal.
     * @return TRUE if all the corresponding counts exist and are within the specified
     * margin, FALSE otherwise.
     */
    bool equals(const CounterMap& other, 
		Count_t precision=std::numeric_limits<Count_t>::epsilon()) const;
    /*!  @} */

    /*!  @name Arithmetic Operators
     *   @{  
     */
    /*!
     * @brief Increments each count by the amount stored in the other CounterMap.
     *
     * Querries the CounterMap for a Counter under each of the keys of the other
     * CounterMap and calls operator+= on the Counter in this CounterMap providing as
     * its argument the corresponding Counter from the other CounterMap.
     * @param rhs The CounterMap whose values are to be added to this CounterMap.
     * @return This modified CounterMap.
     */
    CounterMap& operator+=(CounterMap const& rhs);
    /*!
     * @brief Decrements each count by the amount stored in the other CounterMap.
     *
     * Querries the CounterMap for a Counter under each of the keys of the other
     * CounterMap and calls operator-= on the Counter in this CounterMap providing as
     * its argument the corresponding Counter from the other CounterMap.
     * @param rhs The CounterMap whose values are to be subtracted from thie 
     * CounterMap.
     * @return This modified CounterMap.
     */
    CounterMap& operator-=(CounterMap const& rhs);

    /*!
     * @brief Adds all the counts in two CounterMaps to produce a new CounterMap.
     * @return New CounterMap whose counts are the sums of the two CounterMaps.
     */
    friend CounterMap operator+ <K, V>(CounterMap const&, CounterMap const&);
    /*! @overload operator+ <K, V>(CounterMap const&, CounterMap const&) */
    friend CounterMap operator+ <K, V>(CounterMap     &&, CounterMap const&);
    /*! @overload operator+ <K, V>(CounterMap const&, CounterMap const&) */
    friend CounterMap operator+ <K, V>(CounterMap     &&, CounterMap     &&);
    /*! @overload operator+ <K, V>(CounterMap const&, CounterMap const&) */
    friend CounterMap operator+ <K, V>(CounterMap const&, CounterMap     &&);

    /*!
     * @brief Subtracts all the counts of the CounterMap on the right of the operator
     * from the counts of the CounterMap on the left side.
     * @return New CounterMap whose counts are the differences of the two CounterMaps'
     * counts.
     */
    friend CounterMap operator- <K, V>(CounterMap const&, CounterMap const&);
    /*! @overload operator- <K, V>(CounterMap const&, CounterMap const&) */
    friend CounterMap operator- <K, V>(CounterMap     &&, CounterMap const&);
    /*! @overload operator- <K, V>(CounterMap const&, CounterMap const&) */
    friend CounterMap operator- <K, V>(CounterMap     &&, CounterMap     &&);
    /*! @overload operator- <K, V>(CounterMap const&, CounterMap const&) */

    /*!
     * @brief Multiplies every count stored in this CounterMap by num.
     * @param num Number by which each count is scaled.
     * @return This CounterMap.
     */
    CounterMap& operator*=(Count_t scale);

    /*!
     * @brief Divides every count stored in this CounterMap by num.
     * @param num Number by which each count is divided.
     * @return This CounterMap.
     */
    CounterMap& operator/=(Count_t num);

    /*!
     * @brief Multiplies all the counts of the CounterMap by the number.
     * @param cm CounterMap whose counts are multiplied.
     * @param num Number by which all the counts are multiplied.
     * @return New CounterMap whose counts num times those of cm.
     */
    friend CounterMap operator* <K, V>(CounterMap<K, V> const& cm, Count_t num);
    /*! @overload operator*(CounterMap<K, V> const& cm, Count_t num) */
    friend CounterMap operator* <K, V>(CounterMap<K, V>     && cm, Count_t num);
    /*! @overload operator*(CounterMap<K, V> const& cm, Count_t num) */
    friend CounterMap operator* <K, V>(Count_t num, CounterMap<K, V> const& cm);
    /*! @overload operator*(CounterMap<K, V> const& cm, Count_t num) */
    friend CounterMap operator* <K, V>(Count_t num, CounterMap<K, V>     && cm);

    /*!
     * @brief Divides all the counts of the CounterMap by the number.
     * @param cm CounterMap whose counts are divided.
     * @param num Number by which all the counts are divided.
     * @return New CounterMap whose counts 1.0/num times those of cm.
     */
    friend CounterMap operator/ <K, V>(CounterMap<K, V> const& cm, Count_t num);
    /*! @overload operator/(CounterMap<K, V> const& cm, Count_t num) */
    friend CounterMap operator/ <K, V>(CounterMap<K, V>     && cm, Count_t num);

    /*!  @} */
    
  protected:
    /*!
     * @brief Ensures that the Counter associated with the given key exists, then
     * returns it.
     * @param key Key whose Counter's existence is ensured.
     * @return Counter associated with the given key.
     */
    Counter<V>& ensureCounter(K const& key);

    /*! @overload ensureCounter(K const& key) */
    Counter<V>& ensureCounter(K     && key);

  private:
    CoreMap_t coreMap_;

    CounterFactory<V> *counterFactory_;
    
    // total cache:
    typedef NumCache<Count_t> CountCache;
    CountCache *cachedTotal_;
  };

};

#include "Counters/details/_CounterMap.IMPL.hpp"

#endif //__COUNTER_MAP_H__
