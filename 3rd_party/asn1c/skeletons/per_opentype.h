/*
 * Copyright (c) 2007 Lev Walkin <vlm@lionet.info>. All rights reserved.
 * Redistribution and modifications are permitted subject to BSD license.
 */
#ifndef	_PER_OPENTYPE_H_
#define	_PER_OPENTYPE_H_

#if defined __cplusplus && defined USE_C_LINKAGE
extern "C" {
#endif

asn_dec_rval_t uper_open_type_get(Allocator * allocator, asn_codec_ctx_t *opt_codec_ctx, asn_TYPE_descriptor_t *td, asn_per_constraints_t *constraints, void **sptr, asn_per_data_t *pd);

int uper_open_type_skip(Allocator * allocator, asn_codec_ctx_t *opt_codec_ctx, asn_per_data_t *pd);

int uper_open_type_put(Allocator * allocator, asn_TYPE_descriptor_t *td, asn_per_constraints_t *constraints, void *sptr, asn_per_outp_t *po);

#if defined __cplusplus && defined USE_C_LINKAGE
}
#endif

#endif	/* _PER_OPENTYPE_H_ */
