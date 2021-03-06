#include "postcluster.h"
#include "lsh.h"
#include <assert.h>

const double PostCluster::ClusterSourceOverlapThreshold = 0.8;
const int PostCluster::BucketSize = 2;
const int PostCluster::SlidingWindowSize = 1;
const int PostCluster::PeakThreshold = 5;
const int PostCluster::DayThreshold = 3;
const int PostCluster::QuoteThreshold = 20;
TStrSet PostCluster::BlacklistedQuotes;

void PostCluster::GetTopFilteredClusters(TClusterBase *CB, TDocBase *DB, TQuoteBase *QB, LogOutput& Log, TIntV& TopFilteredClusters, TSecTm& PresentTime, PNGraph& QGraph, bool RemoveClusters) {
  if (RemoveClusters) {
    RemoveOldClusters(QB, DB, CB, Log, PresentTime, QGraph);
  }
  CB->GetTopClusterIdsByFreq(TopFilteredClusters);
  //MergeAllClustersBasedOnSubstrings(QB, TopFilteredClusters, CB);
  //MergeClustersBasedOnSubstrings(QB, TopFilteredClusters, CB);
  //MergeClustersWithCommonSources(QB, TopFilteredClusters, CB);
  FilterBlacklistedQuotes(DB, QB, CB, Log, TopFilteredClusters, PresentTime);
  FilterAndCacheClusterSize(DB, QB, CB, Log, TopFilteredClusters, PresentTime);
  FilterAndCacheClusterPeaks(DB, QB, CB, Log, TopFilteredClusters, PresentTime);
  fprintf(stderr, "Number of top clusters remaining after 2 passes: %d\n", TopFilteredClusters.Len());
  CB->SortClustersByPopularity(DB, QB, TopFilteredClusters, PresentTime);
  Err("Done!\n");
}

void PostCluster::FilterBlacklistedQuotes(TDocBase *DB, TQuoteBase *QB, TClusterBase *CB, LogOutput& Log, TIntV &TopClusters, TSecTm& PresentTime) {
  if (BlacklistedQuotes.Len() == 0) {
    PSIn BlackListFile = TFIn::New("resources/quote_blacklist.txt");
    TStr Blacklisted;
    while (!BlackListFile->Eof() && BlackListFile->GetNextLn(Blacklisted)) {
      BlacklistedQuotes.AddKey(Blacklisted);
    }
  }

  TIntSet DiscardedClusterIds;
  for (int i = 0; i < TopClusters.Len(); i++) {
    TCluster C;
    CB->GetCluster(TopClusters[i], C);
    TIntV QuoteIds;
    C.GetQuoteIds(QuoteIds);
    for (int j = 0; j < QuoteIds.Len(); j++) {
      TQuote Q;
      QB->GetQuote(QuoteIds[j], Q);
      TStr CandidateQuote;
      Q.GetContentString(CandidateQuote);
      if (BlacklistedQuotes.IsKey(CandidateQuote)) {
        DiscardedClusterIds.AddKey(TopClusters[i]);
        break;
      }
    }
  }

  Err("Filtered %d blacklisted clusters out of %d (remaining %d)\n", DiscardedClusterIds.Len(), TopClusters.Len(), TopClusters.Len() - DiscardedClusterIds.Len());

  // delete the discarded clusters from the top clusters list.
  TIntV FilteredTopClusters;
  for (int i = 0; i < TopClusters.Len(); ++i) {
    if (!DiscardedClusterIds.IsKey(TopClusters[i])) {
      FilteredTopClusters.Add(TopClusters[i]);
    }
  }
  TopClusters = FilteredTopClusters;
}


