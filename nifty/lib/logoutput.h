#ifndef logoutput_h
#define logoutput_h

#include "stdafx.h"
#include "quote.h"

class LogOutput {
private:
  THash<TStr, TStr> OutputValues;
  TStr TimeStamp;
  bool ShouldLog;
public:
  static const TStr OutputDirectory;
  static const TStr PercentEdgesDeleted;
  static const TStr PercentEdgesDeletedNotFromSubgraphs;
  static const TStr PercentEdgesDeletedNFSBaseline;
  static const TStr NumOriginalEdges;
  static const TStr NumRemainingEdges;
  static const TStr NumQuotes;
  static const TStr WebDirectory;
  static const TStr NumClusters;
  static const TInt FrequencyCutoff;
  static const TInt PeakThreshold;
  LogOutput();
  void DisableLogging();
  void SetupFiles();
  void LogValue(const TStr Key, TStr Value);
  void LogValue(const TStr Key, TInt Value);
  void LogValue(const TStr Key, TFlt Value);
  void WriteClusteringOutputToFile();
  void OutputClusterInformation(TDocBase *DB, TQuoteBase* QB, TVec<TCluster>& ClusterSummaries);
};

#endif