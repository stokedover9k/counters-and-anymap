#ifndef __ANY_MAP_TESTS_HPP__
#define __ANY_MAP_TESTS_HPP__

#include "AnyMap/AnyMap.hpp"
#include <boost/unordered_map.hpp>
#include <map>

class AnyMapTests : public ::testing::Test
{
public:
  typedef std::string K;
  typedef double V;

  typedef MapTypeErasure::AnyMap<K, V> Map;
  typedef boost::unordered_map<K, V> BoostMap;
  typedef std::map<K, V> StlMap;

protected:
  virtual void SetUp()
  {
    boostMap["one"]   = 1;
    boostMap["two"]   = 2;
    boostMap["three"] = 3;
    boostMap["four"]  = 4;
    stlMap.insert( boostMap.begin(), boostMap.end() );

    extraMap["eleven"]   = 11;
    extraMap["twelve"]   = 12;
    extraMap["thirteen"] = 13;

    dataAndExtraMap.insert( boostMap.begin(), boostMap.end() );
    dataAndExtraMap.insert( extraMap.begin(), extraMap.end() );

    NEW_KEY = "xxxx";
  }
  
  BoostMap boostMap;
  BoostMap emptyBoostMap;

  StlMap stlMap;
  StlMap emptyStlMap;

  BoostMap extraMap;
  BoostMap dataAndExtraMap;

  K NEW_KEY;
};



