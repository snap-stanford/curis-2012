#ifndef quote_h
#define quote_h

#include "stdafx.h"
#include "doc.h"

class TQuote {
private:
  TInt Id;
  TStrV Content;
  TStrV ParsedContent;
  TStr ParsedContentString;
  TIntV Sources;

  void Init();

public:
  static const uint NumSecondsInHour;
  static const uint NumSecondsInWeek;

  TQuote();
  TQuote(TInt Id, const TStrV& Content);
  TQuote(TInt Id, const TStr& ContentString);
  TQuote(TSIn& SIn) : Id(SIn), Content(SIn), ParsedContent(SIn), ParsedContentString(SIn), Sources(SIn) { }
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  void AddSource(TInt SourceId);
  void GetSources(TIntV &RefS);
  void GetContent(TStrV &Ref);
  void GetContentString(TStr &Ref);
  void GetParsedContent(TStrV &Ref);
  void GetParsedContentString(TStr &Ref);
  TInt GetContentNumWords();
  TInt GetParsedContentNumWords();
  TInt GetId();
  TInt GetNumDomains(TDocBase *DocBase);
  TInt GetNumSources();
  bool GraphFreqOverTime(TDocBase *DocBase, TStr Filename);

  static PSwSet StopWordSet;
  static void ParseContentString(const TStr &ContentString, TStrV &ParsedString);
  static void StemAndStopWordsContentString(const TStrV &ContentV, TStrV &NewContent);
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
  TQuote AddQuote(const TStr &ContentString);
  TQuote AddQuote(const TStr &ContentString, TInt SourceId);
  void RemoveQuote(TInt QuoteId);
  TInt GetQuoteId(const TStrV &Content);
  TInt GetNewQuoteId(const TStrV &Content);
  bool GetQuote(TInt QuoteId, TQuote &RefQ);
  void GetAllQuoteIds(TIntV &KeyV);
  int Len();
};

// Compares two quotes by their frequency
class TCmpQuoteByFreq {
private:
  bool IsAsc;
  TQuoteBase *QuoteBase;
public:
  TCmpQuoteByFreq(const bool& AscSort=true, TQuoteBase *QB=NULL) : IsAsc(AscSort) {
    QuoteBase = QB;
  }
  bool operator () (const TInt& P1, const TInt& P2) const {
    TQuote Quote1;
    QuoteBase->GetQuote(P1, Quote1);
    TQuote Quote2;
    QuoteBase->GetQuote(P2, Quote2);
    if (IsAsc) {
      return Quote1.GetNumSources() < Quote2.GetNumSources();
    } else {
      return Quote2.GetNumSources() < Quote1.GetNumSources();
    }
  }
};

#endif
