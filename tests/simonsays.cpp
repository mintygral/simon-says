// DESCRIPTION: Verilator: Verilog example module
//
// This file ONLY is placed under the Creative Commons Public Domain, for
// any use, without warranty, 2017 by Wilson Snyder.
// SPDX-License-Identifier: CC0-1.0
//======================================================================
// Include common routines
#include <verilated.h>
#include "verilated_vcd_c.h"

static int passed_test_count = 0; // every time we perform a test, and the test passes, increment this by one.
static int total_test_count = 0;  // every time we perform a test, increment this by one.
static const std::unique_ptr<VerilatedContext> contextp{new VerilatedContext};
static VerilatedVcdC* tfp;
static int hexmem[256];

// Include model header, generated from Verilating "tb_top.v"
#include "Vsimonsays.h"

// Verilator using new C++?  Need to include these now (sstep2021)
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>

// yeah of course I steal things from stackoverflow! pobody's nerfect!
// http://stackoverflow.com/questions/43432014/ddg#43432296
//////////////
static std::random_device rd;
static std::mt19937 gen(rd());
int randomgen()
{
  std::uniform_int_distribution<> dis(4096,32000);
  int rndnum = dis(gen);
  return rndnum;
}
//////////////

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

// turn a three-digit hex number into a string
std::string hexstr(int n) {
  std::stringstream ss;
  ss << std::hex << n;
  std::string s = ss.str();
  if (s.length() == 1) {
    s = "00" + s;
  } else if (s.length() == 2) {
    s = "0" + s;
  }
  std::transform(s.begin(), s.end(), s.begin(), ::toupper);
  return s;
}

int hexchar(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  } else if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  } else {
    return -1;
  }
}

void update_tests(int passed, int total, std::string test) {
  // add tests to global test variables
  passed_test_count += passed;
  total_test_count += total;
  // perform sanity check on global test variables
  assert(passed_test_count <= total_test_count);
  // let TA know if this set of tests failed
  std::cout << "simonsays: " << std::to_string(passed) << " out of " << std::to_string(total) << " tests passed.\n";
}

void topeval(Vsimonsays* simonsays) {
  contextp->timeInc(1);
  simonsays->eval();
  tfp->dump(contextp->time());
  tfp->flush();
  // std::cout << "time: " << std::to_string(contextp->time()) << "\n";
}

void cycle_clock(Vsimonsays* simonsays, int n) {
  while (n--) {
    simonsays->hz100 = 1; topeval(simonsays);
    simonsays->hz100 = 0; topeval(simonsays);
  }
}
void assert_reset(Vsimonsays* simonsays) {
    simonsays->reset = 1; topeval(simonsays);
    simonsays->reset = 0; topeval(simonsays);
}

int stoa (Vsimonsays* simonsays, std::string arg) {
    if (arg == "hz100") return simonsays->hz100;
    else if (arg == "reset") return simonsays->reset;
    else if (arg == "pb") return simonsays->pb;
    else if (arg == "{left, right}") return ((simonsays->left << 8) | simonsays->right);
    else if (arg == "left") return simonsays->left;
    else if (arg == "right") return simonsays->right;
    else if (arg == "ss7") return simonsays->ss7;
    else if (arg == "ss6") return simonsays->ss6;
    else if (arg == "ss5") return simonsays->ss5;
    else if (arg == "ss4") return simonsays->ss4;
    else if (arg == "ss3") return simonsays->ss3;
    else if (arg == "ss2") return simonsays->ss2;
    else if (arg == "ss1") return simonsays->ss1;
    else if (arg == "ss0") return simonsays->ss0;
    else if (arg == "red") return simonsays->red;
    else if (arg == "green") return simonsays->green;
    else if (arg == "blue") return simonsays->blue;
    else assert(false);
}

