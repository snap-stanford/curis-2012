#include "stdafx.h"
#include "quote_graph.h"
#include "quote.h"

void QuoteGraph::QuoteGraph(TQuoteBase *QB) {
  this->QB = QB;
  CreateGraph();
  CreateEdges();
}

void QuoteGraph::CreateGraph() {
  QGraph = TNGraph::New();
  TIntV QuoteIds = QB->GetAllQuoteIds();
  TVec::TIter QuoteIdsEnd = QuoteIds.EndI();
  for (TVec::TIter QuoteId = QuoteIds.BegI(); QuoteId < QuoteIds.EndI(); QuoteId++) {
    QGraph->AddNode(*QuoteId);
  }
}

void QuoteGraph::CreateEdges() {
  THash<TMd5Sig, TIntSet> Shingles;
  LSH::HashShingles(QB, Shingles);
  TVec<THash<TIntV, TIntSet> > BucketsVector;
  LSH::MinHash(Shingles, BucketsVector);

  for (int i = 0; i < BucketsVector.Len(); i++) {
    TIntV Buckets;
    BucketsVector[i].GetKeyV(Buckets);
    TIntV::TIter BucketEnd = Buckets.EndI();
    for (TIntV::TIter BucketSig = 0; BucketSig < BucketEnd; Bucket++) {
      TIntSet Bucket  = BucketsVector[i].GetDat(*BucketSig);
      for (TIntSet::TIter Quote1 = Bucket.BegI(); Quote1 < Bucket.EndI(); Quote1++) {
        for (TIntSet::TIter Quote2 = Quote1; Quote1 < Bucket.EndI(); Quote1++) {
          if (EdgeShouldBeAdded(Quote1.GetKey(), Quote2.GetKey())) {
            QGraph->AddEdge(Quote1.GetKey(), Quote2.GetKey()); // EDGE ADDED!
          }
        }
      }
    }
  }
}

bool QuoteGraph::EdgeShouldBeCreated(TInt Id1, TInt Id2) {
  TQuote Quote1, Quote2;
  if (QB->GetQuote(Id1, Quote1) && QB->GetQuote(Id2, Quote2)) {

  }
  return true; //TODO: Obviously change this
}

TInt QuoteGraph::LevenshteinDistance(TSTr Content1, TStr Content2) {
  TInt C1Len = Content1.Len() + 1, C2Len = Content2.Len() + 1;
  TInt d[Content1.Len()][Content2.Len()];

  for (int i = 0; i < C1Len ; i++) {
    for (int j = 0; j < C2Len; j++) {
      d[i][j] = 0;
    }
  }

  for (int i = 0; i < C1Len; i++) {
    d[i, 0] = i;
  }

  for (int j = 0; j < C2Len; j++) {
    d[0, j] = j;
  }

  for (int j = 1; j < C2Len; j++) {
    for (int i = 1; i < C1Len; i++)
    {
      if (Content1[i-1] == Content2[j-1]) {
        d[i][j] = d[i-1][j-1];
      } else {
        d[i][j] = d[i-1][j] + 1; // a deletion
        if (d[i][j-1] + 1 < d[i][j]) d[i][j] = d[i][j-1] + 1; // an insertion
        if (d[i-1][j-1] + 1 < d[i][j]) d[i][j] = d[i-1][j-1] + 1; // a substitution
    }
  }

  return d[C1Len][C2Len];
}


double TQuoteBs::QuoteDistance(TInt Qt1, TInt Qt2, THash<TInt, TIntV>& QtToWordIdVH) {
  int idx1 = 0, idx2 = 0, SkipTy = 0;
  const TIntV& WIdV1 = QtToWordIdVH.GetDat(Qt1);
  const TIntV& WIdV2 = QtToWordIdVH.GetDat(Qt2);
  int ShortLen = TMath::Mn(WIdV1.Len(), WIdV2.Len());
  int Overlap = LongestCmnSubSq(WIdV1, WIdV2, idx1, idx2, SkipTy);
  Overlap -= 4; ShortLen -= 4;

  Assert(Overlap >= 0);
  if (ShortLen <= 0 || Overlap > 6) return 0;
  if (Overlap < 2) return (1 - Overlap/double(ShortLen));
  else return (1 - Overlap/double(ShortLen)) * (1 - Overlap / 7.0);
}
