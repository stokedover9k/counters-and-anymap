#include <iostream>

#include "Counter.h"

#include "any_iterator/any_iterator.hpp"
using namespace IteratorTypeErasure;

#include <boost/unordered_map.hpp>
#include <map>


template<typename K, typename V>
class Map
{
public:
  typedef K                       key_type;
  typedef V                       mapped_type;
  typedef std::pair<K const, V>   value_type;
  typedef size_t                  size_type;
  
  typedef any_iterator<      value_type, boost::forward_traversal_tag> iterator;
  typedef any_iterator<value_type const, boost::forward_traversal_tag> const_iterator;

private:

  struct MapConcept
  {
    typedef size_t size_type;
    
    virtual ~MapConcept() {}

    virtual MapConcept* clone() const = 0;

    virtual bool empty() const = 0;
    virtual size_type size() const = 0;
    virtual size_type max_size() const = 0;

    virtual V& operator[](const K& k) = 0;
    virtual V& operator[](K&& k) = 0;

    virtual V      & at(K const& k)       = 0;
    virtual V const& at(K const& k) const = 0;

    virtual typename Map::iterator       find(K const& k)       = 0;
    virtual typename Map::const_iterator find(K const& k) const = 0;

    virtual typename Map::iterator       begin()       = 0;
    virtual typename Map::const_iterator begin() const = 0;
    virtual typename Map::iterator         end()       = 0;
    virtual typename Map::const_iterator   end() const = 0;

    // inserts
    virtual std::pair<iterator, bool> insert(value_type const& val)    = 0;
    virtual void insert(const_iterator i1, const_iterator i2) = 0;

    // erases
    virtual size_type erase(K const& k) = 0;
  };

  template<typename M>
  struct MapModel : MapConcept
  {
    MapModel( const M& m ) : m_( m ) {}
    MapModel( const MapModel& o ) : m_( o.m_ ) {}
    virtual ~MapModel() {}

    MapConcept* clone() const { return new MapModel<M>( m_ ); }

    bool empty() const { return m_.empty(); }
    typename MapConcept::size_type size() const { return m_.size(); }
    typename MapConcept::size_type max_size() const { return m_.max_size(); }

    V& operator[] (const K& k) { return m_[k]; }
    V& operator[] (K && k) { return m_[k]; }

    V      & at(K const& k)       { return m_.at(k); }
    V const& at(K const& k) const { return m_.at(k); }

    typename Map::iterator       find(K const& k)       { 
      return Map::iterator(m_.find(k)); }
    typename Map::const_iterator find(K const& k) const { 
      return Map::const_iterator(m_.find(k)); }

    typename Map::iterator begin() {
      return Map::iterator(m_.begin()); }
    typename Map::const_iterator begin() const { 
      return Map::const_iterator(m_.begin()); }
    typename Map::iterator end() { 
      return Map::iterator(m_.end()); }
    typename Map::const_iterator end() const { 
      return Map::const_iterator(m_.end()); }

    // inserts
    std::pair<iterator, bool> insert(value_type const& val) {
      return m_.insert(val); }
    void insert(const_iterator i1, const_iterator i2) {
      return m_.insert(i1, i2); }

    // erases
    size_type erase(K const& k) { return m_.erase(k); }

  private:
    M m_;
  };

  MapConcept* m_;

public:

  template<typename M> 
  Map( const M& obj ) : m_( new MapModel<M>( obj ) ) {}

  Map( const Map& o ) : m_( o.m_->clone() ) {}
  
  ~Map() { delete m_; }

  template<typename M> 
  Map& operator=(const Map & o) {
    delete m_;
    m_ = o.m_->clone();
    return *this; 
  }

  Map& operator=(Map && o) {
    delete m_;
    m_ = o.m_;
    o.m_ = NULL;  //because temporary's m_ is about to be deleted.
    return *this;
  }
  
  // size and capacity
  bool         empty() const  { return m_->empty(); }
  size_type     size() const  { return m_->size(); }
  size_type max_size() const  { return m_->max_size(); }

  // lookup
  V& operator[] (const K  & k)  { return m_->operator[](k); }
  V& operator[] (      K && k)  { return m_->operator[](k); }
  V      & at(K const& k)       { return m_->at(k); }
  V const& at(K const& k) const { return m_->at(k); }
  iterator       find(K const& k)       { return m_->find(k); }
  const_iterator find(K const& k) const { return m_->find(k); }

  // iterators
  iterator       begin()        { return m_->begin(); }
  const_iterator begin() const  { return m_->begin(); }
  iterator         end()        { return m_->end();   }
  const_iterator   end() const  { return m_->end();   }

  // inserts
  std::pair<iterator, bool> insert(value_type const& val) { 
    return m_->insert(val); }
  void insert(const_iterator i1, const_iterator i2) {
    return m_->insert(i1, i2); }

  // erases
  size_type erase(K const& k) { return m_->erase(k); }
  
};




struct MyHash : boost::hash<std::string>
{
  std::size_t operator()(std::string const& s) const {
    boost::hash<std::string> hasher;
    return hasher(s) + 1;
  }
};

int main()
{
  using namespace std;

  cout << "starting main..." << endl;

  boost::unordered_map<string, double> um;
  map<string, int> m;

  Map<string, double> umm(um);
  Map<string, int> mm(m);

  cout << "empty: " << umm.empty() << endl;

  umm["one"] = 1.0;
  mm["two"] = 2;

  cout << "should be 1: " << umm["one"] << endl;
  cout << "should be 2: " <<  mm["two"] << endl;
  cout << "should be 0: " << umm["two"] << endl;
  cout << "should be 0: " <<  mm["one"] << endl;

  cout << "size: " << umm.size() << endl;
  cout << "max: " << umm.max_size() << endl;
  cout << "empty: " << umm.empty() << endl;

  mm.begin();
  mm.end();

  const Map<string, int> cmm(mm);
  cmm.begin();
  cmm.end();

  cout << "should be false: " << (cmm.begin() == cmm.end()) << endl;
  
  cout << "should be 2: " << cmm.at("two") << endl;
  cout << "should be 0: " << cmm.at("one") << endl;

  m["a"] = 11;
  m["b"] = 22;
  m["c"] = 33;
  Map<string, int>::const_iterator begin( m.begin() );
  Map<string, int>::const_iterator end( m.end() );
  mm.insert( begin, end );
  cout << "should be 5: " << mm.size() << endl;
  for( Map<string, int>::const_iterator i(mm.begin()); i != mm.end(); i++ )
    cout << i->first << " " << i->second << endl;

  um["a"] = 11;
  um["b"] = 22;
  um["c"] = 33;
  Map<string, double>::const_iterator ubegin( um.begin() );
  Map<string, double>::const_iterator uend( um.end() );
  umm.insert( ubegin, uend );
  cout << "should be 5: " << umm.size() << endl;
  for( Map<string, double>::const_iterator i(umm.begin()); i != umm.end(); i++ )
    cout << i->first << " " << i->second << endl;

  umm.erase("two");
  cout << "should be 4: " << umm.size() << endl;
  for( Map<string, double>::const_iterator i(umm.begin()); i != umm.end(); i++ )
    cout << i->first << " " << i->second << endl;

  cout << "should be false: " << (umm.find("one") == umm.end()) << endl;
  cout << "should be true:  " << (umm.find("two") == umm.end()) << endl;
  cout << "should be 4: " << umm.size() << endl;

  ///////////////////////////////////////////
  cout << "finished main." << endl;
}

