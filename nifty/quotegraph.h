#ifndef quotegraph_h
#define quotegraph_h

#include "quote.h"

class QuoteGraph {
public:
  QuoteGraph(TQuoteBase *QB);

private:
  PNGraph QGraph;
  TQuoteBase *QB;
  void CreateNodes();
  void CreateEdges();
  void AddEdgeIfSimilar(TIntId1, TIntId2);
  bool EdgeShouldBeCreated(TQuote Quote1, TQuote Quote2);
  TInt LevenshteinDistance(TSTr Content1, TStr Content2);
};

#endif
