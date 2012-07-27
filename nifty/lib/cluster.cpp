#include "stdafx.h"
#include "cluster.h"
#include "quote.h"
#include "peaks.h"

TCluster::TCluster() {
}

TCluster::TCluster(TIntV& RepresentativeQuoteIds, TInt NumQuotes, TIntV QuoteIds, TQuoteBase *QB) {
  // TODO: Check that URLs are not repeated
  this->RepresentativeQuoteIds = RepresentativeQuoteIds;
  TIntV UniqueSources;
  TCluster::GetUniqueSources(UniqueSources, QuoteIds, QB);
  this->NumQuotes = UniqueSources.Len();
  this->QuoteIds = QuoteIds;
  this->Id = 1;
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
  Popularity.Save(SOut);
  PeakTimesV.Save(SOut);
  FreqV.Save(SOut);
}

void TCluster::Load(TSIn& SIn) {
  RepresentativeQuoteIds.Load(SIn);
  NumQuotes.Load(SIn);
  QuoteIds.Load(SIn);
  Id.Load(SIn);
  Popularity.Load(SIn);
  PeakTimesV.Load(SIn);
  FreqV.Load(SIn);
}

TInt TCluster::GetNumRepresentativeQuoteIds() const {
  return RepresentativeQuoteIds.Len();
}

void TCluster::GetRepresentativeQuoteIds(TIntV& RepQuoteIds) const {
  RepQuoteIds = RepresentativeQuoteIds;
}