/// Merges pairs of clusters if any quote of one is a substring of a quote
//  of the other cluster's. Only merges into the clusters in TopFilterdClusters
void PostCluster::MergeAllClustersBasedOnSubstrings(TQuoteBase *QB, TDocBase *DB, TIntV& TopClusters, TClusterBase *CB) {
  fprintf(stderr, "Merging all clusters based on substrings\n");
  // Hash cluster-ids into buckets based on the word-shingles of the quotes in the cluster
  THash<TMd5Sig, TIntV> TopClustersShingles;
  LSH::HashShinglesOfClusters(QB, CB, TopClusters, LSH::ShingleWordLen, TopClustersShingles);

  // Check all clusters not in TopClusters for possible merging
  TIntV AllClusterIds;
  CB->GetAllClusterIds(AllClusterIds);
  TIntSet TopClustersSet(TopClusters);
  TIntSet ToRemove;

  for (int i = 0; i < AllClusterIds.Len(); i++) {
    if (i % 1000 == 0) {
      fprintf(stderr, "%d out of %d checked for merge\n", i, AllClusterIds.Len());
    }
    if (TopClustersSet.IsKey(AllClusterIds[i])) { continue; }
    TCluster CurrCluster;
    CB->GetCluster(AllClusterIds[i], CurrCluster);
    THashSet<TMd5Sig> ClusterShingles;
    LSH::GetHashedShinglesOfCluster(QB, CurrCluster, LSH::ShingleWordLen, ClusterShingles);
    TIntSet AlreadyCompared;
    bool FoundMatch = false;
    // Compare this cluster to the top clusters that share one or more of the cluster's shingles
    for (THashSet<TMd5Sig>::TIter Shingle = ClusterShingles.BegI(); Shingle < ClusterShingles.EndI(); Shingle++) {
      if (TopClustersShingles.IsKey(*Shingle)) {
        TIntV TopClustersSubset = TopClustersShingles.GetDat(*Shingle);
        for (TIntV::TIter Id = TopClustersSubset.BegI(); Id < TopClustersSubset.EndI(); Id++) {
          if (AlreadyCompared.IsKey(*Id)) { continue; }
          TCluster TopCluster;
          CB->GetCluster(*Id, TopCluster);

          // For testing purposes
          TStr TopClusterStr;
          TopCluster.GetRepresentativeQuoteString(TopClusterStr, QB);
          //fprintf(stderr, "\t%s\n", TopClusterStr.CStr());


          if (ShouldMergeClusters(QB, TopCluster, CurrCluster)) {
            CB->MergeCluster2Into1(TopCluster.GetId(), CurrCluster.GetId(), QB, DB, true);
            FoundMatch = true;

            // For testing, print out which two clusters were merged:
            TStr RepQuoteStr1, RepQuoteStr2;
            TopCluster.GetRepresentativeQuoteString(RepQuoteStr1, QB);
            CurrCluster.GetRepresentativeQuoteString(RepQuoteStr2, QB);
            //fprintf(stderr, "Merged cluster %s INTO %s\n", RepQuoteStr2.CStr(), RepQuoteStr1.CStr());
            break;
          }
          AlreadyCompared.AddKey(*Id);
        }
      }
      if (FoundMatch) {
        ToRemove.AddKey(AllClusterIds[i]);
        break;  // Only want to merge with one top cluster
      }
    }
  }

  // Remove from the ClusterBase those clusters that were merged into another
  for (THashSet<TInt>::TIter Id = ToRemove.BegI(); Id < ToRemove.EndI(); Id++) {
    CB->RemoveCluster(*Id);
  }
}

/// Compares all quotes of the two clusters to check if one is a substring of another;
//  returns true if that is the case, false otherwise.
bool PostCluster::ShouldMergeClusters(TQuoteBase *QB, TCluster& Cluster1, TCluster& Cluster2) {
  TIntV QuoteIds1;
  TIntV QuoteIds2;
  Cluster1.GetQuoteIds(QuoteIds1);
  Cluster2.GetQuoteIds(QuoteIds2);

  for (int q1 = 0; q1 < QuoteIds1.Len(); q1++) {
    for (int q2 = 0; q2 < QuoteIds2.Len(); q2++) {
      if (QB->IsSubstring(QuoteIds1[q1], QuoteIds2[q2])) {
        return true;
      }
    }
  }
  return false;
}

