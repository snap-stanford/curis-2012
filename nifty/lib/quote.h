#ifndef quote_h
#define quote_h

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
  TQuote();
  TQuote(TInt Id, const TStrV& Content);
  TQuote(TInt Id, const TStr& ContentString);
  TQuote(TSIn& SIn) : Id(SIn), Content(SIn), ParsedContent(SIn), ParsedContentString(SIn), Sources(SIn) { }
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  void AddSource(TInt DocId);
  void GetSources(TIntV &RefS);
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

  static PSwSet StopWordSet;
  static void ParseContentString(const TStr& ContentString, TStrV& ParsedString);
  static void FilterSpacesAndSetLowercase(TStr& QtStr);
  static void StemAndStopWordsContentString(const TStrV &ContentV, TStrV& NewContent);
  static void RemovePunctuation(const TStr& OrigString, TStr& NewString);
};

class TQuoteBase {
private:
  TInt QuoteIdCounter;
  THash<TInt, TQuote> IdToTQuotes;
  THash<TStrV, TInt> QuoteToId;
  TInt LongestSubSequenceOfWords(const TStrV& Content1, const TStrV& Content2);
public:
  TQuoteBase();
  // returns true if new TQuote created, false otherwise.
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  TInt AddQuote(const TStr &ContentString);
  TInt AddQuote(const TStr &ContentString, TInt DocId);
  void RemoveQuote(TInt QuoteId);
  TInt GetQuoteId(const TStrV &Content);
  TInt GetNewQuoteId(const TStrV &Content);
  bool GetQuote(TInt QuoteId, TQuote &RefQ) const;
  void GetAllQuoteIds(TIntV &KeyV) const;
  void RemoveQuoteDuplicateSources();
  int Len() const;
  bool IsSubstring(TInt QuoteId1, TInt QuoteId2);
  bool Exists(TInt QuoteId1);
  TInt GetCurCounterValue();
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
