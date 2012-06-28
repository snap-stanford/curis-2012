#ifndef quote_h
#define quote_h

#include "stdafx.h"

class TQuote {
private:
  TInt Id;
  TStrV Content;
  TIntV Sources;

public:
  TQuote(TInt Id, TStrV& Content);
  TQuote(TInt Id, TStr ContentString);
  void AddSource(TInt SourceId);
  TIntV GetSources();
  TStrV GetContent();
  TInt GetId();

  static TStrV ParseContentString(TStr ContentString);
};

class TQuoteBase {
private:
  TInt QuoteIdCounter;
  PHash<TInt, TQuote*> IdToTQuotes;
  PHash<TStrV, TInt> QuoteToId;
public:
  TQuoteBase();
  // returns true if new TQuote created, false otherwise.
  TQuote* AddQuote(TStr ContentString);
  TQuote* AddQuote(TStr ContentString, TInt SourceId);
  void RemoveQuote(TInt QuoteId);
  void RemoveQuote(TQuote *Quote);
  TInt GetQuoteId(TStrV& Content);
  TQuote* GetQuote(TInt QuoteId);
  int Len();
};

#endif
