#ifndef lsh_h
#define lsh_h

#include "stdafx.h"
#include "quote.h"

class LSH {
public:
  static const int BandSize = 5;
  static const int NumBands = 20; // TODO: CHANGE EVERYTHING

  static void HashShingles(TQuoteBase *QuoteBase, TInt ShingleLen, THash<TMd5Sig, TIntSet>& ShingleToQuoteIds);
  static void MinHash(THash<TMd5Sig, TIntSet>& ShingleToQuoteIds, TVec<THash<TIntV, TIntSet> >& SignatureBandBuckets);
};

#endif