int charss(char c) {
  switch (c) {
    // seven segment codes for each of the characters
    case '0': return 0b00111111;
    case '1': return 0b00000110;
    case '2': return 0b01011011;
    case '3': return 0b01001111;
    case '4': return 0b01100110;
    case '5': return 0b01101101;
    case '6': return 0b01111101;
    case '7': return 0b00000111;
    case '8': return 0b01111111;
    case '9': return 0b01100111;
    case 'A': return 0b01110111;
    case 'B': return 0b01111100;
    case 'C': return 0b00111001;
    case 'D': return 0b01011110;
    case 'E': return 0b01111001;
    case 'F': return 0b01110001;
    case 'G': return 0b01101111;
    case 'H': return 0b01110110;
    case 'I': return 0b00010000;
    case 'J': return 0b00011110;
    case 'L': return 0b00111000;
    case 'N': return 0b01010100;
    case 'R': return 0b01010000;
    case 'T': return 0b01111000;
    case 'Y': return 0b01101110;
    case '?': return 0b01010011;
    case ' ': return 0b00000000;
    default: assert(false);
  }
}

char ssrepr(int c) {
  switch (c) {
    // seven segment codes for each of the characters
    case 0b00111111: return '0';
    case 0b00000110: return '1';
    case 0b01011011: return '2';
    case 0b01001111: return '3';
    case 0b01100110: return '4';
    case 0b01101101: return '5';
    case 0b01111101: return '6';
    case 0b00000111: return '7';
    case 0b01111111: return '8';
    case 0b01100111: return '9';
    case 0b01110111: return 'A';
    case 0b01111100: return 'B';
    case 0b00111001: return 'C';
    case 0b01011110: return 'D';
    case 0b01111001: return 'E';
    case 0b01110001: return 'F';
    case 0b01101111: return 'G';
    case 0b01110110: return 'H';
    case 0b00010000: return 'I';
    case 0b00011110: return 'J';
    case 0b00111000: return 'L';
    case 0b01010100: return 'N';
    case 0b01010000: return 'R';
    case 0b01111000: return 'T';
    case 0b01101110: return 'Y';
    case 0b01010011: return '?';
    case 0b00000000: return ' ';
    default: return 'x';
  }
}

void check_display(std::string state, Vsimonsays* simonsays, std::string sentence, int* passed, int* total) {
  assert(sentence.size() == 8);
  bool failed = false;
  failed = failed || ((simonsays->ss7 & 0x7F) != charss(sentence[0]));
  failed = failed || ((simonsays->ss6 & 0x7F) != charss(sentence[1]));
  failed = failed || ((simonsays->ss5 & 0x7F) != charss(sentence[2]));
  failed = failed || ((simonsays->ss4 & 0x7F) != charss(sentence[3]));
  failed = failed || ((simonsays->ss3 & 0x7F) != charss(sentence[4]));
  failed = failed || ((simonsays->ss2 & 0x7F) != charss(sentence[5]));
  failed = failed || ((simonsays->ss1 & 0x7F) != charss(sentence[6]));
  failed = failed || ((simonsays->ss0 & 0x7F) != charss(sentence[7]));
  if (failed) {
    std::cout << "\033[0;31m";
    std::cout<<"[FAIL] ";
    std::cout << "\033[0m";
    std::cout << "In " << state << " state: simonsays->ss7..ss0 == '";
    for (int i = 0; i < 8; i++) {
      std::cout << ssrepr(charss(sentence[i]));
    }
    if (failed) {
      std::cout << "', but got '";
      std::cout << ssrepr(simonsays->ss7 & 0x7F);
      std::cout << ssrepr(simonsays->ss6 & 0x7F);
      std::cout << ssrepr(simonsays->ss5 & 0x7F);
      std::cout << ssrepr(simonsays->ss4 & 0x7F);
      std::cout << ssrepr(simonsays->ss3 & 0x7F);
      std::cout << ssrepr(simonsays->ss2 & 0x7F);
      std::cout << ssrepr(simonsays->ss1 & 0x7F);
      std::cout << ssrepr(simonsays->ss0 & 0x7F);
      std::cout << "'\n";
    }
    else {
      std::cout << "'\n";
    }
  } else {
    (*passed)++;
    // std::cout << "\033[0;32m";
    // std::cout<<"[PASS] ";
  }
  (*total)++;
}

