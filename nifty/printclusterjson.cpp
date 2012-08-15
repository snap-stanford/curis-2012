#include "stdafx.h"
#include <stdio.h>

const int MaxNumQuotesToDisplay = 20;
int TopClustersPerDay = 5;

// For testing
void PrintFreqOverTime(THash<TSecTm, TFltV>& FreqOverTime) {
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

// Adds the frequency data for CurrDate to the data structures
void PrepareForJsonPrinting(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB, THash<TSecTm, TFltV>& FreqOverTime, TVec<TSecTm>& Times, TIntV& ClustersToPrint, THash<TInt, TStr>& ClustersRepQuote, TSecTm& CurrentDate, TInt DaysPassed) {
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

  fprintf(stderr, "FreqOverTime hashtable: \n");
  PrintFreqOverTime(FreqOverTime);
}


void PrintClustersInJson(THash<TSecTm, TFltV>& FreqOverTime, TVec<TSecTm>& Times, TIntV& ClustersToPrint, THash<TInt, TStr>& ClustersRepQuote, TStr& OutputFilename) {
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

int main(int argc, char *argv[]) {
  // Parse Arguments
  LogOutput Log;
  THash<TStr, TStr> Arguments;
  TStr BaseString;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log, BaseString);

  TStr StartString;
  if (!Arguments.IsKeyGetDat("start", StartString)) {
    StartString = "2012-02-01";
    //StartString = "2012-06-01";
    //StartString = "2012-06-30";
  }
  TStr EndString = "2012-07-08";

  TStr Type;  // Can be day, week, or month
  if (!Arguments.IsKeyGetDat("type", Type)) {
    Type = TStr("day");
  }

  IAssert(Type == "day" || Type == "week" || Type == "month");
  fprintf(stderr, "Type: %s\n", Type.CStr());

  TInt NumDaysAhead = 1;
  if (Type == "week") {
    NumDaysAhead = 7;
  } else if (Type == "month") {
    // TODO: Make this an actual month, instead of 4 weeks
    NumDaysAhead = 28;
    TopClustersPerDay = 2;  // To make the number of quotes more manageable
  }

  TSecTm StartDate = TSecTm::GetDtTmFromYmdHmsStr(StartString);
  TSecTm EndDate = TSecTm::GetDtTmFromYmdHmsStr(EndString);
  TSecTm CurrentDate = StartDate;

  while(CurrentDate < EndDate) {
    THash<TSecTm, TFltV> FreqOverTime;
    TVec<TSecTm> Times;
    TIntV ClustersToPrint;
    THash<TInt, TStr> ClustersRepQuote;
    fprintf(stderr, "Preparing to write JSON to file\n");
    for (int i = 0; i < NumDaysAhead && CurrentDate < EndDate; ++i) {
      // Load Cumulative QBDBCB
      TQuoteBase QB;
      TDocBase DB;
      TClusterBase CB;
      PNGraph QGraph;
      fprintf(stderr, "Loading cumulative QBDBCB from %s from file...\n", CurrentDate.GetDtYmdStr().CStr());
      TDataLoader::LoadCumulative(QBDBC_DIRECTORY, CurrentDate.GetDtYmdStr(), QB, DB, CB, QGraph);
      fprintf(stderr, "Done loading cumulative QBDBCB!\n");

      // Get top filtered clusters
      TIntV TopClusters;
      //CB.GetTopClusterIdsByFreq(TopClusters);
      PostCluster::GetTopFilteredClusters(&CB, &DB, &QB, Log, TopClusters, CurrentDate, QGraph);

      if (Type == "day") {  // This will only run once, since NumDaysAhead = 1
        int NumQuotesToDisplay = MaxNumQuotesToDisplay;
        if (TopClusters.Len() < MaxNumQuotesToDisplay) { NumQuotesToDisplay = TopClusters.Len(); }
        fprintf(stderr, "Printing at most %d out of %d clusters\n", MaxNumQuotesToDisplay, TopClusters.Len());

        for (int j = 0; j < NumQuotesToDisplay; j++) {
          ClustersToPrint.Add(TopClusters[j]);
        }
      } else {
        for (int j = 0; j < TopClustersPerDay && j < TopClusters.Len(); j++) {
          if (ClustersToPrint.SearchForw(TopClusters[j]) < 0) {
            ClustersToPrint.Add(TopClusters[j]);
          }
        }
      }
      PrepareForJsonPrinting(&QB, &DB, &CB, FreqOverTime, Times, ClustersToPrint, ClustersRepQuote, CurrentDate, i + 1);
      CurrentDate.AddDays(1);
    }

    CurrentDate.AddDays(-1);
    TStr OutputFilename = "../../../public_html/curis/output/clustering/visualization-" + Type + "/data/clusterinfo-" +
                        CurrentDate.GetDtYmdStr() + ".json";
    PrintClustersInJson(FreqOverTime, Times, ClustersToPrint, ClustersRepQuote, OutputFilename);

    CurrentDate.AddDays(1);
  }

  return 0;
}
