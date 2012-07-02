#ifndef quote_h
#define quote_h

#include "stdafx.h"

class TQuote {
private:
  TInt Id;
  TStrV Content;
  TStrV ParsedContent;
  TIntV Sources;

public:
  TQuote();
  TQuote(TInt Id, TStrV& Content);
  TQuote(TInt Id, TStr ContentString);
  TQuote(TSIn& SIn) : Id(SIn), Content(SIn), ParsedContent(SIn), Sources(SIn) { }
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  void AddSource(TInt SourceId);
  TIntV GetSources();
  TStrV GetContent();
  TStr GetContentString();
  TStrV GetParsedContent();
  TStr GetParsedContentString();
  TInt GetId();

  static PSwSet StopWordSet;
  static TStrV ParseContentString(TStr ContentString);
  static TStrV StemAndStopWordsContentString(TStrV ContentV);
};

class TQuoteBase {
private:
  TInt QuoteIdCounter;
  THash<TInt, TQuote> IdToTQuotes;
  THash<TStrV, TInt> QuoteToId;
public:
  TQuoteBase();
  // returns true if new TQuote created, false otherwise.
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  TQuote AddQuote(TStr ContentString);
  TQuote AddQuote(TStr ContentString, TInt SourceId);
  void RemoveQuote(TInt QuoteId);
  TInt GetQuoteId(TStrV& Content);
  TInt GetNewQuoteId(TStrV& Content);
  bool GetQuote(TInt QuoteId, TQuote& RefQ);
  void GetAllQuoteIds(TIntV& KeyV);
  int Len();
};

#endif
