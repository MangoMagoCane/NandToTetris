typedef unsigned int uint;

bool checkExtension(char* filename, char** extension)
{
    char* dot = strrchr(filename, '.');
    *extension = dot;
    if (!dot || dot == filename || strcmp(dot, ".asm") != 0) {
        return false;
    }
    return true;
}
