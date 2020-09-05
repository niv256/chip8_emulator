//
// Created by niv on 1/6/20.
//

#ifndef CHIP8_EMULATOR_CHIP8_H
#define CHIP8_EMULATOR_CHIP8_H

#include <cstdint>

void init_cpu(void);
void draw_display(void);
void load_program(std::vector<uint8_t> byte_array);
void step(void);

#endif //CHIP8_EMULATOR_CHIP8_H
