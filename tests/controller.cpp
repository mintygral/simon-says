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
#include "Vcontroller.h"

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

int pinsel(int x, int a, int b) {
    assert(a < b);
    int y = 0;
    for (int i = a; i <= b; i++) {
        y |= pin(x, i);
    }
    return y;
}

void update_tests(int passed, int total, std::string test) {
  // add tests to global test variables
  passed_test_count += passed;
  total_test_count += total;
  // perform sanity check on global test variables
  assert(passed_test_count <= total_test_count);
  // let TA know if this set of tests failed
  std::cout << "controller: " << std::to_string(passed) << " out of " << std::to_string(total) << " tests passed.\n";
}

void cycle_clock(Vcontroller* controller) {
    controller->clk = 1; controller->eval();
    controller->clk = 0; controller->eval();
}
void assert_reset(Vcontroller* controller) {
    controller->rst = 1; controller->eval();
    controller->rst = 0; controller->eval();
}

void check_output(std::string state, bool cond, std::string test, int* passed, int* total) {
  /* 1	Blue	9	Light Blue
   2	Green	0	Black
   3	Aqua	10	Light Green
   4	Red	11	Light Aqua
   5	Purple	12	Light Red
   7	White	14	Light Yellow
   8	Gray	15	Bright White */
  if (cond) {
    (*passed)++;
    std::cout << "\033[0;32m";
    std::cout<<"[PASS] ";

  } else {
    std::cout << "\033[0;31m";
    std::cout<<"[FAIL] ";
  }
  std::cout << "\033[0m";
  std::cout << "In " << state << " state: Test " << test << "\n";
  (*total)++;
}

