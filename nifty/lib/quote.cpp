#include "quote.h"
#include "doc.h"
#include "peaks.h"

PSwSet TQuote::StopWordSet = new TSwSet(swstEnMsdn);
const TStr TQuoteBase::TopNewsSourcesFile = TStr("resources/ranked_news_sources.txt");

TQuote::TQuote() {
}

TQuote::TQuote(TInt Id, const TStrV& Content) {
  this->Id = Id;
  this->Content = Content; // TODO: deep copy?
  Init();
}

TQuote::TQuote(TInt Id, const TStr &ContentString) {
  this->Id = Id;
	ParseContentString(ContentString, Content);
	Init();
}

void TQuote::Init() {
  TStr FilteredContentString;
  GetContentString(FilteredContentString);
  FilterSpacesAndSetLowercase(FilteredContentString);
  TStrV FilteredContent;
  ParseContentString(FilteredContentString, FilteredContent);
  StemAndStopWordsContentString(FilteredContent, ParsedContent);
  for (int i = 0; i < ParsedContent.Len(); ++i) {
    if (i > 0) this->ParsedContentString.InsStr(ParsedContentString.Len()," ");
    this->ParsedContentString.InsStr(ParsedContentString.Len(), ParsedContent[i]);
  }
}

void TQuote::Save(TSOut& SOut) const {
  Id.Save(SOut);
  Content.Save(SOut);
  ParsedContent.Save(SOut);
  ParsedContentString.Save(SOut);
  Sources.Save(SOut);
}

void TQuote::Load(TSIn& SIn) {
  Id.Load(SIn);
  Content.Load(SIn);
  ParsedContent.Load(SIn);
  ParsedContentString.Load(SIn);
  Sources.Load(SIn);
}


void TQuote::GetContent(TStrV &Ref) {
  Ref = Content;
}

void TQuote::GetContentString(TStr &Ref) {
  for (int i = 0; i < Content.Len(); ++i) {
    if (i > 0)  Ref.InsStr(Ref.Len()," ");
    Ref.InsStr(Ref.Len(), Content[i]);
  }
}

void TQuote::GetParsedContent(TStrV &Ref) {
  Ref = ParsedContent;
}

void TQuote::GetParsedContentString(TStr &Ref) {
  Ref = ParsedContentString;
}

TInt TQuote::GetContentNumWords() {
  return Content.Len();
}

TInt TQuote::GetParsedContentNumWords() {
  return ParsedContent.Len();
}

TInt TQuote::GetId() {
  return Id;
}

TInt TQuote::GetNumDomains(TDocBase &DocBase) {
  THashSet<TStr> DomSet;
  for (int u = 0; u < Sources.Len(); u++) {
    TDoc Doc;
    if(DocBase.GetDoc(Sources[u], Doc)) {
      TStr DocUrl;
      Doc.GetUrl(DocUrl);
      DomSet.AddKey(TStrUtil::GetDomNm(DocUrl));
    }
  }
  return DomSet.Len();
}

TInt TQuote::GetNumSources() {
  return Sources.Len();
}

void TQuote::AddSource(TInt DocId) {
  Sources.Add(DocId);
}

void TQuote::GetSources(TIntV &RefS) {
  RefS = Sources;
}

void TQuote::RemoveDuplicateSources() {
  TIntV UniqueSources;
  Sources.Sort();
  for (int i = 0; i < Sources.Len(); i++) {
    if (i == 0 || Sources[i] != Sources[i-1]) {
      UniqueSources.Add(Sources[i]);
    }
  }
  Sources = UniqueSources;
}

void TQuote::ParseContentString(const TStr &ContentString, TStrV &ParsedString) {
  ContentString.SplitOnAllAnyCh(" ", ParsedString);
}

void TQuote::FilterSpacesAndSetLowercase(TStr &QtStr) {
  // Three passes...hopefully this isn't too slow.
  TChA QtChA = TChA(QtStr);
  for (int i = 0; i < QtChA.Len(); i++) {
    if (!(isalpha(QtChA[i]) || QtChA[i] == '\'')) {
      QtChA[i] = ' ';
    } else {
      QtChA[i] = tolower(QtChA[i]);
    }
  }
  QtStr = TStr(QtChA);
  TStrV WordV;
  QtStr.SplitOnAllAnyCh(" ", WordV);
  QtStr.Clr();
  for (int i = 0; i < WordV.Len(); ++i) {
    if (i > 0)  QtStr.InsStr(QtStr.Len()," ");
    QtStr.InsStr(QtStr.Len(), WordV[i]);
  }
}

