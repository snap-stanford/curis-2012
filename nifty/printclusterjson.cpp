#include "stdafx.h"
#include <stdio.h>

const int MaxNumQuotesToDisplay = 50;

int main(int argc, char *argv[]) {
  // Parse Arguments
  LogOutput Log;
  THash<TStr, TStr> Arguments;
  TStr BaseString;
  ArgumentParser::ParseArguments(argc, argv, Arguments, Log, BaseString);

  // TODO: Output cluster info for incremental clustering (into separate json files,
  //       one for each day)

  TStr StartString;
  if (!Arguments.IsKeyGetDat("start", StartString)) {
    StartString = "2012-07-07";
  }
  TStr EndString = "2012-07-08";

  /*TStr StartString, EndString, OutputDirectory;
  if (!Arguments.IsKeyGetDat("start", StartString)) {
    StartString = "2012-07-01";
  }
  if (!Arguments.IsKeyGetDat("end", EndString)) {
    EndString = "2012-07-08";
  }
  if (!Arguments.IsKeyGetDat("directory", OutputDirectory)) {
    Log.SetupNewOutputDirectory();
  } else {
    Log.SetDirectory(OutputDirectory);
  }*/

  TSecTm StartDate = TSecTm::GetDtTmFromYmdHmsStr(StartString);
  TSecTm EndDate = TSecTm::GetDtTmFromYmdHmsStr(EndString);

  TSecTm CurrentDate = StartDate;
  while(CurrentDate < EndDate) {
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
    PostCluster::GetTopFilteredClusters(&CB, &DB, &QB, Log, TopClusters, CurrentDate);

    int NumQuotesToDisplay = MaxNumQuotesToDisplay;
    if (TopClusters.Len() < MaxNumQuotesToDisplay) { NumQuotesToDisplay = TopClusters.Len(); }

    fprintf(stderr, "Preparing to write JSON to file\n");
    fprintf(stderr, "Including at most %d out of %d clusters\n", MaxNumQuotesToDisplay, TopClusters.Len());
    THash<TSecTm, TFltV> FreqOverTime;
    TVec<TSecTm> Times;
    for (int i = 0; i < NumQuotesToDisplay; i++) {
      TCluster C;
      CB.GetCluster(TopClusters[i], C);
      TIntV CQuoteIds;
      C.GetQuoteIds(CQuoteIds);
      TIntV CSources;
      TCluster::GetUniqueSources(CSources, CQuoteIds, &QB);
      TFreqTripleV CFreqV;
      Peaks::GetFrequencyVector(&DB, CSources, CFreqV, 24, 1, CurrentDate);

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
    fprintf(stderr, "Writing JSON to file\n");

    TStr OutputFilename = "../../../public_html/curis/output/clustering/visualization-test/data/clusterinfo-" +
                          StartDate.GetDtYmdStr() + "-fix.json";
    FILE *F = fopen(OutputFilename.CStr(), "w");
    fprintf(F, "{\"values\": [");
    TInt NumTimesToPrint = Times.Len() - 1;  // Skip the last time stamp, because it is all zeroes and will 
                                             // mess up the visualizer
    for (int i = 0; i < NumTimesToPrint; i++) {
      //if (Times[i].GetAbsSecs() % (Peaks::NumSecondsInDay) != 0) { continue; }
      fprintf(F, "{\"values\": [");
      TFltV Freqs = FreqOverTime.GetDat(Times[i]);
      for (int j = 0; j < Freqs.Len(); j++) {
        if (j < Freqs.Len() - 1) { fprintf(F, "%d, ", TFlt::Round(Freqs[j].Val)); }
        else { fprintf(F, "%d", TFlt::Round(Freqs[j].Val)); }
      }
      fprintf(F, "], \"label\": \"%s\"}", Times[i].GetYmdTmStr().CStr());
      if (i < NumTimesToPrint - 1) { fprintf(F, ","); }
    }
    fprintf(F, "], \"label\": [");
    for (int i = 0; i < NumQuotesToDisplay; i++) {
      fprintf(F, "\"%d\"", TopClusters[i].Val);
      if (i < NumQuotesToDisplay - 1) { fprintf(F, ", "); }
    }
    fprintf(F, "], \"quote\": [");
    for (int i = 0; i < NumQuotesToDisplay; i++) {
      TCluster C;
      CB.GetCluster(TopClusters[i], C);
      TStr CRepQuote;
      C.GetRepresentativeQuoteString(CRepQuote, &QB);
      fprintf(F, "\"%s\"", CRepQuote.CStr());
      if (i < NumQuotesToDisplay - 1) { fprintf(F, ", "); }
    }
    fprintf(F, "]}");

    fclose(F);
    CurrentDate.AddDays(1);
  }

  return 0;
}
