#include <cstdlib>
#include <ctime>
#include <iostream>
#include <zconf.h>
#include <vector>
#include <cstring>
#include "chip8.h"

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32

#define X ((opcode & 0x0F00) >> 8)
#define Y ((opcode & 0x00F0) >> 4)
#define LAST_TWO_DIGITS (opcode & 0x00FF)
#define LAST_DIGIT (opcode & 0x000F)
#define NNN (opcode & 0x0FFF)

using namespace std;

void exec_opcode(uint16_t opcode);
void clearScreen(void);
void setup_font(void);

struct {
    uint8_t V[16];
    uint16_t I, PC;
    uint16_t stack[24];
    uint16_t SP;
    uint8_t delay_timer, sound_timer;

} cpu_state;

uint8_t memory[0xFFF + 1];
uint8_t display[DISPLAY_WIDTH*DISPLAY_HEIGHT];

void init_cpu(void){
    for (int i = 0; i < 16; i++){
        cpu_state.V[i] = 0;
    }
    cpu_state.I = 0;
    cpu_state.PC = 0x200;
    cpu_state.SP = 0;
    cpu_state.delay_timer = 0;
    cpu_state.sound_timer = 0;

    // initialize font in memory
    setup_font();

    srand(time(NULL)); // set seed for rand()
}

// initialize font in memory
void setup_font(void) {
    // static array of hex values for digits
    std::vector<uint8_t> font {0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20,
                               0x20, 0x70, 0xF0, 0x10, 0xF0, 0x80, 0xF0, 0xF0,
                               0x10, 0xF0, 0x10, 0xF0, 0x90, 0x90, 0xF0, 0x10,
                               0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0, 0xF0, 0x80,
                               0xF0, 0x90, 0xF0, 0xF0, 0x10, 0x20, 0x40, 0x40,
                               0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0,
                               0x10, 0xF0, 0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0,
                               0x90, 0xE0, 0x90, 0xE0, 0xF0, 0x80, 0x80, 0x80,
                               0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0, 0xF0, 0x80,
                               0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80};

    // put in memory
    for(int i = 0; i < font.size(); i++) {
        memory[i] = font[i];
    }
}

// execute next instruction in PC
void step(void) {
    uint16_t opcode = ((uint16_t) memory[cpu_state.PC] << 8) | memory[cpu_state.PC + 1];

    // advance PC for next instruction
    cpu_state.PC += 2;

    // execute the opcode
    exec_opcode(opcode);
}

