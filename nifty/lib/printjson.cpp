#include "stdafx.h"
#include "printjson.h"

void TPrintJson::PrintClustersJson(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB,
                                   TIntV& ClustersToGraph, TIntV& ClustersToTable, const TStr& GraphDir, const TStr& TableDir, TSecTm& StartDate, TSecTm& EndDate) {
  PrintClustersGraphJson(QB, DB, CB, ClustersToGraph, GraphDir, StartDate, EndDate);
  PrintClustersTableJson(QB, DB, CB, ClustersToTable, TableDir, StartDate, EndDate);
}

/// Returns in ClustersToPrint the top NumPerDay clusters per day, ordered by frequency.
//  To be considered, the cluster must peak in that day and be one of the highest peaks in the day.
void TPrintJson::GetTopPeakClustersPerDay(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB,
                              TIntV& ClustersToPrint, TInt NumPerDay, TSecTm& StartDate, TSecTm& EndDate) {
  TIntSet TopClusters;
  TIntV ClusterIdsByFreq;
  CB->GetAllClusterIdsSortByFreq(ClusterIdsByFreq);
  for (int i = 0; i < 50 && i < ClusterIdsByFreq.Len(); i++) {
    ClustersToPrint.Add(ClusterIdsByFreq[i]);
  }

}

