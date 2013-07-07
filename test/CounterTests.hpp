#ifndef __COUNTER_TESTS_HPP__
#define __COUNTER_TESTS_HPP__

#include "Counters/Counter.hpp"
#include "AnyMap/AnyMap.hpp"
#include <ostream>
#include <list>
#include <set>

class CounterTests : public ::testing::Test
{
public:
  
  typedef std::string StringV;
  typedef Counters::Counter<StringV>::Count_t Count;
  typedef Counters::Counter<StringV>::CoreMap_t StringMap;

  CounterTests()
    : chessList( { "king", 
	  "queen", 
	  "bishop", "bishop", 
	  "knight", "knight",
	  "rook", "rook",
	  "pawn", "pawn", "pawn", "pawn", "pawn", "pawn", "pawn", "pawn" } ),
      chessSet( chessList.begin(), chessList.end() )
  {
    
  }

protected:
  virtual void SetUp()
  {
  }

  template <typename T>
  MapTypeErasure::AnyMap<T, typename Counters::Counter<T>::Count_t> toMap( Counters::Counter<T> const& c )
  {
    MapTypeErasure::AnyMap<T, typename Counters::Counter<T>::Count_t> m;
    for( typename Counters::Counter<T>::ConstIterator i(c.begin()); i != c.end(); ++i )
      m[i->first] = i->second;
    return m;
  }

  const std::list<std::string> chessList;
  const std::set<std::string> chessSet;
};