// get 2-byte hex opcode and execute instruction
void exec_opcode(uint16_t opcode){
    uint16_t first_digit = opcode & 0xF000;
    uint16_t res; // to store result - overflow checking
    uint16_t i, j; // iterators

    // giant switch branching for opcode execution
    switch (first_digit){
        case 0x0000: // screen stuff and return from call
            if ((opcode & 0x00F0) == 0x00C0){ // 00CN - scdown N
                //TODO: scroll the screen down LAST_DIGIT times.
                std::cerr << "NOT SUPPORTED YET " << opcode << std::endl;
                break;
            }

            switch (opcode){
                case 0x00E0: // cls
                    for (i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
                        display[i] = 0;
                    }
                    break;

                case 0x00EE: // rts
                    cpu_state.PC = cpu_state.stack[cpu_state.SP];
                    cpu_state.SP--;
                    break;

                case 0x00FB: // scright
                    //TODO: scroll screen 4 pixel right.
                    std::cerr << "NOT SUPPORTED YET " << opcode << std::endl;
                    break;

                case 0x00FC:// scleft
                    //TODO: scroll screen 4 pixel left.
                    std::cerr << "NOT SUPPORTED YET " << opcode << std::endl;
                    break;

                case 0x00FE:// low
                    //TODO: disable extended screen mode.
                    std::cerr << "NOT SUPPORTED YET " << opcode << std::endl;
                    break;

                case 0x00FF:// high
                    //TODO: enable extended screen mode (128 X 64).
                    std::cerr << "NOT SUPPORTED YET " << opcode << std::endl;
                    break;
            }
            break;

        case 0x1000: // 1NNN - jmp nnn
            cpu_state.PC = NNN;
            break;

        case 0x2000: // 2NNN - jsr nnn
            cpu_state.SP++;
            cpu_state.stack[cpu_state.SP] = cpu_state.PC;
            cpu_state.PC = NNN;
            break;

        case 0x3000: // 3XRR - skeq vx, rr
            if(cpu_state.V[X] == LAST_TWO_DIGITS){
                cpu_state.PC += 2;
            }
            break;

        case 0x4000: // 4XRR - skne vx, rr
            if(cpu_state.V[X] != LAST_TWO_DIGITS){
                cpu_state.PC += 2;
            }
            break;

        case 0x5000: // 5XY0 - skeq vx, vy
            if(cpu_state.V[X] == cpu_state.V[Y]){
                cpu_state.PC += 2;
            }
            break;

        case 0x6000: // 6XRR - mov vx, rr
            cpu_state.V[X] = LAST_TWO_DIGITS;
            break;

        case 0x7000: // 7XRR - add vx, rr
            cpu_state.V[X] += LAST_TWO_DIGITS;
            break;

         case 0x8000: // 8XYN - register arithmetic
            switch (LAST_DIGIT) {
                case 0x0000: // 8XY0 - mov vx, vy
                    cpu_state.V[X] = cpu_state.V[Y];
                    break;

                case 0x0001: // 8XY1 - or vx, vy
                    cpu_state.V[X] |= cpu_state.V[Y];
                    break;

                case 0x0002: // 8XY2 - and vx, vy
                    cpu_state.V[X] &= cpu_state.V[Y];
                    break;

                case 0x0003: // 8XY3 - xor vx, vy
                    cpu_state.V[X] ^= cpu_state.V[Y];
                    break;

                case 0x0004: // 8XY4 - add vx, vy
                    res = cpu_state.V[X] + cpu_state.V[Y];
                    cpu_state.V[0xF] = 0;
                    if (res > 0xFF){
                        cpu_state.V[0xF] = 1;
                    }
                    cpu_state.V[X] = res & 0x00FF;
                    break;

                case 0x0005: //8XY5 - sub vx, vy
                    res = cpu_state.V[X] - cpu_state.V[Y];
                    cpu_state.V[0xF] = 0;
                    if (cpu_state.V[X] > cpu_state.V[Y]){
                        cpu_state.V[0xF] = 1;
                    }
                    cpu_state.V[X] = res;
                    break;

                case 0x0006: //8X06 - shr vx, vy
                    cpu_state.V[0xF] = cpu_state.V[X] & 0x0001;
                    cpu_state.V[X] >>= 1;
                    break;

                case 0x0007: //8XY7 - rsb vx, vy
                    res = cpu_state.V[Y] - cpu_state.V[X];
                    cpu_state.V[0xF] = 0;
                    if (cpu_state.V[Y] > cpu_state.V[X]){
                        cpu_state.V[0xF] = 1;
                    }
                    cpu_state.V[X] = res;
                    break;

                case 0x000E: //8X0E - shl vx
                    cpu_state.V[0xF] = ((cpu_state.V[X] & 0x80) == 0x80) ? 1 : 0;
                    cpu_state.V[X] <<= 1;
                    break;

                default:
                    std::cerr << "undefined command in 8" << std::endl;
                    break;

            }
            break;

        case 0x9000: // 9XY0 - skne vx, vy
            if (LAST_DIGIT == 0x0000) {
                if (cpu_state.V[X] != cpu_state.V[Y]) {
                    cpu_state.PC += 2;
                }
            } else {
                std::cerr << "undefined command in 9" << std::endl;
            }
            break;

        case 0xA000: // ANNN - mvi nnn
            cpu_state.I = NNN;
            break;

        case 0xB000: // BNNN - jmi nnn
            cpu_state.PC = cpu_state.V[0] + NNN;
            break;

        case 0xC000: // CXKK - rand vx, kk
            cpu_state.V[X] = rand() & LAST_TWO_DIGITS;
            break;

        case 0xD000: // DXYN - sprite vx, vy, n
            uint8_t cur_mem, pixel;
            uint32_t index;
            cpu_state.V[0xF] = 0;


            for (i = 0; i < LAST_DIGIT; i++) {
                cur_mem = memory[cpu_state.I + i];

                for (j = 0; j < 8; j++) {
                    pixel = (cur_mem >> (7 - j)) & 0x01;
                    index = X + j + (Y + i) * DISPLAY_WIDTH;
                    if ((pixel == 1) && (display[index] == 1)){
                        cpu_state.V[0xF] = 1;
                    }
                    display[index] ^= pixel;
                }
            }
            break;

        case 0xE000:
            switch (LAST_TWO_DIGITS) {
                case 0x009E: // EK9E - skpr k
                    //TODO: skip if key (registed rk) pressed
                    std::cerr << "NOT SUPPORTED YET " << opcode << std::endl;
                    break;

                case 0x00A1: // EKA1 - skup k
                    //TODO: skip if key (registed rk) NOT pressed
                    std::cerr << "NOT SUPPORTED YET " << opcode << std::endl;
                    break;
                default:
                    std::cerr << "undefined command in E" << std::endl;
                    break;
            }
            break;

        case 0xF000:
            switch (LAST_TWO_DIGITS) {
                case 0x0007: // FX07 - gdelay vx
                    cpu_state.V[X] = cpu_state.delay_timer;
                    break;

                case 0x000A: // FR0A - key vr
                    //TODO: wait for for keypress, put key in register vr
                    std::cerr << "NOT SUPPORTED YET " << opcode << std::endl;
                    break;

                case 0x0015: // FX15 - sdelay vx
                    cpu_state.delay_timer = cpu_state.V[X];
                    break;

                case 0x0018: // FX18 - ssound vx
                    cpu_state.sound_timer = cpu_state.V[X];
                    break;

                case 0x001E: // FX1E - adi vx
                    cpu_state.I += cpu_state.V[X];
                    break;

                case 0x0029: // FX29 - font vx
                    cpu_state.I = cpu_state.V[X] * 0x05;
                    break;

                case 0x0030: // FX30 - xfont vx
                    cpu_state.I = cpu_state.V[X] * 0x0A;
                    break;

                case 0x0033: // FX33 - bcd vx
                    memory[cpu_state.I + 0] = (cpu_state.V[X])/100;
                    //memory[cpu_state.I + 1] = ((cpu_state.V[X])/10) % 10;
                    memory[cpu_state.I + 1] = ((cpu_state.V[X]) % 100) / 10;
                    //memory[cpu_state.I + 2] = (cpu_state.V[X]) % 10;
                    memory[cpu_state.I + 2] = cpu_state.V[X] % 10;
                    break;

                case 0x0055: // FX55 - str v0-vx
                    for(i = 0; i <= X; i++) {
                        memory[cpu_state.I + i] = cpu_state.V[i];
                    }
                    break;

                case 0x0065: // FX65 - ldr v0-vx
                    for(i = 0; i <= X; i++) {
                        cpu_state.V[i] = memory[cpu_state.I + i];
                    }
                    break;
                default:
                    std::cerr << "undefined command in F" << std::endl;
                    break;
            }
            break;

        default:
            std::cerr << "undefined command" << std::endl;
            break;
    }
}

// use hacky posix terminal thingy to erase console contents
void clearScreen(void) {
    const char *CLEAR_SCREEN_ANSI = "\e[1;1H\e[2J";
    write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
}

// draw current screen in terminal
void draw_display(void) {
    string line;
    clearScreen();
    for (int i = 0; i < DISPLAY_HEIGHT; i++) {
        line = "";
        for (int j = 0; j < DISPLAY_WIDTH; j++) {
            if (display[i * DISPLAY_WIDTH + j]) {
                line += "*";
            } else {
                line += " ";
            }
            //cout << '*';
        }
       cout << line << endl;
       // cout << endl;
    }
}

// load program into memory starting from 0x200
void load_program(vector<uint8_t> byte_array) {
    for(int i = 0; i < byte_array.size(); i++){
        memory[0x200 + i] = byte_array[i];
    }
}