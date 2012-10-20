#include "stdafx.h"
#include "quote.h"
#include "doc.h"
#include "cluster.h"
#include "printjson.h"
#include "dataloader.h"
#include "postcluster.h"
#include "printclusterjson.h"

const int TPrintClusterJson::NumDaysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
TStr StandardJsonDir = "../../../public_html/curis/output/clustering/webdata/";

TPrintClusterJson::TPrintClusterJson() {
  OutputJsonDir = StandardJsonDir;
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
  TIntV Sources;
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
    TIntV CSources;
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
        NewStartDate.AddDays(-1 * GetNumDaysInMonth(NewStartDate)); // go back another month
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

/// StartString and EndString must be in the form "YYYY-MM-DD", e.g. "2012-02-22"
void TPrintClusterJson::PrintClusterJsonForPeriod(TStr& StartString, TStr& EndString, LogOutput& Log, TStr Type, TStr QBDBCDirectory) {

  TStr TopClusterSelection = "cumulative";  // Can be "cumulative" or "daily" 
  //if (UseDailyTopClusters) {  // Not using this feature, so commenting it out for now
  //  TopClusterSelection = TStr("daily");
  //}

  fprintf(stderr, "Top Cluster Selection: %s\n", TopClusterSelection.CStr());
  IAssert(TopClusterSelection == "cumulative" || TopClusterSelection == "daily");

  fprintf(stderr, "Type: %s\n", Type.CStr());
  IAssert(Type == "day" || Type == "week" || Type == "month" || Type == "3month");

  if (Type == "day") {
    NumTopClustersPerDay = 20;
  } else if (Type == "week") {
    NumTopClustersPerDay = 5;
  } else if (Type == "month" || Type == "3month") {
    NumTopClustersPerDay = 2;
  }

  TSecTm StartDate = TSecTm::GetDtTmFromYmdHmsStr(StartString);
  StartDate = RoundStartDate(StartDate, Type);
  Err("start date: %s\n", StartDate.GetDtYmdStr().CStr());
  TSecTm EndDate = TSecTm::GetDtTmFromYmdHmsStr(EndString);
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

    int i;
    for (i = 0; CurrentDate < EndPeriodDate && CurrentDate < EndDate; ++i) {
      // Load Cumulative QBDBCB
      TQuoteBase QB;
      TDocBase DB;
      TClusterBase CB;
      PNGraph QGraph;
      fprintf(stderr, "Loading cumulative QBDBCB from %s from file...\n", CurrentDate.GetDtYmdStr().CStr());
      TDataLoader::LoadCumulative(QBDBCDirectory, CurrentDate.GetDtYmdStr(), QB, DB, CB, QGraph);
      fprintf(stderr, "Done loading cumulative QBDBCB!\n");

      if (TopClusterSelection == "daily") {
        // Get top clusters
        TIntV TopClusters;
        PostCluster::GetTopFilteredClusters(&CB, &DB, &QB, Log, TopClusters, CurrentDate, QGraph, false);

        for (int j = 0; j < NumTopClustersPerDay && j < TopClusters.Len(); j++) {
          if (ClustersToPrint.SearchForw(TopClusters[j]) < 0) {
            ClustersToPrint.Add(TopClusters[j]);
          } 
        }
        UpdateDataForJsonPrinting(&QB, &DB, &CB, FreqOverTime, Times, ClustersToPrint, ClustersRepQuote, CurrentDate, i + 1, Type);
      } else if (TopClusterSelection == "cumulative") {
        if (QBCumulative.Len() == 0) {
          QBCumulative = QB;
          DBCumulative = DB;
          CBCumulative = CB;
        } else {
          fprintf(stderr, "Merging QBDBCB!\n");
          TDataLoader::MergeQBDBCB(QBCumulative, DBCumulative, CBCumulative, QB, DB, CB, CurrentDate);
        }
      }
      CurrentDate.AddDays(1);
    }

    if (TopClusterSelection == "cumulative") {
      TIntV TopFilteredClusters;
      CBCumulative.GetTopClusterIdsByFreq(TopFilteredClusters);
      PostCluster::FilterAndCacheClusterSize(&DBCumulative, &QBCumulative, &CBCumulative, Log, TopFilteredClusters, CurrentDate);
      PostCluster::FilterAndCacheClusterPeaks(&DBCumulative, &QBCumulative, &CBCumulative, Log, TopFilteredClusters, CurrentDate);

      for (int j = 0; j < 50 && j < TopFilteredClusters.Len(); j++) {
        ClustersToPrint.Add(TopFilteredClusters[j]);
      }

      // Instead of calling UpdateData, call TPrintJson::PrintClustersJson and TPrintJson::PrintClustersDataJson
      // Graph the clusters in ClustersToPrint, and include all the TopFilteredClusters in the table
      TStr OutputJsonDirFinal = OutputJsonDir + Type + "/";
      bool IncludeDate = false;  // Have filename contain YYYY-MM instead of YYYY-MM-DD
      if (Type == "week") { IncludeDate = true; }
      TPrintJson::PrintClustersJson(&QBCumulative, &DBCumulative, &CBCumulative, ClustersToPrint, TopFilteredClusters,
                                    OutputJsonDirFinal, OutputJsonDirFinal, StartPeriodDate, EndPeriodDate, IncludeDate);
      //UpdateDataForJsonPrinting(&QBCumulative, &DBCumulative, &CBCumulative, FreqOverTime, Times, ClustersToPrint,
       //                         ClustersRepQuote, CurrentDate, i + 1, Type);
    } else {

      if (CurrentDate == EndPeriodDate) {
        TStr OutputFilename = "../../../public_html/curis/output/clustering/visualization-" + Type + "-ext/data/clusterinfo-" +
                          StartPeriodDate.GetDtYmdStr() + "-new.json";
        PrintClustersInJson(FreqOverTime, Times, ClustersToPrint, ClustersRepQuote, OutputFilename);
      }
    }
  }

  return;
}