/// Compare all pairs of clusters with frequency cutoff above the threshold
//  FrequencyCutoff, and merges the pair of clusters if any quote of one is
//  a substring of a quote of the other cluster's
//  (Assumes ClusterSummaries contains clusters sorted by decreasing frequency)
void PostCluster::MergeClustersBasedOnSubstrings(TQuoteBase *QB, TDocBase *DB, TIntV &TopClusters, TClusterBase *CB) {
  fprintf(stderr, "Merging only top clusters\n");
  TIntSet ToSkip;  // Contains ids of clusters that have already been merged into another
  TInt NumClusters = TopClusters.Len();

  for (int i = 0; i < NumClusters; i++) {
  // Assumption: the requirements for merging two clusters are transitive
    if (ToSkip.IsKey(TopClusters[i])) { continue; }
    TCluster Ci;
    CB->GetCluster(TopClusters[i], Ci);
    for (int j = i + 1; j < NumClusters; j++) {
      if (ToSkip.IsKey(TopClusters[j])) { continue; }
      TCluster Cj;
      CB->GetCluster(TopClusters[j], Cj);
      if (ShouldMergeClusters(QB, Ci, Cj)) {
        CB->MergeCluster2Into1(TopClusters[i], TopClusters[j], QB, DB, true);
        ToSkip.AddKey(TopClusters[j]);

        // For testing, print out which two clusters were merged:
        TStr RepQuoteStr1, RepQuoteStr2;
        Ci.GetRepresentativeQuoteString(RepQuoteStr1, QB);
        Cj.GetRepresentativeQuoteString(RepQuoteStr2, QB);
        //fprintf(stderr, "Merged cluster %s INTO %s\n", RepQuoteStr2.CStr(), RepQuoteStr1.CStr());
        break;
      }
    }
  }

  // delete merged clusters and remove from ClusterBase
  for (TIntSet::TIter Id = ToSkip.BegI(); Id < ToSkip.EndI(); Id++) {
    CB->RemoveCluster(*Id);
  }
  TIntV NewTopClusters;
  for (int i = 0; i < TopClusters.Len(); i++) {
    if (!ToSkip.IsKey(TopClusters[i])) {
      NewTopClusters.Add(TopClusters[i]);
    }
  }
  TopClusters = NewTopClusters;

  fprintf(stderr, "merged clusters\n");
}

// typical walkthrough function as covered in CS276.
double PostCluster::ComputeClusterSourceOverlap(TVec<TUInt>& Larger, TVec<TUInt>& Smaller) {
  int Li = 0;
  int Si = 0;
  int count = 0;
  while (Li < Larger.Len() && Si < Smaller.Len()) {
    if (Larger[Li] == Smaller[Si]) {
      ++count;
      ++Li;
      ++Si;
    } else if(Larger[Li] > Smaller[Si]) {
      ++Si;
    } else {
      ++Li;
    }
  }
  return count * 1.0 / Smaller.Len();
}

void PostCluster::MergeClustersWithCommonSources(TQuoteBase* QB, TDocBase *DB, TIntV& TopClusters, TClusterBase *CB) {
  fprintf(stderr, "Merging clusters with common sources\n");
  int NumClusters = TopClusters.Len();
  TIntSet ToSkip;  // Contains cluster ids
  for (int i = 0; i < NumClusters; i++) {
    if (ToSkip.IsKey(TopClusters[i])) { continue; }
    TCluster Ci;
    CB->GetCluster(TopClusters[i], Ci);
    TIntV QuoteIds;
    Ci.GetQuoteIds(QuoteIds);
    TVec<TUInt> UniqueSources;
    TCluster::GetUniqueSources(UniqueSources, QuoteIds, QB);
    UniqueSources.Sort(true);
    for (int j = i + 1; j < NumClusters; j++) {
      if (ToSkip.IsKey(TopClusters[j])) { continue; }
      TCluster Cj;
      CB->GetCluster(TopClusters[j], Cj);
      TIntV MoreQuoteIds;
      Cj.GetQuoteIds(MoreQuoteIds);
      TVec<TUInt> MoreUniqueSources;
      TCluster::GetUniqueSources(MoreUniqueSources, MoreQuoteIds, QB);
      MoreUniqueSources.Sort(true);
      double Overlap;
      if (MoreUniqueSources.Len() < UniqueSources.Len()) {
        Overlap = ComputeClusterSourceOverlap(UniqueSources, MoreUniqueSources);
      } else {
        Overlap = ComputeClusterSourceOverlap(MoreUniqueSources, UniqueSources);
      }
      if (Overlap > ClusterSourceOverlapThreshold) {
        TStr RepQuoteStr1, RepQuoteStr2;
        Ci.GetRepresentativeQuoteString(RepQuoteStr1, QB);
        Cj.GetRepresentativeQuoteString(RepQuoteStr2, QB);
        //fprintf(stderr, "CLUSTER1: %s\nCLUSTER2: %s\n", RepQuoteStr2.CStr(), RepQuoteStr1.CStr());

        CB->MergeCluster2Into1(TopClusters[i], TopClusters[j], QB, DB, false);
        ToSkip.AddKey(TopClusters[j]);

        // FOR TESTING
        /*TCluster NewC;
        CB->GetCluster(TopClusters[i], NewC);
        TIntV NewQuoteIds;
        NewC.GetQuoteIds(NewQuoteIds);
        for (int k = 0; k < NewQuoteIds.Len(); k++) {
          TQuote NewQ;
          QB->GetQuote(NewQuoteIds[k], NewQ);
          TStr NewQStr;
          NewQ.GetContentString(NewQStr);
          fprintf(stderr, "\t%s\n", NewQStr.CStr());
        }
        fprintf(stderr, "\n\n"); */

        break; // we really only want to merge once.
      }
    }
  }

  fprintf(stderr, "deleting extra clusters...\n");
  // delete merged clusters and remove from ClusterBase
  for (TIntSet::TIter Id = ToSkip.BegI(); Id < ToSkip.EndI(); Id++) {
    CB->RemoveCluster(*Id);
  }
  TIntV NewTopClusters;
  for (int i = 0; i < TopClusters.Len(); i++) {
    if (!ToSkip.IsKey(TopClusters[i])) {
      NewTopClusters.Add(TopClusters[i]);
    }
  }
  TopClusters = NewTopClusters;
  fprintf(stderr, "done!\n");
}


