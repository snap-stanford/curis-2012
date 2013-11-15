#include "stdafx.h"
#include "cluster.h"
#include "quote.h"
#include "peaks.h"
#include "clustering.h"

const int TClusterBase::FrequencyCutoff = 100;

TCluster::TCluster() {
  Archived = true;
  DeathDate = BirthDate;
}

TCluster::TCluster(TIntV& RepresentativeQuoteIds, TInt NumQuotes, TIntV QuoteIds, TQuoteBase *QB, TDocBase *DB, TSecTm BirthDate) {
  // TODO: Check that URLs are not repeated
  this->RepresentativeQuoteIds = RepresentativeQuoteIds;
  TVec<TUInt> UniqueSources;
  TCluster::GetUniqueSources(UniqueSources, QuoteIds, QB);
  this->NumQuotes = UniqueSources.Len();
  this->QuoteIds = QuoteIds;
  this->Id = 1;
  this->BirthDate = BirthDate;
  this->DeathDate = BirthDate;
  this->Archived = true;
  UpdateMaxPeak(QuoteIds, UniqueSources, QB, DB);

  /*this->RepresentativeQuoteIds = RepresentativeQuoteIds;
  this->NumQuotes = NumQuotes;
  this->QuoteIds = QuoteIds;
  this->Id = 1;*/
}

void TCluster::Save(TSOut& SOut) const {
  RepresentativeQuoteIds.Save(SOut);
  NumQuotes.Save(SOut);
  QuoteIds.Save(SOut);
  Id.Save(SOut);
  PeakTimesV.Save(SOut);
  FreqV.Save(SOut);
  BirthDate.Save(SOut);
  Archived.Save(SOut);
  DiscardState.Save(SOut);
  MaxPeakDateFreq.Save(SOut);
  DeathDate.Save(SOut);
}

void TCluster::Load(TSIn& SIn) {
  RepresentativeQuoteIds.Load(SIn);
  NumQuotes.Load(SIn);
  QuoteIds.Load(SIn);
  Id.Load(SIn);
  PeakTimesV.Load(SIn);
  FreqV.Load(SIn);
  BirthDate.Load(SIn);
  Archived.Load(SIn);
  DiscardState.Load(SIn);
  MaxPeakDateFreq.Load(SIn);
  DeathDate.Load(SIn);
  // ./memeseed -start 2013-03-08 -window 2 -directory log -qbdb /lfs/1/tmp/chantat/nifty/QBDB/ -qbdbc /lfs/1/tmp/curis/QBDBC-new/
}

void TCluster::Archive() {
  Archived = true;
}

bool TCluster::IsArchived() {
  return Archived;
}

TInt TCluster::GetDiscardState() {
  return DiscardState;
}

void TCluster::SetDiscardState(TInt State) {
  DiscardState = State;
}

void TCluster::SetBirthDate(TSecTm& BirthDate) {
  this->BirthDate = BirthDate;
}

void TCluster::GetBirthDate(TSecTm& BirthDate) {
  BirthDate = this->BirthDate;
}

TInt TCluster::GetNumRepresentativeQuoteIds() const {
  return RepresentativeQuoteIds.Len();
}

void TCluster::GetRepresentativeQuoteIds(TIntV& RepQuoteIds) const {
  RepQuoteIds = RepresentativeQuoteIds;
}

TInt TCluster::GetMostPopularQuoteId(TQuoteBase *QB) {
  TInt PopQuote, PopFreq = -1;
  for (int i = 0; i < QuoteIds.Len(); i++) {
    TQuote Quote;
    QB->GetQuote(QuoteIds[i], Quote);
    if (Quote.GetNumSources() > PopFreq) {
      PopFreq = Quote.GetNumSources();
      PopQuote = QuoteIds[i];
    }
  }
  return PopQuote;
}


void TCluster::GetRepresentativeQuoteString(TStr& RepStr, TQuoteBase *QB) const {
  if(RepresentativeQuoteIds.Len() == 0) return;
  TQuote FirstQuote;
  QB->GetQuote(RepresentativeQuoteIds[0], FirstQuote);
  TStr FirstContentString;
  FirstQuote.GetContentString(FirstContentString);
  RepStr += FirstContentString;
  for (int i = 1; i < RepresentativeQuoteIds.Len(); ++i) {
    TQuote Quote;
    QB->GetQuote(RepresentativeQuoteIds[i], Quote);
    TStr ContentString;
    Quote.GetContentString(ContentString);
    RepStr += " / " + ContentString;
  }
}

