#include "stdafx.h"
#include "quotegraph.h"
#include "quote.h"
#include "lsh.h"

TEdgeCreation QuoteGraph::EdgeStyle = LSH;

QuoteGraph::QuoteGraph() {
}

QuoteGraph::~QuoteGraph() {
}

QuoteGraph::QuoteGraph(TQuoteBase *QB, TClusterBase *CB) {
  this->QB = QB;
  this->CB = CB;
  EdgeCount = 0;
}

void QuoteGraph::SetEdgeCreation(TStr EdgeString) {
  if (EdgeString == "lsh") {
    QuoteGraph::EdgeStyle = LSH;
  } else if (EdgeString == "cheap") {
    QuoteGraph::EdgeStyle = Cheap;
  } else if (EdgeString == "words") {
    QuoteGraph::EdgeStyle = Words;
  } else if (EdgeString == "oldwords") {
    QuoteGraph::EdgeStyle = OldWords;
  }
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

void QuoteGraph::CompareUsingMinHash(TVec<THash<TIntV, TIntSet> >& BucketsVector) {
  THashSet<TIntPr> EdgeCache;
  int Count = 0;
  int RealCount = 0;

  printf("Beginning edge creation step...\n");
  for (int i = 0; i < BucketsVector.Len(); i++) {
    printf("Processing band signature %d of %d - %d signatures\n", i+1, BucketsVector.Len(), BucketsVector[i].Len());
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
}

// I embarassingly don't know how templating works.
void QuoteGraph::CompareUsingMinHash(TVec<THash<TMd5Sig, TIntSet> >& BucketsVector) {
  THashSet<TIntPr> EdgeCache;
  int Count = 0;
  int RealCount = 0;

  printf("Beginning edge creation step...\n");
  for (int i = 0; i < BucketsVector.Len(); i++) {
    printf("Processing band signature %d of %d - %d signatures\n", i+1, BucketsVector.Len(), BucketsVector[i].Len());
    TVec<TMd5Sig> Buckets;
    BucketsVector[i].GetKeyV(Buckets);
    TVec<TMd5Sig>::TIter BucketEnd = Buckets.EndI();
    for (TVec<TMd5Sig>::TIter BucketSig = Buckets.BegI(); BucketSig < BucketEnd; BucketSig++) {
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
}


void QuoteGraph::CompareUsingShingles(THash<TMd5Sig, TIntSet>& Shingles) {
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
}

void QuoteGraph::LSHCreateEdges() {
  THash<TMd5Sig, TShingleIdSet> Shingles;
  LSH::HashShingles(QB, CB, LSH::ShingleLen, Shingles);
  TVec<THash<TIntV, TIntSet> > BucketsVector;
  LSH::MinHash(Shingles, BucketsVector);
  CompareUsingMinHash(BucketsVector);

  LogEdges("LSHBefore.txt");
}

void QuoteGraph::ElCheapoCreateEdges() {
  THash<TMd5Sig, TIntSet> Shingles;
  LSH::WordHashing(QB, Shingles);
  CompareUsingShingles(Shingles);

  LogEdges("WordsCheapoBefore.txt");
}

void QuoteGraph::WordsCreateEdges() {
  THashSet<TMd5Sig> Shingles;
  LSH::WordHashing(QB, Shingles);
  TVec<THash<TMd5Sig, TIntSet> > BucketsVector;
  LSH::MinHash(QB, Shingles, BucketsVector);
  CompareUsingMinHash(BucketsVector);
}

void QuoteGraph::OldWordsCreateEdges() {
  THash<TMd5Sig, TIntSet> Shingles;
  LSH::WordHashing(QB, Shingles);
  TVec<THash<TIntV, TIntSet> > BucketsVector;
  LSH::MinHash(Shingles, BucketsVector);
  CompareUsingMinHash(BucketsVector);
}

void QuoteGraph::CreateEdges() {
  if (QuoteGraph::EdgeStyle == LSH) {
    LSHCreateEdges();
  } else if (QuoteGraph::EdgeStyle == Cheap){
    ElCheapoCreateEdges();
  } else if (QuoteGraph::EdgeStyle == OldWords){
    OldWordsCreateEdges();
  } else if (QuoteGraph::EdgeStyle == Words) {
    WordsCreateEdges();
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
