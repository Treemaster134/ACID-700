#include <cstdint>
#include <stdfloat>
#include "raylib.h"

struct Tape{
    int16_t TAPEDATA[65356];
    uint16_t progress = 0;

    void rewind()
    {
        progress = 0;
    }

    int16_t read()
    {
        return TAPEDATA[progress++];
    }

    void write(int16_t data)
    {
        TAPEDATA[progress++] = data;
    }
};

struct Button{
    int posX = 0;
    int posY = 0;

    int width = 0;
    int height = 0;

    std::string text;

    Button(int _posx, int _posy, int _width, int _height, std::string _text)
    {
        posX = _posx;
        posY = _posy;

        width = _width;
        height = _height;

        text = _text;
    }
};

void DrawButton(Button b)
{
    DrawRectangle(b.posX, b.posY, b.width, b.height, DARKGRAY);
    DrawRectangleLines(b.posX, b.posY, b.width, b.height, WHITE);

    Vector2 textWidth = MeasureTextEx(GetFontDefault(), b.text.c_str(), 24, 2.4f);
    DrawText(b.text.c_str(), b.posX + (b.width/2 - textWidth.x/2), b.posY + (b.height/2 - textWidth.y/2), 24, WHITE); 
}

bool clickedOnButton(Button b, Vector2 mPos)
{
    if(mPos.x > b.posX && mPos.x < (b.posX + b.width))
    {
        if(mPos.y > b.posY && mPos.y < (b.posY + b.height))
        {
            return true;
        }
    }
    return false;
}

uint16_t signedToUnsigned(int16_t signedNum)
{
    return (uint16_t)signedNum;
}

_Float16 parseFloat(int16_t i)
{
    _Float16 convertedFloat = 0;
    memcpy(&convertedFloat, &i, sizeof(convertedFloat));
    return convertedFloat;
}

int16_t encodeFloat(_Float16 f)
{
    int16_t convertedInt = 0;
    memcpy(&convertedInt, &f, sizeof(convertedInt));
    return convertedInt;
}

int16_t floatToInt(_Float16 f)
{
    return (int16_t)f;
}

_Float16 intToFloat(int16_t i)
{
    return (_Float16)i;
}

Color color565Converter(uint16_t col)
{
    Color newCol;
    newCol.a = 255;
    newCol.r = 255 * (float)(col & 31)/31.0f;
    newCol.g = 255 * (float)((col & 2016)>>5)/63.0f;
    newCol.b = 255 * (float)((col & 63488)>>11)/31.0f;

    return newCol;
}


enum COLORTEMPLATES{
    RED565 = 0b1111100000000000,
    GREEN565 = 0b0000011111100000,
    BLUE565 = 0b0000000000011111,
    MAGENTA565 = 0b1111100000011111,
    YELLOW565 = 0b1111100000011111
};