void PostCluster::FilterAndCacheClusterPeaks(TDocBase *DB, TQuoteBase *QB, TClusterBase *CB, LogOutput& Log, TIntV& TopClusters, TSecTm& PresentTime) {
  TIntSet DiscardedClusterIds;  // Contains id's of clusters to be discarded
  TVec<TPair<TCluster, TInt> > DiscardedClusters;
  for (int i = 0; i < TopClusters.Len(); ++i) {
    TCluster C;
    CB->GetCluster(TopClusters[i], C);
    TFreqTripleV PeakTimesV;
    TFreqTripleV FreqV;
    C.GetPeaks(DB, QB, PeakTimesV, FreqV, BucketSize, SlidingWindowSize, PresentTime, true);

    // Add clusters with too many peaks to the discard list.
    if (PeakTimesV.Len() > PeakThreshold) {
      C.SetDiscardState(1);
      DiscardedClusterIds.AddKey(TopClusters[i]);
      DiscardedClusters.Add(TPair<TCluster, TInt>(C, TInt(PeakTimesV.Len())));
    }
  }

  Err("Filtered %d overpeaked clusters out of %d (remaining %d)\n", DiscardedClusterIds.Len(), TopClusters.Len(), TopClusters.Len() - DiscardedClusterIds.Len());

  // delete the discarded clusters from the top clusters list.
  TIntV FilteredTopClusters;
  for (int i = 0; i < TopClusters.Len(); ++i) {
    if (!DiscardedClusterIds.IsKey(TopClusters[i])) {
      FilteredTopClusters.Add(TopClusters[i]);
    }
  }
  TopClusters = FilteredTopClusters;

  // log the discarded clusters in a log file.
  Log.OutputDiscardedClusters(QB, DiscardedClusters, PresentTime);
}

