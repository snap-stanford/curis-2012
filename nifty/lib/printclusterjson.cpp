#include "stdafx.h"
#include "quote.h"
#include "doc.h"
#include "cluster.h"
#include "printjson.h"
#include "dataloader.h"
#include "postcluster.h"
#include "printclusterjson.h"

const int TPrintClusterJson::NumDaysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

TPrintClusterJson::TPrintClusterJson() {
  OutputJsonDir = JSON_DIR_DEFAULT;
}

TPrintClusterJson::TPrintClusterJson(TStr& OutputJsonDir) {
  this->OutputJsonDir = OutputJsonDir;
}

bool TPrintClusterJson::IsLeapYear(TInt Year) {
  if (Year % 4 != 0) return false;
  if (Year % 100 != 0 || Year % 400 == 0) return true;
  return false;
}

TInt TPrintClusterJson::GetNumDaysInMonth(TSecTm& Date) {
  if (Date.GetMonthN() == 2 && IsLeapYear(Date.GetYearN())) {
    return 29;
  } else {
    return NumDaysInMonth[Date.GetMonthN() - 1];
  }
}

// rounding down.
TSecTm TPrintClusterJson::RoundStartDate(TSecTm& StartDate, TStr& Type) {
  TSecTm NewStartDate = StartDate;
  if (Type == "week") {  // Round to next Saturday (or don't round if StartDate is a Saturday)
    NewStartDate.AddDays(-1 * StartDate.GetDayOfWeekN());  // DayOfWeekN = 7 for Sat, = 1 for Sun
  } else if (Type == "month" || Type == "3month") {
    NewStartDate.AddDays(-1 * StartDate.GetDayN() + 1); // Round to nearest 1st (or don't round if StartDate is a 1st)
    if (Type == "3month") {
      while ((NewStartDate.GetMonthN() - 1) % 3 != 0) {
        TSecTm PrevDate = NewStartDate;
        PrevDate.AddDays(-1);
        NewStartDate.AddDays(-1 * GetNumDaysInMonth(PrevDate)); // go back another month
      }
    }
  }
  return NewStartDate;
}

TSecTm TPrintClusterJson::CalculateEndPeriodDate(TSecTm& CurrentDate, TStr& Type) {
  TSecTm EndPeriodDate = CurrentDate;
  if (Type == "day") {
    EndPeriodDate.AddDays(1);
  } else if (Type == "week") {
    EndPeriodDate.AddDays(7);
  } else if (Type == "month") {
    EndPeriodDate.AddDays(GetNumDaysInMonth(EndPeriodDate));
  } else if (Type == "3month") {
    for (int i = 0; i < 3; i++) {
      EndPeriodDate.AddDays(GetNumDaysInMonth(EndPeriodDate));
    }
  }
  return EndPeriodDate;
}

bool TPrintClusterJson::IsClusterDuplicate(TQuoteBase *QBCumulative, TClusterBase *CBCumulative, TStrSet& ExistingQuotes, TInt CandidateCluster) {
  TCluster C;
  CBCumulative->GetCluster(CandidateCluster, C);
  TIntV CQuoteIds;
  C.GetQuoteIds(CQuoteIds);
  TStrV CQuoteContents;
  bool SkipCluster = false;
  for (int j = 0; j < CQuoteIds.Len(); j++) {
    TQuote Q;
    QBCumulative->GetQuote(CQuoteIds[j], Q);
    TStr QContent;
    Q.GetContentString(QContent);
    if (ExistingQuotes.IsKey(QContent)) {
      SkipCluster = true;
    }
    CQuoteContents.Add(QContent);
  }
  for (int j = 0; j < CQuoteContents.Len(); j++) {
    ExistingQuotes.AddKey(CQuoteContents[j]);
  }
  return SkipCluster;
}

TInt TPrintClusterJson::QuarterDays(TInt NumDays, TInt Day) {
  TInt Halfway = (NumDays + 1) / 2;
  if (Day < Halfway) {
    if (Day < (Halfway + 1) / 2) {
      return 0;
    } else {
      return 1;
    }
  } else if (Day < (Halfway * 3 + 1) / 2) {
    return 2;
  } else {
    return 3;
  }
}

TInt TPrintClusterJson::BucketDays(TSecTm& Day, TSecTm& StartDate, TStr Type) {
  if (Type == "week") {
    return Day.GetDayOfWeekN();
  } else if (Type == "month") {
    return QuarterDays(GetNumDaysInMonth(Day), Day.GetDayN());
  } else {
    return 2 * (Day.GetMonthN() - StartDate.GetMonthN()) + (QuarterDays(GetNumDaysInMonth(Day), Day.GetDayN()) / 2); // TODO: this is stupid
  }
}

