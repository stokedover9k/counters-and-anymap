#ifndef __NUM_CACHE_H__
#define __NUM_CACHE_H__

namespace Counters
{
  enum NumCachePolicy { CACHE_POLICY_PERSISTENT = 1 << 0,
			CACHE_POLICY_RELAXED    = 1 << 1 };

  /*!
   * @brief A simple Cache template with two modes of caching.
   *
   * The cache has three main method types: \n
   *   1) setting and synchronization \n
   *   2) cached value manipulation \n
   *   3) retrieval \n
   *
   * - Setting and Synchronization \n
   *   When the value is set (other than in the default constructor), the cache
   *   is marked synchronized. This means that isSynched() will return true and
   *   get() will return the value to which the cached value was set (see
   *   Retrieval).
   * - Cached Value Manipulation \n
   *   If the value is manipulated via arithmetic operators, the cached value is
   *   either adjusted accordingly and remains synchronized 
   *   (Counters::CACHE_POLICY_PERSISTENT caching policy) or the cache is
   *   desynchronized (Counters::CACHE_POLICY_RELAXED caching policy). In
   *   relaxed caching policy, the cache will remain unsynchronized until a new 
   *   value is explicitly set thus synchronizing the cache.
   * - Retrieval \n
   *   If the cache is synchronized, the get() method will retrieve the stored
   *   value. Otherwise, the value created by the default constructor of the
   *   parameter NumType is returned. It is therefore important in most 
   *   situations to call isSynched() before calling the get() method.
   *
   * @param NumType A numeric type which has +=, -=, *=, /=, and = operators.
   */
  template <typename NumType>
  class NumCache
  {
    NumType value_;
    NumCachePolicy cachePolicy_;
    bool synched_;

  public:
    /*!
     * @brief Creates a default unsynched cache with a relaxed policy
     * (Counters::CACHE_POLICY_RELAXED)
     */
    NumCache() : cachePolicy_(CACHE_POLICY_RELAXED), synched_(false), value_() {}

    /*!
     * @brief Constructs a synched cache with the given value and policy.
     * @param value Value stored in the cache.
     * @param cachePolicy Caching policy the cache is set to.
     */
    explicit 
    NumCache(NumType value, NumCachePolicy cachePolicy = CACHE_POLICY_RELAXED)
      : value_(value), synched_(true), cachePolicy_(cachePolicy) {}

    /*!
     * @brief Constructs a fully specified cache.
     * @param value Value stored in the cache.
     * @param cachePolicy Caching policy the cache is set to.
     * @param synched Indicate whether the cache is synched or not.
     */
    NumCache(NumType value, NumCachePolicy cachePolicy, bool synched)
      : value_(value), cachePolicy_(cachePolicy), synched_(synched) {}

    /*!
     * @brief Sets the value stored and marks the cache as synched.
     * @param value Value stored in the cache.
     */
    void set(NumType value) {
      value_ = value;
      synched_ = true;
    }

    /*!
     * @brief Retrieves the stored value or the default value if unsynched.
     * @return Stored value if synched, default constructed NumType, otherwise.
     */
    NumType get(void) const {
      return synched_ ? value_ : NumType();
    }

    /*!
     * @brief Sets the caching policy.
     * @param cachePolicy The policy to set the caching policy to
     * (Counters::CACHE_POLICY_RELAXED or Counters::CACHE_POLICY_PERSISTENT).
     */
    void setCachePolicy(NumCachePolicy cachePolicy) {
      cachePolicy_ = cachePolicy;
    }
    
    /*!
     * @brief Returns the current caching policy.
     * @return The current caching policy Counters::CACHE_POLICY_RELAXED or
     * Counters::CACHE_POLICY_PERSISTENT.
     */
    NumCachePolicy getCachePolicy(void) const {
      return cachePolicy_;
    }

    /*!
     * @brief Checks if the cached value is synched.
     * @return TRUE if synched, FALSE otherwise.
     */
    bool isSynched(void) const {
      return synched_;
    }

    /*!
     * @brief Marks the cache unsynched.
     */
    void reset(void) const {
      synched_ = false;
    }
    
    /*! @name Modification of Cached Value
     *  The matching arithmetic operators are applied to the stored value if
     *  the caching policy is set to Counters::CACHE_POLICY_PERSISTENT. Otherwise
     *  the cache is marked unsynched.
     *  @{
     */
    /*! @brief Adds n to stored value or marks cache unsynched. */
    void operator+=(NumType n) {
      if( cachePolicy_ == CACHE_POLICY_PERSISTENT )    value_ += n;
      else                                           synched_  = false;
    }

    /*! @brief Subtracts n from stored value or marks cache unsynched. */
    void operator-=(NumType n) {
      if( cachePolicy_ == CACHE_POLICY_PERSISTENT )    value_ -= n;
      else                                           synched_  = false;
    }

    /*! @brief Multiplies stored value by n or marks cache unsynched. */
    void operator*=(NumType n) {
      if( cachePolicy_ == CACHE_POLICY_PERSISTENT )    value_ *= n;
      else                                           synched_  = false;
    }

    /*! @brief Divides stored value by n or marks cache unsynched. */
    void operator/=(NumType n) {
      if( cachePolicy_ == CACHE_POLICY_PERSISTENT )    value_ /= n;
      else                                           synched_  = false;
    }
    /*! @} */
  };
};

#endif // __NUM_CACHE_H__