TEST_F(CounterTests, APIs)
{
  using namespace std;
  using namespace Counters;
  
  cout << "- Check test data." << endl;
  ASSERT_EQ( 16, chessList.size() );
  ASSERT_EQ(  6, chessSet.size() );
  
  cout << "- Constructors." << endl;

  cout << "  + Default" << endl;
  const Counter<StringV> defaultCounter;
  EXPECT_EQ( 0, defaultCounter.size() );
  EXPECT_NE( 0, defaultCounter.maxSize() );
  EXPECT_TRUE( defaultCounter.empty() );

  cout << "  + Range" << endl;
  const Counter<StringV> rangeCounter(chessList.begin(), chessList.end());
  EXPECT_NE( chessList.size(), rangeCounter.size() );
  EXPECT_EQ(  chessSet.size(), rangeCounter.size() );
  EXPECT_FALSE( rangeCounter.empty() );


  
  cout << "  + Copy" << endl;
  {
    const Counter<StringV> copyDefaultCounter( defaultCounter );
    EXPECT_EQ( defaultCounter, copyDefaultCounter );
    EXPECT_EQ( defaultCounter.size(), copyDefaultCounter.size() );
    EXPECT_EQ( defaultCounter.empty(), copyDefaultCounter.empty() );

    const Counter<StringV> copyRangeCounter( rangeCounter );
    EXPECT_EQ( rangeCounter, copyRangeCounter );
    EXPECT_EQ( rangeCounter.size(), copyRangeCounter.size() );
    EXPECT_EQ( rangeCounter.empty(), copyRangeCounter.empty() );
  }

  
  cout << "  + Copy Temp" << endl;
  {
    Counter<StringV> copyTempDefaultCounter( std::move(Counter<StringV>()) );
    EXPECT_EQ( defaultCounter,         copyTempDefaultCounter );
    EXPECT_EQ( defaultCounter.size(),  copyTempDefaultCounter.size() );
    EXPECT_EQ( defaultCounter.empty(), copyTempDefaultCounter.empty() );

    Counter<StringV> copyTempRangeCounter( std::move(Counter<StringV>(rangeCounter)) );
    EXPECT_EQ(   rangeCounter,           copyTempRangeCounter );
    EXPECT_EQ(   rangeCounter.size(),    copyTempRangeCounter.size() );
    EXPECT_EQ(   rangeCounter.empty(),   copyTempRangeCounter.empty() );
  }


  cout << "  + AnyMap" << endl;
  {
    StringMap defaultStringMap( toMap( defaultCounter ) );
    Counter<StringV> defaultMapCounter( defaultStringMap );
    EXPECT_EQ( defaultCounter, defaultMapCounter );
    EXPECT_EQ( defaultCounter.size(), defaultMapCounter.size() );
    EXPECT_EQ( defaultCounter.empty(), defaultMapCounter.empty() );

    StringMap rangeStringMap( toMap( rangeCounter ) );
    Counter<StringV> rangeMapCounter( rangeStringMap );
    EXPECT_EQ( rangeCounter, rangeMapCounter );
    EXPECT_EQ( rangeCounter.size(), rangeMapCounter.size() );
    EXPECT_EQ( rangeCounter.empty(), rangeMapCounter.empty() );
  }


  cout << "  + Assignment" << endl;
  {
    Counter<StringV> counter(defaultCounter);
    // standard (copy) assign
    counter = rangeCounter;
    EXPECT_EQ( counter, rangeCounter );

    counter = defaultCounter;
    EXPECT_EQ( counter, defaultCounter );

    // move assign from temporaries
    counter = Counter<StringV>(rangeCounter);
    EXPECT_EQ( counter, rangeCounter );

    counter = Counter<StringV>(defaultCounter);
    EXPECT_EQ( counter, defaultCounter );
  }


  {
    cout << "- Lookup" << endl;
    
    //
    const Counter<StringV> defaultCounterCopy(defaultCounter);
    EXPECT_TRUE( defaultCounterCopy.isTotalSynched() );
    EXPECT_FALSE( defaultCounterCopy.contains( *chessList.begin() ) );
    EXPECT_FALSE( defaultCounterCopy.contains( "" ) );
    
    EXPECT_EQ( 0, defaultCounterCopy.getCount( *chessList.begin() ) );
    EXPECT_EQ( 0, defaultCounterCopy.getCount( "" ) );

    ASSERT_EQ( 0, defaultCounterCopy.size() );
    EXPECT_EQ( StringV(), defaultCounterCopy.maxValue() );

    EXPECT_EQ( 0, defaultCounterCopy.totalCount() );
    EXPECT_TRUE( defaultCounterCopy.isTotalSynched() );

    //
    const Counter<StringV> rangeCounterCopy(rangeCounter);
    EXPECT_FALSE( rangeCounterCopy.isTotalSynched() );
    EXPECT_TRUE( rangeCounterCopy.contains( *chessList.begin() ) );
    EXPECT_FALSE( rangeCounterCopy.contains( "" ) );

    EXPECT_NE( 0, rangeCounterCopy.getCount( *chessList.begin() ) );
    EXPECT_EQ( 0, rangeCounterCopy.getCount( "" ) );

    EXPECT_EQ( StringV("pawn"), rangeCounterCopy.maxValue() );

    EXPECT_EQ( chessList.size(), rangeCounterCopy.totalCount() );
    EXPECT_TRUE( rangeCounterCopy.isTotalSynched() );

    //
    const Counter<StringV> synchedRangeCounterCopy( rangeCounterCopy );
    EXPECT_TRUE( synchedRangeCounterCopy.isTotalSynched() );
    
    // make sure we can set policy on const Counters.
    synchedRangeCounterCopy.setCachePolicy(Counters::CACHE_POLICY_RELAXED);
    synchedRangeCounterCopy.setCachePolicy(Counters::CACHE_POLICY_PERSISTENT);

    cout << "- Modification and Caching" << endl;

    Counter<StringV> persistentSynchedRangeCounterCopy(synchedRangeCounterCopy);
    EXPECT_EQ( Counters::CACHE_POLICY_PERSISTENT,
	       persistentSynchedRangeCounterCopy.getCachePolicy() );
    EXPECT_TRUE( persistentSynchedRangeCounterCopy.isTotalSynched() );
    persistentSynchedRangeCounterCopy.incrementCount("pawn", 1.1);
    persistentSynchedRangeCounterCopy.incrementCount("PAWN", 1.1);
    persistentSynchedRangeCounterCopy.setCount("king", 2.2);
    persistentSynchedRangeCounterCopy.setCount("KING", 2.2);
    persistentSynchedRangeCounterCopy.remove("biship");
    EXPECT_TRUE( persistentSynchedRangeCounterCopy.isTotalSynched() );

    Counter<StringV> relaxedSynchedRangeCounterCopy(synchedRangeCounterCopy);
    EXPECT_EQ( Counters::CACHE_POLICY_PERSISTENT,
	       relaxedSynchedRangeCounterCopy.getCachePolicy() );
    relaxedSynchedRangeCounterCopy.setCachePolicy(Counters::CACHE_POLICY_RELAXED);
    EXPECT_TRUE( relaxedSynchedRangeCounterCopy.isTotalSynched() );

    relaxedSynchedRangeCounterCopy.incrementCount("pawn", 1.1);
    EXPECT_FALSE( relaxedSynchedRangeCounterCopy.isTotalSynched() );
    relaxedSynchedRangeCounterCopy.totalCount();
    EXPECT_TRUE( relaxedSynchedRangeCounterCopy.isTotalSynched() );

    relaxedSynchedRangeCounterCopy.incrementCount("PAWN", 1.1);
    EXPECT_FALSE( relaxedSynchedRangeCounterCopy.isTotalSynched() );
    relaxedSynchedRangeCounterCopy.totalCount();
    EXPECT_TRUE( relaxedSynchedRangeCounterCopy.isTotalSynched() );

    relaxedSynchedRangeCounterCopy.setCount("king", 2.2);
    EXPECT_FALSE( relaxedSynchedRangeCounterCopy.isTotalSynched() );
    relaxedSynchedRangeCounterCopy.totalCount();
    EXPECT_TRUE( relaxedSynchedRangeCounterCopy.isTotalSynched() );

    relaxedSynchedRangeCounterCopy.setCount("KING", 2.2);
    EXPECT_FALSE( relaxedSynchedRangeCounterCopy.isTotalSynched() );
    relaxedSynchedRangeCounterCopy.totalCount();
    EXPECT_TRUE( relaxedSynchedRangeCounterCopy.isTotalSynched() );

    relaxedSynchedRangeCounterCopy.normalize();
    EXPECT_TRUE( relaxedSynchedRangeCounterCopy.isTotalSynched() );
    EXPECT_EQ( 1.0, relaxedSynchedRangeCounterCopy.totalCount() );
  }

  cout << "- Traversal" << endl;
  {
    const Counter<StringV> rangeCounterCopy( rangeCounter );

    int howMany = 0;
    for( Counter<StringV>::ConstIterator i( rangeCounterCopy.begin() );
	 i != rangeCounterCopy.end(); ++i )
      {
	EXPECT_EQ( 1, chessSet.count( i->first ) );
	++howMany;
      }
    EXPECT_EQ( chessSet.size(), howMany );
  }

  cout << "- Arithmetic Operators" << endl;
  {
    const double EPSILON = 10.0e-8;

    const Counter<StringV> rangeCopy1( rangeCounter );
    Counter<StringV> rangeCopy2( rangeCounter );
    rangeCopy2.setCachePolicy(Counters::CACHE_POLICY_PERSISTENT);
    rangeCopy2.incrementCount( "PAWN", 1.1 );
    rangeCopy2.remove("knight");
    const Counter<StringV> rangeCopy2Copy( rangeCopy2 );
    rangeCopy1.totalCount();
    rangeCopy2.totalCount();
    EXPECT_TRUE( rangeCopy1.isTotalSynched() );
    EXPECT_TRUE( rangeCopy2.isTotalSynched() );
    Counter<StringV> resultCounter;

    // + and += (counter)
    resultCounter = rangeCopy2 + rangeCopy1;
    rangeCopy2 += rangeCopy1;
    EXPECT_EQ( resultCounter, rangeCopy2 );
    EXPECT_TRUE( rangeCopy2.isTotalSynched() );
    
    for( Counter<StringV>::ConstIterator i( rangeCopy2.begin() );
	 i != rangeCopy2.end(); ++i )
      {
	if( i->first == "PAWN" )
	  EXPECT_EQ( 1.1, rangeCopy2.getCount("PAWN") );
	else if( i->first == "knight" )
	  EXPECT_EQ( rangeCopy1.getCount("knight"), rangeCopy2.getCount("knight") );
	else
	  EXPECT_EQ( rangeCopy1.getCount( i->first ) * 2,
		     rangeCopy2.getCount( i->first ) );
      }

    // - and -= (counter)
    resultCounter = rangeCopy2 - rangeCopy1;
    rangeCopy2 -= rangeCopy1;
    EXPECT_EQ( resultCounter, rangeCopy2 );
    EXPECT_TRUE( rangeCopy2.isTotalSynched() );
    EXPECT_EQ( 0, rangeCopy2.getCount("knight") );
    rangeCopy2.remove("knight");
    EXPECT_EQ( rangeCopy2Copy, rangeCopy2 );
    
    // + and += (count)
    resultCounter = rangeCopy2 + 5.5;
    rangeCopy2 += 5.5;
    EXPECT_EQ( resultCounter, rangeCopy2 );
    EXPECT_TRUE( rangeCopy2.isTotalSynched() );
    EXPECT_EQ( rangeCopy2Copy.totalCount() + 5.5 * rangeCopy2Copy.size(),
	       rangeCopy2.totalCount() );
    EXPECT_EQ( rangeCopy2Copy.size(), rangeCopy2.size() );

    // - and -= (count)
    resultCounter = rangeCopy2 - 5.5;
    rangeCopy2 -= 5.5;
    EXPECT_EQ( resultCounter, rangeCopy2 );
    EXPECT_TRUE( rangeCopy2.equals( rangeCopy2Copy, EPSILON ) );

    // * and *= (count)
    resultCounter = rangeCopy2 * 3.3;
    rangeCopy2 *= 3.3;
    EXPECT_EQ( resultCounter, rangeCopy2 );
    EXPECT_TRUE( rangeCopy2.isTotalSynched() );
    EXPECT_DOUBLE_EQ( rangeCopy2Copy.totalCount() * 3.3, rangeCopy2.totalCount() );
    EXPECT_EQ( rangeCopy2Copy.size(), rangeCopy2.size() );

    // / and /= (count)
    resultCounter = rangeCopy2 / 3.3;
    rangeCopy2 /= 3.3;
    EXPECT_TRUE( rangeCopy2.equals( rangeCopy2Copy, EPSILON ) );
    
  }
}

#endif // __COUNTER_TESTS_HPP__
