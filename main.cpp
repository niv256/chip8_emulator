#include <iostream>
#include <vector>
#include <fstream>
#include <zconf.h>
#include "chip8.h"

using namespace std;

int main() {
    char single_byte1, single_byte2;
    vector<uint8_t> program;

    // open file
    ifstream buffer{"../roms/HEART_MONITOR.ch8"};
    if (!buffer) {
        cerr << "error opening file" << endl;
        return 1;
    }

    // initialize cpu and memory
    init_cpu();

    // extract rom from file
    while (!buffer.eof()) {
        buffer.get(single_byte1);
        buffer.get(single_byte2);
        if ((((uint8_t) single_byte1 << 8) | single_byte2) == 0xEEEE) {
            break;
        }
        program.push_back((uint8_t) single_byte1);
        program.push_back((uint8_t) single_byte2);
    }

    // load the program into memory
    load_program(program);

    // main execution loop
    for(;;) {
        step();
        draw_display();
        sleep(0.25);
    }
    return 0;
}