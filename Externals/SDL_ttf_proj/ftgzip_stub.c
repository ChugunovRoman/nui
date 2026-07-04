// Stub implementations for gzip functions disabled in NUI build.
// FreeType references these when FT_CONFIG_OPTION_USE_ZLIB is defined in ftoption.h.

typedef int FT_Error;
typedef void* FT_Stream;
typedef void* FT_Memory;
typedef unsigned char FT_Byte;
typedef unsigned long FT_ULong;

#define FT_Err_Unimplemented_Feature 18

FT_Error FT_Stream_OpenGzip(FT_Stream stream, FT_Stream source) {
    (void)stream; (void)source;
    return FT_Err_Unimplemented_Feature;
}

FT_Error FT_Gzip_Uncompress(FT_Memory memory, FT_Byte* out, FT_ULong out_len,
                             const FT_Byte* in, FT_ULong in_len) {
    (void)memory; (void)out; (void)out_len; (void)in; (void)in_len;
    return FT_Err_Unimplemented_Feature;
}
