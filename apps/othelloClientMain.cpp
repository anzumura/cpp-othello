#include "OthelloClient.h"

int testAfterFree() {
  int* testInt = new int{7};
  delete testInt;
  return *testInt - 7;
}

char* testAfterStackChar;
void testAfterStack() {
  char stackChar = 'a';
  *testAfterStackChar = stackChar;
}

int main(int argc, char** argv) {
  // Uncomment these functions to test Address Sanitizer:
  // - testAfterStack: gets caught before address sanitizer with 'Trace/BPT trap: 5'
  // - testAfterFree: seems to work properly, i.e., no errors unless compiled with sanitizer flags
  // testAfterStack(); *testAfterStackChar = 'b';
  // return testAfterFree();
  othello::OthelloClient(argc, argv).begin();
  return 0;
}
