#pragma once
#include "unicode.h"

namespace rc {

  bool parseFile (tstring const & fname, tstring & err);
  bool parseFile (tstring const & fname);

}
