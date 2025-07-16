#include <bitset>
#include <concepts>
#include <string>
#include <iostream>
#include "dualshock.h"
#define __WINDOWS_MM__
#include <rtmidi/RtMidi.h>
#include <windows.h>
#include <hidapi/hidapi.h>
#include <chrono>
#include <thread>
#include <cmath>
#include <map>
#define PS4_NAME "Wireless Controller"
#define RGB false
#define BLINK false
std::string wcharToString(wchar_t* input) {
        std::wstring ws = input;
        return std::string( ws.begin(), ws.end() );
}

int nearDiv(double number, double divisor) {
    if (divisor == 0.0) return 0;

    // Compute the nearest multiple of `divisor`
    double nearestMultiple = std::round(number / divisor) * divisor;

    // Compare how far `number` is from this nearest multiple
    return std::fabs(number - nearestMultiple);
}
void updateButton(bool current, bool previous, ButtonState & btn) {
    if (current) {
        if (!previous) {
            btn.DOWN = true;
        } else {
            btn.HOLD = true;
        }
    } else {
        if (previous) {
            btn.UP = true;
        } else {
            btn.NEUTRAL = true;
        }
    }
}

std::map <std::string, ButtonState> parseBumpers(std::bitset<8> bumpers, std::bitset<8> prevBumpers, std::bitset<8> buttons, std::bitset<8> prevButtons) {
    std::map <std::string, ButtonState> btnMap = {{"LBUMP", ButtonState()}, {"RBUMP", ButtonState()}, {"LTRG", ButtonState()}, {"RTRG", ButtonState()}, {"UPAD", ButtonState()}, {"DPAD", ButtonState()}, {"LPAD", ButtonState()}, {"RPAD", ButtonState()}, {"RTRG", ButtonState()}, {"X", ButtonState()}, {"SQR", ButtonState()}, {"CIR", ButtonState()}, {"TRI", ButtonState()}};
    updateButton(bumpers[0], prevBumpers[0], btnMap["LBUMP"]);
    updateButton(bumpers[1], prevBumpers[1], btnMap["RBUMP"]);
    updateButton(bumpers[2], prevBumpers[2], btnMap["LTRG"]);
    updateButton(bumpers[3], prevBumpers[3], btnMap["RTRG"]);
    updateButton(buttons[0], prevButtons[0], btnMap["RPAD"]);
    updateButton(buttons[1], prevButtons[1], btnMap["LPAD"]);
    updateButton(buttons[2], prevButtons[2], btnMap["DPAD"]);
    updateButton(buttons[3], prevButtons[3], btnMap["UPAD"]);
    updateButton(buttons[4], prevButtons[4], btnMap["SQR"]);
    updateButton(buttons[5], prevButtons[5], btnMap["X"]);
    updateButton(buttons[6], prevButtons[6], btnMap["CIR"]);
    updateButton(buttons[7], prevButtons[7], btnMap["TRI"]);

    return btnMap;
}
void buttonDEBUG(std::map<std::string, ButtonState> events) {
    for (auto pair : events) {
        std::cout << pair.first << ": " << std::endl;
        std::cout << "UP" << " " << pair.second.UP << std::endl;
        std::cout << "DOWN" << " " << pair.second.DOWN << std::endl;
        std::cout << "NEUTRAL" << " " << pair.second.NEUTRAL << std::endl;
        std::cout << "HOLD" << " " << pair.second.HOLD << std::endl;
    }
}

void sendMidi(RtMidiOut* midiout, std::vector<unsigned char> message) {
    try {
        midiout->sendMessage( &message );
    } catch (RtMidiError &error) {
        // Handle the exception here
        error.printMessage();
    }
}

void processButtons(std::map<std::string, ButtonState> & btnMap, RtMidiOut* midiout, int & octave) {
    const int c = 60 + octave*12;


    if (btnMap["X"].DOWN) {
        std::vector<unsigned char> message{0x90, c, 127};
        sendMidi(midiout, message);
    } else if (btnMap["X"].UP) {
        std::vector<unsigned char> message{0x80, c, 0};
        sendMidi(midiout, message);
    }
    if (btnMap["SQR"].DOWN) {
        std::vector<unsigned char> message{0x91, c+1, 127};
        sendMidi(midiout, message);
    } else if (btnMap["SQR"].UP) {
        std::vector<unsigned char> message{0x81, c+1, 0};
        sendMidi(midiout, message);
    }
    if (btnMap["CIR"].DOWN) {
        std::vector<unsigned char> message{0x92, c+2, 127};
        sendMidi(midiout, message);
    } else if (btnMap["CIR"].UP) {
        std::vector<unsigned char> message{0x82, c+2, 0};
        sendMidi(midiout, message);
    }
    if (btnMap["TRI"].DOWN) {
        std::vector<unsigned char> message{0x93, c+3, 127};
        sendMidi(midiout, message);
    } else if (btnMap["TRI"].UP) {
        std::vector<unsigned char> message{0x83, c+3, 0};
        sendMidi(midiout, message);
    }
    if (btnMap["LBUMP"].DOWN) {
        std::vector<unsigned char> message{0x90, c+4, 127};
        sendMidi(midiout, message);
    } else if (btnMap["LBUMP"].UP) {
        std::vector<unsigned char> message{0x80, c+4, 0};
        sendMidi(midiout, message);
    }
    if (btnMap["RBUMP"].DOWN) {
        std::vector<unsigned char> message{0x91, c+5, 127};
        sendMidi(midiout, message);
    } else if (btnMap["RBUMP"].UP) {
        std::vector<unsigned char> message{0x81, c+5, 0};
        sendMidi(midiout, message);
    }
    if (btnMap["LTRG"].DOWN) {
        std::vector<unsigned char> message{0x92, c+6, 127};
        sendMidi(midiout, message);
    } else if (btnMap["LTRG"].UP) {
        std::vector<unsigned char> message{0x82, c+6, 0};
        sendMidi(midiout, message);
    }
    if (btnMap["RTRG"].DOWN) {
        std::vector<unsigned char> message{0x93, c+7, 127};
        sendMidi(midiout, message);
    } else if (btnMap["RTRG"].UP) {
        std::vector<unsigned char> message{0x83, c+7, 0};
        sendMidi(midiout, message);
    }

    if (btnMap["UPAD"].DOWN) {
        octave +=1;
    }
    if (btnMap["DPAD"].DOWN) {
        octave -=1;
    }
    // if (btnMap["RPAD"].DOWN) {
    //     int pitchValue = 8192
    //     unsigned char lsb = pitchValue & 0x7F;        // 0x00
    //     unsigned char msb = (pitchValue >> 7) & 0x7F; // 0x40 (64)
    //     std::vector<unsigned char> message{0xE0, 2000, 127};
    // } else if (btnMap["RPAD"].UP) {
    //     std::vector<unsigned char> message{0xE0, 2000, 127};
    //     sendMidi(midiout, message);
    // }
    //
}

