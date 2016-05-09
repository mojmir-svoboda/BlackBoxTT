#pragma once

extern "C" {
	void * Asn1DecoderCalloc (size_t count, size_t size);
	void * Asn1DecoderMalloc (size_t size);
	void * Asn1DecoderRealloc (void * mem, size_t size);
	void Asn1DecoderFree (void * mem);
}
