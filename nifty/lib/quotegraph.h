#ifndef quotegraph_h
#define quotegraph_h

#include "quote.h"

class QuoteGraph {
public:
  QuoteGraph(TQuoteBase *QB);
  void CreateGraph(PNGraph& QGraph);
  static TInt WordLevenshteinDistance(TStrV& Content1, TStrV& Content2);
  static bool EdgeShouldBeFromOneToTwo(TQuote& Quote1, TQuote& Quote2);

private:
  TInt EdgeCount;
  PNGraph QGraph;
  TQuoteBase *QB;
  void CreateNodes();
  void CreateEdges();
  void AddEdgeIfSimilar(TInt Id1 , TInt Id2);
  bool EdgeShouldBeCreated(TQuote& Quote1, TQuote& Quote2);
  TInt LevenshteinDistance(TStr& Content1, TStr& Content2);
};

#endif
