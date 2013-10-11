#include "../adblock/IAdblock.h"
#include "../adblock/Filter.h"
#include <tchar.h>
#include <gtest/gtest.h>

#if _DEBUG
#pragma comment(lib, "gtest-sd.lib")
#elif NDEBUG
#pragma comment(lib, "gtest-s.lib")
#endif

TEST(FilterTest, Normalize) {
  EXPECT_EQ("! *** easylist:easylist/easylist_general_block.txt ***",
    NS_ADBLOCK::Filter::normalize("  \t! *** easylist:easylist/easylist_general_block.txt ***  "));
  std::cout << NS_ADBLOCK::Filter::normalize("nwanime.com##div[style=\"margin: auto; display: block; width: 728px; height: 90px; overflow: hidden;\"]");
}

int main(int argc, TCHAR *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
