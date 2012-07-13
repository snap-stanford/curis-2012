#ifndef logoutput_h
#define logoutput_h

#include "stdafx.h"
#include "quote.h"

class LogOutput {
private:
  THash<TStr, TStr> OutputValues;
  TStr TimeStamp;
public:
  static const TStr OutputDirectory;
  static const TStr PercentEdgesDeleted;
  static const TStr NumOriginalEdges;
  static const TStr NumRemainingEdges;
  static const TStr NumQuotes;
  static const TStr WebDirectory;
  static const TStr NumClusters;
  void SetupFiles();
  void LogValue(const TStr Key, TStr Value);
  void LogValue(const TStr Key, TInt Value);
  void LogValue(const TStr Key, TFlt Value);
  void WriteClusteringOutputToFile();
  void OutputClusterInformation(TQuoteBase* QB, TVec<TTriple<TInt, TInt, TIntV> >& RepQuotesAndFreq);
};

#endif
