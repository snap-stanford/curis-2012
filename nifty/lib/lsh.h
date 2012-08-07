#ifndef lsh_h
#define lsh_h

#include "stdafx.h"
#include "quote.h"

typedef TPair<TInt, TInt> TShingleId; // QuoteId, WordIndex
typedef THashSet<TShingleId> TShingleIdSet;

class LSH {
private:

public:
  static const int BandSize;
  static const int NumBands; // TODO: CHANGE EVERYTHING
  static const int ShingleLen;
  static const int ShingleWordLen;
  static const int WordWindow;

  static void HashShingles(TQuoteBase *QuoteBase, TInt ShingleLen, THash<TMd5Sig,
      TShingleIdSet>& ShingleToQuoteIds);
  static void HashShinglesOfClusters(TQuoteBase *QuoteBase, TClusterBase *ClusterBase,
              TIntV& ClusterIds, TInt ShingleLen, THash<TMd5Sig, TIntV>& ShingleToClusterIds);
  static void MinHash(THash<TMd5Sig, TShingleIdSet>& ShingleToQuoteIds, TVec<THash<TIntV, TIntSet> >& SignatureBandBuckets);
  static void GetHashedShinglesOfCluster(TQuoteBase *QuoteBase, TCluster& C, TInt ShingleLen,
                                         THashSet<TMd5Sig>& HashedShingles);
};

#endif
