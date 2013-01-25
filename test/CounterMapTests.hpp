#ifndef __COUNTER_MAP_TESTS_HPP__
#define __COUNTER_MAP_TESTS_HPP__

#include "CounterMap.h"
#include "MapTypeErasure/AnyMap.hpp"
#include <ostream>
#include <fstream>
#include <stdlib.h>

#include <boost/unordered_map.hpp>
#include <map>
#include <list>
#include <algorithm>
#include <string>

class CounterMapTests : public ::testing::Test
{
public:

  // string clone
  struct Word : public std::string
  {
    Word( const std::string& s ) : std::string(s) {}
  };

  typedef std::string Key_t;
  typedef Word Value_t;

  typedef Counters::Counter<Value_t> Counter_t;
  typedef Counters::CounterMap<Key_t, Value_t> CounterMap_t;
  typedef std::map<typename Counter_t::Value_t, typename Counter_t::Count_t> CounterSTDMap_t;
  typedef boost::unordered_map<typename Counter_t::Value_t, typename Counter_t::Count_t> CounterBoostMap_t;
  typedef std::map<Key_t, Counter_t> CounterMapSTDMap_t;
  typedef boost::unordered_map<Key_t, Counter_t> CounterMapBoostMap_t;
  typedef MapTypeErasure::AnyMap<Key_t, Counter_t> AnyMap_t;

  typedef Counters::CounterFactory<typename Counter_t::Value_t> CounterFactory_t;
  typedef Counters::DefaultCounterFactory<typename Counter_t::Value_t> DefaultCounterFactory_t;
  typedef Counters::CopyCounterFactory<typename Counter_t::Value_t> CopyCounterFactory_t;
  typedef Counters::MapTypeCounterFactory<typename Counter_t::Value_t, CounterSTDMap_t> STDMapCounterFactory_t;
  typedef Counters::MapTypeCounterFactory<typename Counter_t::Value_t, CounterBoostMap_t> BoostMapCounterFactory_t;
  
  CounterMapTests() : copyCounterFactory( Counters::Counter<Word>() )
  {
    using namespace std;
    using namespace Counters;
    using namespace MapTypeErasure;
  }

  ~CounterMapTests()
  {
    for( std::list<typename Counters::CounterFactory<Value_t>*>::const_iterator i(factoryList.begin());
	 i != factoryList.end(); ++i )
      delete *i;
  }

protected:

  bool readFileIntoCounter( const std::string& fileName,
			    CounterMap_t& counterMap )
  {
    using namespace std;
    ifstream infile;
    infile.open( fileName );
    if( !infile ) {
      cerr << "CounterMap Tests cannot open data file." << endl;
      return false;
    }
    string w1, w2;
    infile >> w1;
    transform( w1.begin(), w1.end(), w1.begin(), ::tolower);
    while( infile >> w2 ) {
      transform( w2.begin(), w2.end(), w2.begin(), ::tolower);
      Word word = w2;
      counterMap.incrementCount(w1, word, 1.0);
      w1 = w2;
    }
    infile.close();
    return true;
  }

  virtual void SetUp()
  {
    using namespace std;

    factoryList.push_back( new DefaultCounterFactory_t(defaultCounterFactory) );
    factoryList.push_back( new CopyCounterFactory_t(copyCounterFactory) );
    factoryList.push_back( new STDMapCounterFactory_t(stdMapCounterFactory) );
    factoryList.push_back( new BoostMapCounterFactory_t(boostMapCounterFactory) );
  }

  DefaultCounterFactory_t defaultCounterFactory;
  CopyCounterFactory_t copyCounterFactory;
  STDMapCounterFactory_t stdMapCounterFactory;
  BoostMapCounterFactory_t boostMapCounterFactory;

  std::list<typename Counters::CounterFactory<Value_t>*> factoryList;
};

