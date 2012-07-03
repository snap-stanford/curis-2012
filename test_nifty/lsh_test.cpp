#include "stdafx.h"

int main(int argc, char *argv[]) {
  TQuoteBase *QuoteBase = new TQuoteBase;
  TStr Content = "The Dark Knight Rises is going to be SO AMAZING!!! :D :D :D";

  TQuote Quote = QuoteBase->AddQuote(Content, 1);
  Quote.AddSource(2);

  TStr Content2 = "The Dark Knight Rises is going to be AMAZING!!!";
  QuoteBase->AddQuote(Content2, 3);

  TStr Content3 = "The Dark Knight Rises";
  QuoteBase->AddQuote(Content3, 1);
  Quote.AddSource(2);
  Quote.AddSource(3);
  
  printf("Number of quotes in quote base: %d\n", QuoteBase->Len());

  THash<TMd5Sig, TIntSet> ShingleToQuoteIds;
  LSH::HashShingles(QuoteBase, 4, ShingleToQuoteIds);
    

  return 0;
}