void PostCluster::FilterAndCacheClusterSize(TDocBase *DB, TQuoteBase *QB, TClusterBase *CB, LogOutput& Log, TIntV& TopClusters, TSecTm& PresentTime) {
  TIntSet DiscardedClusterIds;  // Contains id's of clusters to be discarded
  TVec<TCluster> DiscardedClusters;
  for (int i = 0; i < TopClusters.Len(); ++i) {
    TCluster C;
    CB->GetCluster(TopClusters[i], C);
    TInt NumVariants = C.GetNumUniqueQuotes();
    if (NumVariants < 4) {
      TInt QuoteId = C.GetMostPopularQuoteId(QB);
      TQuote Q;
      QB->GetQuote(QuoteId, Q);
      TStrV Contents;
      Q.GetContent(Contents);
      TInt NumWords = Contents.Len();
      //TStr RepStr;
      //C.GetRepresentativeQuoteString(RepStr, QB);
      //TStrV Words;
      //RepStr.SplitOnStr(" ", Words);
      //TInt NumWords = Words.Len();
      if ((NumWords <= 3 && NumVariants < 3) || (NumWords <= 5 && NumVariants < 2)) {
        C.SetDiscardState(2);
        DiscardedClusterIds.AddKey(TopClusters[i]);
        DiscardedClusters.Add(C);
      }
    }
  }

  Err("Filtered %d suspiciously unvarianted clusters out of %d (remaining %d)\n", DiscardedClusterIds.Len(), TopClusters.Len(), TopClusters.Len() - DiscardedClusterIds.Len());

  // delete the discarded clusters from the top clusters list.
  TIntV FilteredTopClusters;
  for (int i = 0; i < TopClusters.Len(); ++i) {
    if (!DiscardedClusterIds.IsKey(TopClusters[i])) {
      FilteredTopClusters.Add(TopClusters[i]);
    }
  }
  TopClusters = FilteredTopClusters;

  // log the discarded clusters in a log file.
  Log.OutputDiscardedClustersBySize(QB, DiscardedClusters, PresentTime);
}

/// Remove clusters whose quotes have fewer than QuoteThreshold sources (total) for the last three days
void PostCluster::RemoveOldClusters(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB, LogOutput& Log, TSecTm& PresentTime, PNGraph& QGraph) {
  fprintf(stderr, "Removing old clusters from the CB...\n");
  TIntV AllClusterIds;
  CB->GetAllClusterIds(AllClusterIds);

  TStr CurDateString = PresentTime.GetDtYmdStr();
  TStr Directory;
  Log.GetDirectory(Directory);
  TStr DeleteClustersFile = Directory + "/text/completed/completed_clusters_" + CurDateString + ".txt";
  Err("Directory: %s\n", DeleteClustersFile.CStr());
  FILE *F2 = fopen(DeleteClustersFile.CStr(), "w");

  TInt NumArchived = 0;
  TInt TotalRemainingQuotes = 0;

  // Time setup
  //TUInt TwoWeekTime = 14 * Peaks::NumSecondsInDay;
  TSecTm NextDay = PresentTime;
  NextDay.AddDays(1);
  TUInt PresentTimeI = TUInt(PresentTime.GetAbsSecs());
  PresentTimeI = TUInt(uint(PresentTimeI / Peaks::NumSecondsInDay + 1) * Peaks::NumSecondsInDay);  // round to next 12am
  TUInt ThresholdTime = PresentTimeI - DayThreshold * Peaks::NumSecondsInDay;

  for (int i = 0; i < AllClusterIds.Len(); i++) {
    // Get cluster, and its sources/histogram
    TCluster C;
    CB->GetCluster(AllClusterIds[i], C);
    TIntV ClusterQuoteIds;
    C.GetQuoteIds(ClusterQuoteIds);
    TVec<TUInt> AllSources;
    TCluster::GetUniqueSources(AllSources, ClusterQuoteIds, QB);
    TFreqTripleV PeakV, FreqV;
    C.GetPeaks(DB, QB, PeakV, FreqV, PEAK_BUCKET, PEAK_WINDOW, NextDay);

    // Calculate number of recent sources.
    int NumSources = AllSources.Len();

    TInt NumRecentSources = 0;
    for (int j = 0; j < NumSources; ++j) {
      TDoc Doc;
      DB->GetDoc(AllSources[j], Doc);
      if (Doc.GetDate().GetAbsSecs() >= ThresholdTime) {
        NumRecentSources += 1;
      }
    }

    // NEW STRATEGY: if the peak hasn't changed in the last two weeks, trash the cluster!
    TFlt MaxSources = -1;
    TInt SourceIndex = -1;
    for (int j = 0; j < FreqV.Len(); j++) {
      if (FreqV[j].Val2 > MaxSources) {
        MaxSources = FreqV[j].Val2;
        SourceIndex = j;
      }
    }

    // If the number of recent sources does not pass the threshold, remove that cluster and all its quotes
    if (NumRecentSources < QuoteThreshold || FreqV.Len() - SourceIndex  > 7 * 24 / PEAK_BUCKET) {
      TStr ClusterString = DCluster::GetClusterString(QB, DB, C, FreqV, TInt(PeakV.Len()), CurDateString);
      fprintf(F2, "%s\n", ClusterString.CStr());
      for (int j = 0; j < ClusterQuoteIds.Len(); j++) {
        TQuote Q;
        QB->GetQuote(ClusterQuoteIds[j], Q);
        fprintf(F2, "\t%s\n", DQuote::GetQuoteString(DB, Q, NextDay).CStr());
        TStrV SourceStrings;
        DQuote::GetQuoteSources(DB, Q, SourceStrings);
        for (int k = 0; k < SourceStrings.Len(); k++) {
          fprintf(F2, "\t\t%s\n", SourceStrings[k].CStr());
        }
        QB->RemoveQuote(ClusterQuoteIds[j]);
        QGraph->DelNode(ClusterQuoteIds[j]);
      }
      CB->RemoveCluster(AllClusterIds[i]);
    } else {
      TotalRemainingQuotes++;
      TFlt AvgFreq = 0;
      TInt Count = 0;
      int stop = FreqV.Len() - 12;
      for (int i = FreqV.Len() - 1; i >= stop && i >= 0; --i) {
        AvgFreq += FreqV[i].Val2;
        Count++;
      }

      // TODO: generalize and optimize! this is crappy code
      if (AvgFreq * 5 / Count < MaxSources) {
        C.Archive(); // won't be included.
        NumArchived++;
      } else {
		C.DeathDate = PresentTime;
	  }
    }
  }

  Err("Number of clusters archived: %d out of %d\n", NumArchived.Val, TotalRemainingQuotes.Val);
  fclose(F2);

  Err("Removing old documents...\n");
  DB->RemoveNullDocs(QB);
  return;
}

