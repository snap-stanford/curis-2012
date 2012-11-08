#include "stdafx.h"
#include "quotegraph.h"
#include "quote.h"
#include "lsh.h"

QuoteGraph::QuoteGraph() {
}

QuoteGraph::~QuoteGraph() {
}

QuoteGraph::QuoteGraph(TQuoteBase *QB, TClusterBase *CB) {
  this->QB = QB;
  this->CB = CB;
  EdgeCount = 0;
}

void QuoteGraph::CreateGraph(PNGraph& QGraph) {
  this->QGraph = TNGraph::New();
  CreateNodes();
  CreateEdges();
  QGraph = this->QGraph;
}

void QuoteGraph::LogEdges(TStr FileName) {
  FILE *Q = fopen(FileName.CStr(), "w"); // Skyfall!!!!
  TNGraph::TEdgeI End = QGraph->EndEI();
  for (TNGraph::TEdgeI iter = QGraph->BegEI(); iter != End; iter++) {
    fprintf(Q, "%d\t%d\n", iter.GetSrcNId(), iter.GetDstNId());
  }
  fclose(Q);
}

void QuoteGraph::CreateNodes() {
  TIntV QuoteIds;
  QB->GetAllQuoteIds(QuoteIds);
  TIntV::TIter QuoteIdsEnd = QuoteIds.EndI();
  for (TIntV::TIter QuoteId = QuoteIds.BegI(); QuoteId < QuoteIdsEnd; QuoteId++) {
    QGraph->AddNode(*QuoteId);
  }
}

void QuoteGraph::LSHCreateEdges() {
  THash<TMd5Sig, TShingleIdSet> Shingles;
  LSH::HashShingles(QB, CB, LSH::ShingleLen, Shingles);
  TVec<THash<TIntV, TIntSet> > BucketsVector;
  LSH::MinHash(Shingles, BucketsVector);

  THashSet<TIntPr> EdgeCache;
  int Count = 0;
  int RealCount = 0;

  printf("Beginning edge creation step...\n");
  for (int i = 0; i < BucketsVector.Len(); i++) {
    printf("Processing band signature %d of %d\n", i+1, BucketsVector.Len());
    TVec<TIntV> Buckets;
    BucketsVector[i].GetKeyV(Buckets);
    TVec<TIntV>::TIter BucketEnd = Buckets.EndI();
    for (TVec<TIntV>::TIter BucketSig = Buckets.BegI(); BucketSig < BucketEnd; BucketSig++) {
      TIntSet Bucket  = BucketsVector[i].GetDat(*BucketSig);
      Count += Bucket.Len() * (Bucket.Len() - 1) / 2;
      for (TIntSet::TIter Quote1 = Bucket.BegI(); Quote1 < Bucket.EndI(); Quote1++) {
        TIntSet::TIter Quote1Copy = Quote1;
        Quote1Copy++;
        for (TIntSet::TIter Quote2 = Quote1Copy; Quote2 < Bucket.EndI(); Quote2++) {
          if (!EdgeCache.IsKey(TIntPr(Quote1.GetKey(), Quote2.GetKey())) && !EdgeCache.IsKey(TIntPr(Quote2.GetKey(), Quote1.GetKey()))) {
            EdgeCache.AddKey(TIntPr(Quote1.GetKey(), Quote2.GetKey()));
            EdgeCache.AddKey(TIntPr(Quote2.GetKey(), Quote1.GetKey()));
            RealCount++;
            AddEdgeIfSimilar(Quote1.GetKey(), Quote2.GetKey());
          }
        }
      }
    }
  }
  fprintf(stderr, "NUMBER OF COMPARES: %d\n", Count);
  fprintf(stderr, "NUMBER OF REAL COMPARES: %d\n", RealCount);
  LogEdges("LSHBefore.txt");
}