void check_output(std::string state, Vsimonsays* simonsays, std::string signal, int exp, int* passed, int* total) {
  /* 1	Blue	9	Light Blue
   2	Green	0	Black
   3	Aqua	10	Light Green
   4	Red	11	Light Aqua
   5	Purple	12	Light Red
   7	White	14	Light Yellow
   8	Gray	15	Bright White */
  if (stoa(simonsays, signal) == exp) {
    (*passed)++;
    // std::cout << "\033[0;32m";
    // std::cout<<"[PASS] ";
  } else {
    std::cout << "\033[0;31m";
    std::cout<<"[FAIL] ";
    std::cout << "\033[0m";
    std::cout << "In " << state << " state: Test " << "simonsays->" << signal << " == 0x" << std::hex << exp;
    if (stoa(simonsays, signal) != exp) {
      std::cout << ", but got 0x" << std::hex << stoa(simonsays, signal);
    }
    std::cout << std::endl;
  }
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
  std::cout << std::endl << dashes << " " << s << " " << dashes << "\n";
}

// write a C++ function that takes Vsimonsays* simonsays, and for each of the ss2..ss0 signals,
// calls the ssrepr function to print out the character that is being displayed on each 7-segment display
// and returns it as a 3-digit hexstring.
std::string ssbtns(Vsimonsays* simonsays) {
  std::string s = "";
  std::cout << bin(simonsays->ss2 & 0x7F) << std::endl;
  s += ssrepr(simonsays->ss2 & 0x7F);
  std::cout << bin(simonsays->ss1 & 0x7F) << std::endl;
  s += ssrepr(simonsays->ss1 & 0x7F);
  std::cout << bin(simonsays->ss0 & 0x7F) << std::endl;
  s += ssrepr(simonsays->ss0 & 0x7F);
  return s;
}