void TPrintJson::PrintClustersGraphJson(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB,
                                        TIntV& ClustersToPrint, const TStr& Directory, TSecTm& StartDate, TSecTm& EndDate) {
  fprintf(stderr, "Preparing to write JSON graph data to file\n");
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
    uint StartDays = StartDate.GetInUnits(tmuDay);
    uint EndDays = EndDate.GetInUnits(tmuDay);
    Peaks::GetFrequencyVector(DB, CSources, CFreqV, 12, 3, EndDate, TInt(EndDays - StartDays + 1));

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
  fprintf(stderr, "Writing JSON graph data to file\n");

  TStr OutputFilename = Directory + "clusterinfo-" + StartDate.GetDtYmdStr() + ".json";
  FILE *F = fopen(OutputFilename.CStr(), "w");
  fprintf(F, "{\"values\": [");
  TInt NumTimesToPrint = Times.Len() - 1;  // Skip the last time stamp, because it is all zeroes and will 
                                             // mess up the visualizer
  for (int i = 0; i < NumTimesToPrint; i++) {
    if (Times[i] < StartDate) { continue; }
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
  for (int i = 0; i < ClustersToPrint.Len(); i++) {
    fprintf(F, "\"%d\"", ClustersToPrint[i].Val);
    if (i < ClustersToPrint.Len() - 1) { fprintf(F, ", "); }
  }
  fprintf(F, "], \"quote\": [");
  for (int i = 0; i < ClustersToPrint.Len(); i++) {
    TCluster C;
    CB->GetCluster(ClustersToPrint[i], C);
    TStr CRepQuote;
    C.GetRepresentativeQuoteString(CRepQuote, QB);
    fprintf(F, "\"%s\"", CRepQuote.CStr());
    if (i < ClustersToPrint.Len() - 1) { fprintf(F, ", "); }
  }
  fprintf(F, "]}");
  fclose(F);
}

void TPrintJson::PrintClustersTableJson(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB,
                                        TIntV& ClustersToPrint, const TStr& Directory, TSecTm& StartDate, TSecTm& EndDate) {
  fprintf(stderr, "Writing JSON table data to file\n");

  TStr OutputFilename = Directory + "clustertable-" + StartDate.GetDtYmdStr() + ".json";
  FILE *F = fopen(OutputFilename.CStr(), "w");
  fprintf(F, "{\"label\": [");
  for (int i = 0; i < ClustersToPrint.Len(); i++) {
    fprintf(F, "\"%d\"", ClustersToPrint[i].Val);
    if (i < ClustersToPrint.Len() - 1) { fprintf(F, ", "); }
  }

  TIntV Frequencies, NumVariants;
  TStrV RepQuotes;

  for (int i = 0; i < ClustersToPrint.Len(); i++) {
    TCluster C;
    CB->GetCluster(ClustersToPrint[i], C);
    TStr CRepQuote;
    C.GetRepresentativeQuoteString(CRepQuote, QB);
    RepQuotes.Add(CRepQuote);

    TIntV CQuoteIds, CUniqueSources;
    C.GetQuoteIds(CQuoteIds);
    TCluster::GetUniqueSources(CUniqueSources, CQuoteIds, QB);
    Frequencies.Add(CUniqueSources.Len());

    NumVariants.Add(C.GetNumUniqueQuotes());
  }

  fprintf(F, "], \"quote\": [");
  for (int i = 0; i < ClustersToPrint.Len(); i++) {
    fprintf(F, "\"%s\"", RepQuotes[i].CStr());
    if (i < ClustersToPrint.Len() - 1) { fprintf(F, ", "); }
  }

  fprintf(F, "], \"frequency\": [");
  for (int i = 0; i < ClustersToPrint.Len(); i++) {
    fprintf(F, "\"%d\"", Frequencies[i].Val);
    if (i < ClustersToPrint.Len() - 1) { fprintf(F, ", "); }
  }

  fprintf(F, "], \"numvariants\": [");
  for (int i = 0; i < ClustersToPrint.Len(); i++) {
    fprintf(F, "\"%d\"", NumVariants[i].Val);
    if (i < ClustersToPrint.Len() - 1) { fprintf(F, ", "); }
  }

  fprintf(F, "]}");
  fclose(F);

}

void TPrintJson::PrintClustersDataJson(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB, TIntV& ClustersToPrint, const TStr& ClusterDataDir, TSecTm& EndDate) {

  for (int i = 0; i < ClustersToPrint.Len(); ++i) {
    TCluster C;
    CB->GetCluster(ClustersToPrint[i], C);
    TStr CRepQuote;
    C.GetRepresentativeQuoteString(CRepQuote, QB);

    TIntV CQuoteIds, CUniqueSources;
    C.GetQuoteIds(CQuoteIds);
    TCluster::GetUniqueSources(CUniqueSources, CQuoteIds, QB);

    TInt Index = ClustersToPrint[i] / 10000;
    TStr Command = "mkdir -p " + ClusterDataDir + Index.GetStr();
    system(Command.CStr());

    TStr COutputFilename = ClusterDataDir + Index.GetStr() + "/" + ClustersToPrint[i].GetStr() + ".json";
    FILE *FC = fopen(COutputFilename.CStr(), "w");
    fprintf(FC, "{\"values\": [");

    TFreqTripleV CFreqV;
    Peaks::GetFrequencyVector(DB, CUniqueSources, CFreqV, 12, 3, TSecTm(0));

    TInt NumTimesToPrint = CFreqV.Len() - 1;  // Skip the last time stamp, because it is all zeroes and will 
                                              // mess up the visualizer
    for (int j = 0; j < NumTimesToPrint; j++) {
      fprintf(FC, "{\"values\": [%d", TFlt::Round(CFreqV[j].Val2));
      fprintf(FC, "], \"label\": \"%s\"}", CFreqV[j].Val3.GetYmdTmStr().CStr());
      if (j < NumTimesToPrint - 1) { fprintf(FC, ", "); }
    }

    fprintf(FC, "], \"quote\": [\"%s\"", CRepQuote.CStr());
    TIntV CFrequencies;
    TStrV CVariants, CVariantUrls;
    for (int j = 0; j < CQuoteIds.Len(); j++) {
      TQuote Q;
      QB->GetQuote(CQuoteIds[j], Q);
      CFrequencies.Add(Q.GetNumSources());

      TStr QContent;
      Q.GetContentString(QContent);
      CVariants.Add(QContent);

      TStr QRepUrl;
      Q.GetRepresentativeUrl(DB, QRepUrl);
      CVariantUrls.Add(QRepUrl);
    }

    fprintf(FC, "], \"frequency\": [");
    for (int j = 0; j < CQuoteIds.Len(); j++) {
      fprintf(FC, "\"%d\"", CFrequencies[j].Val);
      if (j < CQuoteIds.Len() - 1) { fprintf(FC, ", "); }
    }

    fprintf(FC, "], \"variants\": [");
    for (int j = 0; j < CQuoteIds.Len(); j++) {
      fprintf(FC, "\"%s\"", CVariants[j].CStr());
      if (j < CQuoteIds.Len() - 1) { fprintf(FC, ", "); }
    }

    fprintf(FC, "], \"varianturls\": [");
    for (int j = 0; j < CQuoteIds.Len(); j++) {
      fprintf(FC, "\"%s\"", CVariantUrls[j].CStr());
      if (j < CQuoteIds.Len() - 1) { fprintf(FC, ", "); }
    }

    fprintf(FC, "]}");
    fclose(FC);
  }
}
