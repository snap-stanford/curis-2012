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
  bool GraphFreqOverTime(TDocBase *DocBase);

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

// Date comparator
class TCmpDocByDate {
private:
  bool IsAsc;
  TDocBase *DocBase;
public:
  TCmpDocByDate(const bool& AscSort=true, TDocBase *DB=NULL) : IsAsc(AscSort) {
    DocBase = DB;
  }
  bool operator () (const TInt& P1, const TInt& P2) const {
    TDoc Doc1;
    DocBase->GetDoc(P1, Doc1);
    TDoc Doc2;
    DocBase->GetDoc(P2, Doc2);
    if (IsAsc) {
      return Doc1.GetDate().GetAbsSecs() < Doc2.GetDate().GetAbsSecs();
    } else {
      return Doc2.GetDate().GetAbsSecs() < Doc1.GetDate().GetAbsSecs();
    }
  }
};


#endif
