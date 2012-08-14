#include "stdafx.h"
#include <stdio.h>

const int MaxNumQuotesToDisplay = 20;
const int TopClustersPerDay = 5;

void PrintClustersInJson(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB, TStr& ClustersFreq, TStr& ClustersLabel, TStr& ClustersRepQuote, TSecTm& CurrentDate, TIntV& ClustersToPrint) {
  THash<TSecTm, TFltV> FreqOverTime;
  TVec<TSecTm> Times;
  for (int i = 0; i < ClustersToPrint.Len(); i++) {
    TCluster C;
    CB->GetCluster(ClustersToPrint[i], C);
    TIntV CQuoteIds;
    C.GetQuoteIds(CQuoteIds);
    TIntV CSources;
    TCluster::GetUniqueSources(CSources, CQuoteIds, QB);
    TFreqTripleV CFreqV;
    Peaks::GetFrequencyVector(DB, CSources, CFreqV, 12, 1, CurrentDate, 7);

    if (FreqOverTime.Len() == 0) {  // Initialize the values in the hash table
      for (int j = 0; j < CFreqV.Len(); j++) {
        FreqOverTime.AddDat(CFreqV[j].Val3, TFltV());
        Times.Add(CFreqV[j].Val3);
      }
    }
    for (int j = 0; j < CFreqV.Len(); j++) {
      if (!FreqOverTime.IsKey(CFreqV[j].Val3)) {
        fprintf(stderr, "%s \n", CFreqV[j].Val3.GetYmdTmStr().CStr());
        fprintf(stderr, "\t%f\n", CFreqV[j].Val2.Val);
        continue;
      }
      TFltV Freqs = FreqOverTime.GetDat(CFreqV[j].Val3);
      Freqs.Add(CFreqV[j].Val2);
      FreqOverTime.AddDat(CFreqV[j].Val3, Freqs);
    }
  }

  TInt NumTimesToPrint = Times.Len() - 1;  // Skip the last time stamp, because it is all zeroes and will 
                                           // mess up the visualizer
  for (int i = 0; i < NumTimesToPrint; i++) {
    //if (Times[i].GetAbsSecs() % (Peaks::NumSecondsInDay) != 0) { continue; }
    ClustersFreq += "{\"values\": [";
    TFltV Freqs = FreqOverTime.GetDat(Times[i]);
    for (int j = 0; j < Freqs.Len(); j++) {
      ClustersFreq += TInt::GetStr(TFlt::Round(Freqs[j].Val));
      if (j < Freqs.Len() - 1) { ClustersFreq += ", "; }
    }
    ClustersFreq += "], \"label\": \"" + Times[i].GetYmdTmStr() + "\"}, ";
  }

  for (int i = 0; i < ClustersToPrint.Len(); i++) {
    ClustersLabel += "\"" + TInt::GetStr(ClustersToPrint[i]) + "\"";
    ClustersLabel += ", ";
  }

  for (int i = 0; i < ClustersToPrint.Len(); i++) {
    TCluster C;
    CB->GetCluster(ClustersToPrint[i], C);
    TStr CRepQuote;
    C.GetRepresentativeQuoteString(CRepQuote, QB);
    ClustersRepQuote += "\"" + CRepQuote + "\", ";
  }
}

int main(int argc, char *argv[]) {
  // Parse Arguments
  LogOutput Log;
  THash<TStr, TStr> Arguments;
  TStr BaseString;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log, BaseString);

  TStr StartString;
  if (!Arguments.IsKeyGetDat("start", StartString)) {
    StartString = "2012-06-01";
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
  }

  TSecTm StartDate = TSecTm::GetDtTmFromYmdHmsStr(StartString);
  TSecTm EndDate = TSecTm::GetDtTmFromYmdHmsStr(EndString);

  TSecTm CurrentDate = StartDate;
  while(CurrentDate < EndDate) {
    TStr ClustersFreq, ClustersLabel, ClustersRepQuote;
    for (int i = 0; i < NumDaysAhead && CurrentDate < EndDate; ++i) {
      // Load Cumulative QBDBCB
      TQuoteBase QB;
      TDocBase DB;
      TClusterBase CB;
      PNGraph QGraph;
      fprintf(stderr, "Loading cumulative QBDBCB from file...\n");
      TDataLoader::LoadCumulative(QBDBC_DIRECTORY, CurrentDate.GetDtYmdStr(), QB, DB, CB, QGraph);
      fprintf(stderr, "Done loading cumulative QBDBCB!\n");

      // Get top filtered clusters
      TIntV TopClusters;
      //CB.GetTopClusterIdsByFreq(TopClusters);
      PostCluster::GetTopFilteredClusters(&CB, &DB, &QB, Log, TopClusters, CurrentDate, QGraph);
    
      TIntV ClustersToPrint;
      if (Type == "day") {  // This will only run once, since NumDaysAhead = 1
        int NumQuotesToDisplay = MaxNumQuotesToDisplay;
        if (TopClusters.Len() < MaxNumQuotesToDisplay) { NumQuotesToDisplay = TopClusters.Len(); }

        fprintf(stderr, "Preparing to write JSON to file\n");
        fprintf(stderr, "Including at most %d out of %d clusters\n", MaxNumQuotesToDisplay, TopClusters.Len());

        for (int j = 0; j < NumQuotesToDisplay; j++) {
          ClustersToPrint.Add(TopClusters[j]);
        }
        PrintClustersInJson(&QB, &DB, &CB, ClustersFreq, ClustersLabel, ClustersRepQuote, CurrentDate, ClustersToPrint);
      } else {
        for (int j = 0; j < TopClustersPerDay; j++) {
          ClustersToPrint.Add(TopClusters[j]);
        }
        PrintClustersInJson(&QB, &DB, &CB, ClustersFreq, ClustersLabel, ClustersRepQuote, CurrentDate, ClustersToPrint);
      }
      CurrentDate.AddDays(1);
    }

    CurrentDate.AddDays(-1);
    TStr OutputFilename = "../../../public_html/curis/output/clustering/visualization-" + Type + "/data/clusterinfo-" +
                        CurrentDate.GetDtYmdStr() + ".json";
    fprintf(stderr, "Writing JSON to file: %s\n", OutputFilename.CStr());
    FILE *F = fopen(OutputFilename.CStr(), "w");
    TStr ClustersFreqToPrint = ClustersFreq.GetSubStr(0, ClustersFreq.Len() - 3);  // Remove the ending ", "
    fprintf(F, "{\"values\": [%s", ClustersFreqToPrint.CStr());
    TStr ClustersLabelToPrint = ClustersLabel.GetSubStr(0, ClustersLabel.Len() - 3);
    fprintf(F, "], \"label\": [%s", ClustersLabelToPrint.CStr());
    TStr ClustersRepQuoteToPrint = ClustersRepQuote.GetSubStr(0, ClustersRepQuote.Len() - 3);
    fprintf(F, "], \"quote\": [%s", ClustersRepQuoteToPrint.CStr());
    fprintf(F, "]}");
    fclose(F);

    CurrentDate.AddDays(1);
  }

  return 0;
}