void TQuote::StemAndStopWordsContentString(const TStrV &ContentV, TStrV &NewContent) {
  for (int i = 0; i < ContentV.Len(); ++i) {
    if (!StopWordSet->IsIn(ContentV[i], false)) {
      NewContent.Add(TPorterStemmer::StemX(ContentV[i]).GetLc());
    }
  }
}

void TQuote::GetPeaks(TDocBase *DocBase, TVec<TSecTm>& PeakTimesV) {
  GetPeaks(DocBase, PeakTimesV, TInt(1), TInt(1), TSecTm(0));
}

void TQuote::GetPeaks(TDocBase *DocBase, TVec<TSecTm>& PeakTimesV, TInt BucketSize, TInt SlidingWindowSize, TSecTm PresentTime) {
  TFreqTripleV PeakV;
  Peaks::GetPeaks(DocBase, Sources, PeakV, BucketSize, SlidingWindowSize, PresentTime);
  for (int i = 0; i < PeakV.Len(); ++i) {
    PeakTimesV.Add(PeakV[i].Val3);
  }
}

void TQuote::GraphFreqOverTime(TDocBase *DocBase, TStr Filename, TSecTm PresentTime) {
  GraphFreqOverTime(DocBase, Filename, TInt(1), TInt(1), PresentTime);
}

/// If BucketSize is > 1, a sliding window average will not be calculated
//  Otherwise, if BucketSize = 1, a sliding window average of size SlidingWindowSize will be calculated
void TQuote::GraphFreqOverTime(TDocBase *DocBase, TStr Filename, TInt BucketSize, TInt SlidingWindowSize, TSecTm PresentTime) {
  TFreqTripleV PeakTimesV;
  TFreqTripleV FreqTripleV;
  Peaks::GetPeaks(DocBase, Sources, PeakTimesV, FreqTripleV, BucketSize, SlidingWindowSize, PresentTime);

  TVec<TIntFltPr> PeakV;
  for (int i = 0; i < PeakTimesV.Len(); ++i) {
    PeakV.Add(TIntFltPr(PeakTimesV[i].Val1, PeakTimesV[i].Val2));
  }

  TVec<TIntFltPr> FreqV;
  for (int i = 0; i < FreqTripleV.Len(); ++i) {
    FreqV.Add(TIntFltPr(FreqTripleV[i].Val1, FreqTripleV[i].Val2));
  }

  TStr ContentStr;
  GetContentString(ContentStr);
  TGnuPlot GP("./plots/" + Filename, "Frequency of Quote " + Id.GetStr() + " Over Time: " + ContentStr);
  GP.SetXLabel(TStr("Hour Offset"));
  GP.SetYLabel(TStr("Frequency of Quote"));
  GP.AddPlot(FreqV, gpwLinesPoints, "Frequency");
  if (PeakV.Len() > 0) {
    GP.AddPlot(PeakV, gpwPoints, "Peaks");
  }
  TStr SetXTic = TStr("set xtics 24\nset terminal png small size 1000,800");
  GP.SavePng("./plots/" + Filename + ".png", 1000, 800, TStr(), SetXTic);
}

/// Removes all punctuation (i.e. non-alphanumeric/whitespace characters) from the string
void TQuote::RemovePunctuation(const TStr& OrigString, TStr& NewString) {
  TChA OrigChA(OrigString);
  TChA NewChA;
  for (int i = 0; i < OrigChA.Len(); i++) {
    if (TCh::IsAlNum(OrigChA[i]) || TCh::IsWs(OrigChA[i])) {
      NewChA.AddCh(OrigChA[i]);
    }
  }
  NewString = TStr(NewChA);
}

TQuoteBase::TQuoteBase() {
  QuoteIdCounter = 0;
}

