#include "stdafx.h"
#include "quotegraph.h"
#include "quote.h"
#include "lsh.h"

QuoteGraph::QuoteGraph() {

}

QuoteGraph::QuoteGraph(TQuoteBase *QB) {
  this->QB = QB;
  EdgeCount = 0;
}

void QuoteGraph::CreateGraph(PNGraph& QGraph) {
  this->QGraph = TNGraph::New();
  CreateNodes();
  CreateEdges();
  QGraph = this->QGraph;
}

void QuoteGraph::CreateNodes() {
  TIntV QuoteIds;
  QB->GetAllQuoteIds(QuoteIds);
  TIntV::TIter QuoteIdsEnd = QuoteIds.EndI();
  for (TIntV::TIter QuoteId = QuoteIds.BegI(); QuoteId < QuoteIdsEnd; QuoteId++) {
    QGraph->AddNode(*QuoteId);
  }
}

void QuoteGraph::CreateEdges() {
  THash<TMd5Sig, TShingleIdSet> Shingles;
  LSH::HashShingles(QB, LSH::ShingleLen, Shingles);
  TVec<THash<TIntV, TIntSet> > BucketsVector;
  LSH::MinHash(Shingles, BucketsVector);

  printf("Beginning edge creation step...\n");
  for (int i = 0; i < BucketsVector.Len(); i++) {
    printf("Processing band signature %d of %d\n", i+1, BucketsVector.Len());
    TVec<TIntV> Buckets;
    BucketsVector[i].GetKeyV(Buckets);
    TVec<TIntV>::TIter BucketEnd = Buckets.EndI();
    for (TVec<TIntV>::TIter BucketSig = Buckets.BegI(); BucketSig < BucketEnd; BucketSig++) {
      TIntSet Bucket  = BucketsVector[i].GetDat(*BucketSig);
      for (TIntSet::TIter Quote1 = Bucket.BegI(); Quote1 < Bucket.EndI(); Quote1++) {
        TIntSet::TIter Quote1Copy = Quote1;
        Quote1Copy++;
        for (TIntSet::TIter Quote2 = Quote1Copy; Quote2 < Bucket.EndI(); Quote2++) {
          AddEdgeIfSimilar(Quote1.GetKey(), Quote2.GetKey());
        }
      }
    }
  }
  printf("Edge creation complete! %d edges created.\n", EdgeCount.Val);
}

void QuoteGraph::AddEdgeIfSimilar(TInt Id1, TInt Id2) {
  TQuote Quote1, Quote2; // TODO: Why is Id being sorted in its own bucket? {
  if (Id1 != Id2 && QB->GetQuote(Id1, Quote1) && QB->GetQuote(Id2, Quote2)) {
    if (QuoteGraph::EdgeShouldBeCreated(Quote1, Quote2)) {
      if (EdgeShouldBeFromOneToTwo(Quote1, Quote2)) {
        // printf("%d --> %d\n", Id1.Val, Id2.Val);
        QGraph->AddEdge(Id1, Id2);
      } else {
        // printf("%d --> %d\n", Id2.Val, Id1.Val);
        QGraph->AddEdge(Id2, Id1); // EDGE ADDED!
      }
      EdgeCount++;
        
    }
  }
}

/// Decides the direction of an edge between two quotes.
//  Returns true if the edge should be from the first quote to the second
bool QuoteGraph::EdgeShouldBeFromOneToTwo(TQuote& Quote1, TQuote& Quote2) {
  if (Quote1.GetParsedContentNumWords() > Quote2.GetParsedContentNumWords()) {
    return false;
  } else if (Quote2.GetParsedContentNumWords() > Quote1.GetParsedContentNumWords()) {
    return true;
  } else {
    // Break tie based on number of words in original quotes
    if (Quote1.GetContentNumWords() > Quote2.GetContentNumWords()) {
      return false;
    } else if (Quote2.GetContentNumWords() > Quote1.GetContentNumWords()) {
      return true;
    } else {
      // Break tie based on number of characters in original quotes
      TStr Quote1Str;
      TStr Quote2Str;
      Quote1.GetContentString(Quote1Str);
      Quote2.GetContentString(Quote2Str);
      if (Quote1Str.Len() > Quote2Str.Len()) {
        return false;
      } else if (Quote2Str.Len() > Quote1Str.Len()) {
        return true;
      } else {
        // Break tie based on alphabetical order
        if (Quote1Str < Quote2Str) {
          return true;
        } else {
          return false;
        }
      }
    }
  }
}

bool QuoteGraph::EdgeShouldBeCreated(TQuote& Quote1, TQuote& Quote2) {
  TStr Content1;
  Quote1.GetParsedContentString(Content1);
  TStr Content2;
  Quote2.GetParsedContentString(Content2);
  TStrV Content1V;
  TStrV Content2V;
  Content1.SplitOnWs(Content1V);
  Content2.SplitOnWs(Content2V);
  //TInt LDistance = QuoteGraph::WordLevenshteinDistance(Content1V, Content2V);
  TInt LDistance = TQuoteBase::SubWordListEditDistance(Content1V, Content2V);

  // Decision tree from clustering methods paper
  int MinStopLen = min(Content1V.Len(), Content2V.Len());
  int MinLen = min(Quote1.GetContentNumWords(), Quote2.GetContentNumWords());
  //printf("L Distance: %d\tMinStopLen: %d\n", LDistance.Val, MinStopLen);
  //printf("%s\n", Content1.CStr());
  //printf("%s\n", Content2.CStr());
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
  return d[C1Len-1][C2Len-1];
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
  return d[C1Len-1][C2Len-1];
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