// I won't lie - I just want to have a method called "nukecluster". :D Doesn't clear null docs.
void PostCluster::NukeCluster(TQuoteBase *QB, TClusterBase *CB, TInt ClusterId, TSecTm& PresentTime, PNGraph& QGraph, bool record) {
  TCluster C;
  CB->GetCluster(ClusterId, C);
  TIntV ClusterQuoteIds;
  C.GetQuoteIds(ClusterQuoteIds);
  for (int j = 0; j < ClusterQuoteIds.Len(); j++) {
    TQuote Q;
    QB->GetQuote(ClusterQuoteIds[j], Q);
    QB->RemoveQuote(ClusterQuoteIds[j]);
    QGraph->DelNode(ClusterQuoteIds[j]);
  }
  CB->RemoveCluster(ClusterId);
}

void PostCluster::SaveTopFilteredClusters(TStr FileName, TQuoteBase *QB, TDocBase *DB, TClusterBase *CB, TIntV& TopClusters, PNGraph& QGraph) {
  TDocBase topDB(DB->GetCounter());
  TQuoteBase topQB(QB->GetCounter());
  TClusterBase topCB(CB->GetCounter());
  PNGraph topQGraph = TNGraph::New();
  for (int i = 0; i < TopClusters.Len(); i++) {
    TCluster C;
    CB->GetCluster(TopClusters[i], C);
    TIntV ClusterQuoteIds;
    C.GetQuoteIds(ClusterQuoteIds);
    for (int j = 0; j < ClusterQuoteIds.Len(); j++) {
      TQuote Q;
      TInt QuoteId = ClusterQuoteIds[j];
      QB->GetQuote(QuoteId, Q);
      TVec<TUInt> Sources;
      Q.GetSources(Sources);
      for (int k = 0; k < Sources.Len(); k++) {
        TDoc D;
        if(DB->GetDoc(Sources[k], D)) {
          topDB.AddStaticDoc(Sources[k], D);
        }
      }
      topQGraph->AddNode(QuoteId);
      topQB.AddStaticQuote(QuoteId, Q);
    }

    for (int j = 0; j < ClusterQuoteIds.Len(); j++) {
      TNGraph::TNodeI NI = QGraph->GetNI(ClusterQuoteIds[j]);
      for (int k = 0; k < NI.GetOutDeg(); k++) {
        topQGraph->AddEdge(ClusterQuoteIds[j], NI.GetOutNId(k));
      }
    }

    topCB.AddStaticCluster(TopClusters[i], C);
  }

  TDataLoader::SaveQBDBCQ(FileName, &topQB, &topDB, &topCB, topQGraph);
  topQGraph.Clr();
}
