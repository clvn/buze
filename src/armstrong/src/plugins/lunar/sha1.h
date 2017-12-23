/* public api for steve reid's public domain SHA-1 implementation */
/* this file is in the public domain */

#ifndef __SHA1_H
#define __SHA1_H

#if defined(__CYGWIN__) && !defined(HAVE_STDINT_H)
# include <sys/types.h>
# if defined(OLD_CYGWIN_SYS_TYPES)
  /*
   * Old versions of Cygwin have no stdint.h but define "MS types". Some of
   * them conflict with a standard type emulation provided by config_types.h
   * so we do a fixup here.
   */
   typedef u_int8_t uint8_t;
   typedef u_int16_t uint16_t;
   typedef u_int32_t uint32_t;
#endif
#elif defined(_WIN32)
#ifdef _MSC_VER /* Microsoft Visual C+*/

  typedef signed char             int8_t;
  typedef short int               int16_t;
  typedef int                     int32_t;
  typedef __int64                 int64_t;
 
  typedef unsigned char             uint8_t;
  typedef unsigned short int        uint16_t;
  typedef unsigned int              uint32_t;
  /* no uint64_t */

#  define vsnprintf _vsnprintf
#  define snprintf _snprintf

#endif /* _MSC_VER */
#endif

#if defined(POSIX) || defined(__MACOS__)
# include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    uint8_t  buffer[64];
} SHA1_CTX;

#define SHA1_DIGEST_SIZE 20

void SHA1_Init(SHA1_CTX* context);
void SHA1_Update(SHA1_CTX* context, const uint8_t* data, const size_t len);
void SHA1_Final(SHA1_CTX* context, uint8_t digest[SHA1_DIGEST_SIZE]);

#ifdef __cplusplus
}
#endif

#endif /* __SHA1_H */
