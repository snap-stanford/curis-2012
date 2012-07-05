#include "stdafx.h"
#include "quotegraph.h"
#include "quote.h"
#include "lsh.h"

QuoteGraph::QuoteGraph(TQuoteBase *QB) {
  this->QB = QB;
  CreateGraph();CreateGraph();
  CreateEdges();
}

void QuoteGraph::CreateGraph() {
  QGraph = TNGraph::New();
  TIntV QuoteIds;
  QB->GetAllQuoteIds(QuoteIds);
  TIntV::TIter QuoteIdsEnd = QuoteIds.EndI();
  for (TIntV::TIter QuoteId = QuoteIds.BegI(); QuoteId < QuoteIdsEnd; QuoteId++) {
    QGraph->AddNode(*QuoteId);
  }
}

void QuoteGraph::CreateEdges() {
  THash<TMd5Sig, TIntSet> Shingles;
  LSH::HashShingles(QB, LSH::ShingleLen, Shingles);
  TVec<THash<TIntV, TIntSet> > BucketsVector;
  LSH::MinHash(Shingles, BucketsVector);

  for (int i = 0; i < BucketsVector.Len(); i++) {
    TVec<TIntV> Buckets;
    BucketsVector[i].GetKeyV(Buckets);
    TVec<TIntV>::TIter BucketEnd = Buckets.EndI();
    for (TVec<TIntV>::TIter BucketSig = 0; BucketSig < BucketEnd; BucketSig++) {
      TIntSet Bucket  = BucketsVector[i].GetDat(*BucketSig);
      for (TIntSet::TIter Quote1 = Bucket.BegI(); Quote1 < Bucket.EndI(); Quote1++) {
        for (TIntSet::TIter Quote2 = Quote1; Quote1 < Bucket.EndI(); Quote1++) {
          AddEdgeIfSimilar(Quote1.GetKey(), Quote2.GetKey());
        }
      }
    }
  }
}

void QuoteGraph::AddEdgeIfSimilar(TInt Id1, TInt Id2) {
  TQuote Quote1, Quote2;
  if (QB->GetQuote(Id1, Quote1) && QB->GetQuote(Id2, Quote2)) {
    if (EdgeShouldBeCreated(Quote1, Quote2)) {
      if (Quote1.GetParsedContent().Len() > Quote2.GetParsedContent().Len()) {
        QGraph->AddEdge(Id2, Id1); // EDGE ADDED!
      } else if (Quote2.GetParsedContent().Len() > Quote1.GetParsedContent().Len()) {
        QGraph->AddEdge(Id1, Id2); // TODO: we don't account for the "quotes are equal length" case because we're not too sure how to deal with that.
      }
    }
  }
}

bool QuoteGraph::EdgeShouldBeCreated(TQuote& Quote1, TQuote& Quote2) {
  TStr Content1 = Quote1.GetParsedContentString();
  TStr Content2 = Quote2.GetParsedContentString();
  TStrV Content1V;
  TStrV Content2V;
  Content1.SplitOnWs(Content1V);
  Content2.SplitOnWs(Content2V);
  TInt LDistance = WordLevenshteinDistance(Content1V, Content2V);

  // Decision tree from clustering methods paper
  int MinStopLen = min(Content1V.Len(), Content2V.Len());
  int MinLen = min(Quote1.GetContent().Len(), Quote2.GetContent().Len());
  if (LDistance == 0) {
    return true;
  } else if (MinLen == 4 && LDistance <= 1 && MinStopLen == 4) {
    return true;
  } else if (MinLen == 5 && LDistance <= 1 && MinStopLen > 4) {
    return true;
  } else if (MinLen == 6 && LDistance <= 1 && MinStopLen >= 5) {
    return true;
  } else if (MinLen > 6 && LDistance <= 2 && MinStopLen > 3) {
    return true;
  }
  return false;
}

// TODO: Merge this and below into a thing that takes something with a .Len() and an == operator
// because duplicate code is bad.
TInt QuoteGraph::WordLevenshteinDistance(TStrV& Content1, TStrV& Content2) {
  TInt C1Len = Content1.Len() + 1, C2Len = Content2.Len() + 1;
  TInt d[C1Len.Val][C2Len.Val];

  for (int i = 0; i < C1Len ; i++) {
    for (int j = 0; j < C2Len; j++) {
      d[i][j] = 0;
    }
  }

  for (int i = 0; i < C1Len; i++) {
    d[i][0] = i;
  }

  for (int j = 0; j < C2Len; j++) {
    d[0][j] = j;
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
  }
  return d[C1Len][C2Len];
}

TInt QuoteGraph::LevenshteinDistance(TStr& Content1, TStr& Content2) {
  TInt C1Len = Content1.Len() + 1, C2Len = Content2.Len() + 1;
  TInt d[C1Len.Val][C2Len.Val];

  for (int i = 0; i < C1Len ; i++) {
    for (int j = 0; j < C2Len; j++) {
      d[i][j] = 0;
    }
  }

  for (int i = 0; i < C1Len; i++) {
    d[i][0] = i;
  }

  for (int j = 0; j < C2Len; j++) {
    d[0][j] = j;
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
  }
  return d[C1Len][C2Len];
}


/*double TQuoteBs::QuoteDistance(TInt Qt1, TInt Qt2, THash<TInt, TIntV>& QtToWordIdVH) {
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
}*/
