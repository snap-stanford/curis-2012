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

  Response = Start.GetDtYmdStr() + "\t" + End + "\t";
  Response += C.GetNumUniqueQuotes().GetStr() + "\t" + C.GetNumQuotes().GetStr() + "\t";
  Response += NumPeaks.GetStr() + "\t" + TInt(QuoteV.Len()).GetStr() + "\t";
  Response += RepStr + "\t" + RepURL + "\t";
  Response += First.GetYmdTmStr() + "\t" + Last.GetYmdTmStr() + "\t" + Peak.GetYmdTmStr() + "\t";
  Response += TBool::GetStr(TBool(C.IsArchived()));

  return Response;
}

DCluster::DCluster(TStr LineInput) {
  // Parse start/end date
  TStrV Params;
  LineInput.SplitOnStr("\t", Params);
  Start = TSecTm::GetDtTmFromYmdHmsStr(Params[0]);
  End = TSecTm::GetDtTmFromYmdHmsStr(Params[1]);
  uint StartDay = Start.GetInUnits(tmuDay);
  uint EndDay = End.GetInUnits(tmuDay);
  DiffDay = EndDay - StartDay + 1;

  // Stats/size
  Unique = TInt(Params[2].GetInt());
  Size = TInt(Params[3].GetInt());
  NumPeaks = TInt(Params[4].GetInt());
  PopStrLen = TInt(Params[5].GetInt());

  // Strings
  RepStr = Params[6];
  RepURL = Params[7];

  // Peak times
  First = TSecTm::GetDtTmFromYmdHmsStr(Params[8]);
  Last = TSecTm::GetDtTmFromYmdHmsStr(Params[9]);
  Peak = TSecTm::GetDtTmFromYmdHmsStr(Params[10]);
  bool ArchiveBool = true;
  Params[11].IsBool(ArchiveBool);
  Archived = TBool(ArchiveBool);
}

void DCluster::Save(TSOut& SOut) const {
  Start.Save(SOut);
  End.Save(SOut);
  DiffDay.Save(SOut);
  Unique.Save(SOut);
  Size.Save(SOut);
  NumPeaks.Save(SOut);
  PopStrLen.Save(SOut);
  RepStr.Save(SOut);
  RepURL.Save(SOut);
  First.Save(SOut);
  Last.Save(SOut);
  Peak.Save(SOut);
  Archived.Save(SOut);
}

void DCluster::Load(TSIn& SIn) {
  Start.Load(SIn);
  End.Load(SIn);
  DiffDay.Load(SIn);
  Unique.Load(SIn);
  Size.Load(SIn);
  NumPeaks.Load(SIn);
  PopStrLen.Load(SIn);
  RepStr.Load(SIn);
  RepURL.Load(SIn);
  First.Load(SIn);
  Last.Load(SIn);
  Peak.Load(SIn);
  Archived.Load(SIn);
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
    Err("QUOTE: %s", Quote.CStr());
    for (int i = 0; i < FreqV.Len(); i++) {
      Err("%f ", FreqV[i].Val2.Val);
    }
    Err("\n");
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

  TStr Response = Quote.GetNumSources().GetStr() + "\t" + TInt(PeakV.Len()).GetStr() + "\t" + Quote.GetContentNumWords().GetStr() + "\t";
  Response += Str + "\t" + RepURL + "\t";
  Response += First.GetYmdTmStr() + "\t" + Last.GetYmdTmStr() + "\t" + Peak.GetYmdTmStr();

  return Response;
}

DQuote::DQuote(TStr LineInput) {
  TStrV Params;
  LineInput.SplitOnStr("\t", Params);

  // Extract size, numpeaks, numwords
  Size = TInt(Params[0].GetInt());
  NumPeaks = TInt(Params[1].GetInt());
  StrLen = TInt(Params[2].GetInt());

  // Strings
  Str = Params[3];
  RepURL = Params[4];

  // dates
  First = TSecTm::GetDtTmFromYmdHmsStr(Params[5]);
  Last = TSecTm::GetDtTmFromYmdHmsStr(Params[6]);
  Peak = TSecTm::GetDtTmFromYmdHmsStr(Params[7]);

}

void DQuote::Save(TSOut& SOut) const {
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
  Size.Load(SIn);
  NumPeaks.Load(SIn);
  StrLen.Load(SIn);
  Str.Load(SIn);
  RepURL.Load(SIn);
  First.Load(SIn);
  Last.Load(SIn);
  Peak.Load(SIn);
}
