#ifndef stringutil_h
#define stringutil_h

#include "../../snap/snap-core/Snap.h"
#include "niftylib.h"

class TStringUtil {
private:

public:
  static TStrSet PublicSuffixSet;

  static TStrSet LoadPublicSuffixList();
  static TStr GetDomainName(const TStr &Url);
};

#endif
