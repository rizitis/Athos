
    #include <stddef.h>
    #include <stdio.h>
    #include <jpeglib.h>
    int main()
    {
    #if (JPEG_LIB_VERSION >= 70)
    #error JPEG_LIB_VERSION >= 70
    #endif
    }
    