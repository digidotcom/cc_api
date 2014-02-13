#include "CppUTest/CommandLineTestRunner.h"

#include <iostream>

using namespace std;

int main(int ac, char** av)
{
   return CommandLineTestRunner::RunAllTests(ac, av);
}