/// StartString and EndString must be in the form "YYYY-MM-DD", e.g. "2012-02-22"
void TPrintClusterJson::PrintClusterJsonForPeriod(TSecTm& StartDate, TSecTm& PresentTime, TStr& LogDirectory, TStr Type, TStr QBDBCDirectory) {
  IAssert(Type == "day" || Type == "week" || Type == "month" || Type == "3month");

  StartDate = RoundStartDate(StartDate, Type);
  TSecTm EndDate = CalculateEndPeriodDate(StartDate, Type);
  Err("Printing %s JSON from %s to %s\n", Type.CStr(), StartDate.GetDtYmdStr().CStr(), EndDate.GetDtYmdStr().CStr());
  TSecTm CurrentDate = StartDate;

  TIntV ClustersToPrint;
  fprintf(stderr, "Preparing to write JSON to file\n");

  TQuoteBase QBCumulative;
  TDocBase DBCumulative;
  TClusterBase CBCumulative;
  PNGraph QGraphCumulative = PNGraph::New();
  TIntSet MaxPeakClusterIds;  // Store clusters that peaked during this time, with freq >= FreqCutoff
  TInt FreqCutoff = 350;

  LogOutput Log;
  Log.DisableLogging();

  while (CurrentDate <= PresentTime) { // Only look up until "today"
    // Get day start and end information
    TSecTm DayStart = TSecTm(uint(CurrentDate.GetAbsSecs() / Peaks::NumSecondsInDay) * Peaks::NumSecondsInDay);
    TSecTm DayEnd = TSecTm(DayStart.GetAbsSecs() + Peaks::NumSecondsInDay - 1);
        
    // Load Cumulative QBDBCB
    TQuoteBase QB;
    TDocBase DB;
    TClusterBase CB;
    PNGraph QGraph;
    fprintf(stderr, "Loading cumulative QBDBCB from %s from file...\n", CurrentDate.GetDtYmdStr().CStr());
    TDataLoader::LoadCumulative(QBDBCDirectory, CurrentDate.GetDtYmdStr(), QB, DB, CB, QGraph);

    // Add clusters that peaked on this day, with freq > 350, to MaxPeakClusterIds
    // Use postclustering to filter
    TIntV ClusterIds;
    CB.GetAllClusterIds(ClusterIds);
    PostCluster::FilterBlacklistedQuotes(&DB, &QB, &CB, Log, ClusterIds, CurrentDate);

    for (int i = 0; i < ClusterIds.Len(); i++) {
      TCluster C;
      CB.GetCluster(ClusterIds[i], C);
      // Ensure candidate cluster has correct frequency cuttoff...
      if (C.GetNumUniqueSources(&QB) >= FreqCutoff) {
        TDateFreq DF;
        C.GetMaxPeakInfo(DF);
        // And that it peaked on this day (?)
        if (DF.Val1 >= DayStart && DF.Val1 <= DayEnd) {
          MaxPeakClusterIds.AddKey(ClusterIds[i]);
        }
      }
    }

    // Fold in new QBDBCB with previous QBDBCB
    TDataLoader::MergeTopQBDBCB(QBCumulative, DBCumulative, CBCumulative, QGraphCumulative, QB, DB, CB, QGraph, CurrentDate, true);

    CurrentDate.AddDays(1);
  }

  // Use final CBCumulative to make hashmap from peak_date -> (cluster_id, freq_for_peak_date)
  THash<TInt, TVec<TIntPr> > MaxPeakClustersPerDate;
  for (TIntSet::TIter ClusterId = MaxPeakClusterIds.BegI(); ClusterId < MaxPeakClusterIds.EndI(); ClusterId++) {
    TCluster C;
    CBCumulative.GetCluster(*ClusterId, C);
    TDateFreq DF;
    C.GetMaxPeakInfo(DF);
    TInt Bucket = BucketDays(DF.Val1, StartDate, Type);
    TVec<TIntPr> MaxPeakClusters;
    MaxPeakClustersPerDate.IsKeyGetDat(Bucket, MaxPeakClusters);
    MaxPeakClusters.Add(TIntPr(*ClusterId, DF.Val2));
    MaxPeakClustersPerDate.AddDat(Bucket, MaxPeakClusters);
  }

  // For each day, add clusters with largest X max peaks to a set
  int PeaksPerType [] = {1, 3, 6};
  if (Type == "month") { PeaksPerType[0] = 2; PeaksPerType[1] = 5; PeaksPerType[2] = 20; }
  if (Type == "3month") { PeaksPerType[0] = 1; PeaksPerType[1] = 3; PeaksPerType[2] = 8; }
  TIntSet ClustersToPrintSet[3];
  TStrSet ExistingQuotes[3];
  for (THash<TInt, TVec<TIntPr> >::TIter CurI = MaxPeakClustersPerDate.BegI(); CurI < MaxPeakClustersPerDate.EndI(); CurI++) {
    TVec<TIntPr> MaxPeakClusters = CurI.GetDat();
    MaxPeakClusters.SortCmp(TCmpPairByVal2<TInt, TInt>(false));  // Sort in descending order, by the frequency
    for (int i = 0; i < 3; i++) {
      int NumEntered = 0;
      for (int j = 0; NumEntered < PeaksPerType[i] && j < MaxPeakClusters.Len(); j++) {
        if (!IsClusterDuplicate(&QBCumulative, &CBCumulative, ExistingQuotes[i], MaxPeakClusters[j].Val1)) {
          ClustersToPrintSet[i].AddKey(MaxPeakClusters[j].Val1);
          NumEntered++;
        }
      }
    }
  }

  TIntV TopFilteredClusters;
  PNGraph P;
  CBCumulative.GetTopClusterIdsByFreq(TopFilteredClusters);
  PostCluster::FilterBlacklistedQuotes(&DBCumulative, &QBCumulative, &CBCumulative, Log, TopFilteredClusters, CurrentDate);
  PostCluster::FilterAndCacheClusterSize(&DBCumulative, &QBCumulative, &CBCumulative, Log, TopFilteredClusters, CurrentDate);
  PostCluster::FilterAndCacheClusterPeaks(&DBCumulative, &QBCumulative, &CBCumulative, Log, TopFilteredClusters, CurrentDate);

  // Fill the rest of the X quotes for the time period with most frequent quotes during this time period
  TIntV TopFilteredClustersLimit;
  int NumToLimit [] = {10, 30, 100};
  if (Type == "week") NumToLimit[2] = 50;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; ClustersToPrintSet[i].Len() < NumToLimit[i] && j < TopFilteredClusters.Len(); j++) {
      if (!IsClusterDuplicate(&QBCumulative, &CBCumulative, ExistingQuotes[i], TopFilteredClusters[j])) {
        ClustersToPrintSet[i].AddKey(TopFilteredClusters[j]);
      }
    }
  }

  // Prepare clusters for printing - by putting them in a vector
  ClustersToPrintSet[2].GetKeyV(ClustersToPrint);
  ClustersToPrint.SortCmp(TCmpTClusterIdByNumQuotes(false, &CBCumulative));
  TVec<TIntV> IsClusterPrinted;
  for (int i = 0; i < ClustersToPrint.Len(); i++) {
    TIntV CurClusterPrinted;
    for (int j = 0; j < 3; j++) {
      if (ClustersToPrintSet[j].IsKey(ClustersToPrint[i])) {
        CurClusterPrinted.Add(1);
      } else {
        CurClusterPrinted.Add(0);
      }
    }
    IsClusterPrinted.Add(CurClusterPrinted);
  }

  // Instead of calling UpdateData, call TPrintJson::PrintClustersJson and TPrintJson::PrintClustersDataJson
  // Graph the clusters in ClustersToPrint, and include all the TopFilteredClustersWoDups in the table
  TStr OutputJsonDirFinal = OutputJsonDir + Type + "/";
  bool IncludeDate = false;  // Have filename contain YYYY-MM instead of YYYY-MM-DD
  if (Type == "week") { IncludeDate = true; }
  TPrintJson::PrintClustersJson(&QBCumulative, &DBCumulative, &CBCumulative, ClustersToPrint, IsClusterPrinted,
                                OutputJsonDirFinal, OutputJsonDirFinal, StartDate, EndDate, IncludeDate);

  // Print individual cluster JSON info
  TStr ClusterJSONDirectory = LogDirectory + "/web/json/clusters/";
  TSecTm ZeroTime(0);

  for (int j = 0; j < ClustersToPrint.Len(); j++) {
    // The last parameter is ZeroTime so that the Peaks::GetFrequencyVector starts the graph of the cluster at the first source,
    // rather than X days before the TSecTm parameter given
    TPrintJson::PrintClusterJSON(&QBCumulative, &DBCumulative, &CBCumulative, QGraphCumulative, ClusterJSONDirectory, ClustersToPrint[j], ZeroTime);
  }

  return;
}