/// Adds quote string to quote base; returns quote's quote id
TInt TQuoteBase::AddQuote(const TStr &ContentString) {
  TStrV ContentVectorString;
  TQuote::ParseContentString(ContentString, ContentVectorString);
  TInt QuoteId = GetNewQuoteId(ContentVectorString);
  if (IdToTQuotes.IsKey(QuoteId)) {
    return QuoteId; // nothing to do here; quote is already in database
  } else {
    // otherwise, create the new TQuote and proceed.
    //printf("%d: %s\n", QuoteId.Val, ContentString.CStr());
    TQuote NewQuote(QuoteId, ContentVectorString);
    IdToTQuotes.AddDat(QuoteId, NewQuote);
    return NewQuote.GetId();
  }
}

TInt TQuoteBase::AddQuote(const TStr &ContentString, TInt DocId) {
  //TQuote NewQuote = AddQuote(ContentString);
  //NewQuote.AddSource(SourceDoc);
  //return NewQuote;

  TStrV ContentVectorString;
  TQuote::ParseContentString(ContentString, ContentVectorString);
  TInt QuoteId = GetNewQuoteId(ContentVectorString);
  if (IdToTQuotes.IsKey(QuoteId)) {
    TQuote CurQuote =  IdToTQuotes.GetDat(QuoteId); // nothing to do here; quote is already in database
    CurQuote.AddSource(DocId);
    IdToTQuotes.AddDat(QuoteId, CurQuote);
    return CurQuote.GetId();
  } else {
    // otherwise, create the new TQuote and proceed.
    //printf("%d: %s\n", QuoteId.Val, ContentString.CStr());
    TQuote NewQuote(QuoteId, ContentVectorString);
    NewQuote.AddSource(DocId);
    IdToTQuotes.AddDat(QuoteId, NewQuote);
    return NewQuote.GetId();
  }
}

void TQuoteBase::RemoveQuote(TInt QuoteId) {
  // TODO: memory management
  if (IdToTQuotes.IsKey(QuoteId)) {
    TQuote CurQuote = IdToTQuotes.GetDat(QuoteId);
    TStrV CurContent;
    CurQuote.GetContent(CurContent);
    if (QuoteToId.IsKey(CurContent)) {
      QuoteToId.DelKey(CurContent);
    }
    IdToTQuotes.DelKey(QuoteId);
  }
}

TInt TQuoteBase::GetNewQuoteId(const TStrV& Content) {
  if (QuoteToId.IsKey(Content)) {
    return QuoteToId.GetDat(Content);
  } else {
    // this will increment QuoteIdCounter after returning.
    TInt NewId = QuoteIdCounter;
    QuoteIdCounter++;
    QuoteToId.AddDat(Content, NewId);
    return NewId;
  }
}

TInt TQuoteBase::GetQuoteId(const TStrV &Content) {
  if (QuoteToId.IsKey(Content)) {
    return QuoteToId.GetDat(Content);
  } else {
    return -1;
  }
}

bool TQuoteBase::GetQuote(TInt QuoteId, TQuote &RefQ) const {
  if (IdToTQuotes.IsKey(QuoteId)) {
    RefQ = IdToTQuotes.GetDat(QuoteId);
    return true;
  }
  return false;
}

void TQuoteBase::RemoveQuoteDuplicateSources() {
  TIntV QuoteIds;
  GetAllQuoteIds(QuoteIds);
  for (int i = 0; i < QuoteIds.Len(); i++) {
    TQuote Q;
    GetQuote(QuoteIds[i], Q);
    Q.RemoveDuplicateSources();
    IdToTQuotes.AddDat(QuoteIds[i], Q);
  }
}

int TQuoteBase::Len() const {
  return IdToTQuotes.Len();
}

void TQuoteBase::GetAllQuoteIds(TIntV &KeyV) const {
  IdToTQuotes.GetKeyV(KeyV);
}

void TQuoteBase::Save(TSOut &SOut) const {
  QuoteIdCounter.Save(SOut);
  IdToTQuotes.Save(SOut);
  QuoteToId.Save(SOut);
}

void TQuoteBase::Load(TSIn& SIn) {
  QuoteIdCounter.Load(SIn);
  IdToTQuotes.Load(SIn);
  QuoteToId.Load(SIn);
}

