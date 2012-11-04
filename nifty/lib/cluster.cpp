#include "stdafx.h"
#include "cluster.h"
#include "quote.h"
#include "peaks.h"
#include "clustering.h"

const int TClusterBase::FrequencyCutoff = 100;

TCluster::TCluster() {
  Archived = true;
}

TCluster::TCluster(TIntV& RepresentativeQuoteIds, TInt NumQuotes, TIntV QuoteIds, TQuoteBase *QB, TSecTm BirthDate) {
  // TODO: Check that URLs are not repeated
  this->RepresentativeQuoteIds = RepresentativeQuoteIds;
  TIntV UniqueSources;
  TCluster::GetUniqueSources(UniqueSources, QuoteIds, QB);
  this->NumQuotes = UniqueSources.Len();
  this->QuoteIds = QuoteIds;
  this->Id = 1;
  this->BirthDate = BirthDate;
  this->Archived = true;
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
  TIntV UniqueSources;
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

void TCluster::AddQuote(TQuoteBase *QB, const TIntV &QuoteIds) {
  for (int i = 0; i < QuoteIds.Len(); i++) {
    AddQuote(QB, QuoteIds[i]);
  }
}

void TCluster::AddQuote(TQuoteBase *QB, TInt QuoteId) {
  this->QuoteIds.Add(QuoteId);

  // Only count the unique sources for the new frequency of the cluster
  TIntV UniqueSources;
  GetUniqueSources(UniqueSources, QuoteIds, QB);
  NumQuotes = UniqueSources.Len();
}

void TCluster::SetRepresentativeQuoteIds(TIntV& QuoteIds) {
  this->RepresentativeQuoteIds = QuoteIds;
}

void TCluster::SetQuoteIds(TQuoteBase *QB, TIntV& NewQuoteIds) {
  this->QuoteIds = NewQuoteIds;

  TIntV UniqueSources;
  GetUniqueSources(UniqueSources, NewQuoteIds, QB);
  NumQuotes = UniqueSources.Len();
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
  return CurCounter;
}

/// Used when merging clusters
TInt TClusterBase::AddCluster(TCluster& Cluster) {
  TIntV QuoteIds;
  Cluster.GetQuoteIds(QuoteIds);

  TInt CurCounter = -1;
  for (int i = 0; i < QuoteIds.Len(); i++) {
    CurCounter = GetClusterIdFromQuoteId(QuoteIds[i]);
    //if (CurCounter >= 0) break;
    if (CurCounter == Cluster.GetId()) break;
  }

  //if (CurCounter != -1 && CurCounter != Cluster.GetId()) {
  //  fprintf(stderr, "WARNING: Cluster id is different: %d (cumulative) vs. %d\n", CurCounter.Val, Cluster.GetId().Val);
  //}

  if (CurCounter < 0) {  // New cluster, with new quotes
    //fprintf(stderr, "\tNew cluster, with new quotes!\n");
    TCluster TempC;

    //FOR TESTING
    /*if (Cluster.GetId() == 1) {
      fprintf(stderr, "Cluster id: %d\n", Cluster.GetId().Val);
      fprintf(stderr, "# Quote ids: %d\n", QuoteIds.Len());
      fprintf(stderr, "# of first Quote id: %d\n", QuoteIds[0].Val);
    }*/
    //END FOR TESTING

    //IAssert(!GetCluster(Cluster.GetId(), TempC));  // This causes the script to crash sometimes.. not sure why
                                                     // Seems to only happen with spam that will get filtered anyway
    CurCounter = Cluster.GetId();
    if (Cluster.GetId() > ClusterIdCounter) {
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
  return IdToTCluster.IsKeyGetDat(ClusterId, RefC);
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
  fprintf(stderr, "Sorting cluster id's by size frequency and returning top ones...\n");
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
void TClusterBase::MergeCluster2Into1(TInt Id1, TInt Id2, TQuoteBase *QB, bool KeepOneRepId) {
  // Add the quote ids of the second cluster to the first
  TCluster Cluster1, Cluster2;
  if (!IdToTCluster.IsKeyGetDat(Id1, Cluster1) || !IdToTCluster.IsKeyGetDat(Id2, Cluster2)) {
    return;
  }

  TIntV Cluster2QuoteIds;
  Cluster2.GetQuoteIds(Cluster2QuoteIds);

  // Update the mappings in ClusterBase for the new quote ids
  for (int i = 0; i < Cluster2QuoteIds.Len(); i++) {
    AddQuoteToCluster(QB, Cluster2QuoteIds[i], Id1);
  }

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