void TCluster::GetRepresentativeQuoteURL(TQuoteBase *QB, TDocBase *DB, TStr& RepURL) const {
  QB->GetRepresentativeUrl(DB, RepresentativeQuoteIds[0], RepURL);
}

TInt TCluster::GetNumQuotes() const {
  return NumQuotes;
}

TInt TCluster::GetNumUniqueQuotes() const {
  return QuoteIds.Len();
}

void TCluster::GetQuoteIds(TIntV &QuoteIds) const {
  QuoteIds = this->QuoteIds;
}

TInt TCluster::GetId() const {
  return Id;
}

TFlt TCluster::GetPopularity(TQuoteBase *QuoteBase, TDocBase *DocBase, TSecTm CurrentTime) {
  TFlt Popularity;
  TVec<TUInt> UniqueSources;
  GetUniqueSources(UniqueSources, QuoteIds, QuoteBase);
  TFreqTripleV FreqV;
  Peaks::GetFrequencyVector(DocBase, UniqueSources, FreqV, 2, 1, CurrentTime);
  TStr RepQuoteStr;
  GetRepresentativeQuoteString(RepQuoteStr, QuoteBase);
  //fprintf(stderr, "Cluster: %s\n", RepQuoteStr.CStr());
  for (int i = 0; i < FreqV.Len(); i++) {
    //fprintf(stderr, "\t%d\t%f\n", FreqV[i].Val1.Val, FreqV[i].Val2.Val);
    Popularity += FreqV[i].Val2 * exp(FreqV[i].Val1 / 48);
  }
  return Popularity;
}

void TCluster::SetId(TInt Id) {
  this->Id = Id;
}

void TCluster::GetMaxPeakInfo(TDateFreq& MaxPeakDateFreq) {
  MaxPeakDateFreq = this->MaxPeakDateFreq;
}

void TCluster::AddQuote(TQuoteBase *QB, TDocBase *DB, const TIntV &QuoteIds) {
  for (int i = 0; i < QuoteIds.Len(); i++) {
    AddQuote(QB, DB, QuoteIds[i], false);
  }

  TVec<TUInt> UniqueSources;
  GetUniqueSources(UniqueSources, QuoteIds, QB);
  UpdateMaxPeak(this->QuoteIds, UniqueSources, QB, DB);
}

/**
 * UpdatePeak is true by default, and **must** always be true for the last
 * quote added to the TCluster (in order for visualization to work properly).
 */
void TCluster::AddQuote(TQuoteBase *QB, TDocBase *DB, TInt QuoteId, bool UpdatePeak) {
  this->QuoteIds.Add(QuoteId);

  // Only count the unique sources for the new frequency of the cluster
  TVec<TUInt> UniqueSources;
  GetUniqueSources(UniqueSources, QuoteIds, QB);
  NumQuotes = UniqueSources.Len();

  if (UpdatePeak) {
    UpdateMaxPeak(this->QuoteIds, UniqueSources, QB, DB);
  }
}

void TCluster::SetRepresentativeQuoteIds(TIntV& QuoteIds) {
  this->RepresentativeQuoteIds = QuoteIds;
}

void TCluster::SetQuoteIds(TQuoteBase *QB, TDocBase *DB, TIntV& NewQuoteIds) {
  this->QuoteIds = NewQuoteIds;

  TVec<TUInt> UniqueSources;
  GetUniqueSources(UniqueSources, NewQuoteIds, QB);
  NumQuotes = UniqueSources.Len();

  UpdateMaxPeak(NewQuoteIds, UniqueSources, QB, DB);
}

/*void TCluster::ReplaceQuote(TQuoteBase *QB, TInt OldQuoteId, TInt NewQuoteId) {
  TInt QuoteIndex = this->QuoteIds.SearchForw(OldQuoteId);
  if (QuoteIndex == -1) {
    fprintf(stderr, "Warning: Quote Id %d not found in cluster's quote vector\n", OldQuoteId.Val);
  }
  QuoteIds[QuoteIndex] = NewQuoteId;

  // Count the unique sources for the new frequency of the cluster
  TIntV UniqueSources;
  GetUniqueSources(UniqueSources, QuoteIds, QB);
  NumQuotes = UniqueSources.Len();
}*/