TEST_F(AnyMapTests, APIs)
{
  using namespace std;
  
  cout << "- Standard empty maps (test data)." << endl;
  ASSERT_TRUE(        boostMap.size() != 0 );
  ASSERT_TRUE(          stlMap.size() != 0 );
  ASSERT_TRUE(        extraMap.size() != 0 );
  ASSERT_TRUE( dataAndExtraMap.size() != 0 );
  ASSERT_EQ( boostMap.size(), stlMap.size() );
  ASSERT_EQ( 0, emptyBoostMap.size() );
  ASSERT_EQ( 0,   emptyStlMap.size() );
  ASSERT_EQ( boostMap.size() + extraMap.size(), dataAndExtraMap.size() );
  // make sure NEW_KEY is not in any of the data maps
  ASSERT_EQ( 0,        boostMap.count(NEW_KEY) );
  ASSERT_EQ( 0,          stlMap.count(NEW_KEY) );
  ASSERT_EQ( 0,        extraMap.count(NEW_KEY) );
  ASSERT_EQ( 0, dataAndExtraMap.count(NEW_KEY) );


  cout << "- Empty maps." << endl;
  Map emptyMap1(emptyBoostMap);
  Map emptyMap2(emptyStlMap);
  EXPECT_TRUE( emptyMap1.empty() );
  EXPECT_TRUE( emptyMap2.empty() );
  
  cout << "- Non-empty map construction." << endl;
  Map map1( boostMap );
  Map map2(   stlMap );
  EXPECT_FALSE( map1.empty() );
  EXPECT_FALSE( map2.empty() );
  EXPECT_EQ( map1.size(), map2.size() );

  cout << "- operator[]." << endl;
  EXPECT_EQ( 1, map1["one"] );
  EXPECT_EQ( 1, map2["one"] );
  EXPECT_EQ( 2, map1["two"] );
  EXPECT_EQ( 2, map2["two"] );
  EXPECT_EQ( 0, map1[NEW_KEY] );
  EXPECT_EQ( 0, map2[NEW_KEY] );

  cout << "- Contents check." << endl;
  EXPECT_GT( map1.count("one"),   0 );
  EXPECT_GT( map2.count("one"),   0 );
  EXPECT_GT( map1.count(NEW_KEY), 0 );
  EXPECT_GT( map2.count(NEW_KEY), 0 );
  // + 1 for NEW_KEY
  EXPECT_EQ( boostMap.size() + 1, map1.size()     );
  EXPECT_LT( boostMap.size() + 1, map1.max_size() );
  EXPECT_EQ(   stlMap.size() + 1, map2.size()     );
  EXPECT_LT(   stlMap.size() + 1, map2.max_size() );
  EXPECT_FALSE(map1.empty());
  EXPECT_FALSE(map2.empty());

  cout << "- Erase and check." << endl;
  EXPECT_EQ( 1, map1.erase(NEW_KEY) );
  EXPECT_EQ( 1, map2.erase(NEW_KEY) );
  EXPECT_EQ( 0, map1.count(NEW_KEY) );
  EXPECT_EQ( 0, map2.count(NEW_KEY) );
  EXPECT_NE( 0, map1.count("one") );
  EXPECT_NE( 0, map2.count("one") );

  cout << "- Iterators check." << endl;
  EXPECT_EQ( emptyMap1.begin(), emptyMap1.end() );
  EXPECT_EQ( emptyMap2.begin(), emptyMap2.end() );
  EXPECT_NE( map1.begin(), map1.end() );
  EXPECT_NE( map2.begin(), map2.end() );

  {
    Map::const_iterator itr( map1.begin() );
    for( size_t i = 0; i < map1.size(); ++i ) {
      EXPECT_NE( map1.end(), itr );
      ++itr;
    }

    EXPECT_EQ( itr, map1.end() );
  }

  cout << "- Inserts." << endl;
  int currentSize1 = map1.size();
  int currentSize2 = map2.size();
  map1.insert( Map::iterator(extraMap.begin()), Map::iterator(extraMap.end()) );
  map2.insert( Map::iterator(extraMap.begin()), Map::iterator(extraMap.end()) );
  EXPECT_EQ( currentSize1 + extraMap.size(),  map1.size() );
  EXPECT_EQ( currentSize2 + extraMap.size(),  map2.size() );

  cout << "- Equality (equal case)." << endl;
  EXPECT_TRUE( emptyMap1 == emptyMap2 );  // empty map equality
  EXPECT_TRUE( emptyMap2 == emptyMap1 );
  EXPECT_TRUE( emptyMap1 == emptyMap1 );  // empty self equality
  EXPECT_TRUE( emptyMap2 == emptyMap2 );
  EXPECT_FALSE( emptyMap1 != emptyMap2 ); // empty map inequality
  EXPECT_FALSE( emptyMap2 != emptyMap1 );
  EXPECT_FALSE( emptyMap1 != emptyMap1 ); // empty self inequality
  EXPECT_FALSE( emptyMap2 != emptyMap2 );
  
  EXPECT_TRUE( map1 == map2 );  // non-empty map equality
  EXPECT_TRUE( map2 == map1 );
  EXPECT_TRUE( map2 == map2 );  // non-empty self equality
  EXPECT_TRUE( map1 == map1 );
  EXPECT_FALSE( map1 != map2 );  // non-empty map inequality
  EXPECT_FALSE( map2 != map1 );
  EXPECT_FALSE( map2 != map2 );  // non-empty self inequality
  EXPECT_FALSE( map1 != map1 );

  cout << "- Equality (non-equal case)." << endl;
  map1[NEW_KEY] = 100;
  map2.erase(NEW_KEY);
  EXPECT_FALSE( map1 == map2 );
  EXPECT_FALSE( map2 == map1 );

  EXPECT_FALSE( map1 == emptyMap1 );
  EXPECT_FALSE( map1 == emptyMap2 );
  EXPECT_FALSE( map2 == emptyMap1 );
  EXPECT_FALSE( map2 == emptyMap2 );

  EXPECT_FALSE( emptyMap1 == map1 );
  EXPECT_FALSE( emptyMap2 == map1 );
  EXPECT_FALSE( emptyMap1 == map2 );
  EXPECT_FALSE( emptyMap2 == map2 );
  
  // revert and check...
  map1.erase(NEW_KEY);
  EXPECT_TRUE( map1 == map2 );
  
  cout << "- Copy and compare." << endl;
  Map map1Copy( map1 );
  Map map2Copy( map2 );
  EXPECT_EQ( map1Copy, map1      );
  EXPECT_EQ( map2Copy, map2      );
  EXPECT_NE( map1Copy, emptyMap1 );
  EXPECT_NE( map2Copy, emptyMap2 );

  cout << "- Copy-modify-compare." << endl;
  EXPECT_EQ( 0, map1Copy.count(NEW_KEY) );
  EXPECT_EQ( 0, map2Copy.count(NEW_KEY) );
  map1[NEW_KEY] = 444;
  map2[NEW_KEY] = 555;
  EXPECT_NE( map1Copy, map1 );
  EXPECT_NE( map2Copy, map2 );
  EXPECT_NE( map1, map2 );
  EXPECT_EQ( map1Copy, map2Copy );

  // revert and check...
  map1.erase(NEW_KEY);
  map2.erase(NEW_KEY);
  EXPECT_TRUE( map1 == map2 );

  cout << "- Copy constructor and assignment." << endl;
  Map anotherCopy1( map1 );
  Map anotherCopy2 = map1;
  EXPECT_EQ( map1, anotherCopy1 );
  EXPECT_EQ( map1, anotherCopy2 );

  Map anotherCopyBoost( boostMap );
  Map anotherCopy3( anotherCopyBoost );
  Map anotherCopy4 = Map( boostMap );
  EXPECT_EQ( anotherCopyBoost, anotherCopy3 );
  EXPECT_EQ( anotherCopyBoost, anotherCopy4 );

}

#endif // __ANY_MAP_TESTS_HPP__
