#ifndef logoutput_h
#define logoutput_h

#include "quote.h"
#include "cluster.h"

class LogOutput {
private:
  THash<TStr, TStr> OutputValues;
  // TODO: Make constructor with proper load
  TBool ShouldLog;
  FILE *QBDBCBSizeFile;
  TStr Directory;
public:
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
  void Save(TSOut& SOut) const;
  void Load(TSIn& SIn);
  void DisableLogging();
  void EnableLogging();
  void SetupNewOutputDirectory(TStr Directory);
  void SetDirectory(TStr &Directory);
  void GetDirectory(TStr& Directory);
  void LogValue(const TStr Key, TStr Value);
  void LogValue(const TStr Key, TInt Value);
  void LogValue(const TStr Key, TFlt Value);
  void LogAllInformation(TDocBase *DB, TQuoteBase *QB, TClusterBase *CB, TIntV& ClusterIds, TSecTm PresentTime, TIntV& OldTopClusters, TStr& QBDBCDirectory);
  void WriteClusteringStatisticsToFile(TSecTm& Date);
  void OutputClusterInformation(TDocBase *DB, TQuoteBase *QB, TClusterBase *CB, TIntV& ClusterIds, TSecTm PresentTime);
  void OutputClusterInformation(TDocBase *DB, TQuoteBase *QB, TClusterBase *CB, TIntV &ClusterIds, TSecTm PresentTime, TIntV &OldTopClusters);
  void PrintClusterInformation(TDocBase *DB, TQuoteBase *QB, TClusterBase *CB, TIntV& ClusterIds, TSecTm PresentTime, TIntV &OldTopClusters);
  void ComputeOldRankString(THash<TInt, TInt>& OldRankings, TInt& ClusterId, TInt CurRank, TStr& OldRankStr);
  void OutputDiscardedClusters(TQuoteBase *QB, TVec<TPair<TCluster, TInt> >& DiscardedClusters, TSecTm& Date);
  void OutputDiscardedClustersBySize(TQuoteBase *QB, TVec<TCluster>& DiscardedClusters, TSecTm& Date);
  void LogQBDBCBSize(TDocBase *DB, TQuoteBase *QB, TClusterBase *CB, TSecTm PresentTime);
};

#endif
