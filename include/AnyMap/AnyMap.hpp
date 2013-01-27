#ifndef __ANY_MAP_HPP__
#define __ANY_MAP_HPP__

/*! 
 * @file AnyMap.hpp
 * @brief The MapTypeErasure namespace contains the AnyMap template class.
 * This template is a map wrapper which should work for any one-to-one
 * map with a standard interface (e.g. std::map, boost::unordered_map).
 * See MapTypeErasure::AnyMap description for information on the interface
 * required from the map's implementation.
 *
 * @author Yuriy Skobov
 */

#include "IteratorTypeErasure/any_iterator/any_iterator.hpp"

#include <boost/unordered_map.hpp>

#include <ostream>

#include <utility>


/*!
 */
namespace MapTypeErasure
{
  using namespace IteratorTypeErasure;

  /*!
   * @brief AnyMap is a map wrapper which can be constructed using any map which
   * satisfies the interface requirements (see below). AnyMap's interface is
   * similar to the common subset of the STL std::map and Boost 
   * boost::unordered_map. Iterators used by AnyMap are Thomas Becker's 
   * any_iterator.
   *
   * Instead of parametrising AnyMap with the policy types, it only needs to be
   * parametrized with the types of the keys (K) and of the mapped objects (V).
   * The rest is taken care of by the map object passed to AnyMap during 
   * construction. If none is passed, AnyMap with a default map type (currently,
   * boost::unordered_map with default type arguments) is created.
   *
   * Map Interface Requirements
   * The following methods are required from the underlying map of some type
   * MapType. Plus sign (+) indicates optional methods. Important types present
   * in the interface include: value_type, itr, and const_itr. For value_type,
   * see AnyMap::value_type. itr and const_itr are any types from which 
   * AnyMap::iterator and AnyMap::const_iterator respectively can be constructed.
   * Generally this means that they must be dereferenced to a value_type and have
   * the category of at least boost::forward_traversal_tag.
   * - MapType::MapType(MapType const &)
   * - bool MapType::empty() const
   * - size_type MapType::size() const
   * - size_type MapType::max_size() const
   * - V& MapType::operator[](K const &)
   * - V& MapType::operator[](K &&) (+)
   * - V& MapType::at(K const &) (+)
   * - V& MapType::at(K const &) const
   * - size_type MapType::count(K const &) const
   * - itr find(K const &)
   * - const_itr find(K const &) const
   * - itr begin()
   * - const_itr begin() const
   * - itr end()
   * - const_itr end() const
   * - std::pair<iteraotr, bool> insert(value_type const &)
   * - void insert(const_iterator, const_iterator)
   * - size_type erase(K const&)
   * - void clear()
   *
   * @param K Key type
   * @param V Mapped value type
   */
  template<typename K, typename V>
  class AnyMap
  {
  public:

    /*! @brief Type of the keys in the map. */
    typedef K key_type;
    /*! @brief Type of the mapped types stored in the map. */
    typedef V mapped_type;
    /*! @brief The Key-Value pair type stored in the container. */
    typedef std::pair<K const, V> value_type;
    /*! @brief Unsigned integer type that can represent any non-negative value.*/
    typedef size_t size_type;
  
    /*! @brief A forward iterator to value_type. */
    typedef any_iterator<value_type, boost::forward_traversal_tag> iterator;
    /*! @brief A forward iterator to const value_type. */
    typedef any_iterator<value_type const, boost::forward_traversal_tag> const_iterator;
    
    /*! @brief The map used in AnyMap's construction when no other is specified.
     */
    typedef boost::unordered_map<K, V> default_map_type;

  private:

    // struct: MapConcept
    //
    // This class is the parent of specificly map-type templated classes.
    // Its interface mimics that of the standard maps. These must be
    // overloaded in the derived classes.
    //
    // IMPLEMENTED are:
    // * Equality operators (== and !=)
    //   Checking equality is done by iterating through all elements contained
    //   in this map and looking them up in the other map. If all elements
    //   are matched and the numbers of stored elements are equal in both
    //   maps, the MapConcepts are declared to be equal.
    //   Inequality is implemented as !this->operator==(other).
    //
    // NOTE:
    // * Because of the implementation of equality it may be more efficient
    //   to compare two MapConcepts in a particular order. For example, given
    //   a map rbmap implemented as a red-and-black tree and a map hmap,
    //   implementated as a hash map, it should be more efficient to evaluate
    //   rbmap == hmap (O(n*1)) than hmap == rbmap (O(n*log(n))).
    //
    struct MapConcept
    {
      typedef typename AnyMap::size_type size_type;
    
      virtual ~MapConcept() {}

      // clone
      virtual MapConcept* clone() const = 0;

