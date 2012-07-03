#include "stdafx.h"

int main(int argc, char *argv[]) {
  TQuoteBase *QuoteBase = new TQuoteBase;
  TStr Content = "The Dark Knight Rises is going to be SO AMAZING!!! :D :D :D";

  TQuote Quote = QuoteBase->AddQuote(Content, 1);
  Quote.AddSource(2);
  
  printf("Number of quotes: %d\n", QuoteBase->Len());

  TQuote Q;
  QuoteBase->GetQuote(0, Q);
  printf("Content: %s\n", Q.GetContentString().CStr());
  printf("Parsed Content: %s\n", Q.GetParsedContentString().CStr());
  printf("NumSources: %d\n", Q.GetNumSources());

  { TFOut FOut("tmp.bin"); QuoteBase->Save(FOut); }
  printf("Save data successfully\n");

  delete QuoteBase;

  TFIn FIn("tmp.bin");
  printf("Load data successfully\n");
  TQuoteBase *QuoteBase2 = new TQuoteBase;
  QuoteBase2->Load(FIn);

  printf("Number of quotes: %d\n", QuoteBase2->Len());

  TQuote Q2;
  QuoteBase2->GetQuote(0, Q2);
  printf("Content: %s\n", Q2.GetContentString().CStr());
  printf("Parsed Content: %s\n", Q2.GetParsedContentString().CStr());
  printf("NumSources: %d\n", Q2.GetNumSources());

  delete QuoteBase2;

  return 0;
}