void TCluster::GetPeaks(TDocBase *DocBase, TQuoteBase *QuoteBase, TFreqTripleV& PeakTimesV, TFreqTripleV& FreqV, TInt BucketSize, TInt SlidingWindowSize, TSecTm PresentTime, bool reset) {
  if (!reset && this->PeakTimesV.Len() > 0 && this->FreqV.Len() > 0) {
    PeakTimesV = this->PeakTimesV;
    FreqV = this->FreqV;
  } else {
    TVec<TUInt> Sources;
    for (int i = 0; i < QuoteIds.Len(); i++) {
      TQuote Quote;
      if (QuoteBase->GetQuote(QuoteIds[i], Quote)) {
        TVec<TUInt> CurSources;
        Quote.GetSources(CurSources);
        Sources.AddV(CurSources);
      }
    }
    Peaks::GetPeaks(DocBase, Sources, PeakTimesV, FreqV, BucketSize, SlidingWindowSize, PresentTime);
  }
}

void TCluster::GraphFreqOverTime(TDocBase *DocBase, TQuoteBase *QuoteBase, TStr Filename, TSecTm PresentTime) {
  GraphFreqOverTime(DocBase, QuoteBase, Filename, TInt(1), TInt(1), PresentTime);
}

/// If BucketSize is > 1, a sliding window average will not be calculated
//  Otherwise, if BucketSize = 1, a sliding window average of size SlidingWindowSize will be calculated
void TCluster::GraphFreqOverTime(TDocBase *DocBase, TQuoteBase *QuoteBase, TStr Filename, TInt BucketSize, TInt SlidingWindowSize, TSecTm PresentTime) {
  TFreqTripleV PeakTimesV;
  TFreqTripleV FreqTripleV;
  GetPeaks(DocBase, QuoteBase, PeakTimesV, FreqTripleV, BucketSize, SlidingWindowSize, PresentTime);

  TStr ContentStr;
  GetRepresentativeQuoteString(ContentStr, QuoteBase);
  //fprintf(stderr, "Cluster: %s\n", ContentStr.CStr());
  //fprintf(stderr, "Peaks:\n");
  TVec<TIntFltPr> PeakV;
  for (int i = 0; i < PeakTimesV.Len(); ++i) {
    PeakV.Add(TIntFltPr(PeakTimesV[i].Val1, PeakTimesV[i].Val2));
    //fprintf(stderr, "\t%d\t%f\n", PeakTimesV[i].Val1.Val, PeakTimesV[i].Val2.Val);
  }

  //fprintf(stderr, "Points:\n");
  TVec<TIntFltPr> FreqV;
  for (int i = 0; i < FreqTripleV.Len(); ++i) {
    FreqV.Add(TIntFltPr(FreqTripleV[i].Val1, FreqTripleV[i].Val2));
    //fprintf(stderr, "\t%d\t%f\n", FreqTripleV[i].Val1.Val, FreqTripleV[i].Val2.Val);
  }

  TStr EscapedContentStr = TStringUtil::GetEscapedString(ContentStr);
  TGnuPlot GP(Filename, "Frequency of Cluster " + Id.GetStr() + " Over Time: " + EscapedContentStr);
  GP.SetXLabel(TStr("Hour Offset From Present Time"));
  GP.SetYLabel(TStr("Frequency of Cluster"));
  GP.AddPlot(FreqV, gpwLinesPoints, "Frequency");
  if (PeakV.Len() > 0) {
    GP.AddPlot(PeakV, gpwPoints, "Peaks");
  }
  GP.AddCmd("set xtics 24");
  GP.AddCmd("set terminal png small size 1000,800");
  //TStr SetXTic = TStr("set xtics 24\nset terminal png small size 1000,800");
  GP.SavePng(Filename + ".png");
  //GP.SavePng(Filename + ".png", 1000, 800, TStr(), SetXTic);
}

TInt TCluster::GetNumSources(TQuoteBase *QuoteBase) {
  TInt Answer;
  for (int i = 0; i < QuoteIds.Len(); i++) {
    TQuote Q;
    QuoteBase->GetQuote(QuoteIds[i], Q);
    Answer += Q.GetNumSources();
  }
  return Answer;
  //TVec<TUInt> UniqueSources;
  //GetUniqueSources(UniqueSources, QuoteIds, QuoteBase);
  //return UniqueSources.Len();
}