void TCluster::GetRepresentativeQuoteString(TStr& RepStr, TQuoteBase *QB) const {
  if (RepresentativeQuoteIds.Len() <= 0) return;
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

TInt TCluster::GetNumQuotes() const {
  return NumQuotes;
}

TInt TCluster::GetNumUniqueQuotes() const {
  return QuoteIds.Len();
}

void TCluster::GetQuoteIds(TIntV &QuoteIds) const {
  QuoteIds = this->QuoteIds;
}

void TCluster::AddQuote(TQuoteBase *QB, const TIntV &QuoteIds) {
  for (int i = 0; i < QuoteIds.Len(); i++) {
    AddQuote(QB, QuoteIds[i]);
  }
}

void TCluster::AddQuote(TQuoteBase *QB, TInt QuoteId) {
  this->QuoteIds.Add(QuoteId);
  TQuote Q;
  QB->GetQuote(QuoteId, Q);
  this->NumQuotes += Q.GetNumSources();
}

void TCluster::SetRepresentativeQuoteIds(TIntV& QuoteIds) {
  this->RepresentativeQuoteIds = QuoteIds;
}

TInt TCluster::GetId() {
  return Id;
}

TFlt TCluster::GetPopularity() const {
  return Popularity;
}

void TCluster::CalculatePopularity(TQuoteBase *QuoteBase, TDocBase *DocBase, TSecTm CurrentTime) {
  fprintf(stderr, "getting unique sources\n");
  TIntV UniqueSources;
  GetUniqueSources(UniqueSources, QuoteIds, QuoteBase);
  fprintf(stderr, "getting frequency vectors\n");
  TFreqTripleV FreqV;
  Peaks::GetFrequencyVector(DocBase, UniqueSources, FreqV, 2, 1, CurrentTime);
  fprintf(stderr, "calculating popularity\n");
  for (int i = 0; i < FreqV.Len(); i++) {
    Popularity += FreqV[i].Val2 * exp(FreqV[i].Val1 / 48);
  }
}

void TCluster::SetId(TInt Id) {
  this->Id = Id;
}

void TCluster::GetPeaks(TDocBase *DocBase, TQuoteBase *QuoteBase, TFreqTripleV& PeakTimesV, TFreqTripleV& FreqV, TInt BucketSize, TInt SlidingWindowSize, TSecTm PresentTime, bool reset) {
  if (!reset && this->PeakTimesV.Len() > 0 && this->FreqV.Len() > 0) {
    PeakTimesV = this->PeakTimesV;
    FreqV = this->FreqV;
  } else {
    TIntV Sources;
    for (int i = 0; i < QuoteIds.Len(); i++) {
      TQuote Quote;
      if (QuoteBase->GetQuote(QuoteIds[i], Quote)) {
        TIntV CurSources;
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

  TVec<TIntFltPr> PeakV;
  for (int i = 0; i < PeakTimesV.Len(); ++i) {
    PeakV.Add(TIntFltPr(PeakTimesV[i].Val1, PeakTimesV[i].Val2));
  }

  TVec<TIntFltPr> FreqV;
  for (int i = 0; i < FreqTripleV.Len(); ++i) {
    FreqV.Add(TIntFltPr(FreqTripleV[i].Val1, FreqTripleV[i].Val2));
  }

  TStr ContentStr;
  GetRepresentativeQuoteString(ContentStr, QuoteBase);
  TGnuPlot GP(Filename, "Frequency of Cluster " + Id.GetStr() + " Over Time: " + ContentStr);
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

/// Merges OtherCluster into this cluster; **For the quotes in OtherCluster, updates the
//  quote id to cluster id mappings to point to this cluster
void TCluster::MergeWithCluster(TCluster& OtherCluster, TQuoteBase *QB, bool KeepOneRepId) {
  // Put the quote ids of the two clusters together into one vector
  TIntV OtherQuoteIds;
  OtherCluster.GetQuoteIds(OtherQuoteIds);
  QuoteIds.AddV(OtherQuoteIds);

  // Update the mappings in ClusterBase for the new quote ids

  // Only count the unique sources for the new frequency of the cluster
  TIntV UniqueSources;
  GetUniqueSources(UniqueSources, QuoteIds, QB);
  NumQuotes = UniqueSources.Len();

  if (KeepOneRepId) {
    // The new representative quote is the quote with the longer content string
    TStr ThisRepQuoteStr, OtherRepQuoteStr;
    GetRepresentativeQuoteString(ThisRepQuoteStr, QB);
    OtherCluster.GetRepresentativeQuoteString(OtherRepQuoteStr, QB);
    if (OtherRepQuoteStr.Len() > ThisRepQuoteStr.Len()) {
      TIntV OtherRepQuoteIds;
      OtherCluster.GetRepresentativeQuoteIds(OtherRepQuoteIds);
      RepresentativeQuoteIds = OtherRepQuoteIds;
    }
  } else {
    // The new representative quote is both clusters' repIds appended to each other.
    TIntV OtherRepQuoteIds;
    OtherCluster.GetRepresentativeQuoteIds(OtherRepQuoteIds);
    RepresentativeQuoteIds.AddV(OtherRepQuoteIds);
  }
}

/// Calculates the number of unique sources among the quotes in a cluster,
//  to get the frequency of the cluster
void TCluster::GetUniqueSources(TIntV& UniqueSources, TIntV& QuoteIds, TQuoteBase *QB) {
  TIntSet MergedSources;
  for (int i = 0; i < QuoteIds.Len(); i++) {
    TQuote Q;
    QB->GetQuote(QuoteIds[i], Q);
    TIntV QSources;
    Q.GetSources(QSources);
    MergedSources.AddKeyV(QSources);
  }

  for (TIntSet::TIter DocId = MergedSources.BegI(); DocId < MergedSources.EndI(); DocId++) {
    UniqueSources.Add(*DocId);
  }
}

TClusterBase::TClusterBase() {
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

TInt TClusterBase::AddCluster(const TCluster &Cluster) {
  TIntV QuoteIds;
  Cluster.GetQuoteIds(QuoteIds);
  for (int i = 0; i < QuoteIds.Len(); i++) {
    QuoteIdToClusterId.AddDat(QuoteIds[i], ClusterIdCounter);
  }
  IdToTCluster.AddDat(ClusterIdCounter, Cluster);
  return ClusterIdCounter++;
}

bool TClusterBase::AddQuoteToCluster(TQuoteBase *QB, TInt QuoteId, TInt ClusterId) {
  TCluster Cluster;
  if (IdToTCluster.IsKeyGetDat(ClusterId, Cluster)) {
    Cluster.AddQuote(QB, QuoteId);
    IdToTCluster.AddDat(ClusterId, Cluster);
    QuoteIdToClusterId.AddDat(QuoteId, ClusterId);
    return true;
  } else {
    return false;
  }
}

/// Just removes the cluster from the IdToTCluster table;
//  doesn't update the quote id to cluster id mappings
void TClusterBase::RemoveCluster(TInt ClusterId) {
  if (IdToTCluster.IsKey(ClusterId)) {
    TCluster C;
    GetCluster(ClusterId, C);
    IdToTCluster.DelKey(C.GetId());
  }
}

bool TClusterBase::GetCluster(TInt ClusterId, TCluster& RefC) {
  return IdToTCluster.IsKeyGetDat(ClusterId, RefC);
}

// Returns -1 if QuoteId is not found
TInt TClusterBase::GetClusterIdFromQuoteId(TInt QuoteId) {
  TInt ClusterId;
  if (QuoteIdToClusterId.IsKeyGetDat(QuoteId, ClusterId)) {
    return ClusterId;
  } else {
    return -1;
  }
}

void TClusterBase::GetAllClusterIds(TIntV &ClusterIds) {
  IdToTCluster.GetKeyV(ClusterIds);
}

/// Sorts clusters in decreasing order, and finds representative quote for each cluster
//  RepQuotesAndFreq is a vector of cluster results, represented by TClusters
void TClusterBase::GetAllClusterIdsSortByFreq(TIntV &ClusterIds) {
  IdToTCluster.GetKeyV(ClusterIds);
  ClusterIds.SortCmp(TCmpTClusterIdByNumQuotes(false, this));
}

void TClusterBase::Clr() {
  IdToTCluster.Clr();
  QuoteIdToClusterId.Clr();
  ClusterIdCounter = 0;
}

int TClusterBase::Len() {
  return IdToTCluster.Len();
}
