/*-
 * Copyright (c) 2003, 2004 Lev Walkin <vlm@lionet.info>. All rights reserved.
 * Redistribution and modifications are permitted subject to BSD license.
 */
#ifndef	ASN_SET_OF_H
#define	ASN_SET_OF_H

#if defined __cplusplus && defined USE_C_LINKAGE
extern "C" {
#endif

#define	A_SET_OF(type)					\
	struct {					\
		type **array;				\
		int count;	/* Meaningful size */	\
		int size;	/* Allocated size */	\
		void (*free)(type *);			\
	}

#define	ASN_SET_ADD(allocator, headptr, ptr)		\
	asn_set_add(allocator, (headptr), (ptr))

/*******************************************
 * Implementation of the SET OF structure.
 */

/*
 * Add another structure into the set by its pointer.
 * RETURN VALUES:
 * 0 for success and -1/errno for failure.
 */
int  asn_set_add(Allocator * allocator, void *asn_set_of_x, void *ptr);

/*
 * Delete the element from the set by its number (base 0).
 * This is a constant-time operation. The order of elements before the
 * deleted ones is guaranteed, the order of elements after the deleted
 * one is NOT guaranteed.
 * If _do_free is given AND the (*free) is initialized, the element
 * will be freed using the custom (*free) function as well.
 */
void asn_set_del(void *asn_set_of_x, int number, int _do_free);

/*
 * Empty the contents of the set. Will free the elements, if (*free) is given.
 * Will NOT free the set itself.
 */
void asn_set_empty(Allocator * allocator, void *asn_set_of_x);

/*
 * Cope with different conversions requirements to/from void in C and C++.
 * This is mostly useful for support library.
 */
typedef A_SET_OF(void) asn_anonymous_set_;
#define _A_SET_FROM_VOID(ptr)		((asn_anonymous_set_ *)(ptr))
#define _A_CSET_FROM_VOID(ptr)		((const asn_anonymous_set_ *)(ptr))

#if defined __cplusplus && defined USE_C_LINKAGE
}
#endif

#endif	/* ASN_SET_OF_H */
