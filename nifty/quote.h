#ifndef quote_h
#define quote_h

#include "stdafx.h"

class TQuote {
private:
  TInt Id;
  TStrVP Content;
  TIntV Sources;

public:
  TQuote(TInt Id, TStrVP Content);
  TQuote(TInt Id, TStr ContentString);
  void AddSource(TInt SourceId);
  TIntV GetSources();
  TStrVP GetContent();
  TInt GetId();

  static TStrVP ParseContentString(TStr ContentString);
};

class TQuoteBase {
private:
  TInt QuoteIdCounter;
  PHash<TInt, TQuote*> IdToTQuotes;
  PHash<TStrVP, TInt> QuoteToId;
public:
  TQuoteBase();
  // returns true if new TQuote created, false otherwise.
  void AddQuote(TStr ContentString);
  void RemoveQuote(TInt QuoteId);
  void RemoveQuote(TQuote* Quote);
  TInt GetQuoteId(TStrVP Content);
  TQuote* GetQuote(TInt QuoteId);
  static void QuoteFilter(TStr &QtStr);
};

#endif
