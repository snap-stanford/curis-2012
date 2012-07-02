#include "stdafx.h"

int main(int argc, char *argv[]) {
  TQuoteBase QuoteBase;
  TStr Content = "The Dark Knight Rises is going to be SO AMAZING!!! :D :D :D";

  TQuote Quote = QuoteBase.AddQuote(Content, 1);
  Quote.AddSource(2);
  
  printf("Number of quotes: %d\n", QuoteBase.Len());

  TQuote Q;
  QuoteBase.GetQuote(0, Q);
  printf("Content: %s\n", Q.GetContentString().CStr());
  printf("NumSources: %d\n", Q.GetSources().Len());

  return 0;
}