int main (){
    if (hid_init() != 0) {
        std::cerr << "Failed to initialize HIDAPI" << std::endl;
        return 1;
    }
    hid_device_info *devices = hid_enumerate(0, 0);
    hid_device_info *device = devices;
    hid_device *handle;
    unsigned char bufOut[32] = {0};
    unsigned char bufIn[32] = {0};
    bufOut[0] = 0x05;            // Report ID
    bufOut[1] = 0xFF;            // Always 0xFF
    bufOut[2] = 0x00;            // ???
    bufOut[3] = 0x00;            // ???
    bufOut[4] = 0x00;            // Small Rumble
    bufOut[5] = 0x00;            // Big Rumble
    bufOut[6] = 0x00;            // LED Red
    bufOut[7] = 0x00;            // LED Green
    bufOut[8] = 0x00;            // LED Blue
    std::map<int, std::bitset<4>> dpadLookup =
        {{0, 0b1000},{1, 0b1001},{2, 0b0001}, {3, 0b0101}, {4, 0b0100}, {5, 0b0110}, {6, 0b0010}, {7, 0b1010}, {8, 0b0000}};


    bool canWrite = false;
    while (device != NULL) {
        std::string productName = wcharToString(device->product_string);
        if (productName == PS4_NAME) {
            std::cout << "Found PS4 Controller: " << productName << " | " << device->product_id << " | " << wcharToString(device->serial_number) << std::endl;
            handle = hid_open(device->vendor_id, device->product_id, device->serial_number);
            canWrite = true;
            break;
        } else {
            std::cout << "Other HID Device found: " << productName << " | " << device->product_id << " | " << wcharToString(device->serial_number) << std::endl;
            device = device->next;

        }
    }
    if (canWrite) {
        auto *midiout = new RtMidiOut();
        midiout->openPort(1);
        int dir = 1;
        int dir_j = -1;
        int dir_k = 1;
        unsigned i = 0;
        unsigned j = 120;
        unsigned k = 240;
        int octave = 0;
        auto timer{std::chrono::steady_clock::now()};
        std::chrono::steady_clock::time_point pulse;
        int bpm = 140;
        int ms = (60000/bpm);
        int pulseLength = 100;
        std::bitset<8> prevBumpers;
        std::bitset<8> prevButtons;
        while (true) {
            const auto start{std::chrono::steady_clock::now()};
            if (RGB) {
                if (i > 255) {
                    dir = -1;
                } else if (i <= 1) {
                    dir = 1;
                }
                if (j > 255) {
                    dir_j = -1;
                } else if (j <= 1) {
                    dir_j = 1;
                }
                if (k > 255) {
                    dir_k = -1;
                } else if (k <= 1) {
                    dir_k = 1;
                }
                i += dir;
                j += dir_j;
                k += dir_k;
                bufOut[6]= i;
                bufOut[7] = j;
                bufOut[8] = k;
            }
            if (BLINK){
                    if (nearDiv(
                        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timer).
                        count(), ms) <= 5) {
                        bufOut[6] = 255;
                        bufOut[4] = 255;
                        pulse = std::chrono::steady_clock::now();
                        std::cout << "Triggered" << std::endl;
                    } else if (!(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - pulse)
                                 .count() <= pulseLength)) {
                        bufOut[6] = 0;
                        bufOut[4] = 0;
                        //std::cout << std::to_string(((std::chrono::steady_clock::now() - timer).count() / secs));
                                 }
                    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timer).count() << "[ms]" << std::endl;
                    std::cout << nearDiv(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timer).count(), ms) << std::endl;
            }

            int hid_read_success = hid_read(handle, bufIn, 32);
            if (hid_read_success != -1) {
            } else {
                throw(std::runtime_error("Failed to read from device"));
            }

            std::bitset<8> bumpers = bufIn[6];
            std::bitset<8> buttons = bufIn[5];
            std::bitset<4> lower(dpadLookup[(int)(buttons.to_ulong() & 0x0F)]);
            std::bitset<4> upper((buttons.to_ulong() >> 4) & 0x0F);
            buttons = ( (upper.to_ulong() << 4) | lower.to_ulong() );

            auto events = parseBumpers(bumpers, prevBumpers, buttons, prevButtons);
            prevBumpers = bumpers;
            prevButtons = buttons;
            processButtons(events, midiout, octave);

            const auto finish{std::chrono::steady_clock::now()};

        }

    } else {
        std::cout << "Failed to find device" << std::endl;
    }

    hid_free_enumeration(devices);
    hid_exit();
}