// ----------------------------------------------------------------------------------------------------------
/// @file   ftl_tests.cpp
/// @brief  Test suites for the ftl library using Bost.Unit.
// ----------------------------------------------------------------------------------------------------------

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE       FtlTests
#include <boost/test/unit_test.hpp>

#include "list.hpp"
#include "numeric_types.hpp"

BOOST_AUTO_TEST_SUITE( NumericTypeTestSuite )
    
BOOST_AUTO_TEST_CASE( canCreateStaticFtlIntType )
{
    ftl::int_t<2> static_int;
    
    BOOST_CHECK( static_int.runtime_value() == 2 );
}

BOOST_AUTO_TEST_CASE( canCreateStaticFtlSizeType )
{
    ftl::size_t<0> static_size_type;
    
    BOOST_CHECK( static_size_type.runtime_value() == 0 );
}

BOOST_AUTO_TEST_CASE( canUseDefinedDimensionTypes )
{
    using namespace ftl::dim;         // To access dimension variables
    
    BOOST_CHECK( i.runtime_value() == 0 );
    BOOST_CHECK( j.runtime_value() == 1 );
}

BOOST_AUTO_TEST_SUITE_END()
    
BOOST_AUTO_TEST_SUITE( ListTestSuite )
    
BOOST_AUTO_TEST_CASE( testCase )
{
}

BOOST_AUTO_TEST_SUITE_END()