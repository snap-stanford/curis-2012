#ifndef lsh_h
#define lsh_h

#include "stdafx.h"
#include "quote.h"

class LSH {
public:
  static void HashShingles(TQuoteBase *QuoteBase, TInt ShingleLen, THash<TMd5Sig, TIntSet>& ShingleToQuoteIds);
};

#endif
