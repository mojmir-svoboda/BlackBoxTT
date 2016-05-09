#include "alloc.h"

AllocAsn1 s_AllocatorAsn1;

extern "C" {
  void * Asn1DecoderCalloc (size_t count, size_t size);
  void * Asn1DecoderMalloc (size_t size);
  void * Asn1DecoderRealloc (void * mem, size_t size);
  void  Asn1DecoderFree (void * mem);
}
