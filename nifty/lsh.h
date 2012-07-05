#ifndef lsh_h
#define lsh_h

#include "stdafx.h"
#include "quote.h"

class LSH {
public:
  static const int BandSize;
  static const int NumBands; // TODO: CHANGE EVERYTHING
  static const int ShingleLen;

  static void HashShingles(TQuoteBase *QuoteBase, TInt ShingleLen, THash<TMd5Sig, TIntSet>& ShingleToQuoteIds);
  static void MinHash(THash<TMd5Sig, TIntSet>& ShingleToQuoteIds, TVec<THash<TIntV, TIntSet> >& SignatureBandBuckets);
};

#endif
