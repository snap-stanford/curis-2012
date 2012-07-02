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

bool QuoteGraph::EdgeShouldBeCreated(TInt Quote1, TInt Quote2) {
  return true; //TODO: Obviously change this
}
