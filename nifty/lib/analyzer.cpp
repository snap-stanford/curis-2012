#include "analyzer.h"
#include "quote.h"

TStr DCluster::GetClusterString(TQuoteBase *QB, TDocBase *DB, TCluster& C, TFreqTripleV &FreqV, TInt NumPeaks, TStr &End) {
  TStr Response;
  TSecTm Start;
  TStr RepStr, RepURL;
  C.GetBirthDate(Start);
  C.GetRepresentativeQuoteString(RepStr, QB);
  C.GetRepresentativeQuoteURL(QB, DB, RepURL);
  TStrV QuoteV;
  RepStr.SplitOnStr(" ", QuoteV);

  TSecTm First, Last, Peak;
  DCluster::GetFMP(FreqV, First, Last, Peak, RepStr);

  Response = C.GetId().GetStr() + "\t" + Start.GetDtYmdStr() + "\t" + End + "\t";
  Response += C.GetNumUniqueQuotes().GetStr() + "\t" + C.GetNumQuotes().GetStr() + "\t";
  Response += NumPeaks.GetStr() + "\t" + TInt(QuoteV.Len()).GetStr() + "\t";
  Response += RepStr + "\t" + RepURL + "\t";
  Response += First.GetYmdTmStr() + "\t" + Last.GetYmdTmStr() + "\t" + Peak.GetYmdTmStr() + "\t";
  Response += TBool::GetStr(TBool(C.IsArchived())) + "\t" + C.GetDiscardState().GetStr();

  return Response;
}

TStr DCluster::GetDescription() {
  TStr Response = "# Clusters: Id\tStart\tEnd\t";
  Response += "Variants\tSize\tPeaks\tNumWords\tLongest Variant\tRepresentative URL\t";
  Response += "First Mention Time\tLast Mention Time\tPeak Time\tArchived\tDiscard State";
  return Response;
}

DCluster::DCluster(TStr LineInput) {
  // Parse start/end date
  TStrV Params;
  LineInput.SplitOnStr("\t", Params);
  Id = TInt(Params[0].GetInt());
  Start = TSecTm::GetDtTmFromYmdHmsStr(Params[1]);
  End = TSecTm::GetDtTmFromYmdHmsStr(Params[2]);
  uint StartDay = Start.GetInUnits(tmuDay);
  uint EndDay = End.GetInUnits(tmuDay);
  DiffDay = EndDay - StartDay + 1;

  // Stats/size
  Unique = TInt(Params[3].GetInt());
  Size = TInt(Params[4].GetInt());
  NumPeaks = TInt(Params[5].GetInt());
  RepStrLen = TInt(Params[6].GetInt());

  // Strings
  RepStr = Params[7];
  RepURL = Params[8];

  // Peak times
  First = TSecTm::GetDtTmFromYmdHmsStr(Params[9]);
  Last = TSecTm::GetDtTmFromYmdHmsStr(Params[10]);
  Peak = TSecTm::GetDtTmFromYmdHmsStr(Params[11]);
  bool ArchiveBool = true;
  Params[12].IsBool(ArchiveBool);
  Archived = TBool(ArchiveBool);
  DiscardState = TInt(Params[13].GetInt());
}

void DCluster::Save(TSOut& SOut) const {
  Id.Save(SOut);
  Start.Save(SOut);
  End.Save(SOut);
  DiffDay.Save(SOut);
  Unique.Save(SOut);
  Size.Save(SOut);
  NumPeaks.Save(SOut);
  RepStrLen.Save(SOut);
  RepStr.Save(SOut);
  RepURL.Save(SOut);
  First.Save(SOut);
  Last.Save(SOut);
  Peak.Save(SOut);
  Archived.Save(SOut);
  DiscardState.Save(SOut);
  Quotes.Save(SOut);
}

void DCluster::Load(TSIn& SIn) {
  Id.Load(SIn);
  Start.Load(SIn);
  End.Load(SIn);
  DiffDay.Load(SIn);
  Unique.Load(SIn);
  Size.Load(SIn);
  NumPeaks.Load(SIn);
  RepStrLen.Load(SIn);
  RepStr.Load(SIn);
  RepURL.Load(SIn);
  First.Load(SIn);
  Last.Load(SIn);
  Peak.Load(SIn);
  Archived.Load(SIn);
  DiscardState.Load(SIn);
  Quotes.Load(SIn);
}

