#include "stdafx.h"
#include "printjson.h"

void TPrintJson::PrintJSON(TStr& FileName, THash<TStr, TStrV> JSON, TStr& Extra) {
  FILE *F = fopen(FileName.CStr(), "w");
  TStrV Keys;
  JSON.GetKeyV(Keys);
  fprintf(F, "{");
  if (Extra.Len() > 0) {
    fprintf(F, "%s", Extra.CStr());
    if(Keys.Len() > 0) fprintf(F, ", ");
  }
  for (int i = 0; i < Keys.Len(); i++) {
    TStrV Values = JSON.GetDat(Keys[i]);
    fprintf(F, "\"%s\": [", Keys[i].CStr());
    for (int j = 0; j < Values.Len(); j++) {
      fprintf(F, "\"%s\"", Values[j].CStr());
      if (j + 1 < Values.Len()) fprintf(F, ", ");
    }
    fprintf(F, "]");
    if (i + 1 < Keys.Len()) fprintf(F, ", ");
  }
  fprintf(F, "}");
  fclose(F);
}

void TPrintJson::PrintClusterTableJSON(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB,
                                       TStr& FileName, TIntV& Clusters, TStrV& RankStr) {
  TStrV Label, Quote, Frequency, NumVariants;
  int NumClusters = Clusters.Len();
  for (int i = 0; i < NumClusters; i++) {
    Label.Add(Clusters[i].GetStr());
    TCluster C;
    CB->GetCluster(Clusters[i], C);
    TStr CRepQuote;
    C.GetRepresentativeQuoteString(CRepQuote, QB);
    Quote.Add(JSONEscape(CRepQuote));

    TIntV CQuoteIds;
	TVec<TUInt> CUniqueSources;
    C.GetQuoteIds(CQuoteIds);
    TCluster::GetUniqueSources(CUniqueSources, CQuoteIds, QB);
    Frequency.Add(TInt(CUniqueSources.Len()).GetStr());
    NumVariants.Add(C.GetNumUniqueQuotes().GetStr());
  }

  THash<TStr, TStrV> JSON;
  JSON.AddDat("label", Label);
  if (RankStr.Len() > 0)
    JSON.AddDat("prev", RankStr);
  JSON.AddDat("frequency", Frequency);
  JSON.AddDat("numvariants", NumVariants);
  JSON.AddDat("quote", Quote);
  TStr Empty;
  PrintJSON(FileName, JSON, Empty);
}

void TPrintJson::PrintClusterJSON(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB, PNGraph& QGraph, 
                                  TStr& FolderName, TInt& ClusterId, TSecTm PresentTime) {
  //Err("getting peaks\n");
  TStr CurDateString = PresentTime.GetDtYmdStr();
  TCluster C;
  CB->GetCluster(ClusterId, C);
  TFreqTripleV PeakV, FreqV;
  C.GetPeaks(DB, QB, PeakV, FreqV, PEAK_BUCKET, PEAK_WINDOW, PresentTime);

  //Err("before peak\n");

  TStr Plots = "\"plot\": [";

  for (int i = 0; i < FreqV.Len(); i++) {
    Plots += "[" + FreqV[i].Val1.GetStr() + ", " + FreqV[i].Val2.GetStr() + "]";
    if (i + 1 < FreqV.Len()) Plots += ", ";
  }
  Plots += "], \"peak\": [";
  for (int i = 0; i < PeakV.Len(); i++) {
    Plots += "[" + PeakV[i].Val1.GetStr() + ", " + PeakV[i].Val2.GetStr() + "]";
    if (i + 1 < PeakV.Len()) Plots += ", ";
  }
  Plots += "], \"parents\":[";
  TIntV QuoteIds;
  C.GetQuoteIds(QuoteIds);
  for (int j = 0; j < QuoteIds.Len(); j++) {
    TQuote Q;
    if (QB->GetQuote(QuoteIds[j], Q)) {
	  Plots += "[";
	  if (QGraph->GetEdges() < 1) continue;
	  TNGraph::TNodeI NI = QGraph->GetNI(QuoteIds[j]);
	  for (int k = 0; k < NI.GetOutDeg(); k++) {
		Plots += TInt(NI.GetOutNId(k)).GetStr();
		if (k + 1 < NI.GetOutDeg()) Plots += ",";
	  }
	  Plots += "]";
	  if (j + 1 < QuoteIds.Len()) Plots += ",";
    }
  }
  Plots += "]";
  Plots += ", \"modified\": \"" + CurDateString + "\"";

  //Err("after peak\n");

  TStrV Quote, Urls, Frequencies, Quotes, Ids;
  TStr CRepQuote;
  C.GetRepresentativeQuoteString(CRepQuote, QB);
  Quote.Add(CRepQuote);

  
  for (int j = 0; j < QuoteIds.Len(); j++) {
	Ids.Add(QuoteIds[j].GetStr());
    TQuote Q;
    if (QB->GetQuote(QuoteIds[j], Q)) {
      TStr QuoteStr, QuoteURL;
      Q.GetContentString(QuoteStr);
      QB->GetRepresentativeUrl(DB, Q.GetId(), QuoteURL);

      Urls.Add(QuoteURL);
      Frequencies.Add(Q.GetNumSources().GetStr());
      Quotes.Add(JSONEscape(QuoteStr));
    }
  }

  //Err("after filling\n");

  THash<TStr, TStrV> JSON;
  JSON.AddDat("quote", Quote);
  JSON.AddDat("urls", Urls);
  JSON.AddDat("frequencies", Frequencies);
  JSON.AddDat("quotes", Quotes);
  JSON.AddDat("ids", Ids);

  TInt FirstIndex = ClusterId / 10000;
  TInt SecondIndex = FirstIndex / 1000;
  TStr NestedFolderName = FolderName + SecondIndex.GetStr() + "/" + FirstIndex.GetStr();
  TStr FileName = NestedFolderName + "/" + ClusterId.GetStr() + ".json";
  TStr Command = "mkdir -p " + NestedFolderName;
  system(Command.CStr());
  PrintJSON(FileName, JSON, Plots);
  //Err("after JSON\n");
}

