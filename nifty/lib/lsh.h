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
  static const int NumBands;
  static const int ShingleLen;
  static const int ShingleWordLen;
  static const int WordWindow;

  static void HashShingles(TQuoteBase *QuoteBase, TClusterBase *CB, TInt ShingleLen, THash<TMd5Sig,
      TShingleIdSet>& ShingleToQuoteIds);
  static void HashShinglesOfClusters(TQuoteBase *QuoteBase, TClusterBase *ClusterBase,
              TIntV& ClusterIds, TInt ShingleLen, THash<TMd5Sig, TIntV>& ShingleToClusterIds);
  static void ComputeSignatures(THashSet<TMd5Sig>& Shingles, THash<TMd5Sig, TIntV>& Signatures, int NumSignatures);
  static void MinHash(THash<TMd5Sig, TShingleIdSet>& ShingleToQuoteIds, TVec<THash<TIntV, TIntSet> >& SignatureBandBuckets);
  static void MinHash(THash<TMd5Sig, TIntSet>& ShingleToQuoteIds, TVec<THash<TIntV, TIntSet> >& SignatureBandBuckets);
  static void MinHash(TQuoteBase *QB, THashSet<TMd5Sig>& Shingles, TVec<THash<TMd5Sig, TIntSet> >& SignatureBandBuckets);
  static void GetHashedShinglesOfCluster(TQuoteBase *QuoteBase, TCluster& C, TInt ShingleLen,
                                         THashSet<TMd5Sig>& HashedShingles);
  static void ElCheapoHashing(TQuoteBase *QuoteBase, TInt ShingleLen, THash<TMd5Sig, TIntSet>& ShingleToQuoteIds);
  static void WordHashing(TQuoteBase *QuoteBase, THash<TMd5Sig, TIntSet>& ShingleToQuoteIds);
  static void WordHashing(TQuoteBase* QuoteBase, THashSet<TMd5Sig>& Shingles);
};

class TCmpSetByLen {
private:
  bool IsAsc;
  THash<TStr, TIntSet> *Temp;
public:
  TCmpSetByLen(const bool& AscSort=true, THash<TStr, TIntSet> *Temp = NULL) : IsAsc(AscSort) {
    this->Temp = Temp;
  }
  bool operator () (const TStr& P1, const TStr& P2) const {
    TIntSet S1 = Temp->GetDat(P1);
    TIntSet S2 = Temp->GetDat(P2);
    if (IsAsc) {
      return S1.Len() < S2.Len();
    } else {
      return S2.Len() < S1.Len();
    }
  }
};

#endif
