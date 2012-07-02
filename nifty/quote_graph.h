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
};

#endif
