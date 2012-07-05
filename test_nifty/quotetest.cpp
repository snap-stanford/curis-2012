#include "stdafx.h"

int main(int argc, char *argv[]) {
  TQuoteBase *QuoteBase = new TQuoteBase;
  TStr Content = "The Dark Knight Rises is going to be SO AMAZING!!! :D :D :D";

  TQuote Quote = QuoteBase->AddQuote(Content, 1);
  Quote.AddSource(2);
  
  printf("Number of quotes: %d\n", QuoteBase->Len());

  TQuote Q;
  QuoteBase->GetQuote(0, Q);
  TStr QContentString;
  Q.GetContentString(QContentString);
  TStr QParsedContentString;
  Q.GetParsedContentString(QParsedContentString);

  printf("Content: %s\n", QContentString.CStr());
  printf("Parsed Content: %s\n", QParsedContentString.CStr());
  printf("NumSources: %d\n", Q.GetNumSources().Val);

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
  TStr Q2ContentString;
  Q2.GetContentString(Q2ContentString);
  TStr Q2ParsedContentString;
  Q2.GetParsedContentString(Q2ParsedContentString);

  printf("Content: %s\n", Q2ContentString.CStr());
  printf("Parsed Content: %s\n", Q2ParsedContentString.CStr());
  printf("NumSources: %d\n", Q2.GetNumSources().Val);

  delete QuoteBase2;

  return 0;
}
