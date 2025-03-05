// DESCRIPTION: Verilator: Verilog example module
//
// This file ONLY is placed under the Creative Commons Public Domain, for
// any use, without warranty, 2017 by Wilson Snyder.
// SPDX-License-Identifier: CC0-1.0
//======================================================================
// Include common routines
#include <verilated.h>

static int passed_test_count = 0; // every time we perform a test, and the test passes, increment this by one.
static int total_test_count = 0;  // every time we perform a test, increment this by one.

// Include model header, generated from Verilating "tb_top.v"
#include "Vfreerun.h"

// Verilator using new C++?  Need to include these now (sstep2021)
#include <iostream>
using namespace std;

// Current simulation time (64-bit unsigned)
vluint64_t main_time = 0;
// Called by $time in Verilog
double sc_time_stamp()
{
  return main_time; // Note does conversion to real, to match SystemC
}

int concat(int w, int a, int b)
{
  return (a << w) | b;
}

std::string bin(int b)
{
  std::string ans;
  while (b != 0)
  {
    ans += b % 2 == 0 ? "0" : "1";
    b /= 2;
  }
  std::reverse(ans.begin(), ans.end());
  return ans;
}

int pin(int a, int b) {
  return (a >> b) & 1;
}

void update_tests(int passed, int total, std::string test) {
  // add tests to global test variables
  passed_test_count += passed;
  total_test_count += total;
  // perform sanity check on global test variables
  assert(passed_test_count <= total_test_count);
  // let TA know if this set of tests failed
  std::cout << "Part " << test << ": " << std::to_string(passed) << " out of " << std::to_string(total) << " tests passed.\n";
}

void cycle_clock(Vfreerun* freerun) {
    freerun->clk = 1; freerun->eval();
    freerun->clk = 0; freerun->eval();
}

int main(int argc, char **argv, char **env)
{
  // This is a more complicated example, please also see the simpler examples/make_hello_c.

  // Prevent unused variable warnings
  if (0 && argc && argv && env) {}

  // Set debug level, 0 is off, 9 is highest presently used
  // May be overridden by commandArgs
  Verilated::debug(0);

  // Randomization reset policy
  // May be overridden by commandArgs
  Verilated::randReset(2);

  // Verilator must compute traced signals
  Verilated::traceEverOn(true);

  // Pass arguments so Verilated code can see them, e.g. $value$plusargs
  // This needs to be called before you create any model
  Verilated::commandArgs(argc, argv);

  // Construct the Verilated model, from each module after Verilating each module file
  Vfreerun *freerun = new Vfreerun; // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper

  int passed_subtests = 0; // if we have multiple subtests, we need to know how many passed for each
  int total_subtests = 0;

  /*************************************************************************/
  // BEGIN TESTS
  // grade.py parameters - DO NOT DELETE!
  // GRADEPY 1 [15.0]
  /***********************************/
  // freerun - Step 1
  freerun->clk = 0;
  freerun->rst = 0;
  freerun->eval();

  freerun->rst = 1;
  freerun->eval();
  if (freerun->out == 0)
    passed_subtests++;
  total_subtests++;

  freerun->rst = 0;
  freerun->en = 1;
  freerun->max = 255;
  freerun->eval();
  int expected = freerun->out + 1;
  for (int i = 0; i < 256; i++) {
    cycle_clock(freerun);
    if (freerun->out == expected % 256)
      passed_subtests++;
    total_subtests++;
    expected = expected + 1;
  }
  expected = 0;
  freerun->max = 127;
  expected = freerun->out + 1;
  for (int i = 0; i < 256; i++) {
    cycle_clock(freerun);
    if (freerun->out == expected % 128)
      passed_subtests++;
    total_subtests++;
    expected = expected + 1;
  }
  // test enable signal when off
  freerun->en = 0;
  freerun->eval();
  expected--;
  cycle_clock(freerun);
  if (freerun->out == expected % 128)
    passed_subtests++;
  total_subtests++;
  cycle_clock(freerun);
  if (freerun->out == expected % 128)
    passed_subtests++;
  total_subtests++;
  cycle_clock(freerun);
  if (freerun->out == expected % 128)
    passed_subtests++;
  total_subtests++;

  // async reset should work instantly and regardless of enable
  freerun->rst = 1;
  freerun->eval();
  if (freerun->out == 0)
    passed_subtests++;
  total_subtests++;

  update_tests(passed_subtests, total_subtests, "1");
  /***********************************/

  passed_subtests = 0;
  total_subtests = 0;

  /***********************************/
  // END TESTS
  /*************************************************************************/

  // good to have to detect bugs
  assert(passed_test_count <= total_test_count);

  if (passed_test_count == total_test_count)
  {
    std::cout << "ALL " << std::to_string(total_test_count) << " TESTS PASSED"
              << "\n";
  }
  else
  {
    std::cout << "ERROR: " << std::to_string(passed_test_count) << "/" << std::to_string(total_test_count) << " tests passed.\n";
  }

  // Final model cleanups
  freerun->final();

  // Destroy models
  delete freerun;
  freerun = NULL;

  // Fin
  return passed_test_count == total_test_count ? 0 : 1;
}
