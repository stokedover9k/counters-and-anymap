#include "IteratorTypeErasure/any_iterator/any_iterator.hpp"

#include <boost/unordered_map.hpp>

#include <ostream>

namespace MapTypeErasure
{
  using namespace IteratorTypeErasure;

  

  //--------------------------- AnyMap class ------------------------------
  template<typename K, typename V>
  class AnyMap
  {
  public:
    typedef K                       key_type;
    typedef V                       mapped_type;
    typedef std::pair<K const, V>   value_type;
    typedef size_t                  size_type;
  
    typedef any_iterator<value_type,       boost::forward_traversal_tag>       iterator;
    typedef any_iterator<value_type const, boost::forward_traversal_tag> const_iterator;
    
    typedef boost::unordered_map<K, V> default_map_type;

  private:

    // This class is the parent of specific map-type templated classes.
    // 
    //
    struct MapConcept
    {
      typedef typename AnyMap::size_type size_type;
    
      virtual ~MapConcept() {}

      virtual MapConcept* clone() const = 0;

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
	    return false;
	}
	return true;
      }

      bool operator!=(const MapConcept& other) const { return !operator==(other); }

    };

    template<typename MapType>
    struct MapModel : MapConcept
    {
      explicit MapModel( const MapType& m = MapType() ) : m_( m ) {}
      MapModel( const MapModel& o ) : m_( o.m_ ) {}
      virtual ~MapModel() {}

      MapModel* clone() const { return new MapModel<MapType>( m_ ); }

      bool empty() const { return m_.empty(); }
      typename MapConcept::size_type size() const { return m_.size(); }
      typename MapConcept::size_type max_size() const { return m_.max_size(); }

      V& operator[] (const K& k) { return m_[k]; }
      V& operator[] (K && k) { return m_[k]; }

      V      & at(K const& k)       { return m_.at(k); }
      V const& at(K const& k) const { return m_.at(k); }

      size_type count(K const& k) const { return m_.count(k); }

      typename AnyMap::iterator       find(K const& k)       { 
	return AnyMap::iterator(m_.find(k)); }
      typename AnyMap::const_iterator find(K const& k) const { 
	return AnyMap::const_iterator(m_.find(k)); }

      typename AnyMap::iterator begin() {
	return AnyMap::iterator(m_.begin()); }
      typename AnyMap::const_iterator begin() const { 
	return AnyMap::const_iterator(m_.begin()); }
      typename AnyMap::iterator end() { 
	return AnyMap::iterator(m_.end()); }
      typename AnyMap::const_iterator end() const { 
	return AnyMap::const_iterator(m_.end()); }

      // inserts
      std::pair<iterator, bool> insert(value_type const& val) {
	return m_.insert(val); }
      void insert(const_iterator i1, const_iterator i2) {
	return m_.insert(i1, i2); }

      // erases
      size_type erase(K const& k) { return m_.erase(k); }

      // clear
      void clear() { m_.clear(); }

    private:
      MapType m_;
    };

    MapConcept* mapConcept_;

  public:

    AnyMap() : mapConcept_( new MapModel<default_map_type>() ) {}

    template<typename MapType> 
    explicit AnyMap( const MapType& m ) : mapConcept_( new MapModel<MapType>( m ) ) {}

    AnyMap( const AnyMap& o ) : mapConcept_( o.mapConcept_->clone() ) {}
  
    ~AnyMap() { delete mapConcept_; }

    AnyMap& operator=(AnyMap other) {
      other.swap(*this);
      return *this; 
    }
    
    void swap(AnyMap& other) {
      std::swap( mapConcept_, other.mapConcept_ );
    }
  
    // size and capacity
    bool         empty() const  { return mapConcept_->empty(); }
    size_type     size() const  { return mapConcept_->size(); }
    size_type max_size() const  { return mapConcept_->max_size(); }

    // lookup
    V& operator[] (const K  & k)  { return mapConcept_->operator[](k); }
    V& operator[] (      K && k)  { return mapConcept_->operator[](k); }
    V&       at(K const& k)       { return mapConcept_->at(k); }
    V const& at(K const& k) const { return mapConcept_->at(k); }
    iterator       find(K const& k)       { return mapConcept_->find(k); }
    const_iterator find(K const& k) const { return mapConcept_->find(k); }
    size_type count(K const& k) const { return mapConcept_->count(k); }

    // iterators
    iterator       begin()        { return mapConcept_->begin(); }
    const_iterator begin() const  { return mapConcept_->begin(); }
    iterator         end()        { return mapConcept_->end();   }
    const_iterator   end() const  { return mapConcept_->end();   }

    // inserts
    std::pair<iterator, bool> insert(value_type const& val) { return mapConcept_->insert(val); }
    template<typename InputIterator>
    void insert(InputIterator i1, InputIterator i2) { return mapConcept_->insert(i1, i2); }

    // erases
    size_type erase(K const& k) { return mapConcept_->erase(k); }

    // clear
    void clear() { mapConcept_->clear(); }

    // comparisons
    bool operator==(const AnyMap& other) const  { return mapConcept_->operator==(*other.mapConcept_); }
    bool operator!=(const AnyMap& other) const { return mapConcept_->operator!=(*other.mapConcept_); }
  }; // class AnyMap

}; // namespace MapTypeErasure

