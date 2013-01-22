#include <iostream>

#include "IteratorTypeErasure/any_iterator/any_iterator.hpp"

#include "MapTypeErasure/AnyMap.hpp"

#include "CounterFactories.h"
#include "CounterMap.h"

#include <boost/unordered_map.hpp>
#include <map>


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
  using namespace IteratorTypeErasure;
  using namespace MapTypeErasure;

  cout << "starting main..." << endl;

  boost::unordered_map<string, double> um;
  map<string, double> m;

  AnyMap<string, double> umm(um);
  AnyMap<string, double> mm(m);

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

  const AnyMap<string, double> cmm(mm);
  cmm.begin();
  cmm.end();

  cout << "should be false: " << (cmm.begin() == cmm.end()) << endl;
  
  cout << "should be 2: " << cmm.at("two") << endl;
  cout << "should be 0: " << cmm.at("one") << endl;

  m["a"] = 11;
  m["b"] = 22;
  m["c"] = 33;
  AnyMap<string, double>::const_iterator begin( m.begin() );
  AnyMap<string, double>::const_iterator end( m.end() );
  mm.insert( begin, end );
  cout << "should be 5: " << mm.size() << endl;
  for( AnyMap<string, double>::const_iterator i(mm.begin()); i != mm.end(); i++ )
    cout << i->first << " " << i->second << endl;

  um["a"] = 11;
  um["b"] = 22;
  um["c"] = 33;
  AnyMap<string, double>::const_iterator ubegin( um.begin() );
  AnyMap<string, double>::const_iterator uend( um.end() );
  umm.insert( ubegin, uend );
  cout << "should be 5: " << umm.size() << endl;
  for( AnyMap<string, double>::const_iterator i(umm.begin()); i != umm.end(); i++ )
    cout << i->first << " " << i->second << endl;

  umm.erase("two");
  cout << "should be 4: " << umm.size() << endl;
  for( AnyMap<string, double>::const_iterator i(umm.begin()); i != umm.end(); i++ )
    cout << i->first << " " << i->second << endl;

  cout << "should be false: " << (umm.find("one") == umm.end()) << endl;
  cout << "should be true:  " << (umm.find("two") == umm.end()) << endl;
  cout << "should be 4: " << umm.size() << endl;

  umm.clear();
  cout << "should be 0: " << umm.size() << endl;
  cout << "should be false: " << (umm == mm) << endl;
  cout << "should be false: " << (mm == umm) << endl;

  mm.clear();
  cout << "should be 0: " << mm.size() << endl;
  cout << "should be true: " << (umm == mm) << endl;
  cout << "should be true: " << (mm == umm) << endl;

  umm["one"] = 1;
  umm["three"] = 3;
  mm["one"] = 1;
  mm["three"] = 3;
  cout << "should be true: " << (umm == mm) << endl;
  cout << "should be true: " << (mm == umm) << endl;

  cout << "should be false: " << (umm != mm) << endl;
  cout << "should be false: " << (mm != umm) << endl;

  AnyMap<string, int> defMap;
  defMap.begin();

  cout << "Counter Factories" << endl;
  {
    typedef boost::unordered_map<string, double> UnderlyingMapType;
    Counters::MapTypeCounterFactory<std::string, UnderlyingMapType> f;
    Counters::Counter<std::string> counter( f.createCounter() );
    counter.incrementCount("a", 1);
    counter.setCount("b", 2);
    counter.incrementCount("b", 3);
    cout << counter << endl;
  }

  ///////////////////////////////////////////
  cout << "finished main." << endl;
}

