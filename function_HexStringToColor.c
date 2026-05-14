// Get Color from hexadecimal(string) value
Color HexStringToColor(const char *hexstring)
{
    Color result = BLANK;

    int number = (int)strtol(hexstring, NULL, 16);

    result.r = (number >> 24) & 255;
    result.g = (number >> 16) & 255;
    result.b = (number >> 8) & 255;
    result.a = (number ) & 255;

    return result;
}

// example usage
// ClearBackground(HexStringToColor("002020FF"));
