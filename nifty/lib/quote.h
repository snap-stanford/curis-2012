#ifndef quote_h
#define quote_h

#include "doc.h"

class TQuote {
private:
  TInt Id;
  TStrV Content;
  TStrV ParsedContent;
  TStr ParsedContentString;
  TVec<TUInt> Sources;

  void Init();

public:
  TQuote();
  TQuote(TInt Id, const TStrV& Content);
  TQuote(TInt Id, const TStr& ContentString);
  TQuote(TSIn& SIn) : Id(SIn), Content(SIn), ParsedContent(SIn), ParsedContentString(SIn), Sources(SIn) { }
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  void AddSource(TUInt DocId);
  void GetSources(TVec<TUInt> &RefS);
  void RemoveSources(TVec<TUInt> &ToDelete);
  void RemoveDuplicateSources();
  void GetContent(TStrV &Ref);
  void GetContentString(TStr &Ref);
  void GetParsedContent(TStrV &Ref);
  void GetParsedContentString(TStr &Ref);
  TInt GetContentNumWords();
  TInt GetParsedContentNumWords();
  TInt GetId();
  TInt GetNumDomains(TDocBase &DocBase);
  TInt GetNumSources();
  void GetPeaks(TDocBase *DocBase, TVec<TSecTm>& PeakTimesV);
  void GetPeaks(TDocBase *DocBase, TVec<TSecTm>& PeakTimesV, TInt BucketSize, TInt SlidingWindowSize, TSecTm PresentTime);
  void GraphFreqOverTime(TDocBase *DocBase, TStr Filename, TSecTm PresentTime);
  void GraphFreqOverTime(TDocBase *DocBase, TStr Filename, TInt BucketSize, TInt SlidingWindowSize, TSecTm PresentTime);
  void GetRepresentativeUrl(TDocBase *DocBase, TStr& RepUrl);
};

class TQuoteBase {
private:
  TInt QuoteIdCounter;
  THash<TInt, TQuote> IdToTQuotes;
  THash<TStrV, TInt> QuoteToId;

  static const TStr TopNewsSourcesFile;
  static TStrSet TopNewsSources;
  static void InitTopNewsSources();
public:
  TQuoteBase();
  // returns true if new TQuote created, false otherwise.
  TQuoteBase& operator=(const TQuoteBase& QB) {
    QuoteIdCounter = QB.QuoteIdCounter;
    IdToTQuotes = QB.IdToTQuotes;
    QuoteToId = QB.QuoteToId;
    return *this;
  };
  TQuoteBase(TInt OldCounter);

  TInt GetCounter();
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  TInt AddQuote(const TStr &ContentString);
  TInt AddQuote(const TStr &ContentString, TUInt DocId);
  void AddStaticQuote(TInt QuoteId, TQuote &Quote);
  void AddQuoteMerging(TInt QuoteId, const TStr& ContentString, TUInt DocId);
  void RemoveQuote(TInt QuoteId);
  TInt GetQuoteId(const TStrV &Content) const;
  TInt GetNewQuoteId(const TStrV &Content);
  bool GetQuote(TInt QuoteId, TQuote &RefQ) const;
  void GetAllQuoteIds(TIntV &KeyV) const;
  void RemoveQuoteDuplicateSources();
  int Len() const;
  bool IsSubstring(TInt QuoteId1, TInt QuoteId2);
  bool Exists(TInt QuoteId1);
  TInt GetCurCounterValue();
  void GetRepresentativeUrl(TDocBase *DocBase, TInt QuoteId, TStr& RepUrl);
  bool IsContainNullQuote();
  static bool IsUrlTopNewsSource(TStr Url);
  void GetIdToTQuotes(THash<TInt, TQuote>& Pointer);
  void PrintHashTableSizes();
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

// Compares two quotes by their id
class TCmpQuoteById {
private:
  bool IsAsc;
  TQuoteBase *QuoteBase;
public:
  TCmpQuoteById(const bool& AscSort=true, TQuoteBase *QB=NULL) : IsAsc(AscSort) {
    QuoteBase = QB;
  }
  bool operator () (const TInt& P1, const TInt& P2) const {
    TQuote Quote1;
    QuoteBase->GetQuote(P1, Quote1);
    TQuote Quote2;
    QuoteBase->GetQuote(P2, Quote2);
    if (IsAsc) {
      return Quote1.GetId() < Quote2.GetId();
    } else {
      return Quote2.GetId() < Quote1.GetId();
    }
  }
};

#endif
