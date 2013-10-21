#include "../adblock/IAdblock.h"
#include "../adblock/Filter.h"

#include <string>
#include <fstream>
#include <tchar.h>
#include <gtest/gtest.h>

#if _DEBUG
#pragma comment(lib, "gtest-sd.lib")
#elif NDEBUG
#pragma comment(lib, "gtest-s.lib")
#endif

TEST(FilterTest, Normalize) {
  std::string line;
  std::ifstream file;
  
  file.open("easylist.txt");
  if (file.is_open()) {
    while (!file.eof()) {
      std::getline(file, line);
      auto filter = NS_ADBLOCK::Filter::from_text(line);
    }
    file.close();
  }
  system("pause");
}

int main(int argc, TCHAR *argv[]) {
  //testing::InitGoogleTest(&argc, argv);
  //return RUN_ALL_TESTS();

  std::string line;
  std::ifstream file;

  file.open("easylist.txt");
  if (file.is_open()) {
    while (!file.eof()) {
      std::getline(file, line);
      auto filter = NS_ADBLOCK::Filter::from_text(line);
    }
    file.close();
  }
  system("pause");
}