TInt TCluster::GetNumUniqueSources(TQuoteBase *QuoteBase) {
  TVec<TUInt> UniqueSources;
  GetUniqueSources(UniqueSources, QuoteIds, QuoteBase);
  return UniqueSources.Len();
}

/// Calculates the number of unique sources among the quotes in a cluster,
//  to get the frequency of the cluster
void TCluster::GetUniqueSources(TVec<TUInt>& UniqueSources, const TIntV& QuoteIds, TQuoteBase *QB) {
  THashSet<TUInt> MergedSources;
  for (int i = 0; i < QuoteIds.Len(); i++) {
    TQuote Q;
    QB->GetQuote(QuoteIds[i], Q);
    TVec<TUInt> QSources;
    Q.GetSources(QSources);
    MergedSources.AddKeyV(QSources);
  }

  MergedSources.GetKeyV(UniqueSources);
}

void TCluster::UpdateMaxPeak(TIntV QuoteIds, TVec<TUInt> UniqueSources, TQuoteBase *QB, TDocBase *DB) {
  // Calculate MaxPeakDateFreq
  if (UniqueSources.Len() > 0) {
    TFreqTripleV TempFreqV;
    TInt NoSlidingWindow = 1;
    Peaks::GetFrequencyVector(DB, UniqueSources, TempFreqV, Peaks::NumHoursInDay, NoSlidingWindow, TSecTm(0));
    MaxPeakDateFreq = TDateFreq(TempFreqV[0].Val3, TFlt::Round(TempFreqV[0].Val2));
    for (int i = 0; i < TempFreqV.Len(); ++i) {
      if (TFlt::Round(TempFreqV[i].Val2) > MaxPeakDateFreq.Val2) {
        MaxPeakDateFreq.Val1 = TempFreqV[i].Val3;
        MaxPeakDateFreq.Val2 = TempFreqV[i].Val2;
      }
    }
  }
}

TClusterBase::TClusterBase() {
}

TClusterBase::TClusterBase(TInt OldCounter) {
  ClusterIdCounter = OldCounter;
}

void TClusterBase::Save(TSOut &SOut) const {
  ClusterIdCounter.Save(SOut);
  IdToTCluster.Save(SOut);
  QuoteIdToClusterId.Save(SOut);
}

void TClusterBase::Load(TSIn& SIn) {
  ClusterIdCounter.Load(SIn);
  IdToTCluster.Load(SIn);
  QuoteIdToClusterId.Load(SIn);
}

TInt TClusterBase::AddCluster(TCluster &Cluster, const TClusterBase *OldCB, TSecTm& PresentTime) {
  // setup to determine cluster id number
  Cluster.SetBirthDate(PresentTime);
  TIntV QuoteIds;
  Cluster.GetQuoteIds(QuoteIds);
  TInt CurCounter = -1;
  if (OldCB != NULL && QuoteIds.Len() > 0) {
    for (int i = 0; i < QuoteIds.Len(); i++) {
      CurCounter = OldCB->GetClusterIdFromQuoteId(QuoteIds[i]);
      if (CurCounter >= 0) break;
    }
  }
  if (CurCounter < 0) {
    CurCounter = ClusterIdCounter;
    ClusterIdCounter++;
  } else {
    TCluster OldCluster;
    OldCB->GetCluster(CurCounter, OldCluster);
    TSecTm TrueBirthDate;
    OldCluster.GetBirthDate(TrueBirthDate);
    Cluster.SetBirthDate(TrueBirthDate);
  }

  // set cluster counter to quoteidtoclusterid mapping
  for (int i = 0; i < QuoteIds.Len(); i++) {
    QuoteIdToClusterId.AddDat(QuoteIds[i], CurCounter);
  }
  Cluster.SetId(CurCounter);
  IdToTCluster.AddDat(CurCounter, Cluster);
  Cluster.DeathDate = PresentTime;
  return CurCounter;
}

