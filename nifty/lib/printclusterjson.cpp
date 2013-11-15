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

/// For testing
void TPrintClusterJson::PrintFreqOverTime(THash<TSecTm, TFltV>& FreqOverTime) {
  TVec<TSecTm> Times;
  FreqOverTime.GetKeyV(Times);
  Times.Sort();
  for (int i = 0; i < Times.Len(); i++) {
    fprintf(stderr, "%s: [", Times[i].GetYmdTmStr().CStr());
    TFltV Values = FreqOverTime.GetDat(Times[i]);
    for (int v = 0; v < Values.Len(); v++) {
      fprintf(stderr, "%f", Values[v].Val);
      if (v < Values.Len() - 1) { fprintf(stderr, ", "); }
    }
    fprintf(stderr, "]\n");
  }
}

/// Adds the frequency data for CurrDate to the data structures
void TPrintClusterJson::UpdateDataForJsonPrinting(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB, THash<TSecTm, TFltV>& FreqOverTime, TVec<TSecTm>& Times, TIntV& ClustersToPrint, THash<TInt, TStr>& ClustersRepQuote, TSecTm& CurrentDate, TInt DaysPassed, TStr& Type) {
  if (Type == "day") {
    DaysPassed = 7;
  }

  // Initialize the new values in the hash table
  TCluster MostRecentC;
  bool ClusterInCB = CB->GetCluster(ClustersToPrint[ClustersToPrint.Len() - 1], MostRecentC);
  IAssert(ClusterInCB);

  TIntV QuoteIds;
  MostRecentC.GetQuoteIds(QuoteIds);
  TVec<TUInt> Sources;
  TCluster::GetUniqueSources(Sources, QuoteIds, QB);
  TFreqTripleV FreqV;
  TVec<TSecTm> NewTimes;
  Peaks::GetFrequencyVector(DB, Sources, FreqV, 24, 1, CurrentDate, DaysPassed);
  fprintf(stderr, "Length of frequency vector: %d\n", FreqV.Len());
  for (int i = 0; i < FreqV.Len(); i++) {
    if (Times.Len() == 0 || FreqV[i].Val3 > Times[Times.Len() - 1]) {
      FreqOverTime.AddDat(FreqV[i].Val3, TFltV());
      Times.Add(FreqV[i].Val3);
      fprintf(stderr, "Added %s\n", FreqV[i].Val3.GetYmdTmStr().CStr());
      NewTimes.Add(FreqV[i].Val3);
    }
  }

  fprintf(stderr, "Number of clusters to print: %d\n", ClustersToPrint.Len());
  for (int i = 0; i < ClustersToPrint.Len(); i++) {
    TCluster C;
    ClusterInCB = CB->GetCluster(ClustersToPrint[i], C);
    if (!ClusterInCB) {  // Old cluster that has now been removed from the ClusterBase; so frequency is zero
      fprintf(stderr, "Cluster not in clusterbase!\n");
      for (int j = 0; j < NewTimes.Len(); j++) {
        TFltV Freqs = FreqOverTime.GetDat(NewTimes[j]);
        Freqs.Add(0);
        FreqOverTime.AddDat(NewTimes[j], Freqs);
      }
      continue;
    }

    TStr CRepQuote;
    C.GetRepresentativeQuoteString(CRepQuote, QB);
    if (CRepQuote == "") {
      TIntV CRepQuoteIds;
      C.GetRepresentativeQuoteIds(CRepQuoteIds);
      fprintf(stderr, "Warning: Representative Quote (id %d) is blank\n", CRepQuoteIds[0].Val);
    }
    ClustersRepQuote.AddDat(ClustersToPrint[i], CRepQuote);
    
    TIntV CQuoteIds;
    C.GetQuoteIds(CQuoteIds);
    TVec<TUInt> CSources;
    TCluster::GetUniqueSources(CSources, CQuoteIds, QB);
    TFreqTripleV CFreqV;
    Peaks::GetFrequencyVector(DB, CSources, CFreqV, 24, 1, CurrentDate, DaysPassed);

    for (int j = 0; j < CFreqV.Len(); j++) {
      if (!FreqOverTime.IsKey(CFreqV[j].Val3)) {
        fprintf(stderr, "%s \n", CFreqV[j].Val3.GetYmdTmStr().CStr());
        fprintf(stderr, "\t%f\n", CFreqV[j].Val2.Val);
        continue;
      }
      TFltV Freqs = FreqOverTime.GetDat(CFreqV[j].Val3);
      if (Freqs.Len() < i + 1) {
        Freqs.Add(CFreqV[j].Val2);
        FreqOverTime.AddDat(CFreqV[j].Val3, Freqs);
      } else if (CFreqV[j].Val2 > Freqs[i]) {
        Freqs[i] = CFreqV[j].Val2;
        FreqOverTime.AddDat(CFreqV[j].Val3, Freqs);
      }
    }
  }

  //fprintf(stderr, "FreqOverTime hashtable: \n");
  //PrintFreqOverTime(FreqOverTime);
}