/// Returns true if one of the quotes is a "substring" of the other, and false otherwise.
//  Compares the parsed content of the quotes, rather than the original content
//  Based off the method TQuoteBs::IsLinkPhrases in memes.cpp of memecluster.
bool TQuoteBase::IsSubstring(TInt QuoteId1, TInt QuoteId2) {
  TQuote Quote1, Quote2;
  GetQuote(QuoteId1, Quote1);
  GetQuote(QuoteId2, Quote2);
  TInt ShortLen = TMath::Mn(Quote1.GetContentNumWords(), Quote2.GetContentNumWords()); // changed from GetParsedContentNumWords
  if (ShortLen <= 3) return false;  // Skip short quotes
  // Get content of quote, with punctuation removed
  TStr ContentStr1, ContentStr2, ContentStrNoPunc1, ContentStrNoPunc2;
  Quote1.GetContentString(ContentStr1);
  Quote2.GetContentString(ContentStr2);
  TQuote::RemovePunctuation(ContentStr1, ContentStrNoPunc1);
  TQuote::RemovePunctuation(ContentStr2, ContentStrNoPunc2);

  TStrV Content1V, Content2V;
  ContentStrNoPunc1.SplitOnWs(Content1V);
  ContentStrNoPunc2.SplitOnWs(Content2V);

  TInt Overlap = LongestSubSequenceOfWords(Content1V, Content2V);
  if ((ShortLen == 4 || ShortLen == 5) && Overlap == ShortLen) { fprintf(stderr, "reason A\n"); return true; } // full overlap, no skip
  else if ((ShortLen == 6 && Overlap >= 5 )) { fprintf(stderr, "reason B\n"); return true; }
  else if (Overlap/double(ShortLen+3) > 0.5 || Overlap > 10) { fprintf(stderr, "reason C\n"); return true; }
  return false;
}

/// Finds the length of the longest common subsequence of words
//  Based off the method TQuoteBs::LongestCmnSubSq(const TIntV& WIdV1, const TIntV& WIdV2,
//  int& WIdV1Start, int& WIdV2Start, int& SkipId) in memes.cpp of memecluster
TInt TQuoteBase::LongestSubSequenceOfWords(const TStrV& Content1, const TStrV& Content2) {
  const TStrV& V1 = Content1.Len() > Content2.Len() ? Content1:Content2; // longer quote
  const TStrV& V2 = Content1.Len() > Content2.Len() ? Content2:Content1; // shorter quote
  TInt V1Len = V1.Len();
  TInt V2Len = V2.Len();

  THash<TStr, TIntV> SharedWordsH;  // Maps the words in the shorter quote to their position in the longer quote
  THashSet<TStr> V2WordsSet;  // Stores all words in shorter quote

  TInt WordV1Start, WordV2Start, SkipId;
  WordV1Start = WordV2Start = SkipId = 0;
  for (int i = 0; i < V2Len; i++) {
    V2WordsSet.AddKey(V2[i]);
  }
  for (int i = 0; i < V1Len; i++) {
    if (V2WordsSet.IsKey(V1[i])) {
      SharedWordsH.AddDat(V1[i]).Add(i);
    }
  }

  // Counts the length of each subsequence and finds the longest
  int MaxLen = 0;
  for (int w = 0; w < V2Len; w++) {
    TStr Word = V2[w];
    if (! SharedWordsH.IsKey(Word)) { continue; }  // Skip words in shorter quote but not in longer quote
    TIntV& OccV = SharedWordsH.GetDat(Word);  // Occurrences of this word in the longer quote
    for (int o = 0; o < OccV.Len(); o++) {
      int beg = OccV[o];
      int cnt = 0;  // Stores the number of words in the subsequency
      int tmp = 0;
      while (w+cnt < V2Len && beg+cnt < V1Len && V2[w+cnt]==V1[beg+cnt]) { cnt++; tmp=0; }           // no skip
      //while (beg+1+cnt < V1Len && w+cnt < V2Len && V2[w+cnt]==V1[beg+cnt+1]) { cnt++; tmp=-1; }      // skip word in long
      //while (beg+cnt+1 < V1Len && w+cnt+1 < V2Len && V2[w+cnt+1]==V1[beg+cnt+1]) {  cnt++; tmp=-2;}  // skip word in both
      //while (beg+cnt < V1Len && w+cnt+1 < V2Len && V2[w+cnt+1]==V1[beg+cnt]) { cnt++; tmp=-3;}       // skip word in short
      if (MaxLen < cnt) {
        MaxLen = cnt;
        SkipId=tmp;
        WordV1Start = beg;
        WordV2Start = w;
      }
      IAssert(cnt >= 1);
    }
  }
  if (! (Content1.Len()>Content2.Len())) {
    int tmp = WordV1Start;
    WordV1Start = WordV2Start;
    WordV2Start = tmp;
  }
  return MaxLen;
}