/// Used when merging clusters
TInt TClusterBase::AddCluster(TCluster& Cluster) {
  if (Cluster.DeathDate.GetYearN() > 2020) Err("Bah!\n");
  TIntV QuoteIds;
  Cluster.GetQuoteIds(QuoteIds);

  TInt CurCounter = -1;
  bool ClusterIdMatched = false;
  TIntSet IdsMatched;
  for (int i = 0; i < QuoteIds.Len(); i++) {
    CurCounter = GetClusterIdFromQuoteId(QuoteIds[i]);
    if (CurCounter >= 0) {
      IdsMatched.AddKey(CurCounter);
    }
    if (CurCounter == Cluster.GetId()) {
      ClusterIdMatched = true;
    }
  }

   /* start of test code
  if (IdsMatched.Len() > 1) {  // This should never happen
    fprintf(stderr, "ERROR: This cluster's quotes are in more than one cluster\n");
    fprintf(stderr, "Quote ids in this cluster:\n");
    for (int i = 0; i < QuoteIds.Len(); i++) {
      fprintf(stderr, "  %d\n", QuoteIds[i].Val);
    }
      
    TIntV IdsMatchedV;
    IdsMatched.GetKeyV(IdsMatchedV);
    for (int i = 0; i < IdsMatchedV.Len(); i++) {
      TCluster ExistingC;
      GetCluster(IdsMatchedV[i], ExistingC);
      fprintf(stderr, "Quotes ids in existing cluster %d: \n", IdsMatchedV[i].Val);
      TIntV ExistingQuoteIds;
      ExistingC.GetQuoteIds(ExistingQuoteIds);
      for (int i = 0; i < ExistingQuoteIds.Len(); i++) {
        fprintf(stderr, "  %d\n", ExistingQuoteIds[i].Val);
      }
    }
  }
  end of test code */ 
  IAssert(IdsMatched.Len() <= 1);

  if (ClusterIdMatched) {
    CurCounter = Cluster.GetId();
  } else if (IdsMatched.Len() > 0) {
    TIntV IdsMatchedV;
    IdsMatched.GetKeyV(IdsMatchedV);
    CurCounter = IdsMatchedV[0];
  }
  
  //if (CurCounter != -1 && CurCounter != Cluster.GetId()) {
  //  fprintf(stderr, "WARNING: Cluster id is different: %d (cumulative) vs. %d\n", CurCounter.Val, Cluster.GetId().Val);
  //}

  if (CurCounter < 0) {  // for revived cluster, use the old cluster's id
    //fprintf(stderr, "\tNew cluster, with new quotes!\n");

    //FOR TESTING
    /*if (Cluster.GetId() == 1) {
      fprintf(stderr, "Cluster id: %d\n", Cluster.GetId().Val);
      fprintf(stderr, "# Quote ids: %d\n", QuoteIds.Len());
      fprintf(stderr, "# of first Quote id: %d\n", QuoteIds[0].Val);
    }

    if (Cluster.GetId() < ClusterIdCounter) {  // this should never happen
      fprintf(stderr, "ERROR: Counter: %d, Cluster id: %d\n", ClusterIdCounter.Val, Cluster.GetId().Val);
      fprintf(stderr, "Quote ids in this cluster:\n");
      for (int i = 0; i < QuoteIds.Len(); i++) {
        fprintf(stderr, "  %d\n", QuoteIds[i].Val);
      }
      
      TCluster ExistingC;
      GetCluster(Cluster.GetId(), ExistingC);
      fprintf(stderr, "Quotes ids in existing cluster: \n");
      TIntV ExistingQuoteIds;
      ExistingC.GetQuoteIds(ExistingQuoteIds);
      for (int i = 0; i < ExistingQuoteIds.Len(); i++) {
        fprintf(stderr, "  %d\n", ExistingQuoteIds[i].Val);
      }

      fprintf(stderr, "CurCounter is: %d\n", CurCounter.Val);
      if (CurCounter >= 0) {
        TCluster MatchingC;
        GetCluster(CurCounter, MatchingC);
        fprintf(stderr, "Quote ids in matching cluster: \n");
        TIntV MatchingQuoteIds;
        MatchingC.GetQuoteIds(MatchingQuoteIds);
        for (int i = 0; i < MatchingQuoteIds.Len(); i++) {
          fprintf(stderr, "  %d\n", MatchingQuoteIds[i].Val);
        }
      }
    }*/
    //END FOR TESTING

    //if (CurCounter >= 0) {  // remove the old, existing cluster
    //  RemoveCluster(CurCounter);
    //}
    //IAssert(Cluster.GetId() >= ClusterIdCounter); // This should not fail but it does sometimes :(

    CurCounter = Cluster.GetId();
    if (Cluster.GetId() >= ClusterIdCounter) {
      ClusterIdCounter = Cluster.GetId() + 1;
    }
  }

  for (int i = 0; i < QuoteIds.Len(); i++) {
    QuoteIdToClusterId.AddDat(QuoteIds[i], CurCounter);
  }
  Cluster.SetId(CurCounter);
  IdToTCluster.AddDat(CurCounter, Cluster);
  return CurCounter;
}

