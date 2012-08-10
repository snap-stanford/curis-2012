#ifndef quotegraph_h
#define quotegraph_h

#include "quote.h"

class QuoteGraph {
public:
  QuoteGraph();
  virtual ~QuoteGraph();
  QuoteGraph(TQuoteBase *QB);
  void CreateGraph(PNGraph& QGraph);
  static TInt WordLevenshteinDistance(TStrV& Content1, TStrV& Content2);
  static bool EdgeShouldBeFromOneToTwo(TQuote& Quote1, TQuote& Quote2);
  static bool EdgeShouldBeCreated(TQuote& Quote1, TQuote& Quote2);

protected:
  TInt EdgeCount;
  PNGraph QGraph;
  TQuoteBase *QB;
  void CreateNodes();
  void CreateEdges();
  virtual void AddEdgeIfSimilar(TInt Id1 , TInt Id2);
  static TInt LevenshteinDistance(TStr& Content1, TStr& Content2);
};

#endif
