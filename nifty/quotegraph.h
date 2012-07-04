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
  bool EdgeShouldBeCreated(TInt Id1, TInt Id2);
  TInt LevenshteinDistance(TSTr Content1, TStr Content2);
  TInt QuoteGraph::WordLevenshteinDistance(TStrV Content1, TStrV Content2);
};

#endif
