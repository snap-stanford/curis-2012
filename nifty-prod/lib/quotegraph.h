#ifndef quotegraph_h
#define quotegraph_h

#include "quote.h"

typedef enum TEdgeCreation { LSH, Cheap, Words, OldWords } EdgeCreation;

class QuoteGraph {
public:
  QuoteGraph();
  virtual ~QuoteGraph();
  QuoteGraph(TQuoteBase *QB, TClusterBase *CB);
  static void SetEdgeCreation(TStr EdgeString);
  void CreateGraph(PNGraph& QGraph);
  static bool EdgeShouldBeFromOneToTwo(TQuote& Quote1, TQuote& Quote2);
  static bool EdgeShouldBeCreated(TQuote& Quote1, TQuote& Quote2);
  void LogEdges(TStr FileName);
  static TEdgeCreation EdgeStyle;

protected:
  TInt EdgeCount;
  PNGraph QGraph;
  TQuoteBase *QB;
  TClusterBase *CB;
  void CreateNodes();
  void CreateEdges();
  void LSHCreateEdges();
  void CompareUsingShingles(THash<TMd5Sig, TIntSet>& Shingles);
  void CompareUsingMinHash(TVec<THash<TIntV, TIntSet> >& BucketsVector);
  void CompareUsingMinHash(TVec<THash<TMd5Sig, TIntSet> >& BucketsVector);
  void ElCheapoCreateEdges();
  void WordsCreateEdges();
  void OldWordsCreateEdges();
  virtual void AddEdgeIfSimilar(TInt Id1 , TInt Id2);
};

#endif