void TClusterBase::AddStaticCluster(TInt ClusterId, TCluster& Cluster) {
  IdToTCluster.AddDat(ClusterId, Cluster);
  TIntV QuoteIds;
  Cluster.GetQuoteIds(QuoteIds);
  for (int i = 0; i < QuoteIds.Len(); i++) {
    QuoteIdToClusterId.AddDat(QuoteIds[i], ClusterId);
  }
}

bool TClusterBase::AddQuoteToCluster(TQuoteBase *QB, TDocBase *DB, const TIntV& QuoteIds, TInt ClusterId) {
  TCluster Cluster;
  if (IdToTCluster.IsKeyGetDat(ClusterId, Cluster)) {
    Cluster.AddQuote(QB, DB, QuoteIds);
    IdToTCluster.AddDat(ClusterId, Cluster);
    for (int i = 0; i < QuoteIds.Len();++i) {
      QuoteIdToClusterId.AddDat(QuoteIds[i], ClusterId);
    }
    return true;
  } else {
    return false;
  }
}

/*bool TClusterBase::ReplaceQuoteInCluster(TQuoteBase *QB, TInt OldQuoteId, TInt NewQuoteId, TInt ClusterId) {
  TCluster Cluster;
  if (IdToTCluster.IsKeyGetDat(ClusterId, Cluster)) {
    Cluster.ReplaceQuote(QB, OldQuoteId, NewQuoteId);
    QuoteIdToClusterId.DelKey(OldQuoteId);
    QuoteIdToClusterId.AddDat(NewQuoteId, ClusterId);
    IdToTCluster.AddDat(ClusterId, Cluster);
    return true;
  }
  return false;
}*/

/// Just removes the cluster from the IdToTCluster table;
//  doesn't update the quote id to cluster id mappings
void TClusterBase::RemoveCluster(TInt ClusterId) {
  if (IdToTCluster.IsKey(ClusterId)) {
    TCluster C = IdToTCluster.GetDat(ClusterId);
    TIntV QuoteIds;
    C.GetQuoteIds(QuoteIds);
    for (int i = 0; i < QuoteIds.Len(); ++i) {
      if (QuoteIdToClusterId.IsKey(QuoteIds[i])) {
        QuoteIdToClusterId.DelKey(QuoteIds[i]);
      }
    }
    IdToTCluster.DelKey(ClusterId);
  }
}

bool TClusterBase::GetCluster(TInt ClusterId, TCluster& RefC) const {
  if (IdToTCluster.IsKeyGetDat(ClusterId, RefC)) {
  if (RefC.DeathDate.GetYearN() > 2020) Err("asdf");
  return true;}
  return false;
  //IdToTCluster.IsKeyGetDat(ClusterId, RefC);
}

// Returns -1 if QuoteId is not found
TInt TClusterBase::GetClusterIdFromQuoteId(TInt QuoteId) const {
  TInt ClusterId;
  if (QuoteIdToClusterId.IsKeyGetDat(QuoteId, ClusterId)) {
    return ClusterId;
  } else {
    return -1;
  }
}

void TClusterBase::GetAllClusterIds(TIntV &ClusterIds) const {
  IdToTCluster.GetKeyV(ClusterIds);
}

/// Sorts clusters in decreasing order, and finds representative quote for each cluster
//  RepQuotesAndFreq is a vector of cluster results, represented by TClusters
void TClusterBase::GetAllClusterIdsSortByFreq(TIntV &ClusterIds) {
  fprintf(stderr, "Sorting cluster id's by size frequency...\n");
  IdToTCluster.GetKeyV(ClusterIds);
  ClusterIds.SortCmp(TCmpTClusterIdByNumQuotes(false, this));
}

void TClusterBase::GetTopClusterIdsByFreq(TIntV &TopClusterIds) {
  TIntV ClusterIds;
  GetAllClusterIdsSortByFreq(ClusterIds);
  for (int i = 0; i < ClusterIds.Len(); i++) {
    TCluster Cluster;
    GetCluster(ClusterIds[i], Cluster);
    if (Cluster.GetNumQuotes() < LogOutput::FrequencyCutoff) {
      break;
    } else {
      TopClusterIds.Add(ClusterIds[i]);
    }
  }
}