void DCluster::GetFMP(TFreqTripleV& FreqV, TSecTm& First, TSecTm& Last, TSecTm& Peak, TStr &Quote) {
  TInt FreqLength = FreqV.Len();
  TInt MaxVal = -1, MinIndex = -1, MaxIndex = 0;
  for (int i = 0; i < FreqLength; ++i) {
    // calculate first and last.
    if (FreqV[i].Val2 > 0) {
      MaxIndex = i;
      if (MinIndex < 0) {
        MinIndex = i;
      }
    }
    // calculate peak
    if (FreqV[i].Val2 > MaxVal) {
      MaxVal = FreqV[i].Val2;
      Peak = FreqV[i].Val3;
    }
  }
  if (MinIndex >= 0) {
    First = FreqV[MinIndex].Val3;
  } else {
    /*Err("QUOTE: %s", Quote.CStr());
    for (int i = 0; i < FreqV.Len(); i++) {
      Err("%f ", FreqV[i].Val2.Val);
    }
    Err("\n");*/
    First = FreqV[0].Val3;
  }
  Last = FreqV[MaxIndex].Val3;
}

TStr DQuote::GetQuoteString(TDocBase *DB, TQuote& Quote, TSecTm &PresentTime) {
  TStr Str, RepURL;
  Quote.GetContentString(Str);
  Quote.GetRepresentativeUrl(DB, RepURL);

  TFreqTripleV FreqV, PeakV;
  TIntV Sources;
  Quote.GetSources(Sources);
  Peaks::GetPeaks(DB, Sources, PeakV, FreqV, PEAK_BUCKET, PEAK_WINDOW, PresentTime);
  TSecTm First, Last, Peak;
  DCluster::GetFMP(FreqV, First, Last, Peak, Str);

  TStr Response = Quote.GetId().GetStr() + "\t" + Quote.GetNumSources().GetStr() + "\t" + TInt(PeakV.Len()).GetStr() + "\t" + Quote.GetContentNumWords().GetStr() + "\t";
  Response += Str + "\t" + RepURL + "\t";
  Response += First.GetYmdTmStr() + "\t" + Last.GetYmdTmStr() + "\t" + Peak.GetYmdTmStr();

  return Response;
}

void DQuote::GetQuoteSources(TDocBase *DB, TQuote& Quote, TStrV& SourceStrings) {
  TIntV Sources;
  Quote.GetSources(Sources);

  for (int i = 0; i < Sources.Len(); i++) {
    TDoc Doc;
    DB->GetDoc(Sources[i], Doc);
    TStr URL;
    Doc.GetUrl(URL);
    SourceStrings.Add(Doc.GetId().GetStr() + "\t" + Doc.GetDate().GetYmdTmStr() + "\t" + URL);
  }
}

TStr DQuote::GetDescription() {
  return "# Quotes: Id\tSize\tPeaks\tNumWords\tQuote String\tRepresentative URL\tFirst Mention\tLast Mention\tPeak Time";
}

DQuote::DQuote(TStr LineInput) {
  TStrV Params;
  LineInput.SplitOnStr("\t", Params);

  // Extract size, numpeaks, numwords
  Id = TInt(Params[0].GetInt());
  Size = TInt(Params[1].GetInt());
  NumPeaks = TInt(Params[2].GetInt());
  StrLen = TInt(Params[3].GetInt());

  // Strings
  Str = Params[4];
  RepURL = Params[5];

  // dates
  First = TSecTm::GetDtTmFromYmdHmsStr(Params[6]);
  Last = TSecTm::GetDtTmFromYmdHmsStr(Params[7]);
  Peak = TSecTm::GetDtTmFromYmdHmsStr(Params[8]);

}

void DQuote::Save(TSOut& SOut) const {
  Id.Save(SOut);
  Size.Save(SOut);
  NumPeaks.Save(SOut);
  StrLen.Save(SOut);
  Str.Save(SOut);
  RepURL.Save(SOut);
  First.Save(SOut);
  Last.Save(SOut);
  Peak.Save(SOut);
}

void DQuote::Load(TSIn& SIn) {
  Id.Load(SIn);
  Size.Load(SIn);
  NumPeaks.Load(SIn);
  StrLen.Load(SIn);
  Str.Load(SIn);
  RepURL.Load(SIn);
  First.Load(SIn);
  Last.Load(SIn);
  Peak.Load(SIn);
}
