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
#include "Vssdec_ext.h"

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
  Vssdec_ext *ssdec_ext = new Vssdec_ext; // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper

  int passed_subtests = 0; // if we have multiple subtests, we need to know how many passed for each
  int total_subtests = 0;

  /*************************************************************************/
  // BEGIN TESTS
  // grade.py parameters - DO NOT DELETE!
  // GRADEPY 1 [15.0]
  /***********************************/
  // ssdec_ext - Step 4
  ssdec_ext->in = 0;
  ssdec_ext->enable = 1;
  ssdec_ext->eval();

  for(int i = 0; i < 16; i++) {
    ssdec_ext->in = i;
    ssdec_ext->eval();

    int r3 = (i==15) || (i==14) || (i==13) || (i==12) || (i==11) || (i==10) || (i==9) || (i==8);
    int r2 = (i==15) || (i==14) || (i==13) || (i==12) || (i==7) || (i==6) || (i==5) || (i==4);
    int r1 = (i==15) || (i==14) || (i==11) || (i==10) || (i==7) || (i==6) || (i==3) || (i==2);
    // way too lazy to be writing out all odd-numbered bits of i
    int r0 = i % 2 != 0;

    int expected = concat(3, r3, concat(2, r2, concat(1, r1, r0)));

    int ps = passed_subtests;
    std::cout << "out value: " << std::to_string(ssdec_ext->out & 0x7F);
    std::cout << ", expected: " << std::to_string(expected) << "\n";

    switch (expected) {
      case  0: passed_subtests += ((ssdec_ext->out & 0x7F) == 63)   ? 1 : 0; break;
      case  1: passed_subtests += ((ssdec_ext->out & 0x7F) == 6)   ? 1 : 0; break;
      case  2: passed_subtests += ((ssdec_ext->out & 0x7F) == 91)  ? 1 : 0; break;
      case  3: passed_subtests += ((ssdec_ext->out & 0x7F) == 79)  ? 1 : 0; break;
      case  4: passed_subtests += ((ssdec_ext->out & 0x7F) == 102) ? 1 : 0; break;
      case  5: passed_subtests += ((ssdec_ext->out & 0x7F) == 109) ? 1 : 0; break;
      case  6: passed_subtests += ((ssdec_ext->out & 0x7F) == 125) ? 1 : 0; break;
      case  7: passed_subtests += ((ssdec_ext->out & 0x7F) == 7)   ? 1 : 0; break;
      case  8: passed_subtests += ((ssdec_ext->out & 0x7F) == 127) ? 1 : 0; break;
      case  9: passed_subtests += ((ssdec_ext->out & 0x7F) == 103) ? 1 : 0; break;
      case 10: passed_subtests += ((ssdec_ext->out & 0x7F) == 119) ? 1 : 0; break;
      case 11: passed_subtests += ((ssdec_ext->out & 0x7F) == 124) ? 1 : 0; break;
      case 12: passed_subtests += ((ssdec_ext->out & 0x7F) == 57)  ? 1 : 0; break;
      case 13: passed_subtests += ((ssdec_ext->out & 0x7F) == 94)  ? 1 : 0; break;
      case 14: passed_subtests += ((ssdec_ext->out & 0x7F) == 121) ? 1 : 0; break;
      case 15: passed_subtests += ((ssdec_ext->out & 0x7F) == 113) ? 1 : 0; break;
    }
    total_subtests++;
  }

  for(int i = 16; i < 16+11; i++) {
    ssdec_ext->in = i;
    ssdec_ext->eval();

    int ps = passed_subtests;
    std::cout << "out value: " << std::to_string(ssdec_ext->out & 0x7F);
    int expected = 1;

    switch (i-16) {
      case  0: expected = 0b1101111; std::cout << ", expected: g" << std::endl; break;   // 0 g
      case  1: expected = 0b1110110; std::cout << ", expected: H" << std::endl; break;   // 1 H
      case  2: expected = 0b0010000; std::cout << ", expected: i" << std::endl; break;   // 2 i
      case  3: expected = 0b0011110; std::cout << ", expected: j" << std::endl; break;   // 3 j
      case  4: expected = 0b0111000; std::cout << ", expected: L" << std::endl; break;   // 4 L
      case  5: expected = 0b1010100; std::cout << ", expected: n" << std::endl; break;   // 5 n
      case  6: expected = 0b1010000; std::cout << ", expected: r" << std::endl; break;   // 6 r
      case  7: expected = 0b1111000; std::cout << ", expected: t" << std::endl; break;   // 7 t
      case  8: expected = 0b1101110; std::cout << ", expected: y" << std::endl; break;   // 8 y
      case  9: expected = 0b1010011; std::cout << ", expected: ?" << std::endl; break;   // 9 ?
      default: expected = 0b0;       std::cout << ", expected: ' '" << std::endl; break;
    }
    if ((ssdec_ext->out & 0x7F) == expected) {
	passed_subtests++;
    }
    else {
        std::cout << "Error, expected ssX value = " << std::to_string(expected) << std::endl;
    }
    // passed_subtests += ((ssdec_ext->out & 0x7F) == expected) ? 1 : 0;
    total_subtests++;
  }

  ssdec_ext->enable = 0;
  ssdec_ext->eval();
  passed_subtests += (ssdec_ext->out == 0) ? 1 : 0;
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
  ssdec_ext->final();

  // Destroy models
  delete ssdec_ext;
  ssdec_ext = NULL;

  // Fin
  return passed_test_count == total_test_count ? 0 : 1;
}
