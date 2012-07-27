#ifndef incrementalquotegraph_h
#define incrementalquotegraph_h

#include "quote.h"

class IncrementalQuoteGraph : QuoteGraph {
public:
  IncrementalQuoteGraph(TQuoteBase *QB, TIntSet& NewQuotes, PNGraph& ExistingGraph);
  void UpdateGraph(PNGraph& QGraph);
  void GetAffectedNodes(TIntSet& AffectedNodes);

private:
  TIntSet NewQuotes;
  TIntSet AffectedNodes;
  void AddNodes();
  void AddEdges();
  void AddEdgeIfSimilar(TInt Id1 , TInt Id2);
};

#endif