void TClusterBase::Clr() {
  IdToTCluster.Clr();
  QuoteIdToClusterId.Clr();
  ClusterIdCounter = 0;
}

int TClusterBase::Len() {
  return IdToTCluster.Len();
}

TInt TClusterBase::GetCounter() {
  return ClusterIdCounter;
}

bool TClusterBase::IsQuoteInArchivedCluster(TInt& QuoteId) {
  TInt ClusterId;
  if (QuoteIdToClusterId.IsKeyGetDat(QuoteId, ClusterId)) {
    TCluster Cluster = IdToTCluster.GetDat(ClusterId);
    return Cluster.IsArchived();
  }
  return false;
}

/// Merges the second cluster into the first. For the quotes in the second cluster, updates
//  the quote id to cluster id mappings to point to the first cluster
void TClusterBase::MergeCluster2Into1(TInt Id1, TInt Id2, TQuoteBase *QB, TDocBase *DB, bool KeepOneRepId) {
  // Add the quote ids of the second cluster to the first
  TCluster Cluster1, Cluster2;
  if (!IdToTCluster.IsKeyGetDat(Id1, Cluster1) || !IdToTCluster.IsKeyGetDat(Id2, Cluster2)) {
    return;
  }

  TIntV Cluster2QuoteIds;
  Cluster2.GetQuoteIds(Cluster2QuoteIds);

  // Update the mappings in ClusterBase for the new quote ids
  AddQuoteToCluster(QB, DB, Cluster2QuoteIds, Id1);

  // Get the new cluster 1, with the quotes from cluster 2 added
  IdToTCluster.IsKeyGetDat(Id1, Cluster1);

  if (KeepOneRepId) {
    // The new representative quote is the quote with the longer content string
    TStr RepQuoteStr1, RepQuoteStr2;
    Cluster1.GetRepresentativeQuoteString(RepQuoteStr1, QB);
    Cluster2.GetRepresentativeQuoteString(RepQuoteStr2, QB);
    if (RepQuoteStr2.Len() > RepQuoteStr1.Len()) {
      TIntV RepQuoteIds2;
      Cluster2.GetRepresentativeQuoteIds(RepQuoteIds2);
      Cluster1.SetRepresentativeQuoteIds(RepQuoteIds2);
      IdToTCluster.AddDat(Id1, Cluster1);
    }
  } else {
    // The new representative quote is both clusters' repIds appended to each other.
    TIntV RepQuoteIds1, RepQuoteIds2;
    Cluster1.GetRepresentativeQuoteIds(RepQuoteIds1);
    Cluster2.GetRepresentativeQuoteIds(RepQuoteIds2);
    RepQuoteIds1.AddV(RepQuoteIds2);
    Cluster1.SetRepresentativeQuoteIds(RepQuoteIds1);
    IdToTCluster.AddDat(Id1, Cluster1);
  }
}

TStr TClusterBase::ContainsEmptyClusters() {
  TIntV AllClusters;
  IdToTCluster.GetKeyV(AllClusters);
  TInt NumClusters = AllClusters.Len();
  for (int i = 0; i < NumClusters; ++i) {
    TCluster CurCluster;
    IdToTCluster.IsKeyGetDat(AllClusters[i], CurCluster);
    if (CurCluster.GetNumQuotes() < 1) {
      return "EMPTY CLUSTERS";
    }
  }
  return "safe!";
}

void TClusterBase::SortClustersByPopularity(TDocBase *DB, TQuoteBase *QB, TIntV& Clusters, TSecTm& CurrentTime) {
  Err("Sorting by popularity...\n");
  TVec<TPair<TInt, TFlt> > PopularityVec;
  for (int i = 0; i < Clusters.Len(); i++) {
    TCluster C;
    GetCluster(Clusters[i], C);
    // Only add good clusters
    if (C.GetDiscardState() == 0) {
      TFlt Score = C.GetPopularity(QB, DB, CurrentTime);
      PopularityVec.Add(TPair<TInt, TFlt>(Clusters[i], Score));
    }
  }
  PopularityVec.SortCmp(TCmpTClusterByPopularity(false));

  TIntV TopFilteredClustersByPopularity;
  for (int i = 0; i < PopularityVec.Len(); i++) {
    TopFilteredClustersByPopularity.Add(PopularityVec[i].Val1);
  }
  Clusters = TopFilteredClustersByPopularity;
}