void TPrintClusterJson::PrintClustersInJson(THash<TSecTm, TFltV>& FreqOverTime, TVec<TSecTm>& Times, TIntV& ClustersToPrint, THash<TInt, TStr>& ClustersRepQuote, TStr& OutputFilename) {
  fprintf(stderr, "Writing JSON to file: %s\n", OutputFilename.CStr());

  FILE *F = fopen(OutputFilename.CStr(), "w");
  fprintf(F, "{\"values\": [");

  TInt NumTimesToPrint = Times.Len() - 1;  // Skip the last time stamp, because it is all zeroes and will 
                                           // mess up the visualizer
  for (int i = 0; i < NumTimesToPrint; i++) {
    //if (Times[i].GetAbsSecs() % (Peaks::NumSecondsInDay) != 0) { continue; }
    fprintf(F, "{\"values\": [");
    TFltV Freqs = FreqOverTime.GetDat(Times[i]);
    for (int j = 0; j < Freqs.Len(); j++) {
      fprintf(F, "%d", TFlt::Round(Freqs[j].Val));
      if (j < Freqs.Len() - 1) { fprintf(F, ", "); }
    }
    fprintf(F, "], \"label\": \"%s\"}", Times[i].GetYmdTmStr().CStr());
    if (i < NumTimesToPrint - 1) { fprintf(F, ", "); }
  }

  fprintf(F, "], \"label\": [");
  for (int i = 0; i < ClustersToPrint.Len(); i++) {
    fprintf(F, "\"%d\"", ClustersToPrint[i].Val);
    if (i < ClustersToPrint.Len() - 1) { fprintf(F, ", "); }
  }

  fprintf(F, "], \"quote\": [");
  for (int i = 0; i < ClustersToPrint.Len(); i++) {
    TStr CRepQuote = ClustersRepQuote.GetDat(ClustersToPrint[i]);
    fprintf(F, "\"%s\"", CRepQuote.CStr());
    if (i < ClustersToPrint.Len() - 1) { fprintf(F, ", "); }
  }

  fprintf(F, "]}");
  fclose(F);
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

void TPrintClusterJson::PrintClusterJSONForPeriod(TStr& CurTimeString, TStr Type, TStr QBDBCDirectory) {
  if (!(Type == "week" || Type == "month" || Type == "3month")) return;
  TSecTm CurrentDate = TSecTm::GetDtTmFromYmdHmsStr(CurTimeString);
  TSecTm StartDate = RoundStartDate(CurrentDate, Type);
  TSecTm EndDate = CalculateEndPeriodDate(StartDate, Type);
  CurrentDate = StartDate;

  Err("Printing %s JSON from %s to %s\n", Type.CStr(), StartDate.GetDtYmdStr().CStr(), EndDate.GetDtYmdStr().CStr());

  TQuoteBase QBCumulative;
  TDocBase DBCumulative;
  TClusterBase CBCumulative;

  TIntSet TopClusters;

  while (CurrentDate < EndDate) {
    TQuoteBase QB;
    TDocBase DB;
    TClusterBase CB;
    PNGraph QGraph;
    Err("Loading top QBDBCB for %s from file...\n", CurrentDate.GetDtYmdStr().CStr());
    TDataLoader::LoadCumulative(QBDBCDirectory, CurrentDate.GetDtYmdStr(), QB, DB, CB, QGraph);

    TIntV TopFilteredClusters;
    CB.GetTopClusterIdsByFreq(TopFilteredClusters);
    CB.SortClustersByPopularity(&DB, &QB, TopFilteredClusters, CurrentDate);
    int NumClusters = 24; // #clusters per day to remember
    if (Type == "month") NumClusters = 6;
    else if (Type == "3month") NumClusters = 2;
    for (int i = 0; i < NumClusters && i < TopFilteredClusters.Len(); i++) {
      TopClusters.AddKey(TopFilteredClusters[i]);
    }
    TDataLoader::MergeQBDBCB(QBCumulative, DBCumulative, CBCumulative, QB, DB, CB, CurrentDate);
    CurrentDate.AddDays(1);
  }

  TIntV TopClustersV;
  TopClusters.GetKeyV(TopClustersV);
  TopClustersV.Sort(true);

  TStr OutputJsonDirFinal = OutputJsonDir + Type + "/";
  bool IncludeDate = (Type == "week");  // Have filename contain YYYY-MM instead of YYYY-MM-DD
  TPrintJson::PrintClustersJson(&QBCumulative, &DBCumulative, &CBCumulative, TopClustersV, TopClustersV,
                                OutputJsonDirFinal, OutputJsonDirFinal, StartDate, EndDate, IncludeDate);

}


void TPrintClusterJson::FilterDuplicateClusters(TQuoteBase *QBCumulative, TClusterBase *CBCumulative, TIntV& TopFilteredClusters, TIntV& TopFilteredClustersWoDups) {
  TStrSet SeenQuoteContents;
  for (int i = 0; i < TopFilteredClusters.Len(); i++) {
    bool SkipCluster = false;
    TCluster C;
    CBCumulative->GetCluster(TopFilteredClusters[i], C);
    TIntV CQuoteIds;
    C.GetQuoteIds(CQuoteIds);
    TStrV CQuoteContents;
    for (int j = 0; j < CQuoteIds.Len(); j++) {
      TQuote Q;
      QBCumulative->GetQuote(CQuoteIds[j], Q);
      TStr QContent;
      Q.GetContentString(QContent);
      if (SeenQuoteContents.IsKey(QContent)) {
        SkipCluster = true;
        break;
      }
      CQuoteContents.Add(QContent);
    }
    if (!SkipCluster) {
      // don't add the quote contents until we know the cluster doesn't contain any duplicate quotes (so that future clusters aren't skipped unnecessarily)
      for (int j = 0; j < CQuoteContents.Len(); j++) {
        SeenQuoteContents.AddKey(CQuoteContents[j]);
      }
      TopFilteredClustersWoDups.Add(TopFilteredClusters[i]);
    }
  }
}

/// StartString and EndString must be in the form "YYYY-MM-DD", e.g. "2012-02-22"
void TPrintClusterJson::PrintClusterJsonForPeriod(TStr& StartString, TStr& EndString, TStr& LogDirectory, TStr Type, TStr QBDBCDirectory) {

  TStr TopClusterSelection = "cumulative";  // Can be "cumulative" or "daily" 
  //if (UseDailyTopClusters) {  // Not using this feature, so commenting it out for now
  //  TopClusterSelection = TStr("daily");
  //

  fprintf(stderr, "Top Cluster Selection: %s\n", TopClusterSelection.CStr());
  IAssert(TopClusterSelection == "cumulative" || TopClusterSelection == "daily");

  fprintf(stderr, "Type: %s\n", Type.CStr());
  IAssert(Type == "day" || Type == "week" || Type == "month" || Type == "3month");

  /*if (Type == "day") {
    NumTopClustersPerDay = 20;
  } else if (Type == "week") {
    NumTopClustersPerDay = 5;
  } else if (Type == "month" || Type == "3month") {
    NumTopClustersPerDay = 2;
  }*/

  TSecTm StartDate = TSecTm::GetDtTmFromYmdHmsStr(StartString);
  StartDate = RoundStartDate(StartDate, Type);
  Err("start date: %s\n", StartDate.GetDtYmdStr().CStr());
  TSecTm EndDate = CalculateEndPeriodDate(StartDate, Type);
  //TSecTm EndDate = TSecTm::GetDtTmFromYmdHmsStr(EndString);
  fprintf(stderr, "End date: %s\n", EndDate.GetDtYmdStr().CStr());
  TSecTm CurrentDate = StartDate;

  while (CurrentDate < EndDate) {
    // Writes cluster information to JSON file for the current day/week/month interval
    TSecTm StartPeriodDate = CurrentDate;
    TSecTm EndPeriodDate = CalculateEndPeriodDate(CurrentDate, Type);

    THash<TSecTm, TFltV> FreqOverTime;
    TVec<TSecTm> Times;
    TIntV ClustersToPrint;
    THash<TInt, TStr> ClustersRepQuote;
    fprintf(stderr, "Preparing to write JSON to file\n");

    TQuoteBase QBCumulative;
    TDocBase DBCumulative;
    TClusterBase CBCumulative;
    TIntSet MaxPeakClusterIds;  // Store clusters that peaked during this time, with freq >= FreqCutoff
    TInt FreqCutoff = 350;
 
    LogOutput Log;
    Log.DisableLogging();

    int i;
    for (i = 0; CurrentDate < EndPeriodDate && CurrentDate < EndDate; ++i) {
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
      PostCluster::GetTopFilteredClusters(&CB, &DB, &QB, Log, ClusterIds, CurrentDate, QGraph, false);

      //CB.GetAllClusterIds(ClusterIds);
      for (int i = 0; i < ClusterIds.Len(); i++) {
        TCluster C;
        CB.GetCluster(ClusterIds[i], C);
        //if (C.GetNumQuotes() >= FreqCutoff) {
        if (C.GetNumUniqueSources(&QB) >= FreqCutoff) {
          TDateFreq DF;
          C.GetMaxPeakInfo(DF);
          TSecTm DayStart = TSecTm(uint(CurrentDate.GetAbsSecs() / Peaks::NumSecondsInDay) * Peaks::NumSecondsInDay);
          TSecTm DayEnd = TSecTm(DayStart.GetAbsSecs() + Peaks::NumSecondsInDay - 1);
          if (DF.Val1 >= DayStart && DF.Val1 <= DayEnd) {
            MaxPeakClusterIds.AddKey(ClusterIds[i]);
          }
        }
      }

      fprintf(stderr, "Done loading cumulative QBDBCB!\n");

      /*if (TopClusterSelection == "daily") {
        // Get top clusters
        TIntV TopClusters;
        PostCluster::GetTopFilteredClusters(&CB, &DB, &QB, Log, TopClusters, CurrentDate, QGraph, false);

        for (int j = 0; j < NumTopClustersPerDay && j < TopClusters.Len(); j++) {
          if (ClustersToPrint.SearchForw(TopClusters[j]) < 0) {
            ClustersToPrint.Add(TopClusters[j]);
          } 
        }
        UpdateDataForJsonPrinting(&QB, &DB, &CB, FreqOverTime, Times, ClustersToPrint, ClustersRepQuote, CurrentDate, i + 1, Type);
      } else if (TopClusterSelection == "cumulative") {*/

      if (QBCumulative.Len() == 0) {
        QBCumulative = QB;
        DBCumulative = DB;
        CBCumulative = CB;
      } else {
        fprintf(stderr, "Merging QBDBCB!\n");
        TDataLoader::MergeQBDBCB2(QBCumulative, DBCumulative, CBCumulative, QB, DB, CB, CurrentDate, true);
      }

      //}
      CurrentDate.AddDays(1);
    }

    // Use final CBCumulative to make hashmap from peak_date -> (cluster_id, freq_for_peak_date)
    THash<TSecTm, TVec<TIntPr> > MaxPeakClustersPerDate;
    for (TIntSet::TIter ClusterId = MaxPeakClusterIds.BegI(); ClusterId < MaxPeakClusterIds.EndI(); ClusterId++) {
      TCluster C;
      CBCumulative.GetCluster(*ClusterId, C);
      TDateFreq DF;
      C.GetMaxPeakInfo(DF);
      TVec<TIntPr> MaxPeakClusters;
      MaxPeakClustersPerDate.IsKeyGetDat(DF.Val1, MaxPeakClusters);
      MaxPeakClusters.Add(TIntPr(*ClusterId, DF.Val2));
      MaxPeakClustersPerDate.AddDat(DF.Val1, MaxPeakClusters);
    }

    // For each day, add clusters with largest X max peaks to a set
    TInt NumMaxPeakPerDay = 5;
    if (Type == "month") NumMaxPeakPerDay = 3;
    if (Type == "3month") NumMaxPeakPerDay = 2;
    TIntSet ClustersToPrintSet;
    for (THash<TSecTm, TVec<TIntPr> >::TIter CurI = MaxPeakClustersPerDate.BegI(); CurI < MaxPeakClustersPerDate.EndI(); CurI++) {
      TVec<TIntPr> MaxPeakClusters = CurI.GetDat();
      MaxPeakClusters.SortCmp(TCmpPairByVal2<TInt, TInt>(false));  // Sort in descending order, by the frequency
      for (int i = 0; i < NumMaxPeakPerDay && i < MaxPeakClusters.Len(); i++) {
        ClustersToPrintSet.AddKey(MaxPeakClusters[i].Val1);
      }
    }

    //if (TopClusterSelection == "cumulative") {

    TIntV TopFilteredClusters;
    CBCumulative.GetTopClusterIdsByFreq(TopFilteredClusters);
    LogOutput TmpLog;
    TmpLog.DisableLogging();
    PostCluster::FilterAndCacheClusterSize(&DBCumulative, &QBCumulative, &CBCumulative, TmpLog, TopFilteredClusters, CurrentDate);
    PostCluster::FilterAndCacheClusterPeaks(&DBCumulative, &QBCumulative, &CBCumulative, TmpLog, TopFilteredClusters, CurrentDate);

    // Fill the rest of the X quotes for the time period with most frequent quotes during this time period
    TIntV TopFilteredClustersLimit;
    TInt NumToLimit = 250;
    if (Type == "month") NumToLimit = 500;
    else if (Type == "3month") NumToLimit = 1000;
    for (int i = 0; ClustersToPrintSet.Len() < NumToLimit && i < TopFilteredClusters.Len(); i++) {
      ClustersToPrintSet.AddKey(TopFilteredClusters[i]);
    }
    TopFilteredClusters.Clr();
    ClustersToPrintSet.GetKeyV(TopFilteredClusters);
    TopFilteredClusters.SortCmp(TCmpTClusterIdByNumQuotes(false, &CBCumulative));

    // Filter out clusters that have duplicate quote content (remove the less popular duplicate cluster(s))
    TIntV TopFilteredClustersWoDups;
    FilterDuplicateClusters(&QBCumulative, &CBCumulative, TopFilteredClusters, TopFilteredClustersWoDups);
    
    for (int j = 0; j < 100 && j < TopFilteredClustersWoDups.Len(); j++) {
      ClustersToPrint.Add(TopFilteredClustersWoDups[j]);
    }

    // Instead of calling UpdateData, call TPrintJson::PrintClustersJson and TPrintJson::PrintClustersDataJson
    // Graph the clusters in ClustersToPrint, and include all the TopFilteredClustersWoDups in the table
    TStr OutputJsonDirFinal = OutputJsonDir + Type + "/";
    bool IncludeDate = false;  // Have filename contain YYYY-MM instead of YYYY-MM-DD
    if (Type == "week") { IncludeDate = true; }
    TPrintJson::PrintClustersJson(&QBCumulative, &DBCumulative, &CBCumulative, ClustersToPrint, TopFilteredClustersWoDups,
                                  OutputJsonDirFinal, OutputJsonDirFinal, StartPeriodDate, EndPeriodDate, IncludeDate);

    // Print individual cluster JSON info
    TStr ClusterJSONDirectory = LogDirectory + "/web/json/clusters/";
    TSecTm ZeroTime(0);
	// TODO: FIX
	//PNGraph QGraph = TNGraph::New();
    for (int j = 0; j < ClustersToPrint.Len(); j++) {
      // The last parameter is ZeroTime so that the Peaks::GetFrequencyVector starts the graph of the cluster at the first source,
      // rather than X days before the TSecTm parameter given
      TCluster C;
      CBCumulative.GetCluster(ClustersToPrint[j], C);
      TQuoteBase QB;
      TDocBase DB;
      TClusterBase CB;
      PNGraph QGraph;
      TDataLoader::LoadCumulative(QBDBCDirectory, C.DeathDate.GetDtYmdStr(), QB, DB, CB, QGraph);
      
      TPrintJson::PrintClusterJSON(&QBCumulative, &DBCumulative, &CBCumulative, QGraph, ClusterJSONDirectory, ClustersToPrint[j], ZeroTime);

    }
    //UpdateDataForJsonPrinting(&QBCumulative, &DBCumulative, &CBCumulative, FreqOverTime, Times, ClustersToPrint,
     //                         ClustersRepQuote, CurrentDate, i + 1, Type);
    /*} else {

      // TODO: wtf is this?! o.o
      if (CurrentDate == EndPeriodDate) {
        TStr OutputFilename = "../../../public_html/curis/output/clustering/visualization-" + Type + "-ext/data/clusterinfo-" +
                          StartPeriodDate.GetDtYmdStr() + "-new.json";
        PrintClustersInJson(FreqOverTime, Times, ClustersToPrint, ClustersRepQuote, OutputFilename);
      }
    }*/
  }

  return;
}