TEST_F(CounterMapTests, APIs)
{
  using namespace std;
  using namespace Counters;
  using namespace MapTypeErasure;

  const string testFileName("test/data/rock-n-roll-nerd");

  const string a("a");
  const string b("b");
  const string x("x");
  const Word one("one");
  const Word two("two");
  const Word three("three");
  const Word xxx("xxx");
      
  
  {
    cout << "- Default Constructor." << endl;
    CounterMap_t songCounter;

    ASSERT_TRUE( readFileIntoCounter( testFileName, songCounter ) );
    ASSERT_TRUE( songCounter.size() > 0 );

    cout << "- Map & Factory Constructor, Copy Constructor, Temp Move Constructor, Assignment.\n  + with different factories: ";  cout.flush();
    for( std::list<typename Counters::CounterFactory<Value_t>*>::const_iterator i(factoryList.begin()); i != factoryList.end(); ++i )
      {
	cout << ".";  cout.flush();
	CounterMap_t song1( AnyMap_t(  CounterMapSTDMap_t()), **i );
	CounterMap_t song2( AnyMap_t(CounterMapBoostMap_t()), **i );
	
	song1 += songCounter;
	song2 += songCounter;
	
	EXPECT_EQ( songCounter, song1 );
	EXPECT_EQ( songCounter, song2 );
	EXPECT_EQ(       song1, song2 );

	// copy constructor
	CounterMap_t song1Copy( song1 );
	CounterMap_t song2Copy( song2 );
	
	EXPECT_EQ( songCounter, song1Copy );
	EXPECT_EQ( songCounter, song2Copy );
	EXPECT_EQ(   song1Copy, song2Copy );

	// tmp move constructor
	CounterMap_t song1TempCopy( std::move(CounterMap_t(song1)) );
	CounterMap_t song2TempCopy( std::move(CounterMap_t(song2)) );

	EXPECT_EQ(   songCounter, song1TempCopy );
	EXPECT_EQ(   songCounter, song2TempCopy );
	EXPECT_EQ( song1TempCopy, song2TempCopy );

	// standard assignment
	CounterMap_t songAssign1 = song1;
	CounterMap_t songAssign2 = song2;

	EXPECT_EQ( songCounter, songAssign1 );
	EXPECT_EQ( songCounter, songAssign2 );
	EXPECT_EQ( songAssign1, songAssign2 );

	// move assignment
	songAssign1.remove( songAssign1.begin()->first );
	songAssign2.remove( songAssign2.begin()->first );
	songAssign1 = std::move(CounterMap_t(song1));
	songAssign2 = std::move(CounterMap_t(song2));

	EXPECT_EQ( songCounter, songAssign1 );
	EXPECT_EQ( songCounter, songAssign2 );
	EXPECT_EQ( songAssign1, songAssign2 );
      }
    cout << endl;

    cout << "- Modifiers." << endl;
    {
      cout << "  + incrementCount" << endl;

      CounterMap_t emptyCounterMap1;

      // increment with args: const ref, const ref
      
      emptyCounterMap1.incrementCount( a, one, 1 );

      emptyCounterMap1.incrementCount( a, two, 1.5 );
      emptyCounterMap1.incrementCount( a, two, 0.5 );

      emptyCounterMap1.incrementCount( b, three, 1 );
      emptyCounterMap1.incrementCount( b, three, 1 );
      emptyCounterMap1.incrementCount( b, three, 1 );
      
      EXPECT_EQ( 1, emptyCounterMap1.getCount(a, one) );
      EXPECT_EQ( 2, emptyCounterMap1.getCount(a, two) );
      EXPECT_EQ( 3, emptyCounterMap1.getCount(b, three) );
      EXPECT_EQ( 0, emptyCounterMap1.getCount(x, one) );
      EXPECT_EQ( 0, emptyCounterMap1.getCount(b, xxx) );
      EXPECT_EQ( 2, emptyCounterMap1.size() );

      // increment with args: const ref, temp
      {
	CounterMap_t tmpCounterMap;
	tmpCounterMap.incrementCount( a, std::move(Word(one)), 1 );
	tmpCounterMap.incrementCount( a, std::move(Word(two)), 1.5 );
	tmpCounterMap.incrementCount( a, std::move(Word(two)), 0.5 );
	tmpCounterMap.incrementCount( b, std::move(Word(three)), 1 );
	tmpCounterMap.incrementCount( b, std::move(Word(three)), 1 );
	tmpCounterMap.incrementCount( b, std::move(Word(three)), 1 );
	EXPECT_EQ( emptyCounterMap1, tmpCounterMap );
      }
      // increment with args: tmp, tmp
      {
	CounterMap_t tmpCounterMap;
	tmpCounterMap.incrementCount( std::move(string(a)), 
				      std::move(Word(one)), 1 );
	tmpCounterMap.incrementCount( std::move(string(a)), 
				      std::move(Word(two)), 1.5 );
	tmpCounterMap.incrementCount( std::move(string(a)), 
				      std::move(Word(two)), 0.5 );
	tmpCounterMap.incrementCount( std::move(string(b)), 
				      std::move(Word(three)), 1 );
	tmpCounterMap.incrementCount( std::move(string(b)), 
				      std::move(Word(three)), 1 );
	tmpCounterMap.incrementCount( std::move(string(b)), 
				      std::move(Word(three)), 1 );
	EXPECT_EQ( emptyCounterMap1, tmpCounterMap );
      }
      // increment with args: tmp, const ref
      {
	CounterMap_t tmpCounterMap;
	tmpCounterMap.incrementCount( std::move(string(a)), one, 1 );
	tmpCounterMap.incrementCount( std::move(string(a)), two, 1.5 );
	tmpCounterMap.incrementCount( std::move(string(a)), two, 0.5 );
	tmpCounterMap.incrementCount( std::move(string(b)), three, 1 );
	tmpCounterMap.incrementCount( std::move(string(b)), three, 1 );
	tmpCounterMap.incrementCount( std::move(string(b)), three, 1 );
	EXPECT_EQ( emptyCounterMap1, tmpCounterMap );
      }

      cout << "  + setCount" << endl;
      
      CounterMap_t emptyCounterMap2;
      emptyCounterMap2.setCount( a, one, 1 );
      emptyCounterMap2.setCount( a, two, 1 );  // reset this
      emptyCounterMap2.setCount( a, two, 2 );
      emptyCounterMap2.setCount( b, three, 3 );
      
      EXPECT_EQ( emptyCounterMap1, emptyCounterMap2 );

      // set with args: const ref, temp
      {
	CounterMap_t tmpCounterMap;
	tmpCounterMap.setCount( a, std::move(Word(one)), 1 );
	tmpCounterMap.setCount( a, std::move(Word(two)), 1 );  // reset this
	tmpCounterMap.setCount( a, std::move(Word(two)), 2 );
	tmpCounterMap.setCount( b, std::move(Word(three)), 3 );
	EXPECT_EQ( emptyCounterMap2, tmpCounterMap );
      }
      // set with args: tmp, tmp
      {
	CounterMap_t tmpCounterMap;
	tmpCounterMap.setCount( std::move(string(a)), std::move(Word(one)), 1 );
	tmpCounterMap.setCount( std::move(string(a)), std::move(Word(two)), 1 );  // reset this
	tmpCounterMap.setCount( std::move(string(a)), std::move(Word(two)), 2 );
	tmpCounterMap.setCount( std::move(string(b)), std::move(Word(three)), 3 );
	EXPECT_EQ( emptyCounterMap2, tmpCounterMap );
      }
      // set with args: tmp, const ref
      {
	CounterMap_t tmpCounterMap;
	tmpCounterMap.setCount( std::move(string(a)), one, 1 );
	tmpCounterMap.setCount( std::move(string(a)), two, 1 );  // reset this
	tmpCounterMap.setCount( std::move(string(a)), two, 2 );
	tmpCounterMap.setCount( std::move(string(b)), three, 3 );
	EXPECT_EQ( emptyCounterMap2, tmpCounterMap );
      }

      cout << "  + remove" << endl;
      {
	CounterMap_t tmpCounter1(emptyCounterMap2);
	tmpCounter1.setCount( "x", Word("xxx"), 11 );
	tmpCounter1.setCount( "x", Word("yyy"), 22 );

	CounterMap_t tmpCounter2(tmpCounter1);
	tmpCounter1.setCount( "x", Word("zzz"), 33 );

	EXPECT_NE( tmpCounter1, tmpCounter2 );
	EXPECT_NE( emptyCounterMap2, tmpCounter2 );

	tmpCounter1.remove( "x", Word("zzz") );
	EXPECT_EQ( tmpCounter1, tmpCounter2 );

	tmpCounter2.remove( "x" );
	EXPECT_EQ( emptyCounterMap2, tmpCounter2 );
	EXPECT_NE( emptyCounterMap2, tmpCounter1 );

	tmpCounter1.remove( "x", Word("xxx") );
	tmpCounter1.remove( "x", Word("yyy") );
	EXPECT_NE( emptyCounterMap2, tmpCounter1 );
	EXPECT_EQ( 0, tmpCounter1.size("x") );
	
	tmpCounter1.remove( "x" );
	EXPECT_EQ( emptyCounterMap2, tmpCounter1 );
	
	EXPECT_EQ( tmpCounter1, tmpCounter2 );
	tmpCounter1.remove( "---" );
	EXPECT_EQ( tmpCounter1, tmpCounter2 );
	tmpCounter1.remove( "---", Word("---") );
	EXPECT_EQ( tmpCounter1, tmpCounter2 );
	EXPECT_EQ( NULL, tmpCounter1.getCounter("---") );
      }
      
      cout << "  + conditionalNormalize" << endl;
      {
	cout << "    + empty" << endl;
	CounterMap_t emptyCounter;
	EXPECT_EQ( 0, emptyCounter.totalCount() );
	EXPECT_TRUE( emptyCounter.empty() );
	emptyCounter.conditionalNormalize();
	EXPECT_EQ( 0, emptyCounter.totalCount() );
	EXPECT_TRUE( emptyCounter.empty() );

	cout << "    + non-empty" << endl;
	CounterMap_t nonEmptyCounter( emptyCounterMap2 );
	EXPECT_FALSE( nonEmptyCounter.empty() );
	nonEmptyCounter.conditionalNormalize();
	EXPECT_FALSE( nonEmptyCounter.empty() );
	EXPECT_DOUBLE_EQ( 1.0 * nonEmptyCounter.size(), 
			  nonEmptyCounter.totalCount() );
	// note that second counter map isn't really empty (it's just how it was
	// constructed)
	EXPECT_EQ( nonEmptyCounter.size(), emptyCounterMap2.size() );
      }
    }
    
    cout << "- Lookup." << endl;
    {
      CounterMap_t scratchCounter;
      scratchCounter.setCount( b, three, 3 );
      scratchCounter.setCount( a, two, 2 );
      scratchCounter.setCount( a, one, 1 );
      const CounterMap_t counterMap123( scratchCounter );
      
      cout << "  + contains" << endl;
      EXPECT_TRUE( counterMap123.contains( a, two ) );
      EXPECT_TRUE( counterMap123.contains( b ) );
      EXPECT_FALSE( counterMap123.contains( a, xxx ) );
      EXPECT_FALSE( counterMap123.contains( x, two ) );
      EXPECT_FALSE( counterMap123.contains( x ) );
      CounterMap_t anotherScratch( scratchCounter );

      anotherScratch.setCount( x, xxx, 777 );
      EXPECT_TRUE( anotherScratch.contains( x ) );
      EXPECT_TRUE( anotherScratch.contains( x, xxx ) );

      anotherScratch.remove( x, xxx );
      EXPECT_TRUE( anotherScratch.contains( x ) );
      EXPECT_FALSE( anotherScratch.contains( x, xxx ) );
      
      cout << "  + size" << endl;
      EXPECT_EQ( 2, counterMap123.size() );
      EXPECT_EQ( 2, counterMap123.size( a ) );
      EXPECT_EQ( 0, counterMap123.size( x ) );
      
      EXPECT_EQ( 3, anotherScratch.size() );
      EXPECT_TRUE( anotherScratch.contains( x ) );
      EXPECT_EQ( 0, anotherScratch.size( x ) );

      cout << "  + empty" << endl;
      anotherScratch = CounterMap_t();
      EXPECT_TRUE( anotherScratch.empty() );
      anotherScratch.setCount( a, one, 1 );
      EXPECT_FALSE( anotherScratch.empty() );
      anotherScratch.remove( a, one );
      EXPECT_FALSE( anotherScratch.empty() );
      anotherScratch.remove( a );
      EXPECT_TRUE( anotherScratch.empty() );
      anotherScratch.setCount( a, one, 1 );
      anotherScratch.remove( a );
      EXPECT_TRUE( anotherScratch.empty() );

      cout << "  + getCount" << endl;
      EXPECT_DOUBLE_EQ( 1, counterMap123.getCount(a, one) );
      EXPECT_DOUBLE_EQ( 0, counterMap123.getCount(a, xxx) );
      EXPECT_DOUBLE_EQ( 0, counterMap123.getCount(b, one) );
      EXPECT_DOUBLE_EQ( 0, counterMap123.getCount(x, one) );
      
      anotherScratch.incrementCount( x, xxx, 1 );
      anotherScratch.remove( x, xxx );
      EXPECT_TRUE( anotherScratch.contains(x) );
      EXPECT_FALSE( anotherScratch.contains(x, xxx) );
      EXPECT_DOUBLE_EQ( 0, counterMap123.getCount(x, xxx) );
      anotherScratch.remove( x );

      cout << "  + totalCount" << endl;
      EXPECT_DOUBLE_EQ( 6, counterMap123.totalCount() );
      EXPECT_DOUBLE_EQ( 3, counterMap123.totalCount( a ) );
      EXPECT_DOUBLE_EQ( 3, counterMap123.totalCount( b ) );
      EXPECT_DOUBLE_EQ( 0, counterMap123.totalCount( x ) );

      anotherScratch = counterMap123;
      anotherScratch.incrementCount( x, xxx, 1 );
      anotherScratch.remove( x, xxx );
      EXPECT_DOUBLE_EQ( 0, anotherScratch.totalCount( x ) );
      EXPECT_DOUBLE_EQ( 6, anotherScratch.totalCount() );
    }

    cout << "- Counters." << endl;
    {
      CounterMap_t tmp;
      tmp.setCount( a, one, 1 );
      tmp.setCount( a, two, 2 );
      tmp.setCount( b, three, 3 );
      const CounterMap_t counterMap(tmp);

      CounterMap_t::Counter_t const *counterA = counterMap.getCounter( a );
      CounterMap_t::Counter_t const *counterB = counterMap.getCounter( b );
      CounterMap_t::Counter_t const *counterX = counterMap.getCounter( x );
      EXPECT_TRUE( counterA == counterMap.getCounter( std::move(string(a)) ) );
      EXPECT_TRUE( counterB == counterMap.getCounter( std::move(string(b)) ) );
      EXPECT_TRUE( counterX == counterMap.getCounter( std::move(string(x)) ) );

      ASSERT_TRUE( NULL != counterA );
      ASSERT_TRUE( NULL != counterB );
      EXPECT_TRUE( NULL == counterX );

      EXPECT_EQ( 2, counterA->size() );
      EXPECT_TRUE(  counterA->contains( one ) );
      EXPECT_TRUE(  counterA->contains( two ) );
      EXPECT_FALSE( counterA->contains( three ) );
      EXPECT_FALSE( counterA->contains( xxx ) );

      EXPECT_EQ( 1, counterB->size() );
      EXPECT_FALSE( counterB->contains( one ) );
      EXPECT_FALSE( counterB->contains( two ) );
      EXPECT_TRUE(  counterB->contains( three ) );
      EXPECT_FALSE( counterB->contains( xxx ) );
    }

    cout << "- Traversal." << endl;
    {
      CounterMap_t tmpCounterMap;

      const CounterMap_t emptyCounterMap( tmpCounterMap );
      EXPECT_EQ( emptyCounterMap.begin(), emptyCounterMap.end() );

      tmpCounterMap.setCount( a, one, 1 );
      tmpCounterMap.setCount( a, two, 2 );
      tmpCounterMap.setCount( b, three, 3 );

      const CounterMap_t counterMap123( tmpCounterMap );
      EXPECT_NE( counterMap123.begin(), counterMap123.end() );
      
      typename CounterMap_t::ConstIterator itr( counterMap123.begin() );
      int howMany = 0;
      for( ; itr != counterMap123.end(); ++itr )
	{
	  EXPECT_TRUE( &itr->second == counterMap123.getCounter( itr->first ) );
	  ++howMany;
	}
      EXPECT_EQ( howMany, counterMap123.size() );
    }

    cout << "- Equality." << endl;
    {
      AnyMap_t anyMapStd( std::move(CounterMapSTDMap_t()) );
      AnyMap_t anyMapBoost( std::move(CounterMapBoostMap_t()) );

      STDMapCounterFactory_t stdFactory;
      BoostMapCounterFactory_t boostFactory;

      CounterMap_t counterMap1( anyMapStd, boostFactory );
      CounterMap_t counterMap2( anyMapBoost, stdFactory );

      EXPECT_TRUE( counterMap1 == counterMap2 );
      EXPECT_TRUE( counterMap2 == counterMap1 );
      EXPECT_TRUE( counterMap2 == counterMap2 );
      EXPECT_FALSE( counterMap1 != counterMap2 );
      EXPECT_FALSE( counterMap2 != counterMap1 );
      EXPECT_FALSE( counterMap1 != counterMap1 );
      EXPECT_TRUE( counterMap1.equals( counterMap2 ) );

      counterMap1.setCount( a, one, 1 );
      EXPECT_FALSE( counterMap1 == counterMap2 );
      EXPECT_FALSE( counterMap2 == counterMap1 );
      EXPECT_TRUE(  counterMap1 != counterMap2 );
      EXPECT_TRUE(  counterMap2 != counterMap1 );
      EXPECT_FALSE( counterMap1.equals( counterMap2, 0.1 ) );
      EXPECT_FALSE( counterMap1.equals( counterMap2, 2 ) );

      counterMap2.setCount( a, one, 1.5 );
      EXPECT_FALSE( counterMap1 == counterMap2 );
      EXPECT_FALSE( counterMap2 == counterMap1 );
      EXPECT_TRUE(  counterMap1 != counterMap2 );
      EXPECT_TRUE(  counterMap2 != counterMap1 );
      EXPECT_FALSE( counterMap1.equals( counterMap2, 0.1 ) );
      EXPECT_TRUE( counterMap1.equals( counterMap2, 1 ) );
    }

    cout << "- Arithmetic." << endl;
    {
      const CounterMap_t backup___;

      CounterMap_t counterMap000;
      counterMap000.setCount( a, one, 0 );
      counterMap000.setCount( a, two, 0 );
      counterMap000.setCount( b, three, 0 );
      const CounterMap_t backup000( counterMap000 );

      CounterMap_t counterMap123;
      counterMap123.setCount( a, one, 1 );
      counterMap123.setCount( a, two, 2 );
      counterMap123.setCount( b, three, 3 );
       const CounterMap_t backup123( counterMap123 );

      CounterMap_t counterMap246;
      counterMap246.setCount( a, one, 2 );
      counterMap246.setCount( a, two, 4 );
      counterMap246.setCount( b, three, 6 );
      const CounterMap_t backup246( counterMap246 );

      // +=
      counterMap123 += backup123;
      EXPECT_TRUE( counterMap123.equals( backup246 ) );
      counterMap123 = backup123;

      counterMap123 += counterMap123;
      EXPECT_TRUE( counterMap123.equals( backup246 ) );
      counterMap123 = backup123;

      // -=
      counterMap246 -= backup123;
      EXPECT_TRUE( counterMap246.equals( backup123 ) );
      counterMap246 = backup246;

      counterMap246 -= counterMap246;
      EXPECT_TRUE( counterMap246.equals( backup000 ) );
      counterMap246 = backup246;

      // +
      EXPECT_TRUE( backup246.equals(                        backup123   +                        backup123   ) );
      EXPECT_TRUE( backup246.equals(                        backup123   + std::move(CounterMap_t(backup123)) ) );
      EXPECT_TRUE( backup246.equals( std::move(CounterMap_t(backup123)) + std::move(CounterMap_t(backup123)) ) );
      EXPECT_TRUE( backup246.equals( std::move(CounterMap_t(backup123)) +                        backup123   ) );

      EXPECT_TRUE( backup123.equals(                        backup___   +                        backup123   ) );
      EXPECT_TRUE( backup123.equals(                        backup___   + std::move(CounterMap_t(backup123)) ) );
      EXPECT_TRUE( backup123.equals( std::move(CounterMap_t(backup___)) + std::move(CounterMap_t(backup123)) ) );
      EXPECT_TRUE( backup123.equals( std::move(CounterMap_t(backup___)) +                        backup123   ) );

      // -
      EXPECT_TRUE( backup123.equals(                        backup246   -                        backup123   ) );
      EXPECT_TRUE( backup123.equals(                        backup246   - std::move(CounterMap_t(backup123)) ) );
      EXPECT_TRUE( backup123.equals( std::move(CounterMap_t(backup246)) - std::move(CounterMap_t(backup123)) ) );
      EXPECT_TRUE( backup123.equals( std::move(CounterMap_t(backup246)) -                        backup123   ) );
    }
  }


}

#endif // __COUNTER_MAP_TESTS_HPP__