void derive_buttons(Vsimonsays* simonsays, int* simon_says, int* mem_i) {
  char btn2 = ssrepr(simonsays->ss2 & 0x7F);
  char btn1 = ssrepr(simonsays->ss1 & 0x7F);
  char btn0 = ssrepr(simonsays->ss0 & 0x7F);
  if (btn2 == 'x' || btn1 == 'x' || btn0 == 'x') {
    std::cout << "================================================================================\n";
    std::cout << "                                FATAL ERROR.                                    \n";
    std::cout << "     The testbench was unable to determine what set of buttons was displayed    \n";
    std::cout << "      on ss2-ss0 in the PLAY state.  We do this to permit more flexibility      \n";
    std::cout << "     in grading, so long as the buttons being chosen are from the press.mem     \n";
    std::cout << "      file generated by this testbench.  Please ensure that ssdec_ext is        \n";
    std::cout << "        generating the right seven-segment codes for 0-F, and that the          \n";
    std::cout << "        buttons are correctly appearing in the PLAY state as intended.          \n";
    std::cout << "                                                                                \n";
    std::cout << "            The output on the seven-segment displays was: ";
    std::cout << ssrepr(simonsays->ss7 & 0x7F);
    std::cout << ssrepr(simonsays->ss6 & 0x7F);
    std::cout << ssrepr(simonsays->ss5 & 0x7F);
    std::cout << ssrepr(simonsays->ss4 & 0x7F);
    std::cout << ssrepr(simonsays->ss3 & 0x7F);
    std::cout << ssrepr(simonsays->ss2 & 0x7F);
    std::cout << ssrepr(simonsays->ss1 & 0x7F);
    std::cout << ssrepr(simonsays->ss0 & 0x7F) << std::endl;
    std::cout << "================================================================================\n";
    exit(1);
  }
  int h = hexchar(btn2);
  int g = hexchar(btn1);
  int f = hexchar(btn0);
  int btn = (h << 8) | (g << 4) | f;
  int t;
  for (t = 0; t < 256; t++) {
    if (hexmem[t] == btn)
      break;
  }
  if (t == 256) {
    std::cout << "================================================================================\n";
    std::cout << "                                FATAL ERROR.                                    \n";
    std::cout << "     The testbench was unable to map the buttons displayed to the ones given    \n";
    std::cout << "     in the press.mem file generated by the testbench.  You should not have     \n";
    std::cout << "     to provide your own, and you must not be hardcoding the generated values   \n";
    std::cout << "     you got from the project step 1 page while writing the mem module.         \n";
    std::cout << "     Ensure that your mem module is looking for a press.mem file with the       \n";
    std::cout << "     $readmemh function as described in step 1, and that it is correctly        \n";
    std::cout << "     assigning values from the file to the mem output.                          \n";
    std::cout << "                                                                                \n";
    std::cout << "            The output on the seven-segment displays was: ";
    std::cout << ssrepr(simonsays->ss7 & 0x7F);
    std::cout << ssrepr(simonsays->ss6 & 0x7F);
    std::cout << ssrepr(simonsays->ss5 & 0x7F);
    std::cout << ssrepr(simonsays->ss4 & 0x7F);
    std::cout << ssrepr(simonsays->ss3 & 0x7F);
    std::cout << ssrepr(simonsays->ss2 & 0x7F);
    std::cout << ssrepr(simonsays->ss1 & 0x7F);
    std::cout << ssrepr(simonsays->ss0 & 0x7F) << std::endl;
    std::cout << "================================================================================\n";
    exit(1);
  }
  else {
    (*mem_i) = t;
  }
  (*simon_says) = 0;
  for (int i = 0; i < 12; i++) {
    (*simon_says) ^= (hexmem[(*mem_i)] >> i) & 1;
  }
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
  tfp = new VerilatedVcdC;

  // Pass arguments so Verilated code can see them, e.g. $value$plusargs
  // This needs to be called before you create any model
  Verilated::commandArgs(argc, argv);

  // if file already exists, open and read the file into hexmem instead
  std::ifstream pressmem_in("press.mem");
  if (pressmem_in.is_open()) {
    std::cout << "press.mem file found.  Reading in values from file.\n";
    for (int i = 0; i < 256; i++) {
      pressmem_in >> std::hex >> hexmem[i];
    }
    pressmem_in.close();
  }
  else {
    std::cout << "press.mem file not found.  Generating values and saving to file.\n";
    // it should be the same one used by mem.sv, so we provide it before we create the model
    // create a file output stream named press.mem
    std::ofstream pressmem("press.mem");
    // to this file stream, write 256 lines containing three random hex digits and a newline
    for (int i = 0; i < 256; i++) {
      hexmem[i] = randomgen() % 4096;
      pressmem << hexstr(hexmem[i]) << std::endl;
    }
    // close the file stream
    pressmem.close();
  }

  // Construct the Verilated model, from each module after Verilating each module file
  Vsimonsays *simonsays = new Vsimonsays; // Or use a const unique_ptr, or the VL_UNIQUE_PTR wrapper

  // enable tracing
  // simonsays->trace(tfp, 2);  
  // tfp->open("simonsays.vcd");

  int passed_subtests = 0; // if we have multiple subtests, we need to know how many passed for each
  int total_subtests = 0;

  /*************************************************************************/
  // BEGIN TESTS
  /***********************************/
  // simonsays
  int READY = 0;
  int PLAY = 1;
  int FAIL = 2;
  int PASS = 3;
  int WIN = 4;

  // port headers
  /*
  input hz100, reset, 
  input [19:0] pb,
  output [7:0] left, right, 
  output [7:0] ss7, ss6, ss5, ss4, ss3, ss2, ss1, ss0,
  output red, green, blue
  */

  // hold down reset.  We must see "READY 0" on the seven-segment displays (ss7-ss0), the left/right LEDs must all be lit, and the red/green/blue LEDs must all be off.
  print_header("Power-on: testing reset");
  simonsays->reset = 0; cycle_clock(simonsays, 1); 
  simonsays->hz100 = 0;
  simonsays->pb = 0;
  simonsays->reset = 1; cycle_clock(simonsays, 1); 
  // must respect reset=1, regardless of clocking
  cycle_clock(simonsays, 100);
  check_output("RESET", simonsays, "red", 0, &passed_subtests, &total_subtests);
  check_output("RESET", simonsays, "blue", 0, &passed_subtests, &total_subtests);
  check_output("RESET", simonsays, "green", 0, &passed_subtests, &total_subtests);
  check_output("RESET", simonsays, "left", 0xFF, &passed_subtests, &total_subtests);
  check_output("RESET", simonsays, "right", 0xFF, &passed_subtests, &total_subtests);
  check_display("RESET", simonsays, "READY? 0", &passed_subtests, &total_subtests);

  // release reset.
  print_header("Deasserting reset");
  simonsays->reset = 0; topeval(simonsays);

  // determine shiftdown speed and throw error if not CLKDIV_LIM
  print_header("Checking shiftdown timing");
  int CLKDIV_LIM = 6;
  int div = 0;
  for (div = 0; div < 100; div++) {
    cycle_clock(simonsays, 1);
    if (simonsays->left != 0xFF)
      break;
  }
  /*
  std::cout << "clkdiv went high after " << std::to_string(div) << " cycles, ";
  std::cout << "expected " << std::to_string(CLKDIV_LIM) << " cycles." << std::endl;
  if (div != CLKDIV_LIM) {
    std::cout << "================================================================================\n";
    std::cout << "                                FATAL ERROR.                                    \n";
    std::cout << "       It appears that you have not set the CLKDIV_LIM parameter to 6.          \n";
    std::cout << "                  Please do so, and rerun the make command.                     \n";
    std::cout << "         While testing manually, you should be changing CLKDIV_LIM              \n";
    std::cout << "          via its instantiation in the top module, not the default             \n";
    std::cout << "                    value within the simonsays port header.                    \n";
    std::cout << "================================================================================\n";
    return 1;
  }
  else {
  */
    check_output("RESET", simonsays, "{left, right}", 0x7FFF, &passed_subtests, &total_subtests);
  //}


  int mem_i = 0;
  print_header("Part 1: Win all 10 rounds");
  for (int round = 0; round < 10; round++) {
    int buffer = 0x7FFF;
    
    print_header("Round " + std::to_string(round));
    for (int i = 0; i < 16; i++) {
      check_output("READY", simonsays, "red", 0, &passed_subtests, &total_subtests);
      check_output("READY", simonsays, "{left, right}", buffer, &passed_subtests, &total_subtests);
      if (i < 15) {
        check_output("READY", simonsays, "blue", 0, &passed_subtests, &total_subtests);
        check_output("READY", simonsays, "green", (round == 0) ? 0 : 1, &passed_subtests, &total_subtests);
        check_display("READY", simonsays, "READY? " + std::to_string(round), &passed_subtests, &total_subtests);
      }
      buffer >>= 1;
      cycle_clock(simonsays, (CLKDIV_LIM+1)*2);
    }

    // since it is way too difficult to track the clocking for determining buttons, and since students will probably 
    // have all sorts of different timing characteristics, this testbench will not check if the word 
    // is correctly chosen.  If the buttons displayed exist in the press.mem file provided to the 
    // testbench, that's good enough.
    int simon_says = 0;
    derive_buttons(simonsays, &simon_says, &mem_i);
    print_header("Buttons: " + hexstr(hexmem[mem_i]));
    print_header("Simon Says? " + std::string((simon_says == 1) ? "Yes": "No"));
    
    check_display("PLAY", simonsays, "H0LD " + hexstr(hexmem[mem_i]), &passed_subtests, &total_subtests);
    // create simon_says int with the XOR of all bits of hexmem[mem_i]
    // set simonsays->pb to the correct button presses

    buffer = 0xFFFF;
    int rnd = (randomgen() % 12) + 2;
    for (int i = 0; i < 16; i++) {
      // std::cout << "Iteration " << i << std::endl;
      check_output("PLAY", simonsays, "red", 0, &passed_subtests, &total_subtests);
      check_output("PLAY", simonsays, "blue", simon_says, &passed_subtests, &total_subtests);
      check_output("PLAY", simonsays, "green", 0, &passed_subtests, &total_subtests);
      // randomly press the right set of buttons
      // but do it early enough so that scankey has time 
      // to register the button press
      if (simon_says == 1 && i == ((rnd % 12) + 2) && simonsays->pb == 0) {
          std::cout << "Pressed " + hexstr(hexmem[mem_i]) + " at loop iteration " << std::to_string(i) << std::endl;
          simonsays->pb  = (1 << (hexmem[mem_i] & 0xF));
          simonsays->pb |= (1 << ((hexmem[mem_i] >> 4) & 0xF));
          simonsays->pb |= (1 << ((hexmem[mem_i] >> 8) & 0xF));
          // cycle_clock(simonsays, (CLKDIV_LIM+1)*2);
          // buffer >>= 1;
          // i++;
          // std::cout << "pb: " << bin(simonsays->pb) << std::endl;
          // then, two clock cycles later, strobe goes high, and fr_en is reenabled
      }
      check_output("PLAY", simonsays, "{left, right}", buffer, &passed_subtests, &total_subtests);
      check_display("PLAY", simonsays, "H0LD " + hexstr(hexmem[mem_i]), &passed_subtests, &total_subtests);
      buffer >>= 1;
      cycle_clock(simonsays, (CLKDIV_LIM+1)*2);
    }

    cycle_clock(simonsays, (CLKDIV_LIM+1)*2);
    cycle_clock(simonsays, 1);
    if (round == 9) {
      check_output("WIN", simonsays, "red", 0, &passed_subtests, &total_subtests);
      check_output("WIN", simonsays, "blue", 0, &passed_subtests, &total_subtests);
      check_output("WIN", simonsays, "green", 1, &passed_subtests, &total_subtests);
      check_display("WIN", simonsays, "G00D J0B", &passed_subtests, &total_subtests);
    }
    // check that we have passed the round
    else {
      check_output("PASS", simonsays, "red", 0, &passed_subtests, &total_subtests);
      check_output("PASS", simonsays, "blue", 0, &passed_subtests, &total_subtests);
      check_output("PASS", simonsays, "green", 1, &passed_subtests, &total_subtests);
      check_display("PASS", simonsays, "READY? " + std::to_string(round+1), &passed_subtests, &total_subtests);
    }

    // release all buttons
    simonsays->pb = 0;
    simonsays->eval();
    cycle_clock(simonsays, (CLKDIV_LIM+1)*2);
  }

  print_header("Post-game: async 3-0-W reset");
  simonsays->reset = 0; cycle_clock(simonsays, 1); 
  simonsays->hz100 = 0;
  simonsays->pb = 0;
  // must respect reset=1, regardless of clocking
  simonsays->reset = 1; cycle_clock(simonsays, 100); 
  check_output("RESET", simonsays, "red", 0, &passed_subtests, &total_subtests);
  check_output("RESET", simonsays, "blue", 0, &passed_subtests, &total_subtests);
  check_output("RESET", simonsays, "green", 0, &passed_subtests, &total_subtests);
  check_output("RESET", simonsays, "left", 0xFF, &passed_subtests, &total_subtests);
  check_output("RESET", simonsays, "right", 0xFF, &passed_subtests, &total_subtests);
  check_display("RESET", simonsays, "READY? 0", &passed_subtests, &total_subtests);

  // now intentionally do the wrong thing...
  print_header("Part 2: Press no/wrong buttons (pathway 1)");
  // ignore all display checking until PLAY, seeing as it's already been checked earlier
  simonsays->reset = 0; simonsays->eval(); cycle_clock(simonsays, 16*(CLKDIV_LIM+1)*2); 
  // we need to test both kinds of failures - not pressing the buttons in time, and pressing buttons 
  // when simon_says = 0.  
  int fin_test_type = 0;
  while (fin_test_type != 3) {
    int simon_says = 0;
    simonsays->eval();
    derive_buttons(simonsays, &simon_says, &mem_i);
    cycle_clock(simonsays, (CLKDIV_LIM+1)*2);
    // we should now be in PLAY, and the display should be "HOLD DDD", so redetermine the buttons
    check_display("PLAY", simonsays, "H0LD " + hexstr(hexmem[mem_i]), &passed_subtests, &total_subtests);
    int buffer = 0xFFFF;
    // until we have tested both types of fails, keep going
    int rnd = (randomgen() % 12) + 2;
    // enter the PLAY round and either press/do not press the buttons to go to FAIL
    for (int i = 0; i < 16; i++) {
      check_output("PLAY", simonsays, "green", 0, &passed_subtests, &total_subtests);
      check_output("PLAY", simonsays, "red", 0, &passed_subtests, &total_subtests);
      check_output("PLAY", simonsays, "blue", simon_says, &passed_subtests, &total_subtests);
      // press the right set of buttons if we already finished fin_test_type = 2 (so we can skip this set of buttons)
      if (((fin_test_type == 1) || (fin_test_type == 2)) && simon_says == 1) {
        print_header("Simon says is true, and pressing buttons");
        if (i == ((rnd % 12) + 2) && simonsays->pb == 0) {
          std::cout << "Pressed " + hexstr(hexmem[mem_i]) + " at loop iteration " << std::to_string(i) << std::endl;
          simonsays->pb  = (1 << (hexmem[mem_i] & 0xF));
          simonsays->eval();
          simonsays->pb |= (1 << ((hexmem[mem_i] >> 4) & 0xF));
          simonsays->eval();
          simonsays->pb |= (1 << ((hexmem[mem_i] >> 8) & 0xF));
          simonsays->eval();
        }
      }
      else if ((fin_test_type == 2 && simon_says == 0) || (fin_test_type == 0 && simon_says == 0)) {
        std::cout << "Pressed " << hexstr(hexmem[mem_i]) << " at loop iteration " << std::to_string(i) << std::endl;
        simonsays->pb  = (1 << (hexmem[mem_i] & 0xF));
        simonsays->eval();
        simonsays->pb |= (1 << ((hexmem[mem_i] >> 4) & 0xF));
        simonsays->eval();
        simonsays->pb |= (1 << ((hexmem[mem_i] >> 8) & 0xF));
        simonsays->eval();
        cycle_clock(simonsays, (CLKDIV_LIM+1)*2);
        // we should see TRYAGAIN
        check_output("FAIL", simonsays, "red", 1, &passed_subtests, &total_subtests);
        check_output("FAIL", simonsays, "blue", 0, &passed_subtests, &total_subtests);
        check_output("FAIL", simonsays, "green", 0, &passed_subtests, &total_subtests);
        check_display("FAIL", simonsays, "TRYAGAIN", &passed_subtests, &total_subtests);
        fin_test_type = (fin_test_type == 0) ? 1 : 3;
        // need to clear strobe!
        simonsays->pb = 0;
        simonsays->eval();
        cycle_clock(simonsays, (CLKDIV_LIM+1)*2);
        break;
      }
      // else {
      //   std::cout << "Will not press " << hexstr(hexmem[mem_i]) << " at loop iteration " << std::to_string(i) << std::endl;
      // }
      check_output("PLAY", simonsays, "{left, right}", buffer, &passed_subtests, &total_subtests);
      check_display("PLAY", simonsays, "H0LD " + hexstr(hexmem[mem_i]), &passed_subtests, &total_subtests);
      buffer >>= 1;
      cycle_clock(simonsays, (CLKDIV_LIM+1)*2);
    }

    // print the value of fin_test_type
    // std::cout << "fin_test_type = " << std::to_string(fin_test_type) << std::endl;
    // std::cout << "simon_says = " << std::to_string(simon_says) << std::endl;
    // std::cout << "red = " << std::to_string(simonsays->red) << std::endl;

    if (fin_test_type <= 1) {
      // if we're in 0? 
      cycle_clock(simonsays, (CLKDIV_LIM+1)*2);
      check_output("FAIL", simonsays, "red", 1, &passed_subtests, &total_subtests);
      check_output("FAIL", simonsays, "blue", 0, &passed_subtests, &total_subtests);
      check_output("FAIL", simonsays, "green", 0, &passed_subtests, &total_subtests);
      check_display("FAIL", simonsays, "TRYAGAIN", &passed_subtests, &total_subtests);
      // if we did the "not pressing buttons" test, we need to do the "pressing buttons" test
      // or vice-versa
      print_header("Resetting design after no-button-press test");
      fin_test_type = 2;
      simonsays->reset = 1; simonsays->eval(); cycle_clock(simonsays, 10); 
      simonsays->reset = 0; simonsays->eval(); cycle_clock(simonsays, 16*(CLKDIV_LIM+1)*2); 
    }
    else {
      simonsays->pb = 0;
      simonsays->eval();
      cycle_clock(simonsays, (CLKDIV_LIM+1)*2);
      cycle_clock(simonsays, 16*(CLKDIV_LIM+1)*2);
    }
  }

  update_tests(passed_subtests, total_subtests, "1");
  /***********************************/

  passed_subtests = 0;
  total_subtests = 0;

  /***********************************/
  // END TESTS
  /*************************************************************************/

  // remove the press.mem file
  // remove("press.mem");

  // close trace file
  tfp->close();

  // good to have to detect bugs in the testbench
  assert(passed_test_count <= total_test_count);

  if (passed_test_count == total_test_count)
  {
    std::cout << " _________________________________________\n";
    std::cout << "/ Congratulations! Make sure to demo your \\\n";
    std::cout << "\\ project before Dec 2!                   /\n";
    std::cout << " -----------------------------------------\n";
    std::cout << "      \\                    / \\  //\\\n";
    std::cout << "       \\    |\\___/|      /   \\//  \\\\\n";
    std::cout << "            /0  0  \\__  /    //  | \\ \\    \n";
    std::cout << "           /     /  \\/_/    //   |  \\  \\  \n";
    std::cout << "           @_^_@'/   \\/_   //    |   \\   \\ \n";
    std::cout << "           //_^_/     \\/_ //     |    \\    \\\n";
    std::cout << "        ( //) |        \\///      |     \\     \\\n";
    std::cout << "      ( / /) _|_ /   )  //       |      \\     _\\\n";
    std::cout << "    ( // /) '/,_ _ _/  ( ; -.    |    _ _\\.-~        .-~~~^-.\n";
    std::cout << "  (( / / )) ,-{        _      `-.|.-~-.           .~         `.\n";
    std::cout << " (( // / ))  '/\\      /                 ~-. _ .-~      .-~^-.  \\\n";
    std::cout << " (( /// ))      `.   {            }                   /      \\  \\\n";
    std::cout << "  (( / ))     .----~-.\\        \\-'                 .~         \\  `. \\^-.\n";
    std::cout << "             ///.----..>        \\             _ -~             `.  ^-`  ^-_\n";
    std::cout << "               ///-._ _ _ _ _ _ _}^ - - - - ~                     ~-- ,.-~\n";
    std::cout << "                                                                  /.-~\n";
    std::cout << "ALL " << std::to_string(total_test_count) << " TESTS PASSED"
              << "\n";
  }
  else
  {
    std::cout << "\nYou can find FAILed tests by pressing Ctrl-Shift-F in your terminal, or Ctrl-F after clicking ";
    std::cout << "inside the Kate Build Output box, and searching for FAIL.  (You won't get color output in the latter.)\n\n";
    std::cout << "ERROR: " << std::to_string(passed_test_count) << "/" << std::to_string(total_test_count) << " tests passed.\n";
  }

  // Final model cleanups
  simonsays->final();

  // Destroy models
  delete simonsays;
  simonsays = NULL;

  // Fin
  return passed_test_count == total_test_count ? 0 : 1;
}