      // size and capasity
      virtual bool empty() const = 0;
      virtual size_type size() const = 0;
      virtual size_type max_size() const = 0;

      // lookup
      virtual V& operator[](const K& k) = 0;
      virtual V& operator[](K&& k) = 0;
      virtual V      & at(K const& k)       = 0;
      virtual V const& at(K const& k) const = 0;
      virtual typename AnyMap::iterator       find(K const& k)       = 0;
      virtual typename AnyMap::const_iterator find(K const& k) const = 0;
      virtual size_type count(K const& k) const = 0;

      // traversal iterators
      virtual typename AnyMap::iterator       begin()       = 0;
      virtual typename AnyMap::const_iterator begin() const = 0;
      virtual typename AnyMap::iterator         end()       = 0;
      virtual typename AnyMap::const_iterator   end() const = 0;

      // inserts
      virtual std::pair<iterator, bool> insert(value_type const& val)    = 0;
      virtual void insert(const_iterator i1, const_iterator i2) = 0;

      // erases
      virtual size_type erase(K const& k) = 0;

      // clear
      virtual void clear() = 0;

      // comparison
      bool operator==(const MapConcept& other) const {
	if( this == &other ) return true;
	if( size() != other.size() ) { return false; }
	for( AnyMap::const_iterator i(begin()); i != end(); ++i ) {
	  AnyMap::const_iterator const& oi( other.find( i->first ) );
	  if( oi == other.end() || oi->second != i->second )
	    {
	      return false;
	    }
	}
	return true;
      }

      bool operator!=(const MapConcept& other) const { return !operator==(other); }
    };

    // struct: MapModel
    // Templated with a map type. Derived from struct MapConcept.
    //
    // MapModel is a templat wrapper for a map type object. MapModel is
    // parametrized by the map's type. The majority of MapModel's methods
    // forward the call to the actual map object via a method with the same
    // name and a compatible signature.
    //
    template<typename MapType>
    struct MapModel : MapConcept
    {
      MapModel() : map_() {}

      // construct from map
      explicit MapModel( MapType m ) 
      { using std::swap;  swap(map_, m); }

      // copy constructor
      MapModel( const MapModel& o ) : map_( o.map_ ) {}

      // move constuctor
      MapModel( MapModel && other )
      { using std::swap;  swap(map_, other.map_); }
      
      virtual ~MapModel() {}

      // clone 
      MapModel* clone() const { return new MapModel<MapType>( map_ ); }

      // size and capasity
      bool empty() const                              { return map_.empty();    }
      typename MapConcept::size_type size() const     { return map_.size();     }
      typename MapConcept::size_type max_size() const { return map_.max_size(); }

      // lookup
      V& operator[] (const K  & k)          { return map_[k];    }
      V& operator[] (      K && k)          { return map_[k];    }
      V      & at(K const& k)               { return map_.at(k); }
      V const& at(K const& k) const         { return map_.at(k); }
      size_type count(K const& k) const                      { return map_.count(k); }
      typename AnyMap::iterator       find(K const& k)       { return AnyMap::iterator(map_.find(k)); }
      typename AnyMap::const_iterator find(K const& k) const { return AnyMap::const_iterator(map_.find(k)); }

      // traversal iterators
      typename AnyMap::iterator       begin()       { return AnyMap::iterator(map_.begin()); }
      typename AnyMap::const_iterator begin() const { return AnyMap::const_iterator(map_.begin()); }
      typename AnyMap::iterator         end()       { return AnyMap::iterator(map_.end()); }
      typename AnyMap::const_iterator   end() const { return AnyMap::const_iterator(map_.end()); }

      // inserts
      std::pair<iterator, bool> insert(value_type const& val)                { return map_.insert(val);    }
      void                      insert(const_iterator i1, const_iterator i2) { return map_.insert(i1, i2); }

      // erases
      size_type erase(K const& k) { return map_.erase(k); }

      // clear
      void clear() { map_.clear(); }

    private:
      MapType map_;
    };

  public:

    /*! @brief Constructs an AnyMap with a default_map_type. */
    AnyMap() : mapConcept_( new MapModel<default_map_type>() ) {}

    /*! @brief Constructs an AnyMap with a copy of the specified map. Uses move
    *   semantics. */
    template<typename MapType>
    explicit AnyMap( MapType m )
      : mapConcept_( new MapModel<MapType>( std::move(m) ) ) {
    }

    /*! @brief Copies the other AnyMap's underlying container. */
    AnyMap( AnyMap const & o ) : mapConcept_( o.mapConcept_->clone() ) {}

    /*! @brief Steals the temporary AnyMap's underlying container. */
    AnyMap( AnyMap && o ) : mapConcept_( o.mapConcept_ ) { o.mapConcept_ = 0; }
    
