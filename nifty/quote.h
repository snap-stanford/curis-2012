#ifndef quote_h
#define quote_h

#include "stdafx.h"

class TQuote {
private:
  TInt Id;
  TStrV Content;
  TIntV Sources;

public:
  TQuote(TInt Id, TStrVP Content);
  TQuote(TInt Id, TStr ContentString);
  void AddSource(TInt SourceId);
  TStrVP GetSources();

  static TStrVP ParseContentString(TStr ContentString);
};

class TQuoteBase {
private:
  TInt QuoteIDCounter = 0;
  PHash<TInt, TQuote*> IdToTQuotes;
  PHash<TStrVP, TInt> QuoteToId;
public:
  TQuoteBase();
  // returns true if new TQuote created, false otherwise.
  bool AddQuote(TStr ContentString);
  bool RemoveQuote(TInt QuoteId);
  TInt GetQuoteId(TStrV Content);
  TQuote GetQuote(TInt QuoteId);
  static void QuoteFilter(TStr &QtStr);
};

#endif
