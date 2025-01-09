//standard c++ headers
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <stdfloat>
#include "time.h"
#include <limits>
//raylib headers
#include "raylib.h"
#include "libs/raymath.h"
#include "helper.hpp"
#include "sound.hpp"

#define MAX_SAMPLES_PER_UPDATE   4096


Texture2D tapeImage;

uint16_t programCounter = 0;

uint8_t callStackPointer = 0;
uint16_t callStack[256];

uint8_t stackPointer = 0;
int16_t stack[256];

int16_t ROM[4096];
int16_t RAM[61440];

Tape TAPE1;
Tape TAPE2;

int16_t xRegister = 0;
int16_t yRegister = 0;
int16_t zRegister = 0;

int8_t flagRegister = 0;

char terminal[33][41];
int cursorPosition = 0;


int16_t valueAtAddress(uint16_t address)
{
    if(address > 4095)
    {
        return RAM[address - 4096];
    }
    else
    {
        return ROM[address];
    }
}

struct{
    
    Channel voice0;
    Channel voice1;
    Channel voice2;

    void SetSoundChipParams(uint16_t _startAddress, uint16_t _count)
    {
        uint16_t _shouldLoop = (zRegister) & 1;
        uint16_t _channel = (zRegister >> 1) & 3;
        uint16_t _volume = xRegister;

        if(_channel == 0)
        {
            voice0.progress = 0;
            voice0.startAddress = _startAddress;
            voice0.length = _count;
            voice0.volume = _volume;
            voice0.loop = _shouldLoop;
        }
        else if(_channel == 1)
        {
            voice1.progress = 0;
            voice1.startAddress = _startAddress;
            voice1.length = _count;
            voice1.volume = _volume;
            voice1.loop = _shouldLoop;
        }
        else if(_channel == 2)
        {
            voice2.progress = 0;
            voice2.startAddress = _startAddress;
            voice2.length = _count;
            voice2.volume = _volume;
            voice2.loop = _shouldLoop;
        }
    }

    void UpdateSounds()
    {
        if(voice0.progress < voice0.length)
        {
            VOICE0Wave.frequency = ((int)valueAtAddress(voice0.startAddress + voice0.progress)) & 16383;
            VOICE0Wave.waveType = ((int)valueAtAddress(voice0.startAddress + voice0.progress) & 49152) >> 14;
            
            voice0.progress += 1;
        }
        else if(voice0.loop == true)
        {
            voice0.progress = 0;
        }
        else
        {
            VOICE0Wave.frequency = 0;
        }
        
        if(voice1.progress < voice1.length)
        {
            VOICE1Wave.frequency = ((int)valueAtAddress(voice1.startAddress + voice1.progress)) & 16383;
            VOICE1Wave.waveType = ((int)valueAtAddress(voice1.startAddress + voice1.progress) & 49152) >> 14;
            
            voice1.progress += 1;
        }
        else if(voice1.loop == true)
        {
            voice1.progress = 0;
        }
        else
        {
            VOICE1Wave.frequency = 0;
        }

        if(voice2.progress < voice2.length)
        {
            VOICE2Wave.frequency = ((int)valueAtAddress(voice2.startAddress + voice2.progress)) & 16383;
            VOICE2Wave.waveType = ((int)valueAtAddress(voice2.startAddress + voice2.progress) & 49152) >> 14;
            
            voice2.progress += 1;
        }
        else if(voice2.loop == true)
        {
            voice2.progress = 0;
        }
        else
        {
            VOICE2Wave.frequency = 0;
        }

    }

} SoundChip;

void setFlagsInt(int16_t val1, int16_t val2)
{
    int16_t signedResult = val1 - val2;
    int32_t resultBig = (int32_t)val1 - (int32_t)val2;

    flagRegister = flagRegister & 8;

    //overflow
    if(signedResult != resultBig)
    {
        flagRegister = flagRegister | 0x1;
    }

    //zero
    if(signedResult == 0)
    {
        flagRegister = flagRegister | 0x2;
    }

    //negative
    if(signedResult < 0)
    {
        flagRegister = flagRegister | 0x4;
    }
}

void setFlagsFloat(_Float16 val1, _Float16 val2)
{
    _Float16 signedResult = val1 - val2;
    _Float32 resultBig = (_Float32)val1 - (_Float32)val2;

    flagRegister = flagRegister & 8;

    //overflow
    if(signedResult != resultBig)
    {
        flagRegister = flagRegister | 0x1;
    }

    //zero
    if(signedResult == 0)
    {
        flagRegister = flagRegister | 0x2;
    }

    //negative
    if(signedResult < 0)
    {
        flagRegister = flagRegister | 0x4;
    }
}

void setMulFlagsInt(int16_t val1, int16_t val2)
{
    int16_t signedResult = val1 * val2;
    int32_t resultBig = (int32_t)val1 * (int32_t)val2;

    flagRegister = flagRegister & 8;

    //overflow
    if(signedResult != resultBig)
    {
        flagRegister = flagRegister | 0x1;
    }

    //zero
    if(signedResult == 0)
    {
        flagRegister = flagRegister | 0x2;
    }

    //negative
    if(signedResult < 0)
    {
        flagRegister = flagRegister | 0x4;
    }
}

void setMulFlagsFloat(_Float16 val1, _Float16 val2)
{
    _Float16 signedResult = val1 * val2;
    _Float32 resultBig = (_Float32)fabsf((_Float32)val1 * (_Float32)val2);

    flagRegister = flagRegister & 8;

    //overflow
    if(resultBig > std::numeric_limits<_Float16>::max())
    {
        flagRegister = flagRegister | 0x1;
    }

    //zero
    if(signedResult == 0)
    {
        flagRegister = flagRegister | 0x2;
    }

    //negative
    if(signedResult < 0)
    {
        flagRegister = flagRegister | 0x4;
    }
}

void setDivFlagsInt(int16_t val1, int16_t val2)
{
    int16_t signedResult = val1 / val2;
    int32_t resultBig = (int32_t)val1 / (int32_t)val2;

    flagRegister = flagRegister & 8;

    //overflow
    if(signedResult != resultBig)
    {
        flagRegister = flagRegister | 0x1;
    }

    //zero
    if(signedResult == 0)
    {
        flagRegister = flagRegister | 0x2;
    }

    //negative
    if(signedResult < 0)
    {
        flagRegister = flagRegister | 0x4;
    }
}

void setDivFlagsFloat(_Float16 val1, _Float16 val2)
{
    _Float16 signedResult = val1 / val2;
    _Float32 resultBig = (_Float32)fabsf((_Float32)val1 / (_Float32)val2);

    flagRegister = flagRegister & 8;

    //overflow
    if(resultBig > std::numeric_limits<_Float16>::max())
    {
        flagRegister = flagRegister | 0x1;
    }

    //zero
    if(signedResult == 0)
    {
        flagRegister = flagRegister | 0x2;
    }

    //negative
    if(signedResult < 0)
    {
        flagRegister = flagRegister | 0x4;
    }
}

void terminalOutput(uint16_t character)
{
    //std::cout << character << "\n";

    switch (character)
    {
    case 257: //enter
        for (int i = 0; i < 31; i++)
        {
            for (int j = 0; j < 40; j++)
            {
                terminal[i][j] = terminal[i + 1][j];
            }
            cursorPosition = 0;
            
        }
        break;
    case 259: //backspace
        if(cursorPosition > 0)
        {
            cursorPosition -= 1;
            terminal[30][cursorPosition] = 0;
        }
        break;
    case 261: //delete
        terminal[30][cursorPosition] = 0;
        break;
    default:
        terminal[30][cursorPosition] = (char)character;
        cursorPosition += 1;
        if(cursorPosition > 40)
        {
            for (int i = 0; i < 31; i++)
            {
                for (int j = 0; j < 40; j++)
                {
                    terminal[i][j] = terminal[i + 1][j];
                }
                cursorPosition = 0;
                
            }
        }
    }
    

}



int16_t valueAtNestedAddress(uint16_t address)
{
    uint16_t newAdd = 0;

    if(address > 4095)
    {
        newAdd =  signedToUnsigned(RAM[address - 4096]);
    }
    else
    {
        newAdd = signedToUnsigned(ROM[address]);
    }

    return valueAtAddress(newAdd);
}

void setValueAtAddress(uint16_t address, int16_t value)
{
    if(address > 4095)
    {
        RAM[address - 4096] = value;
    }
}

void setValueAtNestedAddress(uint16_t address, int16_t value)
{
    uint16_t newAdd = 0;

    if(address > 4095)
    {
        newAdd =  signedToUnsigned(RAM[address - 4096]);
    }

    setValueAtAddress(newAdd, value);
}