/// If the parameter IncludeDate = true, the date is included in the output file's name
void TPrintJson::PrintClustersJson(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB,
                                   TIntV& ClustersToGraph, TIntV& ClustersToTable, const TStr& GraphDir, const TStr& TableDir, TSecTm& StartDate, TSecTm& EndDate, bool IncludeDate) {
  // Make the directories if they don't already exist
  TStr Command = "mkdir -p " + GraphDir;
  system(Command.CStr());
  Command = "mkdir -p " + TableDir;
  system(Command.CStr());
  PrintClustersGraphJson(QB, DB, CB, ClustersToGraph, GraphDir, StartDate, EndDate, IncludeDate);
  PrintClustersTableJson(QB, DB, CB, ClustersToTable, TableDir, StartDate, EndDate, IncludeDate);
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
                                        TIntV& ClustersToPrint, const TStr& Directory, TSecTm& StartDate, TSecTm& EndDate, bool IncludeDate) {
  fprintf(stderr, "Preparing to write JSON graph data to file\n");
  THash<TSecTm, TFltV> FreqOverTime;
  TVec<TSecTm> Times;
  for (int i = 0; i < ClustersToPrint.Len(); i++) {
    TCluster C;
    CB->GetCluster(ClustersToPrint[i], C);
    TIntV CQuoteIds;
    C.GetQuoteIds(CQuoteIds);
    TVec<TUInt> CSources;
    TCluster::GetUniqueSources(CSources, CQuoteIds, QB);
    TFreqTripleV CFreqV;
    uint StartDays = StartDate.GetInUnits(tmuDay);
    uint EndDays = EndDate.GetInUnits(tmuDay);
    Peaks::GetFrequencyVector(DB, CSources, CFreqV, 6, 3, EndDate, TInt(EndDays - StartDays + 1));

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

  TStr StartDateStr = StartDate.GetDtYmdStr();
  if (!IncludeDate) {
    StartDateStr = StartDateStr.Left(StartDateStr.Len() - 3);
  }

  TStr OutputFilename = Directory + "clusterinfo-" + StartDateStr + ".json";
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
    fprintf(F, "\"%s\"", JSONEscape(CRepQuote).CStr());
    if (i < ClustersToPrint.Len() - 1) { fprintf(F, ", "); }
  }
  fprintf(F, "]}");
  fclose(F);
}

TStr TPrintJson::JSONEscape(TStr& String) {
  TStr Response = "";
  for(int i = 0; i < String.Len(); i++) {
    switch (String[i]) {
      case '\\': Response += "\\\\";; break;
      case '"': Response += "\\\""; break;
      case '/': Response += "\\/"; break;
      case '\b': Response += "\\b"; break;
      case '\f': Response += "\\f"; break;
      case '\n': Response += "\\n"; break;
      case '\r': Response += "\\r"; break;
      case '\t': Response += "\\t"; break;
      default: Response += TStr(TCh(String[i])); break;
    }
  }
  return Response;
}

void TPrintJson::PrintClustersTableJson(TQuoteBase *QB, TDocBase *DB, TClusterBase *CB,
                                        TIntV& ClustersToPrint, const TStr& Directory, TSecTm& StartDate, TSecTm& EndDate, bool IncludeDate) {
  fprintf(stderr, "Writing JSON table data to file\n");

  TStr StartDateStr = StartDate.GetDtYmdStr();
  if (!IncludeDate) {
    StartDateStr = StartDateStr.Left(StartDateStr.Len() - 3);
  }

  TStr OutputFilename = Directory + "clustertable-" + StartDateStr + ".json";
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

    TIntV CQuoteIds;
	TVec<TUInt> CUniqueSources;
    C.GetQuoteIds(CQuoteIds);
    TCluster::GetUniqueSources(CUniqueSources, CQuoteIds, QB);
    Frequencies.Add(CUniqueSources.Len());

    NumVariants.Add(C.GetNumUniqueQuotes());
  }

  fprintf(F, "], \"quote\": [");
  for (int i = 0; i < ClustersToPrint.Len(); i++) {
    fprintf(F, "\"%s\"", JSONEscape(RepQuotes[i]).CStr());
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

    TIntV CQuoteIds;
	TVec<TUInt> CUniqueSources;
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
