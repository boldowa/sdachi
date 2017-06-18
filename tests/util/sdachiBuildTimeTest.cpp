#include "CppUTest/TestHarness.h"
#include "sdachiBuildTime.h"

TEST_GROUP(sdachiBuildTime)
{
  sdachiBuildTime* projectBuildTime;

  void setup()
  {
    projectBuildTime = new sdachiBuildTime();
  }
  void teardown()
  {
    delete projectBuildTime;
  }
};

TEST(sdachiBuildTime, Create)
{
  CHECK(0 != projectBuildTime->GetDateTime());
}

