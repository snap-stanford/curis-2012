#include "stdafx.h"

int main(int argc, char *argv[]) {
  TQuoteBase *QuoteBase = new TQuoteBase;
  TStr Content = "The Dark Knight Rises is going to be SO AMAZING!!! :D :D :D";

  TQuote Quote = QuoteBase->AddQuote(Content, 1);
  Quote.AddSource(2);

  TStr Content2 = "The Dark Knight Rises is going to be AMAZING!!! :D :D :D";
  QuoteBase->AddQuote(Content2, 3);

  TStr Content3 = "The Dark Knight Rises is going to be AWFUL!!! D: D: D:";
  QuoteBase->AddQuote(Content3, 1);
  Quote.AddSource(2);
  Quote.AddSource(3);

  TStr Content4 = "I can't wait to see Ice Age!! ^^";
  QuoteBase->AddQuote(Content4, 4);
  Quote.AddSource(5);
  Quote.AddSource(6);
  Quote.AddSource(7);
  
  printf("Number of quotes in quote base should be 4; actually is: %d\n", QuoteBase->Len());

  QuoteGraph GraphCreator(QuoteBase);
  PNGraph QGraph;
  GraphCreator.CreateGraph(QGraph);
  Clustering ClusterJob;
  ClusterJob.SetGraph(QGraph);
  TIntSet RootNodes;
  TVec<TIntV> Clusters;
  ClusterJob.BuildClusters(RootNodes, Clusters, QuoteBase);
  TVec<TPair<TStr, TInt> > RepQuotesAndFreq;
  ClusterJob.SortClustersByFreq(RepQuotesAndFreq, Clusters, QuoteBase);

  for (int i = 0; i < RepQuotesAndFreq.Len(); i++) {
    printf("%d\t%s\n", RepQuotesAndFreq[i].Val2.Val, RepQuotesAndFreq[i].Val1.CStr());
  }

  delete QuoteBase;
  
  return 0;
}
