#include "stdafx.h"
#include "incrementalquotegraph.h"
#include "quote.h"
#include "lsh.h"

IncrementalQuoteGraph::IncrementalQuoteGraph(TQuoteBase *QB, TClusterBase *CB, TIntSet& NewQuotes, PNGraph& ExistingGraph) {
  this->QB = QB;
  this->CB = CB;
  fprintf(stderr, "QG creater Merged Quote Counter: %d\n", this->QB->GetCurCounterValue().Val);
  this->NewQuotes = NewQuotes;
  QGraph = ExistingGraph;
  EdgeCount = 0;
  IncrementCount = 0;
}

IncrementalQuoteGraph::~IncrementalQuoteGraph() {
}

void IncrementalQuoteGraph::UpdateGraph(PNGraph& QGraph) {
  AddNodes();
  CreateEdges();
  QGraph = this->QGraph;
}

void IncrementalQuoteGraph::AddNodes() {
  TIntSet::TIter QuoteIdsEnd = NewQuotes.EndI();
  for (TIntSet::TIter QuoteId = NewQuotes.BegI(); QuoteId < QuoteIdsEnd; QuoteId++) {
    if (!QGraph->IsNode(QuoteId.GetKey()))
      QGraph->AddNode(QuoteId.GetKey());
  }
}

void IncrementalQuoteGraph::AddEdgeIfSimilar(TInt Id1, TInt Id2) {
  if (!NewQuotes.IsKey(Id1) && !NewQuotes.IsKey(Id2)) return;
  ++IncrementCount;
  TQuote Quote1, Quote2; // TODO: Why is Id being sorted in its own bucket? {
  if (Id1 != Id2 && QB->GetQuote(Id1, Quote1) && QB->GetQuote(Id2, Quote2)) {
    if (QuoteGraph::EdgeShouldBeCreated(Quote1, Quote2)) {
      if (EdgeShouldBeFromOneToTwo(Quote1, Quote2)) {
        // printf("%d --> %d\n", Id1.Val, Id2.Val);
        QGraph->AddEdge(Id1, Id2);
        AffectedNodes.AddKey(Id1);
      } else {
        // printf("%d --> %d\n", Id2.Val, Id1.Val);
        QGraph->AddEdge(Id2, Id1); // EDGE ADDED!
        AffectedNodes.AddKey(Id2);
      }
      EdgeCount++;

    }
  }
}

void IncrementalQuoteGraph::GetAffectedNodes(TIntSet& AffectedNodes) {
  AffectedNodes = this->AffectedNodes;
}
