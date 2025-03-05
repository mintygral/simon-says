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
#include "Vmem.h"

// Verilator using new C++?  Need to include these now (sstep2021)
#include <iostream>
#include <fstream>
#include <sstream>
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

void cycle_clock(Vmem* mem) {
    mem->clk = 1; mem->eval();
    mem->clk = 0; mem->eval();
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

  // it should be the same one used by mem.sv, so we provide it before we create the model
  int hexmem[256];
  // create a file output stream named press.mem
  ofstream pressmem("press.mem");
  // to this file stream, write 256 lines containing three random hex digits and a newline
  for (int i = 0; i < 256; i++) {
    hexmem[i] = rand() % 4096;
    pressmem << hex << hexmem[i] << "\n";
  }
  // close the file stream
  pressmem.close();

  // we generate press.mem file and save it BEFORE instantiating module, otherwise will complain about missing press.mem

  // Construct the Verilated model, from each module after Verilating each module file
  Vmem *mem = new Vmem; // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper

  int passed_subtests = 0; // if we have multiple subtests, we need to know how many passed for each
  int total_subtests = 0;

  /*************************************************************************/
  // BEGIN TESTS
  /*************************************************************************/
  // mem
  mem->sel = 0;
  mem->en = 0;
  mem->eval();

  mem->sel = 2;
  mem->en = 0;
  mem->eval();
  if (mem->btns != hexmem[2]) {
    passed_subtests++;
  }
  else {
    std::cout << "mem no-clock no-enable test failed: value changed unexpectedly before clk was toggled. Make sure you assigning btns on the next rising edge of clk, not combinationally." << "\n";
  }
  total_subtests++;

  mem->en = 1;
  for(int i = 0; i < 256; i++) {
    mem->sel = i;
    mem->eval();
    cycle_clock(mem);
    if (mem->btns == hexmem[i]) {
      passed_subtests++;
    }
    else {
      std::cout << "mem test failed: mem->btns = " << std::hex << mem->btns << " but should be " << hexmem[i] << " for sel = " << i << std::endl;
    }
    total_subtests++;
  }

  update_tests(passed_subtests, total_subtests, "1");
  /***********************************/

  passed_subtests = 0;
  total_subtests = 0;

  /***********************************/
  // END TESTS
  /*************************************************************************/

  // remove the press.mem file
  remove("press.mem");

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
  mem->final();

  // Destroy models
  delete mem;
  mem = NULL;

  // Fin
  return passed_test_count == total_test_count ? 0 : 1;
}
