/*
 * Generated by asn1c-0.9.28 (http://lionet.info/asn1c)
 * From ASN.1 module "ProtocolModule"
 * 	found in "C:/devel/BlackBoxTT/bbproto/bbproto.asn1"
 * 	`asn1c -gen-PER -S`
 */

#include "BB32WMMsg.h"

static asn_TYPE_member_t asn_MBR_BB32WMMsg_1[] = {
	{ ATF_NOFLAGS, 0, offsetof(struct BB32WMMsg, wmmsg),
		(ASN_TAG_CLASS_CONTEXT | (0 << 2)),
		-1,	/* IMPLICIT tag at current level */
		&asn_DEF_NativeInteger,
		0,	/* Defer constraints checking to the member type */
		0,	/* No PER visible constraints */
		0,
		"wmmsg"
		},
};
static const ber_tlv_tag_t asn_DEF_BB32WMMsg_tags_1[] = {
	(ASN_TAG_CLASS_UNIVERSAL | (16 << 2))
};
static const asn_TYPE_tag2member_t asn_MAP_BB32WMMsg_tag2el_1[] = {
    { (ASN_TAG_CLASS_CONTEXT | (0 << 2)), 0, 0, 0 } /* wmmsg */
};
static asn_SEQUENCE_specifics_t asn_SPC_BB32WMMsg_specs_1 = {
	sizeof(struct BB32WMMsg),
	offsetof(struct BB32WMMsg, _asn_ctx),
	asn_MAP_BB32WMMsg_tag2el_1,
	1,	/* Count of tags in the map */
	0, 0, 0,	/* Optional elements (not needed) */
	-1,	/* Start extensions */
	-1	/* Stop extensions */
};
asn_TYPE_descriptor_t asn_DEF_BB32WMMsg = {
	"BB32WMMsg",
	"BB32WMMsg",
	SEQUENCE_free,
	SEQUENCE_print,
	SEQUENCE_constraint,
	SEQUENCE_decode_ber,
	SEQUENCE_encode_der,
	SEQUENCE_decode_xer,
	SEQUENCE_encode_xer,
	SEQUENCE_decode_uper,
	SEQUENCE_encode_uper,
	0,	/* Use generic outmost tag fetcher */
	asn_DEF_BB32WMMsg_tags_1,
	sizeof(asn_DEF_BB32WMMsg_tags_1)
		/sizeof(asn_DEF_BB32WMMsg_tags_1[0]), /* 1 */
	asn_DEF_BB32WMMsg_tags_1,	/* Same as above */
	sizeof(asn_DEF_BB32WMMsg_tags_1)
		/sizeof(asn_DEF_BB32WMMsg_tags_1[0]), /* 1 */
	0,	/* No PER visible constraints */
	asn_MBR_BB32WMMsg_1,
	1,	/* Elements count */
	&asn_SPC_BB32WMMsg_specs_1	/* Additional specs */
};

