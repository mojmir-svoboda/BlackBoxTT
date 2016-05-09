#pragma once

namespace asn1 {

  /********************************************
   * Outer protocol structure is very simple:
   * Header | Payload
   * where Payload is an octet string of length m_len
   *
   * Payload itself is encoded with ASN.1
   ********************************************/
  struct Header
  {
    unsigned m_version : 8;
    unsigned m_len : 24;  // 3 Byte length of payload

    Header () : m_version(1), m_len(0) { }
    void Reset () { m_version = 1; m_len = 0; }
  };

	inline Header & encode_header(char * buff, size_t buff_sz)
	{
		Header * h = new (buff) Header;
		return *h;
	}

}
