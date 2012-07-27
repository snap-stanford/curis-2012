#ifndef lsh_h
#define lsh_h

#include "stdafx.h"
#include "quote.h"

class LSH {
private:
  static void GetHashedShinglesOfCluster(TCluster& C, TInt ShingleLen, THashSet<TMd5Sig>& HashedShingles);

public:
  static const int BandSize;
  static const int NumBands; // TODO: CHANGE EVERYTHING
  static const int ShingleLen;
  static const int ShingleWordLen;

  static void HashShingles(TQuoteBase *QuoteBase, TInt ShingleLen, THash<TMd5Sig,
                           TIntSet>& ShingleToQuoteIds);
  static void HashShinglesOfClusters(TQuoteBase *QuoteBase, TClusterBase *ClusterBase,
                           TInt ShingleLen, THash<TMd5Sig, TIntSet>& ShingleToClusterIds);
  static void MinHash(THash<TMd5Sig, TIntSet>& ShingleToQuoteIds, TVec<THash<TIntV, TIntSet> >& SignatureBandBuckets);
};

#endif