// write a C++ function that takes a string, and pads it with dashes on both sides of the string 
// such that the total length of the output string is 80 characters, and prints it out.
void print_header(std::string s) {
  int n = s.length();
  int pad = (50 - n) / 2;
  std::string dashes = "";
  for (int i = 0; i < pad; i++) {
    dashes += "-";
  }
  std::cout << dashes << " " << s << " " << dashes << "\n";
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
  Vcontroller *controller = new Vcontroller; // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper

  int passed_subtests = 0; // if we have multiple subtests, we need to know how many passed for each
  int total_subtests = 0;

  /*************************************************************************/
  // BEGIN TESTS
  /***********************************/
  // controller
  int READY = 0;
  int PLAY = 1;
  int FAIL = 2;
  int PASS = 3;
  int WIN = 4;

  // test controller reset
  controller->clk = 0; 
  controller->simon_says = 0;
  controller->sk_strobe = 0;
  controller->round_passed = 0;
  controller->sd_is_empty = 0;
  controller->eval();
  print_header("Power-on: testing reset");
  assert_reset(controller);
  check_output("RESET", controller->state == READY, "state == READY", &passed_subtests, &total_subtests);
  check_output("RESET", controller->sd_srst == 1, "sd_srst == 1", &passed_subtests, &total_subtests);
  check_output("RESET", controller->fr_en == 1, "fr_en == 1", &passed_subtests, &total_subtests);
  check_output("RESET", controller->is_wrong == 0, "is_wrong == 0", &passed_subtests, &total_subtests);
  check_output("RESET", controller->is_correct == 0, "is_correct == 0", &passed_subtests, &total_subtests);
  check_output("RESET", controller->mem_en == 1, "mem_en == 1", &passed_subtests, &total_subtests);
  check_output("RESET", controller->score == 0, "score == 0", &passed_subtests, &total_subtests);
  
  // test controller ready state after reset
  // do this 16 times as if we're playing the game
  std::cout << std::endl; print_header("Enter READY state");
  for (int i = 0; i < 16; i++) {
    print_header("Clock " + std::to_string(i+1) + " times");
    cycle_clock(controller);
    check_output("READY", controller->state == READY, "state == READY", &passed_subtests, &total_subtests);
    check_output("READY", controller->fr_en == 1, "fr_en == 1", &passed_subtests, &total_subtests);
    check_output("READY", controller->mem_en == 1, "mem_en == 1", &passed_subtests, &total_subtests);
    // reset goes low to allow timer/shiftdown to start shifting down
    check_output("READY", controller->sd_srst == 0, "sd_srst == 0", &passed_subtests, &total_subtests);
  }

  print_header("shiftdown is now empty");
  // then, shiftdown should be empty, so we assert the corresponding signal
  controller->sd_is_empty = 1; controller->eval();
  cycle_clock(controller);
  check_output("READY", controller->state == PLAY, "state == PLAY", &passed_subtests, &total_subtests);
  check_output("READY", controller->sd_srst == 1, "sd_srst == 1", &passed_subtests, &total_subtests);
  check_output("READY", controller->fr_en == 0, "fr_en == 0", &passed_subtests, &total_subtests);
  check_output("READY", controller->is_wrong == 0, "is_wrong == 0", &passed_subtests, &total_subtests);
  check_output("READY", controller->is_correct == 0, "is_correct == 0", &passed_subtests, &total_subtests);
  check_output("READY", controller->mem_en == 0, "mem_en == 0", &passed_subtests, &total_subtests);
  check_output("READY", controller->score == 0, "score == 0", &passed_subtests, &total_subtests);

  // shiftdown reset is asserted, so sd_is_empty should not be true anymore
  controller->sd_is_empty = 0; controller->eval();
  // we now play the game. Test a combination of buttons ABC
  // so simon_says should be the XOR of each of the bits 12'b101010111100 = 1.
  // so at some point the buttons must be pressed.
  std::cout << std::endl; print_header("Enter PLAY state");
  controller->simon_says = 1; controller->eval();
  print_header("Simon Says: 1");
  for (int i = 0; i < 4; i++) {
    print_header("Clock " + std::to_string(i+1) + " times");
    cycle_clock(controller);
    check_output("PLAY", controller->state == PLAY, "state == PLAY", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->sd_srst == 0, "sd_srst == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->fr_en == 0, "fr_en == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->is_wrong == 0, "is_wrong == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->is_correct == 0, "is_correct == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->mem_en == 0, "mem_en == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->score == 0, "score == 0", &passed_subtests, &total_subtests);
  }
  print_header("Asserting strobe from scankey");
  // now, we press the buttons, which is indicated by a single rising edge on sk_strobe
  controller->sk_strobe = 1; controller->eval();
  for (int i = 4; i < 14; i++) {
    print_header("Clock " + std::to_string(i+1) + " times");
    cycle_clock(controller);
    // should NOT fail - simon says is high
    check_output("PLAY", controller->state == PLAY, "state == PLAY", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->sd_srst == 0, "sd_srst == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->fr_en == 1, "fr_en == 1", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->is_wrong == 0, "is_wrong == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->is_correct == 0, "is_correct == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->mem_en == 0, "mem_en == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->score == 0, "score == 0", &passed_subtests, &total_subtests);
  }

  // let's say we pressed all the right buttons at the same time at the very end
  // BUT we should not yet advance - time has not run out.
  print_header("Correct buttons were pressed");
  controller->round_passed = 1; controller->eval();
  print_header("Clock 15 times");
  cycle_clock(controller);
  check_output("PLAY", controller->state == PLAY, "state == PLAY", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->sd_srst == 0, "sd_srst == 0", &passed_subtests, &total_subtests);
  // up to students if they want to keep the counter running after the first round - undefined in step description
  // check_output("PLAY", controller->fr_en == 1, "fr_en == 1", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->is_wrong == 0, "is_wrong == 0", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->is_correct == 0, "is_correct == 0", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->mem_en == 0, "mem_en == 0", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->score == 0, "score == 0", &passed_subtests, &total_subtests);

  // shiftdown is now empty
  print_header("Shiftdown is now empty");
  controller->sd_is_empty = 1; controller->eval();
  print_header("Clock 16 times, expecting PASS state");
  cycle_clock(controller);
  check_output("PLAY", controller->state == PASS, "state == PASS", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->sd_srst == 1, "sd_srst == 1", &passed_subtests, &total_subtests);
  // up to students if they want to keep the counter running after the first round - undefined in step description
  // check_output("PLAY", controller->fr_en == 1, "fr_en == 1", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->is_wrong == 0, "is_wrong == 0", &passed_subtests, &total_subtests);
  // students may choose to assert is_correct earlier if they wish, so we'll wait until PASS to check.
  // check_output("PLAY", controller->is_correct == 0, "is_correct == 0", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->mem_en == 0, "mem_en == 0", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->score == 0, "score == 0", &passed_subtests, &total_subtests);

  // now, we go to PASS
  controller->sd_is_empty = 0; controller->eval();
  print_header("Leaving PASS state to go to READY");
  cycle_clock(controller);
  check_output("PASS", controller->state == READY, "state == READY", &passed_subtests, &total_subtests);
  check_output("PASS", controller->sd_srst == 0, "sd_srst == 0", &passed_subtests, &total_subtests);
  // up to students if they want to keep the counter running after the first round - undefined in step description
  // check_output("PASS", controller->fr_en == 1, "fr_en == 1", &passed_subtests, &total_subtests);
  check_output("PASS", controller->is_wrong == 0, "is_wrong == 0", &passed_subtests, &total_subtests);
  check_output("PASS", controller->is_correct == 1, "is_correct == 1", &passed_subtests, &total_subtests);
  check_output("PASS", controller->mem_en == 0, "mem_en == 0", &passed_subtests, &total_subtests);
  check_output("PASS", controller->score == 1, "score == 1", &passed_subtests, &total_subtests);
  
  // now, we go back to READY
  cycle_clock(controller);
  std::cout << std::endl; print_header("Returned to READY state for round 2");
  print_header("Should indicate passed round");
  for (int i = 0; i < 16; i++) {
    print_header("Clock " + std::to_string(i+1) + " times");
    cycle_clock(controller);
    check_output("READY", controller->state == READY, "state == READY", &passed_subtests, &total_subtests);
    check_output("READY", controller->fr_en == 1, "fr_en == 1", &passed_subtests, &total_subtests);
    check_output("READY", controller->mem_en == 1, "mem_en == 1", &passed_subtests, &total_subtests);
    // reset goes low to allow timer/shiftdown to start shifting down
    check_output("READY", controller->sd_srst == 0, "sd_srst == 0", &passed_subtests, &total_subtests);
    check_output("READY", controller->is_correct == 1, "is_correct == 1", &passed_subtests, &total_subtests);
    check_output("PASS", controller->score == 1, "score == 1", &passed_subtests, &total_subtests);
  }

  print_header("shiftdown is now empty");
  // then, shiftdown should be empty, so we assert the corresponding signal
  controller->sd_is_empty = 1; controller->eval();
  cycle_clock(controller);
  check_output("READY", controller->state == PLAY, "state == PLAY", &passed_subtests, &total_subtests);
  check_output("READY", controller->sd_srst == 1, "sd_srst == 1", &passed_subtests, &total_subtests);
  check_output("READY", controller->fr_en == 0, "fr_en == 0", &passed_subtests, &total_subtests);
  check_output("READY", controller->is_wrong == 0, "is_wrong == 0", &passed_subtests, &total_subtests);
  check_output("READY", controller->is_correct == 0, "is_correct == 0", &passed_subtests, &total_subtests);
  check_output("READY", controller->mem_en == 0, "mem_en == 0", &passed_subtests, &total_subtests);
  check_output("READY", controller->score == 1, "score == 1", &passed_subtests, &total_subtests);

  // shiftdown reset is asserted, so sd_is_empty should not be true anymore
  controller->sd_is_empty = 0; controller->eval();
  // we now play the game. Test a combination of buttons BCD
  // so simon_says should be the XOR of each of the bits 12'b101111001101 = 0.
  // at no point should the buttons be pressed.
  print_header("Enter PLAY state");
  controller->simon_says = 0; controller->eval();
  print_header("Simon Says: 0");
  for (int i = 0; i < 16; i++) {
    print_header("Clock " + std::to_string(i+1) + " times");
    controller->simon_says = 1; controller->eval();
    cycle_clock(controller);
    check_output("PLAY", controller->state == PLAY, "state == PLAY", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->sd_srst == 0, "sd_srst == 0", &passed_subtests, &total_subtests);
    // up to students if they want to keep the counter running after the first round - undefined in step description
    // check_output("PLAY", controller->fr_en == 0, "fr_en == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->is_wrong == 0, "is_wrong == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->is_correct == 0, "is_correct == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->mem_en == 0, "mem_en == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->score == 1, "score == 1", &passed_subtests, &total_subtests);
  }

  // since simon did not say, and we didn't need to press anything, we should be in PASS
  print_header("Shiftdown is now empty");
  controller->sd_is_empty = 1; controller->eval();
  print_header("Clock 16 times, expecting PASS state");
  cycle_clock(controller);
  check_output("PLAY", controller->state == PASS, "state == PASS", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->sd_srst == 1, "sd_srst == 1", &passed_subtests, &total_subtests);
  // up to students if they want to keep the counter running after the first round - undefined in step description
  // check_output("PLAY", controller->fr_en == 1, "fr_en == 1", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->is_wrong == 0, "is_wrong == 0", &passed_subtests, &total_subtests);
  // students may choose to assert is_correct earlier if they wish, so we'll wait until PASS to check.
  // check_output("PLAY", controller->is_correct == 0, "is_correct == 0", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->mem_en == 0, "mem_en == 0", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->score == 1, "score == 1", &passed_subtests, &total_subtests);

  // now, we go to PASS
  controller->sd_is_empty = 0; controller->eval();
  print_header("Leaving PASS state to go to READY");
  cycle_clock(controller);
  check_output("PASS", controller->state == READY, "state == READY", &passed_subtests, &total_subtests);
  check_output("PASS", controller->sd_srst == 0, "sd_srst == 0", &passed_subtests, &total_subtests);
  // up to students if they want to keep the counter running after the first round - undefined in step description
  // check_output("PASS", controller->fr_en == 1, "fr_en == 1", &passed_subtests, &total_subtests);
  check_output("PASS", controller->is_wrong == 0, "is_wrong == 0", &passed_subtests, &total_subtests);
  check_output("PASS", controller->is_correct == 1, "is_correct == 1", &passed_subtests, &total_subtests);
  check_output("PASS", controller->mem_en == 0, "mem_en == 0", &passed_subtests, &total_subtests);
  check_output("PASS", controller->score == 2, "score == 2", &passed_subtests, &total_subtests);

  // now, we go back to READY
  cycle_clock(controller);
  std::cout << std::endl; print_header("Returned to READY state for round 3");
  print_header("Should indicate passed round");
  for (int i = 0; i < 16; i++) {
    print_header("Clock " + std::to_string(i+1) + " times");
    cycle_clock(controller);
    check_output("READY", controller->state == READY, "state == READY", &passed_subtests, &total_subtests);
    check_output("READY", controller->fr_en == 1, "fr_en == 1", &passed_subtests, &total_subtests);
    check_output("READY", controller->mem_en == 1, "mem_en == 1", &passed_subtests, &total_subtests);
    // reset goes low to allow timer/shiftdown to start shifting down
    check_output("READY", controller->sd_srst == 0, "sd_srst == 0", &passed_subtests, &total_subtests);
    check_output("READY", controller->is_correct == 1, "is_correct == 1", &passed_subtests, &total_subtests);
    check_output("PASS", controller->score == 2, "score == 2", &passed_subtests, &total_subtests);
  }

  print_header("shiftdown is now empty");
  // then, shiftdown should be empty, so we assert the corresponding signal
  controller->sd_is_empty = 1; controller->eval();
  cycle_clock(controller);
  check_output("READY", controller->state == PLAY, "state == PLAY", &passed_subtests, &total_subtests);
  check_output("READY", controller->sd_srst == 1, "sd_srst == 1", &passed_subtests, &total_subtests);
  check_output("READY", controller->fr_en == 0, "fr_en == 0", &passed_subtests, &total_subtests);
  check_output("READY", controller->is_wrong == 0, "is_wrong == 0", &passed_subtests, &total_subtests);
  check_output("READY", controller->is_correct == 0, "is_correct == 0", &passed_subtests, &total_subtests);
  check_output("READY", controller->mem_en == 0, "mem_en == 0", &passed_subtests, &total_subtests);
  check_output("READY", controller->score == 2, "score == 2", &passed_subtests, &total_subtests);

    // shiftdown reset is asserted, so sd_is_empty should not be true anymore
  controller->sd_is_empty = 0; controller->eval();
  // we now play the game. Test a combination of buttons EF0
  // so simon_says should be the XOR of each of the bits 12'b111011110000 = 1.
  // so at some point the buttons must be pressed.
  // but for this round, we'll not press the right buttons.
  std::cout << std::endl; print_header("Enter PLAY state");
  controller->simon_says = 1; controller->eval();
  print_header("Simon Says: 1");
  for (int i = 0; i < 4; i++) {
    print_header("Clock " + std::to_string(i+1) + " times");
    cycle_clock(controller);
    check_output("PLAY", controller->state == PLAY, "state == PLAY", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->sd_srst == 0, "sd_srst == 0", &passed_subtests, &total_subtests);
    // students may choose to assert is_correct earlier if they wish, so we'll wait until PASS/FAIL to check.
    // check_output("PLAY", controller->fr_en == 0, "fr_en == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->is_wrong == 0, "is_wrong == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->is_correct == 0, "is_correct == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->mem_en == 0, "mem_en == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->score == 2, "score == 2", &passed_subtests, &total_subtests);
  }
  print_header("Asserting strobe from scankey");
  // now, we press the buttons, which is indicated by a single rising edge on sk_strobe
  controller->sk_strobe = 1; controller->eval();
  for (int i = 4; i < 14; i++) {
    print_header("Clock " + std::to_string(i+1) + " times");
    cycle_clock(controller);
    // should NOT fail - simon says is high
    check_output("PLAY", controller->state == PLAY, "state == PLAY", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->sd_srst == 0, "sd_srst == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->fr_en == 1, "fr_en == 1", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->is_wrong == 0, "is_wrong == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->is_correct == 0, "is_correct == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->mem_en == 0, "mem_en == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->score == 2, "score == 2", &passed_subtests, &total_subtests);
  }
  controller->sk_strobe = 0; controller->eval();

  // let's say we did not press all the right buttons
  // BUT we should not yet advance - time has not run out.
  print_header("Incorrect buttons were pressed");
  controller->round_passed = 0; controller->eval();
  print_header("Clock 15 times");
  cycle_clock(controller);
  check_output("PLAY", controller->state == PLAY, "state == PLAY", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->sd_srst == 0, "sd_srst == 0", &passed_subtests, &total_subtests);
  // up to students if they want to keep the counter running after the first round - undefined in step description
  // check_output("PLAY", controller->fr_en == 1, "fr_en == 1", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->is_wrong == 0, "is_wrong == 0", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->is_correct == 0, "is_correct == 0", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->mem_en == 0, "mem_en == 0", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->score == 2, "score == 2", &passed_subtests, &total_subtests);

  // shiftdown is now empty
  print_header("Shiftdown is now empty");
  controller->sd_is_empty = 1; controller->eval();
  print_header("Clock 16 times, expecting FAIL state");
  cycle_clock(controller);
  check_output("PLAY", controller->state == FAIL, "state == FAIL", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->sd_srst == 1, "sd_srst == 1", &passed_subtests, &total_subtests);
  // students may choose to assert is_correct earlier if they wish, so we'll wait until PASS/FAIL to check.
  // check_output("PLAY", controller->fr_en == 1, "fr_en == 1", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->is_correct == 0, "is_correct == 0", &passed_subtests, &total_subtests);
  // students may choose to assert is_correct earlier if they wish, so we'll wait until PASS to check.
  // check_output("PLAY", controller->is_wrong == 0, "is_wrong == 0", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->mem_en == 0, "mem_en == 0", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->score == 2, "score == 2", &passed_subtests, &total_subtests);

  // and so we should be in FAIL
  controller->sd_is_empty = 0; controller->eval();
  std::cout << std::endl; print_header("Must be stuck in FAIL state forever");
  for (int i = 0; i < 10; i++) {
    cycle_clock(controller);
    print_header("Clock " + std::to_string(i+1) + " times");
    check_output("FAIL", controller->state == FAIL, "state == FAIL", &passed_subtests, &total_subtests);
    // students may choose to assert is_wrong earlier if they wish, so we'll wait until FAIL to check.
    check_output("FAIL", controller->is_wrong == 1, "is_wrong == 1", &passed_subtests, &total_subtests);
    check_output("FAIL", controller->score == 2, "score == 2", &passed_subtests, &total_subtests);
    // we're not checking other outputs here since we really don't care about them
    // in the FAIL state.
  }
  
  // async reset everything
  print_header("Resetting everything");
  assert_reset(controller);
  check_output("RESET2", controller->state == READY, "state == READY", &passed_subtests, &total_subtests);
  check_output("RESET2", controller->sd_srst == 1, "sd_srst == 1", &passed_subtests, &total_subtests);
  check_output("RESET2", controller->fr_en == 1, "fr_en == 1", &passed_subtests, &total_subtests);
  check_output("RESET2", controller->is_wrong == 0, "is_wrong == 0", &passed_subtests, &total_subtests);
  check_output("RESET2", controller->is_correct == 0, "is_correct == 0", &passed_subtests, &total_subtests);
  check_output("RESET2", controller->mem_en == 1, "mem_en == 1", &passed_subtests, &total_subtests);
  check_output("RESET2", controller->score == 0, "score == 0", &passed_subtests, &total_subtests);
  
  std::cout << std::endl; print_header("Enter READY state (part 2)");
  for (int i = 0; i < 16; i++) {
    print_header("Clock " + std::to_string(i+1) + " times");
    cycle_clock(controller);
    check_output("READY", controller->state == READY, "state == READY", &passed_subtests, &total_subtests);
    check_output("READY", controller->fr_en == 1, "fr_en == 1", &passed_subtests, &total_subtests);
    check_output("READY", controller->mem_en == 1, "mem_en == 1", &passed_subtests, &total_subtests);
    // reset goes low to allow timer/shiftdown to start shifting down
    check_output("READY", controller->sd_srst == 0, "sd_srst == 0", &passed_subtests, &total_subtests);
  }

  print_header("shiftdown is now empty");
  // then, shiftdown should be empty, so we assert the corresponding signal
  controller->sd_is_empty = 1; controller->eval();
  cycle_clock(controller);
  check_output("READY", controller->state == PLAY, "state == PLAY", &passed_subtests, &total_subtests);
  check_output("READY", controller->sd_srst == 1, "sd_srst == 1", &passed_subtests, &total_subtests);
  check_output("READY", controller->fr_en == 0, "fr_en == 0", &passed_subtests, &total_subtests);
  check_output("READY", controller->is_wrong == 0, "is_wrong == 0", &passed_subtests, &total_subtests);
  check_output("READY", controller->is_correct == 0, "is_correct == 0", &passed_subtests, &total_subtests);
  check_output("READY", controller->mem_en == 0, "mem_en == 0", &passed_subtests, &total_subtests);
  check_output("READY", controller->score == 0, "score == 0", &passed_subtests, &total_subtests);

  // shiftdown reset is asserted, so sd_is_empty should not be true anymore
  controller->sd_is_empty = 0; controller->eval();
  // we now play the game. Test a combination of buttons BCD
  // so simon_says should be the XOR of each of the bits 12'b101111001101 = 0.
  // so at some point the buttons must be pressed.
  std::cout << std::endl; print_header("Enter PLAY state (part 2)");
  controller->simon_says = 0; controller->eval();
  print_header("Simon Says: 0");
  for (int i = 0; i < 4; i++) {
    print_header("Clock " + std::to_string(i+1) + " times");
    cycle_clock(controller);
    check_output("PLAY", controller->state == PLAY, "state == PLAY", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->sd_srst == 0, "sd_srst == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->fr_en == 0, "fr_en == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->is_wrong == 0, "is_wrong == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->is_correct == 0, "is_correct == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->mem_en == 0, "mem_en == 0", &passed_subtests, &total_subtests);
    check_output("PLAY", controller->score == 0, "score == 0", &passed_subtests, &total_subtests);
  }
  print_header("Asserting strobe from scankey");
  // now, we press the buttons, which is indicated by a single rising edge on sk_strobe
  // BUT since simon says = 0, THIS SHOULD GO TO FAIL IMMEDIATELY.
  controller->sk_strobe = 1; controller->eval();

  print_header("Clock 5 times");
  cycle_clock(controller);
  // should NOT fail - simon says is high
  check_output("PLAY", controller->state == FAIL, "state == FAIL", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->sd_srst == 0, "sd_srst == 0", &passed_subtests, &total_subtests);
  // students may choose to put this in the "else" block, which is... fine
  // check_output("PLAY", controller->fr_en == 1, "fr_en == 1", &passed_subtests, &total_subtests);
  // students may choose to assert is_wrong earlier if they wish, so we'll wait until FAIL to check.
  // check_output("PLAY", controller->is_wrong == 0, "is_wrong == 0", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->is_correct == 0, "is_correct == 0", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->mem_en == 0, "mem_en == 0", &passed_subtests, &total_subtests);
  check_output("PLAY", controller->score == 0, "score == 0", &passed_subtests, &total_subtests);

  // and so we should be in FAIL
  controller->sd_is_empty = 0; controller->eval();
  std::cout << std::endl; print_header("Must be stuck in FAIL state forever");
  for (int i = 0; i < 10; i++) {
    print_header("Clock " + std::to_string(i+1) + " times");
    cycle_clock(controller);
    check_output("FAIL", controller->state == FAIL, "state == FAIL", &passed_subtests, &total_subtests);
    // students may choose to assert is_wrong earlier if they wish, so we'll wait until FAIL to check.
    check_output("FAIL", controller->is_wrong == 1, "is_wrong == 1", &passed_subtests, &total_subtests);
    check_output("FAIL", controller->score == 0, "score == 0", &passed_subtests, &total_subtests);
    // we're not checking other outputs here since we really don't care about them
    // in the FAIL state.
  }

  // now to check for WIN state.  We'll do this by setting the score to 9.
  // we do that by having simon_says and round_passed be 1 throughout.
  assert_reset(controller);
  check_output("RESET3", controller->state == READY, "state == READY", &passed_subtests, &total_subtests);
  check_output("RESET3", controller->sd_srst == 1, "sd_srst == 1", &passed_subtests, &total_subtests);
  check_output("RESET3", controller->fr_en == 1, "fr_en == 1", &passed_subtests, &total_subtests);
  check_output("RESET3", controller->is_wrong == 0, "is_wrong == 0", &passed_subtests, &total_subtests);
  check_output("RESET3", controller->is_correct == 0, "is_correct == 0", &passed_subtests, &total_subtests);
  check_output("RESET3", controller->mem_en == 1, "mem_en == 1", &passed_subtests, &total_subtests);
  check_output("RESET3", controller->score == 0, "score == 0", &passed_subtests, &total_subtests);  

  controller->clk = 0; 
  controller->simon_says = 0;
  controller->sk_strobe = 0;
  controller->round_passed = 0;
  controller->sd_is_empty = 0;
  controller->eval();
  std::cout << std::endl; print_header("Waiting till WIN state with simon_says = 1");
  for (int i = 0; i < 10; i++) {
    print_header("Round " + std::to_string(i+1) + ": Enter READY state");
    check_output("READY", controller->state == READY, "state == READY", &passed_subtests, &total_subtests);
    cycle_clock(controller); cycle_clock(controller); cycle_clock(controller);
    controller->sd_is_empty = 1; controller->eval();
    cycle_clock(controller);
    print_header("Round " + std::to_string(i+1) + ": Enter PLAY state");
    check_output("PLAY", controller->state == PLAY, "state == PLAY", &passed_subtests, &total_subtests);
    controller->sd_is_empty = 0; controller->eval();
    controller->simon_says = 1; controller->eval();
    cycle_clock(controller); cycle_clock(controller); cycle_clock(controller);
    controller->sk_strobe = 1; controller->eval();
    cycle_clock(controller); cycle_clock(controller); cycle_clock(controller);
    controller->round_passed = 1; controller->eval();
    cycle_clock(controller);
    controller->sd_is_empty = 1; controller->eval();
    cycle_clock(controller);
    print_header("Round " + std::to_string(i+1) + ": Enter PASS state");
    check_output("PASS", controller->state == PASS, "state == PASS", &passed_subtests, &total_subtests);
    controller->sd_is_empty = 0; controller->eval();
    cycle_clock(controller);
    // if score is already 9, up to student if they want to increment it or not.
    if (i < 9)
      check_output("PASS", controller->score == (i+1), "score == "+std::to_string(i+1), &passed_subtests, &total_subtests);
  }
  print_header("Should be in WIN state");
  check_output("WIN", controller->state == WIN, "state == WIN", &passed_subtests, &total_subtests);
  std::cout << std::endl;
 
  // now to check for WIN state again.  We'll do this by setting the score to 9.
  // we do that by having simon_says = 0 throughout.
  assert_reset(controller);
  check_output("RESET3", controller->state == READY, "state == READY", &passed_subtests, &total_subtests);
  check_output("RESET3", controller->sd_srst == 1, "sd_srst == 1", &passed_subtests, &total_subtests);
  check_output("RESET3", controller->fr_en == 1, "fr_en == 1", &passed_subtests, &total_subtests);
  check_output("RESET3", controller->is_wrong == 0, "is_wrong == 0", &passed_subtests, &total_subtests);
  check_output("RESET3", controller->is_correct == 0, "is_correct == 0", &passed_subtests, &total_subtests);
  check_output("RESET3", controller->mem_en == 1, "mem_en == 1", &passed_subtests, &total_subtests);
  check_output("RESET3", controller->score == 0, "score == 0", &passed_subtests, &total_subtests);  

  controller->clk = 0; 
  controller->simon_says = 0;
  controller->sk_strobe = 0;
  controller->round_passed = 0;
  controller->sd_is_empty = 0;
  controller->eval();
  std::cout << std::endl; print_header("Waiting till WIN state with simon_says = 0");
  for (int i = 0; i < 10; i++) {
    print_header("Round " + std::to_string(i+1) + ": Enter READY state");
    check_output("READY", controller->state == READY, "state == READY", &passed_subtests, &total_subtests);
    cycle_clock(controller); cycle_clock(controller); cycle_clock(controller);
    controller->sd_is_empty = 1; controller->eval();
    cycle_clock(controller);
    print_header("Round " + std::to_string(i+1) + ": Enter PLAY state");
    check_output("PLAY", controller->state == PLAY, "state == PLAY", &passed_subtests, &total_subtests);
    controller->sd_is_empty = 0; controller->eval();
    controller->simon_says = 0; controller->eval();
    cycle_clock(controller); cycle_clock(controller); cycle_clock(controller);
    controller->sk_strobe = 0; controller->eval();
    cycle_clock(controller); cycle_clock(controller); cycle_clock(controller);
    controller->round_passed = 0; controller->eval();
    cycle_clock(controller);
    controller->sd_is_empty = 1; controller->eval();
    cycle_clock(controller);
    print_header("Round " + std::to_string(i+1) + ": Enter PASS state");
    check_output("PASS", controller->state == PASS, "state == PASS", &passed_subtests, &total_subtests);
    controller->sd_is_empty = 0; controller->eval();
    cycle_clock(controller);
    // if score is already 9, up to student if they want to increment it or not.
    if (i < 9)
      check_output("PASS", controller->score == (i+1), "score == "+std::to_string(i+1), &passed_subtests, &total_subtests);
  }
  print_header("Should be in WIN state");
  check_output("WIN", controller->state == WIN, "state == WIN", &passed_subtests, &total_subtests);
  std::cout << std::endl;
 
  update_tests(passed_subtests, total_subtests, "1");
  /***********************************/

  passed_subtests = 0;
  total_subtests = 0;

  /***********************************/
  // END TESTS
  /*************************************************************************/

  // good to have to detect bugs in the testbench
  assert(passed_test_count <= total_test_count);

  if (passed_test_count == total_test_count)
  {
    std::cout << "\n                                                ";
    std::cout << "\n     Good job!⠀⠀⠀⠀⢠⣿⣶⣄⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀   ";
    std::cout << "\n          ⠀⠀⠀⠀⠀⠀⢀⣴⣿⣿⣿⣿⣿⣿⣿⣿⣿⣶⣦⣄⣀⡀⣠⣾⡇⠀⠀⠀⠀   ";
    std::cout << "\n          ⠀⠀⠀⠀⠀⣴⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇⠀⠀⠀⠀   ";
    std::cout << "\n          ⠀⠀⠀⢀⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠿⠿⢿⣿⣿⡇⠀⠀⠀⠀   ";
    std::cout << "\n          ⣶⣿⣦⣜⣿⣿⣿⡟⠻⣿⣿⣿⣿⣿⣿⣿⡿⢿⡏⣴⣺⣦⣙⣿⣷⣄⠀⠀⠀   ";
    std::cout << "\n          ⣯⡇⣻⣿⣿⣿⣿⣷⣾⣿⣬⣥⣭⣽⣿⣿⣧⣼⡇⣯⣇⣹⣿⣿⣿⣿⣧⠀⠀   ";
    std::cout << "\n          ⠹⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠸⣿⣿⣿⣿⣿⣿⣿⣷⠀   ";
    std::cout << "\n                                                ";
    std::cout << "\n        ⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛\n";
    std::cout << "ALL " << std::to_string(total_test_count) << " TESTS PASSED"
              << "\n";
  }
  else
  {
    std::cout << "ERROR: " << std::to_string(passed_test_count) << "/" << std::to_string(total_test_count) << " tests passed.\n";
  }

  // Final model cleanups
  controller->final();

  // Destroy models
  delete controller;
  controller = NULL;

  // Fin
  return passed_test_count == total_test_count ? 0 : 1;
}
