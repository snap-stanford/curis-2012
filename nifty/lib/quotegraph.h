#ifndef quotegraph_h
#define quotegraph_h

#include "quote.h"

class QuoteGraph {
public:
  QuoteGraph();
  virtual ~QuoteGraph();
  QuoteGraph(TQuoteBase *QB, TClusterBase *CB);
  void CreateGraph(PNGraph& QGraph);
  static bool EdgeShouldBeFromOneToTwo(TQuote& Quote1, TQuote& Quote2);
  static bool EdgeShouldBeCreated(TQuote& Quote1, TQuote& Quote2);
  void LogEdges(TStr FileName);

protected:
  TInt EdgeCount;
  PNGraph QGraph;
  TQuoteBase *QB;
  TClusterBase *CB;
  void CreateNodes();
  void CreateEdges();
  void LSHCreateEdges();
  void ElCheapoCreateEdges();
  virtual void AddEdgeIfSimilar(TInt Id1 , TInt Id2);
};

#endif