bool executeInstruction(int16_t opcode, int16_t arg1, int16_t arg2, int16_t kbin, int16_t joyport1, int16_t joyport2)
{
    //terminalOutput(259);

    if(opcode != 0)
    {
        //std::cout << opcode << " " << arg1 << " " << arg2 << "\n";
    }

    switch (opcode)
    {
    case 0:
        break;
    case 1:
        flagRegister = (int8_t)(arg1 & 0b0000000011111111);
        break;
    case 2:
        setValueAtAddress(signedToUnsigned(arg1), (int16_t)flagRegister);
        break;
    case 3: //int to float $
    {
        _Float16 f = intToFloat(valueAtAddress(signedToUnsigned(arg1)));
        int16_t encodedF = encodeFloat(f);
        setValueAtAddress(signedToUnsigned(arg1), encodedF);
        break;
    }
    case 4: //int to float &
    {
        _Float16 f = intToFloat(valueAtNestedAddress(signedToUnsigned(arg1)));
        int16_t encodedF = encodeFloat(f);
        setValueAtNestedAddress(signedToUnsigned(arg1), encodedF);
        break;
    }
    case 5: //float to int $
    {
        _Float16 f = parseFloat(valueAtAddress(signedToUnsigned(arg1)));
        int16_t converted = floatToInt(f);
        setValueAtAddress(signedToUnsigned(arg1), converted);
        break;
    }
    case 6: //float to int &
    {
        _Float16 f = parseFloat(valueAtNestedAddress(signedToUnsigned(arg1)));
        int16_t converted = floatToInt(f);
        setValueAtNestedAddress(signedToUnsigned(arg1), converted);
        break;
    }
    case 7: //rand imm imm
    {
        int16_t randomNumber = (rand() % (arg2 - arg1 + 1)) + arg1;
        xRegister = randomNumber;
        break;
    }
    case 8: //rand imm $
    {
        int16_t randomNumber = (rand() % (valueAtAddress(signedToUnsigned(arg2)) - arg1 + 1)) + arg1;
        xRegister = randomNumber;
        break;
    }
    case 9: //rand imm &
    {
        int16_t randomNumber = (rand() % (valueAtNestedAddress(signedToUnsigned(arg2)) - arg1 + 1)) + arg1;
        xRegister = randomNumber;
        break;
    }
    case 10: //rand $ imm
    {
        int16_t randomNumber = (rand() % (arg2 - valueAtAddress(signedToUnsigned(arg1)) + 1)) + valueAtAddress(signedToUnsigned(arg1));
        xRegister = randomNumber;
        break;
    }
    case 11: //rand $ $
    {
        int16_t randomNumber = (rand() % (valueAtAddress(signedToUnsigned(arg2)) - valueAtAddress(signedToUnsigned(arg1)) + 1)) + valueAtAddress(signedToUnsigned(arg1));
        xRegister = randomNumber;
        break;
    }
    case 12: //rand $ &
    {
        int16_t randomNumber = (rand() % (valueAtNestedAddress(signedToUnsigned(arg2)) - valueAtAddress(signedToUnsigned(arg1)) + 1)) + valueAtAddress(signedToUnsigned(arg1));
        xRegister = randomNumber;
        break;
    }
    case 13: //rand & imm
    {
        int16_t randomNumber = (rand() % (arg2 - valueAtNestedAddress(signedToUnsigned(arg1)) + 1)) + valueAtNestedAddress(signedToUnsigned(arg1));
        xRegister = randomNumber;
        break;
    }
    case 14: //rand & $
    {
        int16_t randomNumber = (rand() % (valueAtAddress(signedToUnsigned(arg2)) - valueAtNestedAddress(signedToUnsigned(arg1)) + 1)) + valueAtNestedAddress(signedToUnsigned(arg1));
        xRegister = randomNumber;
        break;
    }
    case 15: //rand & &
    {
        int16_t randomNumber = (rand() % (valueAtNestedAddress(signedToUnsigned(arg2)) - valueAtNestedAddress(signedToUnsigned(arg1)) + 1)) + valueAtNestedAddress(signedToUnsigned(arg1));
        xRegister = randomNumber;
        break;
    }
    case 16: //compare $ imm
    {
        int16_t val1 = valueAtAddress(signedToUnsigned(arg1));
        int16_t val2 = arg2;

        setFlagsInt(val1, val2);

        break;
    }
    case 17: //compare $add, $add
    {
        int16_t val1 = valueAtAddress(signedToUnsigned(arg1));
        int16_t val2 = valueAtAddress(signedToUnsigned(arg2));
        
        setFlagsInt(val1, val2);

        break;
    }
    case 18: //compare $add, &add
    {
        int16_t val1 = valueAtAddress(signedToUnsigned(arg1));
        int16_t val2 = valueAtNestedAddress(signedToUnsigned(arg2));

        setFlagsInt(val1, val2);

        break;
    }
    case 19: //compare & imm
    {
        int16_t val1 = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t val2 = arg2;

        setFlagsInt(val1, val2);

        break;
    }
    case 20: //compare &add, $add
    {
        int16_t val1 = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t val2 = valueAtAddress(signedToUnsigned(arg2));

        setFlagsInt(val1, val2);

        break;
    }
    case 21: //compare &add, &add
    {
        int16_t val1 = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t val2 = valueAtNestedAddress(signedToUnsigned(arg2));
        
        setFlagsInt(val1, val2);

        break;
    }
    case 22: //load x imm
    {
        xRegister = arg1;
        break;
    }
    case 23: //load x $add
    {
        xRegister = valueAtAddress(signedToUnsigned(arg1));
        break;
    }
    case 24: //load x &add
    {
        xRegister = valueAtNestedAddress(signedToUnsigned(arg1));
        break;
    }
    case 25: //load y imm
    {
        yRegister = arg1;
        break;
    }
    case 26: //load y $add
    {
        yRegister = valueAtAddress(signedToUnsigned(arg1));
        break;
    }
    case 27: //load y &add
    {
        yRegister = valueAtNestedAddress(signedToUnsigned(arg1));
        break;
    }
    case 28: //load z imm
    {
        zRegister = arg1;
        break;
    }
    case 29: //load z $add
    {
        zRegister = valueAtAddress(signedToUnsigned(arg1));
        break;
    }
    case 30: //load z &add
    {
        zRegister = valueAtNestedAddress(signedToUnsigned(arg1));
        break;
    }
    case 31: //store x in $add
    {
        setValueAtAddress(signedToUnsigned(arg1), xRegister);
        break;
    }
    case 32: //store x in $add
    {
        setValueAtNestedAddress(signedToUnsigned(arg1), xRegister);
        break;
    }
    case 33: //store y in $add
    {
        setValueAtAddress(signedToUnsigned(arg1), yRegister);
        break;
    }
    case 34: //store y in $add
    {
        setValueAtNestedAddress(signedToUnsigned(arg1), yRegister);
        break;
    }
    case 35: //store z in $add
    {
        setValueAtAddress(signedToUnsigned(arg1), zRegister);
        break;
    }
    case 36: //store z in $add
    {
        setValueAtNestedAddress(signedToUnsigned(arg1), zRegister);
        break;
    }
    case 37: //load int $add imm
    {
        setValueAtAddress(signedToUnsigned(arg1), arg2);
        break;
    }
    case 38: //load int &add imm
    {
        setValueAtNestedAddress(signedToUnsigned(arg1), arg2);
        break;
    }
    case 39: //load float $add imm
    {
        setValueAtAddress(signedToUnsigned(arg1), arg2);
        break;
    }
    case 40: //load float &add imm
    {
        setValueAtNestedAddress(signedToUnsigned(arg1), arg2);
        break;
    }
    case 41: //move/copy to $ from $
    {
        setValueAtAddress(signedToUnsigned(arg1), valueAtAddress(signedToUnsigned(arg2)));
        break;
    }
    case 42: //move/copy to $ from &
    {
        setValueAtAddress(signedToUnsigned(arg1), valueAtNestedAddress(signedToUnsigned(arg2)));
        break;
    }
    case 43: //move/copy to & from $
    {
        setValueAtNestedAddress(signedToUnsigned(arg1), valueAtAddress(signedToUnsigned(arg2)));
        break;
    }
    case 44: //move/copy to & from &
    {
        setValueAtNestedAddress(signedToUnsigned(arg1), valueAtNestedAddress(signedToUnsigned(arg2)));
        break;
    }
    case 45: //swap $ $
    {
        int16_t temp = valueAtAddress(signedToUnsigned(arg1));

        setValueAtAddress(signedToUnsigned(arg1), valueAtAddress(signedToUnsigned(arg2)));
        setValueAtAddress(signedToUnsigned(arg2), temp);
        break;
    }
    case 46: //swap $ &
    {
        int16_t temp = valueAtAddress(signedToUnsigned(arg1));

        setValueAtAddress(signedToUnsigned(arg1), valueAtNestedAddress(signedToUnsigned(arg2)));
        setValueAtNestedAddress(signedToUnsigned(arg2), temp);
        break;
    }
    case 47: //swap & $
    {
        int16_t temp = valueAtNestedAddress(signedToUnsigned(arg1));

        setValueAtNestedAddress(signedToUnsigned(arg1), valueAtAddress(signedToUnsigned(arg2)));
        setValueAtAddress(signedToUnsigned(arg2), temp);
        break;
    }
    case 48: //swap & &
    {
        int16_t temp = valueAtNestedAddress(signedToUnsigned(arg1));

        setValueAtNestedAddress(signedToUnsigned(arg1), valueAtNestedAddress(signedToUnsigned(arg2)));
        setValueAtNestedAddress(signedToUnsigned(arg2), temp);
        break;
    }
    case 49: //add int $ imm
    {
        int16_t originalValue = valueAtAddress(signedToUnsigned(arg1));
        int16_t valueToAdd = arg2;

        setFlagsInt(originalValue, -1 * valueToAdd);

        setValueAtAddress(signedToUnsigned(arg1), originalValue + valueToAdd);
        break;
    }
    case 50: //add int & imm
    {
        int16_t originalValue = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t valueToAdd = arg2;

        setFlagsInt(originalValue, -1 * valueToAdd);

        setValueAtNestedAddress(signedToUnsigned(arg1), originalValue + valueToAdd);
        break;
    }
    case 51: //add int $ $
    {
        int16_t originalValue = valueAtAddress(signedToUnsigned(arg1));
        int16_t valueToAdd = valueAtAddress(signedToUnsigned(arg2));

        setFlagsInt(originalValue, -1 * valueToAdd);

        setValueAtAddress(signedToUnsigned(arg1), originalValue + valueToAdd);
        break;
    }
    case 52: //add int & $
    {
        int16_t originalValue = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t valueToAdd = valueAtAddress(signedToUnsigned(arg2));

        setFlagsInt(originalValue, -1 * valueToAdd);

        setValueAtNestedAddress(signedToUnsigned(arg1), originalValue + valueToAdd);
        break;
    }
    case 53: //add int $ &
    {
        int16_t originalValue = valueAtAddress(signedToUnsigned(arg1));
        int16_t valueToAdd = valueAtNestedAddress(signedToUnsigned(arg2));

        setFlagsInt(originalValue, -1 * valueToAdd);

        setValueAtAddress(signedToUnsigned(arg1), originalValue + valueToAdd);
        break;
    }
    case 54: //add int & &
    {
        int16_t originalValue = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t valueToAdd = valueAtNestedAddress(signedToUnsigned(arg2));

        setFlagsInt(originalValue, -1 * valueToAdd);

        setValueAtNestedAddress(signedToUnsigned(arg1), originalValue + valueToAdd);
        break;
    }
    case 55: //add float $ imm
    {
        _Float16 originalValue = parseFloat(valueAtAddress(signedToUnsigned(arg1)));
        _Float16 valueToAdd = parseFloat(arg2);

        setFlagsFloat(originalValue, -1 * valueToAdd);
        

        setValueAtAddress(signedToUnsigned(arg1), encodeFloat(originalValue + valueToAdd));
        break;
    }
    case 56: //add float & imm
    {
        _Float16 originalValue = parseFloat(valueAtNestedAddress(signedToUnsigned(arg1)));
        _Float16 valueToAdd = parseFloat(arg2);

        setFlagsFloat(originalValue, -1 * valueToAdd);

        setValueAtNestedAddress(signedToUnsigned(arg1), encodeFloat(originalValue + valueToAdd));
        break;
    }
    case 57: //add float $ $
    {
        _Float16 originalValue = parseFloat(valueAtAddress(signedToUnsigned(arg1)));
        _Float16 valueToAdd = parseFloat(valueAtAddress(signedToUnsigned(arg2)));

        setFlagsFloat(originalValue, -1 * valueToAdd);

        setValueAtAddress(signedToUnsigned(arg1), encodeFloat(originalValue + valueToAdd));
        break;
    }
    case 58: //add float & $
    {
        _Float16 originalValue = parseFloat(valueAtNestedAddress(signedToUnsigned(arg1)));
        _Float16 valueToAdd = parseFloat(valueAtAddress(signedToUnsigned(arg2)));

        setFlagsFloat(originalValue, -1 * valueToAdd);

        setValueAtNestedAddress(signedToUnsigned(arg1), encodeFloat(originalValue + valueToAdd));
        break;
    }
    case 59: //add float $ &
    {
        _Float16 originalValue = parseFloat(valueAtAddress(signedToUnsigned(arg1)));
        _Float16 valueToAdd = parseFloat(valueAtNestedAddress(signedToUnsigned(arg2)));

        setFlagsFloat(originalValue, -1 * valueToAdd);

        setValueAtAddress(signedToUnsigned(arg1), encodeFloat(originalValue + valueToAdd));
        break;
    }
    case 60: //add float & &
    {
        _Float16 originalValue = parseFloat(valueAtNestedAddress(signedToUnsigned(arg1)));
        _Float16 valueToAdd = parseFloat(valueAtNestedAddress(signedToUnsigned(arg2)));

        setFlagsFloat(originalValue, -1 * valueToAdd);

        setValueAtNestedAddress(signedToUnsigned(arg1), encodeFloat(originalValue + valueToAdd));
        break;
    }
    case 61: //sub int $ imm
    {
        int16_t originalValue = valueAtAddress(signedToUnsigned(arg1));
        int16_t valueToSub = arg2;

        setFlagsInt(originalValue, valueToSub);

        setValueAtAddress(signedToUnsigned(arg1), originalValue - valueToSub);
        break;
    }
    case 62: //sub int & imm
    {
        int16_t originalValue = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t valueToSub = arg2;

        setFlagsInt(originalValue, valueToSub);

        setValueAtNestedAddress(signedToUnsigned(arg1), originalValue - valueToSub);
        break;
    }
    case 63: //sub int $ $
    {
        int16_t originalValue = valueAtAddress(signedToUnsigned(arg1));
        int16_t valueToSub = valueAtAddress(signedToUnsigned(arg2));

        

        setFlagsInt(originalValue, valueToSub);

        setValueAtAddress(signedToUnsigned(arg1), originalValue - valueToSub);
        break;
    }
    case 64: //sub int & $
    {
        int16_t originalValue = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t valueToSub = valueAtAddress(signedToUnsigned(arg2));

        setFlagsInt(originalValue, valueToSub);

        setValueAtNestedAddress(signedToUnsigned(arg1), originalValue - valueToSub);
        break;
    }
    case 65: //sub int $ &
    {
        int16_t originalValue = valueAtAddress(signedToUnsigned(arg1));
        int16_t valueToSub = valueAtNestedAddress(signedToUnsigned(arg2));

        setFlagsInt(originalValue, valueToSub);

        setValueAtAddress(signedToUnsigned(arg1), originalValue - valueToSub);
        break;
    }
    case 66: //sub int & &
    {
        int16_t originalValue = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t valueToSub = valueAtNestedAddress(signedToUnsigned(arg2));

        setFlagsInt(originalValue, valueToSub);

        setValueAtNestedAddress(signedToUnsigned(arg1), originalValue - valueToSub);
        break;
    }
    case 67: //sub float $ imm
    {
        _Float16 originalValue = parseFloat(valueAtAddress(signedToUnsigned(arg1)));
        _Float16 valueToSub = parseFloat(arg2);

        setFlagsFloat(originalValue, valueToSub);

        setValueAtAddress(signedToUnsigned(arg1), encodeFloat(originalValue - valueToSub));
        break;
    }
    case 68: //sub float & imm
    {
        _Float16 originalValue = parseFloat(valueAtNestedAddress(signedToUnsigned(arg1)));
        _Float16 valueToSub = parseFloat(arg2);

        setFlagsFloat(originalValue, valueToSub);

        setValueAtNestedAddress(signedToUnsigned(arg1), encodeFloat(originalValue - valueToSub));
        break;
    }
    case 69: //sub float $ $
    {
        _Float16 originalValue = parseFloat(valueAtAddress(signedToUnsigned(arg1)));
        _Float16 valueToSub = parseFloat(valueAtAddress(signedToUnsigned(arg2)));

        setFlagsFloat(originalValue, valueToSub);

        setValueAtAddress(signedToUnsigned(arg1), encodeFloat(originalValue - valueToSub));
        break;
    }
    case 70: //sub float & $
    {
        _Float16 originalValue = parseFloat(valueAtNestedAddress(signedToUnsigned(arg1)));
        _Float16 valueToSub = parseFloat(valueAtAddress(signedToUnsigned(arg2)));

        setFlagsFloat(originalValue, valueToSub);

        setValueAtNestedAddress(signedToUnsigned(arg1), encodeFloat(originalValue - valueToSub));
        break;
    }
    case 71: //sub float $ &
    {
        _Float16 originalValue = parseFloat(valueAtAddress(signedToUnsigned(arg1)));
        _Float16 valueToSub = parseFloat(valueAtNestedAddress(signedToUnsigned(arg2)));

        setFlagsFloat(originalValue, valueToSub);

        setValueAtAddress(signedToUnsigned(arg1), encodeFloat(originalValue - valueToSub));
        break;
    }
    case 72: //sub float & &
    {
        _Float16 originalValue = parseFloat(valueAtNestedAddress(signedToUnsigned(arg1)));
        _Float16 valueToSub = parseFloat(valueAtNestedAddress(signedToUnsigned(arg2)));

        setFlagsFloat(originalValue, valueToSub);

        setValueAtNestedAddress(signedToUnsigned(arg1), encodeFloat(originalValue - valueToSub));
        break;
    }
    case 73: //mul int $ imm
    {
        int16_t val1 = valueAtAddress(signedToUnsigned(arg1));
        int16_t val2 = arg2;

        setMulFlagsInt(val1, val2);
        
        setValueAtAddress(signedToUnsigned(arg1), val1 * val2);

        break;
    }
    case 74: //mul int & imm
    {
        int16_t val1 = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t val2 = arg2;

        setMulFlagsInt(val1, val2);
        
        setValueAtNestedAddress(signedToUnsigned(arg1), val1 * val2);

        break;
    }
    case 75: //mul int $ $
    {
        int16_t val1 = valueAtAddress(signedToUnsigned(arg1));
        int16_t val2 = valueAtAddress(signedToUnsigned(arg2));

        setMulFlagsInt(val1, val2);
        
        setValueAtAddress(signedToUnsigned(arg1), val1 * val2);

        break;
    }
    case 76: //mul int & $
    {
        int16_t val1 = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t val2 = valueAtAddress(signedToUnsigned(arg2));

        setMulFlagsInt(val1, val2);
        
        setValueAtNestedAddress(signedToUnsigned(arg1), val1 * val2);

        break;
    }
    case 77: //mul int $ &
    {
        int16_t val1 = valueAtAddress(signedToUnsigned(arg1));
        int16_t val2 = valueAtNestedAddress(signedToUnsigned(arg2));

        setMulFlagsInt(val1, val2);
        
        setValueAtAddress(signedToUnsigned(arg1), val1 * val2);

        break;
    }
    case 78: //mul int & &
    {
        int16_t val1 = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t val2 = valueAtNestedAddress(signedToUnsigned(arg2));

        setMulFlagsInt(val1, val2);
        
        setValueAtNestedAddress(signedToUnsigned(arg1), val1 * val2);

        break;
    }
    case 79: //mul float $ imm
    {
        _Float16 val1 = parseFloat(valueAtAddress(signedToUnsigned(arg1)));
        _Float16 val2 = parseFloat(arg2);

        setMulFlagsInt(val1, val2);
        
        setValueAtAddress(signedToUnsigned(arg1), encodeFloat(val1 * val2));

        break;
    }
    case 80: //mul float & imm
    {
        _Float16 val1 = parseFloat(valueAtNestedAddress(signedToUnsigned(arg1)));
        _Float16 val2 = parseFloat(arg2);

        setMulFlagsInt(val1, val2);
        
        setValueAtNestedAddress(signedToUnsigned(arg1), encodeFloat(val1 * val2));

        break;
    }
    case 81: //mul float $ $
    {
        _Float16 val1 = parseFloat(valueAtAddress(signedToUnsigned(arg1)));
        _Float16 val2 = parseFloat(valueAtAddress(signedToUnsigned(arg2)));

        setMulFlagsInt(val1, val2);
        
        setValueAtAddress(signedToUnsigned(arg1), encodeFloat(val1 * val2));

        break;
    }
    case 82: //mul float & $
    {
        _Float16 val1 = parseFloat(valueAtNestedAddress(signedToUnsigned(arg1)));
        _Float16 val2 = parseFloat(valueAtAddress(signedToUnsigned(arg2)));

        setMulFlagsInt(val1, val2);
        
        setValueAtNestedAddress(signedToUnsigned(arg1), encodeFloat(val1 * val2));

        break;
    }
    case 83: //mul float $ &
    {
        _Float16 val1 = parseFloat(valueAtAddress(signedToUnsigned(arg1)));
        _Float16 val2 = parseFloat(valueAtNestedAddress(signedToUnsigned(arg2)));

        setMulFlagsInt(val1, val2);
        
        setValueAtAddress(signedToUnsigned(arg1), encodeFloat(val1 * val2));

        break;
    }
    case 84: //mul float & &
    {
        _Float16 val1 = parseFloat(valueAtNestedAddress(signedToUnsigned(arg1)));
        _Float16 val2 = parseFloat(valueAtNestedAddress(signedToUnsigned(arg2)));

        setMulFlagsInt(val1, val2);
        
        setValueAtNestedAddress(signedToUnsigned(arg1), encodeFloat(val1 * val2));

        break;
    }
    case 85: //div int $ imm
    {
        int16_t val1 = valueAtAddress(signedToUnsigned(arg1));
        int16_t val2 = arg2;

        setDivFlagsInt(val1, val2);
        
        setValueAtAddress(signedToUnsigned(arg1), val1 / val2);

        break;
    }
    case 86: //div int & imm
    {
        int16_t val1 = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t val2 = arg2;

        setDivFlagsInt(val1, val2);
        
        setValueAtNestedAddress(signedToUnsigned(arg1), val1 / val2);

        break;
    }
    case 87: //div int $ $
    {
        int16_t val1 = valueAtAddress(signedToUnsigned(arg1));
        int16_t val2 = valueAtAddress(signedToUnsigned(arg2));

        setDivFlagsInt(val1, val2);
        
        setValueAtAddress(signedToUnsigned(arg1), val1 / val2);

        break;
    }
    case 88: //div int & $
    {
        int16_t val1 = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t val2 = valueAtAddress(signedToUnsigned(arg2));

        setDivFlagsInt(val1, val2);
        
        setValueAtNestedAddress(signedToUnsigned(arg1), val1 / val2);

        break;
    }
    case 89: //div int $ &
    {
        int16_t val1 = valueAtAddress(signedToUnsigned(arg1));
        int16_t val2 = valueAtNestedAddress(signedToUnsigned(arg2));

        setDivFlagsInt(val1, val2);
        
        setValueAtAddress(signedToUnsigned(arg1), val1 / val2);

        break;
    }
    case 90: //div int & &
    {
        int16_t val1 = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t val2 = valueAtNestedAddress(signedToUnsigned(arg2));

        setDivFlagsInt(val1, val2);
        
        setValueAtNestedAddress(signedToUnsigned(arg1), val1 / val2);

        break;
    }
    case 91: //div float $ imm
    {
        _Float16 val1 = parseFloat(valueAtAddress(signedToUnsigned(arg1)));
        _Float16 val2 = parseFloat(arg2);

        setDivFlagsInt(val1, val2);
        
        setValueAtAddress(signedToUnsigned(arg1), encodeFloat(val1 / val2));

        break;
    }
    case 92: //div float & imm
    {
        _Float16 val1 = parseFloat(valueAtNestedAddress(signedToUnsigned(arg1)));
        _Float16 val2 = parseFloat(arg2);

        setDivFlagsFloat(val1, val2);
        
        setValueAtNestedAddress(signedToUnsigned(arg1), encodeFloat(val1 / val2));

        break;
    }
    case 93: //div float $ $
    {
        _Float16 val1 = parseFloat(valueAtAddress(signedToUnsigned(arg1)));
        _Float16 val2 = parseFloat(valueAtAddress(signedToUnsigned(arg2)));

        setDivFlagsFloat(val1, val2);
        
        setValueAtAddress(signedToUnsigned(arg1), encodeFloat(val1 / val2));

        break;
    }
    case 94: //div float & $
    {
        _Float16 val1 = parseFloat(valueAtNestedAddress(signedToUnsigned(arg1)));
        _Float16 val2 = parseFloat(valueAtAddress(signedToUnsigned(arg2)));

        setDivFlagsFloat(val1, val2);
        
        setValueAtNestedAddress(signedToUnsigned(arg1), encodeFloat(val1 / val2));

        break;
    }
    case 95: //div float $ &
    {
        _Float16 val1 = parseFloat(valueAtAddress(signedToUnsigned(arg1)));
        _Float16 val2 = parseFloat(valueAtNestedAddress(signedToUnsigned(arg2)));

        setDivFlagsFloat(val1, val2);
        
        setValueAtAddress(signedToUnsigned(arg1), encodeFloat(val1 / val2));

        break;
    }
    case 96: //div float & &
    {
        _Float16 val1 = parseFloat(valueAtNestedAddress(signedToUnsigned(arg1)));
        _Float16 val2 = parseFloat(valueAtNestedAddress(signedToUnsigned(arg2)));

        setDivFlagsFloat(val1, val2);
        
        setValueAtNestedAddress(signedToUnsigned(arg1), encodeFloat(val1 / val2));

        break;
    }
    case 97: //xor $ imm
    {
        int16_t val1 = valueAtAddress(signedToUnsigned(arg1));
        int16_t val2 = arg2;

        setValueAtAddress(signedToUnsigned(arg1), val1 ^ val2);

        break;
    }
    case 98: //xor $ $
    {
        int16_t val1 = valueAtAddress(signedToUnsigned(arg1));
        int16_t val2 = valueAtAddress(signedToUnsigned(arg2));

        setValueAtAddress(signedToUnsigned(arg1), val1 ^ val2);

        break;
    }
    case 99: //xor $ &
    {
        int16_t val1 = valueAtAddress(signedToUnsigned(arg1));
        int16_t val2 = valueAtNestedAddress(signedToUnsigned(arg2));

        setValueAtAddress(signedToUnsigned(arg1), val1 ^ val2);

        break;
    }
    case 100: //xor & imm
    {
        int16_t val1 = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t val2 = arg2;

        setValueAtNestedAddress(signedToUnsigned(arg1), val1 ^ val2);

        break;
    }
    case 101: //xor & $
    {
        int16_t val1 = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t val2 = valueAtAddress(signedToUnsigned(arg2));

        setValueAtNestedAddress(signedToUnsigned(arg1), val1 ^ val2);

        break;
    }
    case 102: //xor & &
    {
        int16_t val1 = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t val2 = valueAtNestedAddress(signedToUnsigned(arg2));

        setValueAtNestedAddress(signedToUnsigned(arg1), val1 ^ val2);

        break;
    }
    case 103: //or $ imm
    {
        int16_t val1 = valueAtAddress(signedToUnsigned(arg1));
        int16_t val2 = arg2;

        setValueAtAddress(signedToUnsigned(arg1), val1 | val2);

        break;
    }
    case 104: //or $ $
    {
        int16_t val1 = valueAtAddress(signedToUnsigned(arg1));
        int16_t val2 = valueAtAddress(signedToUnsigned(arg2));

        setValueAtAddress(signedToUnsigned(arg1), val1 | val2);

        break;
    }
    case 105: //or $ &
    {
        int16_t val1 = valueAtAddress(signedToUnsigned(arg1));
        int16_t val2 = valueAtNestedAddress(signedToUnsigned(arg2));

        setValueAtAddress(signedToUnsigned(arg1), val1 | val2);

        break;
    }
    case 106: //or & imm
    {
        int16_t val1 = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t val2 = arg2;

        setValueAtNestedAddress(signedToUnsigned(arg1), val1 | val2);

        break;
    }
    case 107: //or & $
    {
        int16_t val1 = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t val2 = valueAtAddress(signedToUnsigned(arg2));

        setValueAtNestedAddress(signedToUnsigned(arg1), val1 | val2);

        break;
    }
    case 108: //or & &
    {
        int16_t val1 = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t val2 = valueAtNestedAddress(signedToUnsigned(arg2));

        setValueAtNestedAddress(signedToUnsigned(arg1), val1 | val2);

        break;
    }
    case 109: //and $ imm
    {
        int16_t val1 = valueAtAddress(signedToUnsigned(arg1));
        int16_t val2 = arg2;

        setValueAtAddress(signedToUnsigned(arg1), val1 & val2);

        break;
    }
    case 110: //and $ $
    {
        int16_t val1 = valueAtAddress(signedToUnsigned(arg1));
        int16_t val2 = valueAtAddress(signedToUnsigned(arg2));

        setValueAtAddress(signedToUnsigned(arg1), val1 & val2);

        break;
    }
    case 111: //and $ &
    {
        int16_t val1 = valueAtAddress(signedToUnsigned(arg1));
        int16_t val2 = valueAtNestedAddress(signedToUnsigned(arg2));

        setValueAtAddress(signedToUnsigned(arg1), val1 & val2);

        break;
    }
    case 112: //and & imm
    {
        int16_t val1 = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t val2 = arg2;

        setValueAtNestedAddress(signedToUnsigned(arg1), val1 & val2);

        break;
    }
    case 113: //and & $
    {
        int16_t val1 = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t val2 = valueAtAddress(signedToUnsigned(arg2));

        setValueAtNestedAddress(signedToUnsigned(arg1), val1 & val2);

        break;
    }
    case 114: //and & &
    {
        int16_t val1 = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t val2 = valueAtNestedAddress(signedToUnsigned(arg2));

        setValueAtNestedAddress(signedToUnsigned(arg1), val1 & val2);

        break;
    }
    case 115: //not $
    {
        int16_t val1 = valueAtAddress(signedToUnsigned(arg1));

        setValueAtAddress(signedToUnsigned(arg1), ~val1);

        break;
    }
    case 116: //not &
    {
        int16_t val1 = valueAtAddress(signedToUnsigned(arg1));

        setValueAtAddress(signedToUnsigned(arg1), ~val1);

        break;
    }
    case 117: //arithmetic right $ imm
    {

        int16_t val = valueAtAddress(signedToUnsigned(arg1));
        int16_t count = arg2;

        bool sign = val < 0;

        val = val >> count;

        if(sign == true)
        {
            val = val | 0b1000000000000000;
        }

        setValueAtAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 118: //arithmetic right $ $
    {

        int16_t val = valueAtAddress(signedToUnsigned(arg1));
        int16_t count = valueAtAddress(signedToUnsigned(arg2));

        bool sign = val < 0;

        val = val >> count;

        if(sign == true)
        {
            val = val | 0b1000000000000000;
        }

        setValueAtAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 119: //arithmetic right $ &
    {

        int16_t val = valueAtAddress(signedToUnsigned(arg1));
        int16_t count = valueAtNestedAddress(signedToUnsigned(arg2));

        bool sign = val < 0;

        val = val >> count;

        if(sign == true)
        {
            val = val | 0b1000000000000000;
        }

        setValueAtAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 120: //arithmetic right & imm
    {

        int16_t val = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t count = arg2;

        bool sign = val < 0;

        val = val >> count;

        if(sign == true)
        {
            val = val | 0b1000000000000000;
        }

        setValueAtNestedAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 121: //arithmetic right & $
    {

        int16_t val = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t count = valueAtAddress(signedToUnsigned(arg2));

        bool sign = val < 0;

        val = val >> count;

        if(sign == true)
        {
            val = val | 0b1000000000000000;
        }

        setValueAtNestedAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 122: //arithmetic right & &
    {

        int16_t val = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t count = valueAtNestedAddress(signedToUnsigned(arg2));

        bool sign = val < 0;

        val = val >> count;

        if(sign == true)
        {
            val = val | 0b1000000000000000;
        }

        setValueAtNestedAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 123: //arithmetic left $ imm
    {

        int16_t val = valueAtAddress(signedToUnsigned(arg1));
        int16_t count = arg2;

        val = val << count;

        setValueAtAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 124: //arithmetic left $ $
    {

        int16_t val = valueAtAddress(signedToUnsigned(arg1));
        int16_t count = valueAtAddress(signedToUnsigned(arg2));

        val = val << count;

        setValueAtAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 125: //arithmetic left $ &
    {

        int16_t val = valueAtAddress(signedToUnsigned(arg1));
        int16_t count = valueAtNestedAddress(signedToUnsigned(arg2));

        val = val << count;

        setValueAtAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 126: //arithmetic left & imm
    {

        int16_t val = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t count = arg2;

        val = val << count;

        setValueAtNestedAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 127: //arithmetic left & $
    {

        int16_t val = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t count = valueAtAddress(signedToUnsigned(arg2));

        val = val << count;

        setValueAtNestedAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 128: //arithmetic left & &
    {

        int16_t val = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t count = valueAtNestedAddress(signedToUnsigned(arg2));

        val = val << count;

        setValueAtNestedAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 129: //logical right $ imm
    {

        int16_t val = valueAtAddress(signedToUnsigned(arg1));
        int16_t count = arg2;

        val = (val >> count) & 0b0111111111111111;

        setValueAtAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 130: //logical right $ $
    {

        int16_t val = valueAtAddress(signedToUnsigned(arg1));
        int16_t count = valueAtAddress(signedToUnsigned(arg2));

        val = (val >> count) & 0b0111111111111111;

        setValueAtAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 131: //logical right $ &
    {

        int16_t val = valueAtAddress(signedToUnsigned(arg1));
        int16_t count = valueAtNestedAddress(signedToUnsigned(arg2));

        val = (val >> count) & 0b0111111111111111;

        setValueAtAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 132: //logical right & imm
    {

        int16_t val = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t count = arg2;

        val = (val >> count) & 0b0111111111111111;

        setValueAtNestedAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 133: //logical right & $
    {

        int16_t val = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t count = valueAtAddress(signedToUnsigned(arg2));

        val = (val >> count) & 0b0111111111111111;

        setValueAtNestedAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 134: //logical right & &
    {

        int16_t val = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t count = valueAtNestedAddress(signedToUnsigned(arg2));

        val = (val >> count) & 0b0111111111111111;

        setValueAtNestedAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 135: //logical left $ imm
    {

        int16_t val = valueAtAddress(signedToUnsigned(arg1));
        int16_t count = arg2;

        val = (val << count) & 0b1111111111111110;

        setValueAtAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 136: //logical left $ $
    {

        int16_t val = valueAtAddress(signedToUnsigned(arg1));
        int16_t count = valueAtAddress(signedToUnsigned(arg2));

        val = (val << count) & 0b1111111111111110;

        setValueAtAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 137: //logical left $ &
    {

        int16_t val = valueAtAddress(signedToUnsigned(arg1));
        int16_t count = valueAtNestedAddress(signedToUnsigned(arg2));

        val = (val << count) & 0b1111111111111110;

        setValueAtAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 138: //logical left & imm
    {

        int16_t val = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t count = arg2;

        val = (val << count) & 0b1111111111111110;

        setValueAtNestedAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 139: //logical left & $
    {

        int16_t val = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t count = valueAtAddress(signedToUnsigned(arg2));

        val = (val << count) & 0b1111111111111110;

        setValueAtNestedAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 140: //logical left & &
    {

        int16_t val = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t count = valueAtNestedAddress(signedToUnsigned(arg2));

        val = (val << count) & 0b1111111111111110;

        setValueAtNestedAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 141: //rotate right $ imm
    {
        int16_t val = valueAtAddress(signedToUnsigned(arg1));
        int16_t count = arg2;

        int16_t newBit = val << 15;

        val = (val >> count) | newBit;

        setValueAtAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 142: //rotate right $ $
    {
        int16_t val = valueAtAddress(signedToUnsigned(arg1));
        int16_t count = valueAtAddress(signedToUnsigned(arg2));

        int16_t newBit = val << 15;

        val = (val >> count) | newBit;

        setValueAtAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 143: //rotate right $ &
    {
        int16_t val = valueAtAddress(signedToUnsigned(arg1));
        int16_t count = valueAtAddress(signedToUnsigned(arg2));

        int16_t newBit = val << 15;

        val = (val >> count) | newBit;

        setValueAtAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 144: //rotate right & imm
    {
        int16_t val = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t count = arg2;

        int16_t newBit = val << 15;

        val = (val >> count) | newBit;

        setValueAtNestedAddress(signedToUnsigned(arg1), val);

        break;
        break;
    }
    case 145: //rotate right & $
    {
        int16_t val = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t count = valueAtAddress(signedToUnsigned(arg2));

        int16_t newBit = val << 15;

        val = (val >> count) | newBit;

        setValueAtNestedAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 146: //rotate right & &
    {
        int16_t val = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t count = valueAtAddress(signedToUnsigned(arg2));

        int16_t newBit = val << 15;

        val = (val >> count) | newBit;

        setValueAtNestedAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 147: //rotate left $ imm
    {
        int16_t val = valueAtAddress(signedToUnsigned(arg1));
        int16_t count = arg2;

        int16_t newBit = val >> 15;

        val = (val << count) | newBit;

        setValueAtAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 148: //rotate left $ $
    {
        int16_t val = valueAtAddress(signedToUnsigned(arg1));
        int16_t count = valueAtAddress(signedToUnsigned(arg2));

        int16_t newBit = val >> 15;

        val = (val << count) | newBit;

        setValueAtAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 149: //rotate left $ &
    {
        int16_t val = valueAtAddress(signedToUnsigned(arg1));
        int16_t count = valueAtAddress(signedToUnsigned(arg2));

        int16_t newBit = val >> 15;

        val = (val << count) | newBit;

        setValueAtAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 150: //rotate left & imm
    {
        int16_t val = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t count = arg2;

        int16_t newBit = val >> 15;

        val = (val << count) | newBit;

        setValueAtNestedAddress(signedToUnsigned(arg1), val);

        break;
        break;
    }
    case 151: //rotate left & $
    {
        int16_t val = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t count = valueAtAddress(signedToUnsigned(arg2));

        int16_t newBit = val >> 15;

        val = (val << count) | newBit;

        setValueAtNestedAddress(signedToUnsigned(arg1), val);

        break;
    }
    case 152: //rotate left & &
    {
        int16_t val = valueAtNestedAddress(signedToUnsigned(arg1));
        int16_t count = valueAtAddress(signedToUnsigned(arg2));

        int16_t newBit = val >> 15;

        val = (val << count) | newBit;

        setValueAtNestedAddress(signedToUnsigned(arg1), val);

        break;
    }

    case 153: //push $
    {
        stack[stackPointer] = valueAtAddress(signedToUnsigned(arg1));
        stackPointer += 1;
        break;
    }
    case 154: //push &
    {
        stack[stackPointer] = valueAtNestedAddress(signedToUnsigned(arg1));
        stackPointer += 1;
        break;
    }
    case 155: //push imm
    {
        stack[stackPointer] = arg1;
        stackPointer += 1;
        break;
    }

    case 156: //pop $
    {
        stackPointer -= 1;
        setValueAtAddress(signedToUnsigned(arg1), stack[stackPointer]);
        break;
    }
    case 157: //pop &
    {
        stackPointer -= 1;
        setValueAtNestedAddress(signedToUnsigned(arg1), stack[stackPointer]);
        break;
    }

    case 158: //branch unconditional
    {
        programCounter += (arg1 - 3);
        break;
    }
    case 159: //branch positive
    {
        if((flagRegister & 0x4 )== 0 && (flagRegister & 0x2 )== 0)
        {
            programCounter += (arg1 - 3);
        }
        break;
    }
    case 160: //branch zero
    {
        if(flagRegister & 0x2)
        {
            programCounter += (arg1 - 3);
        }
        break;
    }
    case 161: //branch negative
    {
        if(flagRegister & 0x4)
        {
            programCounter += (arg1 - 3);
        }
        break;
    }

    case 162: //jump unconditional
    {
        programCounter = signedToUnsigned(arg1);
        break;
    }
    case 163: //jump positive
    {
        if((flagRegister & 0x4 )== 0)
        {
            programCounter = signedToUnsigned(arg1);
        }
        break;
    }
    case 164: //jump zero
    {
        if(flagRegister & 0x2)
        {
            programCounter = signedToUnsigned(arg1);
        }
        break;
    }
    case 165: //jump negative
    {
        if(flagRegister & 0x4)
        {
            programCounter = signedToUnsigned(arg1);
        }
        break;
    }

    case 166: //call offset 
    {
        callStack[callStackPointer] = (programCounter);
        callStackPointer += 1;

        programCounter += (arg1 - 3);
        break;
    }

    case 167://output to imm imm
        if(arg1 == 1)
        {
            TAPE1.write(arg2);
        }
        if(arg1 == 2)
        {
            TAPE2.write(arg2);
        }
        if(arg1 == 4)
        {
            terminalOutput(signedToUnsigned(arg2));
        }
        break;
    case 168://output to imm $
        if(arg1 == 1)
        {
            TAPE1.write(valueAtAddress(signedToUnsigned(arg2)));
        }
        if(arg1 == 2)
        {
            TAPE2.write(valueAtAddress(signedToUnsigned(arg2)));
        }
        if(arg1 == 4)
        {
            terminalOutput(signedToUnsigned(valueAtAddress(signedToUnsigned(arg2))));
        }
        break;
    case 169://output to imm &
        if(arg1 == 1)
        {
            TAPE1.write(valueAtNestedAddress(signedToUnsigned(arg2)));
        }
        if(arg1 == 2)
        {
            TAPE2.write(valueAtNestedAddress(signedToUnsigned(arg2)));
        }
        if(arg1 == 4)
        {
            terminalOutput(signedToUnsigned(valueAtNestedAddress(signedToUnsigned(arg2))));
        }
        break;

    case 170: //input from imm $
    {
        if(arg1 == 0)
        {
            setValueAtAddress(signedToUnsigned(arg2), kbin);
            return true;
        }
        if(arg1 == 1)
        {
            setValueAtAddress(signedToUnsigned(arg2), TAPE1.read());
        }
        if(arg1 == 2)
        {
            setValueAtAddress(signedToUnsigned(arg2), TAPE2.read());
        }
        if(arg1 == 5)
        {
            setValueAtAddress(signedToUnsigned(arg2), joyport1);
        }
        if(arg1 == 6)
        {
            setValueAtAddress(signedToUnsigned(arg2), joyport2);
        }
        break;
    }
    case 171: //input from imm &
    {
        if(arg1 == 0)
        {
            setValueAtNestedAddress(signedToUnsigned(arg2), kbin);
            return true;
        }
        if(arg1 == 1)
        {
            setValueAtNestedAddress(signedToUnsigned(arg2), TAPE1.read());
        }
        if(arg1 == 2)
        {
            setValueAtNestedAddress(signedToUnsigned(arg2), TAPE2.read());
        }
        if(arg1 == 5)
        {
            setValueAtNestedAddress(signedToUnsigned(arg2), joyport1);
        }
        if(arg1 == 6)
        {
            setValueAtNestedAddress(signedToUnsigned(arg2), joyport2);
        }
        break;
    }

    case 172: //return
    {
        callStackPointer -= 1;
        programCounter = callStack[callStackPointer];
        break;
    }

    case 173: //WAIT FOR VBLANK
    {
        flagRegister = flagRegister | 16;
        break;
    }

    case 174: //call jump 
    {
        callStack[callStackPointer] = (programCounter);
        callStackPointer += 1;

        programCounter = arg1;
        break;
    }

    case 175:
    {
        SoundChip.SetSoundChipParams(signedToUnsigned(arg1), signedToUnsigned(arg2));
    }

    case 176: //cos add imm
    {
        _Float16 val1 = parseFloat(arg2);
        
        _Float16 res = cosf(val1);

        setValueAtAddress(signedToUnsigned(arg1), encodeFloat(res));

        break;
    }
    case 177: //cos add add
    {
        _Float16 val1 = parseFloat(valueAtAddress(signedToUnsigned(arg2)));
        
        _Float16 res = cosf(val1);

        setValueAtAddress(signedToUnsigned(arg1), encodeFloat(res));

        break;
    }
    case 178: //cos add nest
    {
        _Float16 val1 = parseFloat(valueAtNestedAddress(signedToUnsigned(arg2)));
        
        _Float16 res = cosf(val1);

        setValueAtAddress(signedToUnsigned(arg1), encodeFloat(res));

        break;
    }

    case 179: //cos nest imm
    {
        _Float16 val1 = parseFloat(arg2);
        
        _Float16 res = cosf(val1);

        setValueAtNestedAddress(signedToUnsigned(arg1), encodeFloat(res));

        break;
    }
    case 180: //cos nest add
    {
        _Float16 val1 = parseFloat(valueAtAddress(signedToUnsigned(arg2)));
        
        _Float16 res = cosf(val1);

        setValueAtNestedAddress(signedToUnsigned(arg1), encodeFloat(res));

        break;
    }
    case 181: //cos nest nest
    {
        _Float16 val1 = parseFloat(valueAtNestedAddress(signedToUnsigned(arg2)));
        
        _Float16 res = cosf(val1);

        setValueAtNestedAddress(signedToUnsigned(arg1), encodeFloat(res));

        break;
    }

    case 182: //sin add imm
    {
        _Float16 val1 = parseFloat(arg2);
        
        _Float16 res = sinf(val1);

        setValueAtAddress(signedToUnsigned(arg1), encodeFloat(res));

        break;
    }
    case 183: //sin add add
    {
        _Float16 val1 = parseFloat(valueAtAddress(signedToUnsigned(arg2)));
        
        _Float16 res = sinf(val1);

        setValueAtAddress(signedToUnsigned(arg1), encodeFloat(res));

        break;
    }
    case 184: //sin add nest
    {
        _Float16 val1 = parseFloat(valueAtNestedAddress(signedToUnsigned(arg2)));
        
        _Float16 res = sinf(val1);

        setValueAtAddress(signedToUnsigned(arg1), encodeFloat(res));

        break;
    }

    case 185: //sin nest imm
    {
        _Float16 val1 = parseFloat(arg2);
        
        _Float16 res = sinf(val1);

        setValueAtNestedAddress(signedToUnsigned(arg1), encodeFloat(res));

        break;
    }
    case 186: //sin nest add
    {
        _Float16 val1 = parseFloat(valueAtAddress(signedToUnsigned(arg2)));
        
        _Float16 res = sinf(val1);

        setValueAtNestedAddress(signedToUnsigned(arg1), encodeFloat(res));

        break;
    }
    case 187: //sin nest nest
    {
        _Float16 val1 = parseFloat(valueAtNestedAddress(signedToUnsigned(arg2)));
        
        _Float16 res = sinf(val1);

        setValueAtNestedAddress(signedToUnsigned(arg1), encodeFloat(res));

        break;
    }

    default:
        break;
    }

    return false;
}

int16_t JoystickInput(int port)
{
    int16_t data = 0;
    if(port == 5)
    {
        if(IsGamepadAvailable(0))
        {
            if(GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) > 0.0f)
            {
                data = data | 2;
            }
            else if(GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) < 0.0f)
            {
                data = data | 1;
            }

            if(GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) > 0.0f)
            {
                data = data | 8;
            }
            else if(GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) < 0.0f)
            {
                data = data | 4;
            }

            if(IsGamepadButtonPressed(1, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
            {
                data = data | 16;
            }
        }
        
        return data;
    }
    else if(port == 6)
    {
        if(IsGamepadAvailable(1))
        {
            if(GetGamepadAxisMovement(1, GAMEPAD_AXIS_LEFT_Y) > 0.0f)
            {
                data = data | 2;
            }
            else if(GetGamepadAxisMovement(1, GAMEPAD_AXIS_LEFT_Y) < 0.0f)
            {
                data = data | 1;
            }

            if(GetGamepadAxisMovement(1, GAMEPAD_AXIS_LEFT_X) > 0.0f)
            {
                data = data | 8;
            }
            else if(GetGamepadAxisMovement(1, GAMEPAD_AXIS_LEFT_X) < 0.0f)
            {
                data = data | 4;
            }

            if(IsGamepadButtonPressed(1, GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
            {
                data = data | 16;
            }
        }
        
        return data;
    }

    return data;
}

int main()
{
    //screen width and height
    const int screenWidth = 1300;
    const int screenHeight = 800;
    
    //initialization
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "ACID 700");
    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(MAX_SAMPLES_PER_UPDATE);
    SetTargetFPS(60);
    srand(time(0));

    tapeImage = LoadTexture("assets/tape.png");

    VOICE0Wave.stream = LoadAudioStream(sampleRate, 16, 1);
    SetAudioStreamCallback(VOICE0Wave.stream, VOICE0InputCallback);
    PlayAudioStream(VOICE0Wave.stream);

    VOICE1Wave.stream = LoadAudioStream(sampleRate, 16, 1);
    SetAudioStreamCallback(VOICE1Wave.stream, VOICE1InputCallback);
    PlayAudioStream(VOICE1Wave.stream);

    VOICE2Wave.stream = LoadAudioStream(sampleRate, 16, 1);
    SetAudioStreamCallback(VOICE2Wave.stream, VOICE2InputCallback);
    PlayAudioStream(VOICE2Wave.stream);


    //Buttons for controlling the primary tape drive
        Button rewindTape1Button = Button(1065, 10, 192, 30, "REWIND TAPE");

        Button forwardByte4Tape1 = Button(1091, 192, 20, 20, "+");
        Button forwardByte3Tape1 = Button(1131, 192, 20, 20, "+");
        Button forwardByte2Tape1 = Button(1171, 192, 20, 20, "+");
        Button forwardByte1Tape1 = Button(1211, 192, 20, 20, "+");

        Button backByte4Tape1 = Button(1091, 252, 20, 20, "-");
        Button backByte3Tape1 = Button(1131, 252, 20, 20, "-");
        Button backByte2Tape1 = Button(1171, 252, 20, 20, "-");
        Button backByte1Tape1 = Button(1211, 252, 20, 20, "-");
    //Rectangle for displaying programcounter
        Button pcDisplay = Button(1065, 350, 192, 30, "");
    //Buttons for controlling the secondary tape drive
        Button rewindTape2Button = Button(1065, 430, 192, 30, "REWIND TAPE");

        Button forwardByte4Tape2 = Button(1091, 612, 20, 20, "+");
        Button forwardByte3Tape2 = Button(1131, 612, 20, 20, "+");
        Button forwardByte2Tape2 = Button(1171, 612, 20, 20, "+");
        Button forwardByte1Tape2 = Button(1211, 612, 20, 20, "+");

        Button backByte4Tape2 = Button(1091, 672, 20, 20, "-");
        Button backByte3Tape2 = Button(1131, 672, 20, 20, "-");
        Button backByte2Tape2 = Button(1171, 672, 20, 20, "-");
        Button backByte1Tape2 = Button(1211, 672, 20, 20, "-");
    //

    //load config
        std::string rompath;
        std::string tape1path;
        std::string tape2path;
        std::ifstream configFile;
        configFile.open("system/config.txt");
        if(configFile.is_open())
        {
            std::getline(configFile, rompath);
            std::getline(configFile, tape1path);
            std::getline(configFile, tape2path);
        }
        configFile.close();
    //
    
    //Load ROM

        FILE *rom = fopen(rompath.c_str(),"r");
        std::vector<int16_t> unprocessedBytes;
        int16_t b;

        if(rom != NULL)
        {
            while ((b = getc(rom)) != EOF)
            {
                unprocessedBytes.push_back(b);
            }
            fclose(rom);
        }

        for (int i = 0; i < (int)unprocessedBytes.size()/2; i++)
        {
            int16_t processedValue = (unprocessedBytes[i*2] << 8) + (unprocessedBytes[i*2 + 1]);
            ROM[i] = processedValue;
        }
    
    //load tapes
        FILE *tapefile = fopen(tape1path.c_str(),"r");
        unprocessedBytes.clear();
        if(unprocessedBytes.empty() != true)
        {
            unprocessedBytes.clear();
        }
        b = 0;

        if(tapefile != NULL)
        {
            while ((b = getc(tapefile)) != EOF)
            {
                unprocessedBytes.push_back(b);
            }
            fclose(tapefile);
        }

        for (int i = 0; i < (int)unprocessedBytes.size()/2; i++)
        {
            int16_t processedValue = (unprocessedBytes[i*2] << 8) + (unprocessedBytes[i*2 + 1]);
            TAPE1.TAPEDATA[i] = processedValue;
        }

    //--//
        
        tapefile = fopen(tape2path.c_str(),"r");
        unprocessedBytes.clear();
        if(unprocessedBytes.empty() != true)
        {
            unprocessedBytes.clear();
        }
        b = 0;

        if(tapefile != NULL)
        {
            while ((b = getc(tapefile)) != EOF)
            {
               unprocessedBytes.push_back(b);
            }
            fclose(tapefile);
        }

        for (int i = 0; i < (int)unprocessedBytes.size()/2; i++)
        {
            int16_t processedValue = (unprocessedBytes[i*2] << 8) + (unprocessedBytes[i*2 + 1]);
            TAPE2.TAPEDATA[i] = processedValue;
        }
    //

    struct{
        int16_t op;
        int16_t a1;
        int16_t a2;
    } decodedInstruction;
    
    int keyBoardInput = 0;


    //huvudloop
    while (!WindowShouldClose())
    {

        //std::cout << (float)parseFloat(valueAtAddress(0x2F58)) << "  "<< (float)parseFloat(valueAtAddress(0x2F59)) << "  "<< (float)parseFloat(valueAtAddress(0x2F5A)) << "\n";

        flagRegister = flagRegister & 0b1111111111101111;

        int oldInput = keyBoardInput;
        keyBoardInput = GetKeyPressed();

        if(IsKeyDown(oldInput)){keyBoardInput = oldInput;}
        
        int16_t port5 = JoystickInput(5);
        int16_t port6 = JoystickInput(6);

        
        SoundChip.UpdateSounds();


        for (uint16_t c = 0; c < 65500; c++)
        {
            if(flagRegister & 16){continue;}

            int16_t oldVideoMode = flagRegister & 40;
            decodedInstruction.op = valueAtAddress(programCounter);
            decodedInstruction.a1 = valueAtAddress(programCounter + 1);
            decodedInstruction.a2 = valueAtAddress(programCounter + 2);


            programCounter += 3;
            
            bool readKey = executeInstruction(decodedInstruction.op, decodedInstruction.a1, decodedInstruction.a2, keyBoardInput, port5, port6);
            //if(readKey == true){keyBoardInput = 0;}

            if((flagRegister & 40) != oldVideoMode)
            {
                for (int y = 0; y < 31; y++)
                {
                    for (int x = 0; x < 40; x++)
                    {
                        terminal[y][x] = ' ';
                    }
                    
                }

                cursorPosition = 0;

                for (int y = 0; y < 80; y++)
                {
                    for (int x = 0; x < 100; x++)
                    {
                        RAM[x + y*100] = 0;
                    }
                    
                }
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);
        if((flagRegister & 32) && (flagRegister & 8))
        {
            
            for (int y = 0; y < 70; y++)
            {
                for (int x = 0; x < 100; x++)
                {
                    DrawRectangle(10 + x * 10, y * 10, 10, 10, color565Converter(signedToUnsigned(RAM[x + y*100])));
                }
                
            }

            for (int y = 28; y < 31; y++)
            {
                for (int x = 0; x < 40; x++)
                {
                    //TextFormat("%c", terminal[y][x])
                    DrawText(TextFormat("%c", terminal[y][x]), 5 + x * 25, 10 + y * 25, 25, WHITE);
                }
                
            }
        }
        else if(flagRegister & 8)
        {
            for (int y = 0; y < 80; y++)
            {
                for (int x = 0; x < 100; x++)
                {
                    DrawRectangle(10 + x * 10, y * 10, 10, 10, color565Converter(signedToUnsigned(RAM[x + y*100])));
                }
                
            }
            
        }
        else
        {
            for (int y = 0; y < 31; y++)
            {
                for (int x = 0; x < 40; x++)
                {
                    //TextFormat("%c", terminal[y][x])
                    DrawText(TextFormat("%c", terminal[y][x]), 5 + x * 25, 10 + y * 25, 25, WHITE);
                }
                
            }
        }


        //Sidepanel for tapes and such
            Vector2 mouseposition = GetMousePosition();
            DrawRectangle(1020, 0, 280, 800, GRAY);
            DrawButton(rewindTape1Button);
            if(clickedOnButton(rewindTape1Button, mouseposition) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){TAPE1.rewind();}

            DrawButton(forwardByte4Tape1);
            if(clickedOnButton(forwardByte4Tape1, mouseposition) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){TAPE1.progress += 0x1000;}
            DrawButton(forwardByte3Tape1);
            if(clickedOnButton(forwardByte3Tape1, mouseposition) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){TAPE1.progress += 0x0100;}
            DrawButton(forwardByte2Tape1);
            if(clickedOnButton(forwardByte2Tape1, mouseposition) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){TAPE1.progress += 0x0010;}
            DrawButton(forwardByte1Tape1);
            if(clickedOnButton(forwardByte1Tape1, mouseposition) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){TAPE1.progress += 0x0001;}

            DrawText(TextFormat("%X", (TAPE1.progress >> 12) & 0xF), forwardByte4Tape1.posX + 4, forwardByte4Tape1.posY  + 30, 24, WHITE);
            DrawText(TextFormat("%X", (TAPE1.progress >> 8) & 0xF), forwardByte3Tape1.posX + 4, forwardByte3Tape1.posY  + 30, 24, WHITE);
            DrawText(TextFormat("%X", (TAPE1.progress >> 4) & 0xF), forwardByte2Tape1.posX + 4, forwardByte2Tape1.posY  + 30, 24, WHITE);
            DrawText(TextFormat("%X", TAPE1.progress & 0xF), forwardByte1Tape1.posX + 4, forwardByte1Tape1.posY  + 30, 24, WHITE);

            DrawButton(backByte4Tape1);
            if(clickedOnButton(backByte4Tape1, mouseposition) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){TAPE1.progress -= 0x1000;}
            DrawButton(backByte3Tape1);
            if(clickedOnButton(backByte3Tape1, mouseposition) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){TAPE1.progress -= 0x0100;}
            DrawButton(backByte2Tape1);
            if(clickedOnButton(backByte2Tape1, mouseposition) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){TAPE1.progress -= 0x0010;}
            DrawButton(backByte1Tape1);
            if(clickedOnButton(backByte1Tape1, mouseposition) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){TAPE1.progress -= 0x0001;}

            DrawTexturePro(tapeImage, {0,0,96,64},{1065, 50, 192, 128},{0,0}, 0.0f, WHITE);
        ////////////////////////////////////////////////////////////
            DrawButton(rewindTape2Button);
            if(clickedOnButton(rewindTape2Button, mouseposition) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){TAPE2.rewind();}

            DrawButton(forwardByte4Tape2);
            if(clickedOnButton(forwardByte4Tape2, mouseposition) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){TAPE2.progress += 0x1000;}
            DrawButton(forwardByte3Tape2);
            if(clickedOnButton(forwardByte3Tape2, mouseposition) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){TAPE2.progress += 0x0100;}
            DrawButton(forwardByte2Tape2);
            if(clickedOnButton(forwardByte2Tape2, mouseposition) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){TAPE2.progress += 0x0010;}
            DrawButton(forwardByte1Tape2);
            if(clickedOnButton(forwardByte1Tape2, mouseposition) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){TAPE2.progress += 0x0001;}

            DrawText(TextFormat("%X", (TAPE2.progress >> 12) & 0xF), forwardByte4Tape2.posX + 4, forwardByte4Tape2.posY  + 30, 24, WHITE);
            DrawText(TextFormat("%X", (TAPE2.progress >> 8) & 0xF), forwardByte3Tape2.posX + 4, forwardByte3Tape2.posY  + 30, 24, WHITE);
            DrawText(TextFormat("%X", (TAPE2.progress >> 4) & 0xF), forwardByte2Tape2.posX + 4, forwardByte2Tape2.posY  + 30, 24, WHITE);
            DrawText(TextFormat("%X", TAPE2.progress & 0xF), forwardByte1Tape2.posX + 4, forwardByte1Tape2.posY  + 30, 24, WHITE);

            DrawButton(backByte4Tape2);
            if(clickedOnButton(backByte4Tape2, mouseposition) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){TAPE2.progress -= 0x1000;}
            DrawButton(backByte3Tape2);
            if(clickedOnButton(backByte3Tape2, mouseposition) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){TAPE2.progress -= 0x0100;}
            DrawButton(backByte2Tape2);
            if(clickedOnButton(backByte2Tape2, mouseposition) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){TAPE2.progress -= 0x0010;}
            DrawButton(backByte1Tape2);
            if(clickedOnButton(backByte1Tape2, mouseposition) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){TAPE2.progress -= 0x0001;}

            DrawTexturePro(tapeImage, {0,0,96,64},{1065, 470, 192, 128},{0,0}, 0.0f, WHITE);

        DrawFPS(1220,770);
        EndDrawing();
    }

    UnloadTexture(tapeImage);

    UnloadAudioStream(VOICE0Wave.stream);
    UnloadAudioStream(VOICE1Wave.stream);
    UnloadAudioStream(VOICE2Wave.stream);

    //Writing the tapes back to file for safe storage :)
    std::ofstream outputStream;

    outputStream.open("system/tape1.bin", std::ofstream::out);
    if(outputStream.is_open())
    {
        for(int i = 0; i < 65356; i++)
        {   
            char highBits = (TAPE1.TAPEDATA[i] >> 8) & 0xFF;
            char lowBits = TAPE1.TAPEDATA[i] & 0xFF;

            outputStream<<highBits;
            outputStream<<lowBits;
        }
        outputStream.close();
    }

    outputStream.open("system/tape2.bin", std::ofstream::out);
    if(outputStream.is_open())
    {
        for(int i = 0; i < 65356; i++)
        {   
            char highBits = (TAPE2.TAPEDATA[i] >> 8) & 0xFF;
            char lowBits = TAPE2.TAPEDATA[i] & 0xFF;

            outputStream<<highBits;
            outputStream<<lowBits;
        }
        outputStream.close();
    }

    CloseWindow();
    
}