void QuoteGraph::ElCheapoCreateEdges() {
  THash<TMd5Sig, TIntSet> Shingles;
  LSH::ElCheapoHashing(QB, LSH::ShingleLen, Shingles);
  int Count = 0;
  int RealCount = 0;
  TVec<TMd5Sig> ShingleKeys;
  Shingles.GetKeyV(ShingleKeys);
  THashSet<TIntPr> EdgeCache;

  for (int i = 0; i < ShingleKeys.Len(); i++) {
    if (i % 100 == 0) {
      Err("Processed %d out of %d shingles, count = %d\n", i, ShingleKeys.Len(), Count);
    }
    TIntSet Bucket;
    Shingles.IsKeyGetDat(ShingleKeys[i], Bucket);

    for (TIntSet::TIter Quote1 = Bucket.BegI(); Quote1 < Bucket.EndI(); Quote1++) {
      TIntSet::TIter Quote1Copy = Quote1;
      Quote1Copy++;
      for (TIntSet::TIter Quote2 = Quote1Copy; Quote2 < Bucket.EndI(); Quote2++) {
        if (!EdgeCache.IsKey(TIntPr(Quote1.GetKey(), Quote2.GetKey())) && !EdgeCache.IsKey(TIntPr(Quote2.GetKey(), Quote1.GetKey()))) {
          EdgeCache.AddKey(TIntPr(Quote1.GetKey(), Quote2.GetKey()));
          EdgeCache.AddKey(TIntPr(Quote2.GetKey(), Quote1.GetKey()));
          RealCount++;
          AddEdgeIfSimilar(Quote1.GetKey(), Quote2.GetKey());
        }
      }
    }
    int Len = Bucket.Len() * (Bucket.Len() - 1) / 2;
    Count += Len;
  }
  fprintf(stderr, "NUMBER OF COMPARES: %d\n", Count);
  fprintf(stderr, "NUMBER OF REAL COMPARES: %d\n", RealCount);
  //LogEdges("ElCheapoBefore.txt");
}

void QuoteGraph::CreateEdges() {
  if (USE_LSH) {
    LSHCreateEdges();
  } else {
    ElCheapoCreateEdges();
//    printf("Edge creation complete! %d edges created.\n", EdgeCount.Val);
//
//    THash<TMd5Sig, TIntSet> Shingles;
//    LSH::ElCheapoHashing(QB, LSH::ShingleLen, Shingles);
//    int Count = 0;
//    TVec<TMd5Sig> ShingleKeys;
//    Shingles.GetKeyV(ShingleKeys);
//    for (int i = 0; i < ShingleKeys.Len(); i++) {
//      TIntSet CurSet;
//      Shingles.IsKeyGetDat(ShingleKeys[i], CurSet);
//      int Len = CurSet.Len() * CurSet.Len();
//      Count += Len;
//    }
//    fprintf(stderr, "NUMBER OF COMPARES: %d\n", Count);
//    exit(0);
  }
}

void QuoteGraph::AddEdgeIfSimilar(TInt Id1, TInt Id2) {
  TQuote Quote1, Quote2;
  if (Id1 != Id2 && QB->GetQuote(Id1, Quote1) && QB->GetQuote(Id2, Quote2)) {
    if (QuoteGraph::EdgeShouldBeCreated(Quote1, Quote2)) {
      if (EdgeShouldBeFromOneToTwo(Quote1, Quote2)) {
        QGraph->AddEdge(Id1, Id2);
      } else {
        QGraph->AddEdge(Id2, Id1);
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
  //TInt LDistance = TStringUtil::WordLevenshteinDistance(Content1V, Content2V);
  TInt LDistance = TStringUtil::SubWordListEditDistance(Content1V, Content2V);

  // Decision tree from clustering methods paper
  int MinStopLen = min(Content1V.Len(), Content2V.Len());
  int MinLen = min(Quote1.GetContentNumWords(), Quote2.GetContentNumWords());
  //printf("L Distance: %d\tMinStopLen: %d\n", LDistance.Val, MinStopLen);
  //printf("%s\n", Content1.CStr());
  //printf("%s\n", Content2.CStr());
  if (LDistance == 0 && MinStopLen >= 2) {
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