    ~AnyMap() { delete mapConcept_; }

    /*! @brief Copies the contents of the other map.
     *  @return This AnyMap. */
    AnyMap& operator=(AnyMap const& other)
    {
      if( this != &other ) {
	AnyMap<K, V> tmp(other);
	swap( tmp );
      }
      return *this;
    }

    /*! @brief Swaps contents with the temporary other map.
     *  @return This AnyMap. */
    AnyMap& operator=(AnyMap && other) { other.swap(*this);  return *this; }
    
    /*! @brief Swaps contents with the other AnyMap. */
    void swap(AnyMap& other) 
    { using std::swap;  swap( mapConcept_, other.mapConcept_ ); }
  
    /*!
     * @name Size and Capacity
     * @{ 
     */
    /*! @brief Returns true if the map stores no elements. */
    bool empty() const { return mapConcept_->empty(); }
    /*! @brief Returns the number of elements stored in the map. */
    size_type size() const  { return mapConcept_->size(); }
    /*! @brief Returns the maximum number of elements the map can store. */
    size_type max_size() const  { return mapConcept_->max_size(); }
    /*! @} */
    
    /*!
     * @name Lookup
     * @{
     */
    /*! @brief Gets the value associated with the given key. */
    V& operator[] (const K  & k)  { return mapConcept_->operator[](k); }
    /*! @brief Gets the value associated with the given key. */
    V& operator[] (      K && k)  { return mapConcept_->operator[](k); }
    /*! @brief Gets the value associated with the given key. */
    V&       at(K const& k)       { return mapConcept_->at(k); }
    /*! @brief Gets the value associated with the given key. */
    V const& at(K const& k) const { return mapConcept_->at(k); }
    /*! @brief Gets an iterator pointing to the value_type with the given key. */
    iterator       find(K const& k)       { return mapConcept_->find(k); }
    /*! @brief Gets an iterator pointing to the value_type with the given key. */
    const_iterator find(K const& k) const { return mapConcept_->find(k); }
    /*! @brief Counts the number elements with the given key stored in the map.*/
    size_type count(K const& k) const { return mapConcept_->count(k); }
    /*! @} */

    /*!
     * @name Traversal Iterators
     * @{
     */
    /*! @brief An iterator referring to the first element of the container, or if
     *  the container is empty the past-the-end value for the container.*/
    iterator       begin()        { return mapConcept_->begin(); }
    /*! @brief A const_iterator referring to the first element of the container,
     *  or if the container is empty the past-the-end value for the container.*/
    const_iterator begin() const  { return mapConcept_->begin(); }
    /*! @brief An iterator which refers to the past-the-end value for the 
     *  container. */
    iterator         end()        { return mapConcept_->end();   }
    /*! @brief An const_iterator which refers to the past-the-end value for the 
     *  container. */
    const_iterator   end() const  { return mapConcept_->end();   }
    /*! @} */

    /*!
     * @name Modifiers
     * @{
     */
    /*! @brief Inserts the value provided. */
    std::pair<iterator, bool> insert(value_type const& val) { return mapConcept_->insert(val); }

    /*! @brief Inserts all values starting at the value pointed to by i1 and
    *   ending at but not including the value pointed to by i2. */
    template<typename InputIterator>
    void insert(InputIterator i1, InputIterator i2) { return mapConcept_->insert(i1, i2); }

    /*! @brief Removes the key and it's associated element from the container. */
    size_type erase(K const& k) { return mapConcept_->erase(k); }

    /*! @brief Removes all keys and associted elements from the container. */
    void clear() { mapConcept_->clear(); }
    /*! @} */
    
    /*! @brief Compares the maps based on their contents. 
     *
     *  Checking equality is done by iterating through all elements contained
     *  in this map and looking them up in the other map. If all elements
     *  are matched and the numbers of stored elements are equal in both
     *  maps, the MapConcepts are declared to be equal.
     *
     *  NOTE:
     *  *  Because of the implementation of equality it may be more efficient
     *     to compare two MapConcepts in a particular order. For example, given
     *     a map rbmap implemented as a red-and-black tree and a map hmap,
     *     implementated as a hash map, it should be more efficient to evaluate
     *     rbmap == hmap (O(n*1)) than hmap == rbmap (O(n*log(n))).
     */
    bool operator==(const AnyMap& other) const  { return mapConcept_->operator==(*other.mapConcept_); }
    /*! @brief Compares the maps based on their contents. Implemented as a
     *  negation of the equality operator. */
    bool operator!=(const AnyMap& other) const  { return mapConcept_->operator!=(*other.mapConcept_); }

  private:
    MapConcept* mapConcept_;

  }; // class AnyMap

}; // namespace MapTypeErasure

#endif // __ANY_MAP_HPP__