bool TQuoteBase::Exists(TInt QuoteId) {
  return IdToTQuotes.IsKey(QuoteId);
}

TInt TQuoteBase::GetCurCounterValue() {
  return QuoteIdCounter;
}

void TQuoteBase::InitTopNewsSources() {
  TFIn F(TopNewsSourcesFile.CStr());
  TStr NewsSource;
  while (!F.Eof() && F.GetNextLn(NewsSource)) {
    TopNewsSources.AddKey(NewsSource);
  }
}

bool TQuoteBase::IsUrlTopNewsSource(TStr Url) {
  if (TopNewsSources.Len() == 0) {
    InitTopNewsSources();
  }

  TStr NewUrl(Url);
  // Remove the http://
  TStr HttpStr("http://");
  if (Url.SearchStr(HttpStr) >= 0) {
    TStr Ignore;
    Url.SplitOnStr(Ignore, HttpStr, NewUrl);
    if (NewUrl.Len() == 0) { return false; }
  }

  TStrV PeriodVector;
  NewUrl.SplitOnAllAnyCh(".", PeriodVector);

  if (PeriodVector.Len() >= 2) {
    TStrV SlashVector;
    PeriodVector[PeriodVector.Len() - 1].SplitOnAllAnyCh("/", SlashVector);
    if (SlashVector.Len() >= 1) {
      TStr DomainName = PeriodVector[PeriodVector.Len() - 2] + "." + SlashVector[0];
      if (TopNewsSources.IsKey(DomainName)) { return true; }
      // Deal with domains ending in co.uk
      TStr CoUk("co.uk");
      if (DomainName == CoUk && PeriodVector.Len() > 2) {
        DomainName = PeriodVector[PeriodVector.Len() - 3] + DomainName;
        if (TopNewsSources.IsKey(DomainName)) { return true; }
      }
    }
  }
  return false;
}

/// Picks the first url with the domain in the whitelist of top news sources.
//  If no url qualifies, picks the first one within the time period of the first peak
void TQuoteBase::GetRepresentativeUrl(TDocBase *DocBase, TInt QuoteId, TStr& RepUrl) {
  TQuote Q;
  if (!IdToTQuotes.IsKeyGetDat(QuoteId, Q)) { return; }

  // Sort the sources by time (ascending)
  TIntV SourcesSorted;
  Q.GetSources(SourcesSorted);
  SourcesSorted.SortCmp(TCmpDocByDate(true, DocBase));

  // Pick the first url with the domain in the whitelist
  for (int i = 0; i < SourcesSorted.Len(); i++) {
    TDoc Doc;
    TStr DocUrl;
    DocBase->GetDoc(SourcesSorted[i], Doc);
    Doc.GetUrl(DocUrl);
    if (IsUrlTopNewsSource(DocUrl)) {
      RepUrl = DocUrl;
      fprintf(stderr, "URL matched news source on whitelist!\n");
      return;
    }
  }

  // If no url qualifies, pick the first one within the time period of the first peak
  fprintf(stderr, "No match :(\n");

  TVec<TSecTm> PeakTimesV;
  Q.GetPeaks(DocBase, PeakTimesV, 2, 1, TSecTm(0));

  // If there are no peaks, return the first document
  if (PeakTimesV.Len() == 0) {
    TDoc Doc;
    DocBase->GetDoc(SourcesSorted[0], Doc);
    Doc.GetUrl(RepUrl);
    return;
  }

  TDoc Doc;
  for(int i = 0; i < SourcesSorted.Len(); i++) {
    DocBase->GetDoc(SourcesSorted[i], Doc);
    if (PeakTimesV[0] <= Doc.GetDate()) { break; }
  }
  Doc.GetUrl(RepUrl);
}

