#ifndef quote_graph_h
#define quote_graph_h

#include "quote.h"

class QuoteGraph {
public:
  QuoteGraph(TQuoteBase *QB);

private:
  PNGraph QGraph;
  TQuoteBase *QB;
  void CreateNodes();
  void CreateEdges();
  bool EdgeShouldBeCreated(TInt Quote1, TInt Quote2);
};

#endif
