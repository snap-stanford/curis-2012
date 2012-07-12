#ifndef logoutput_h
#define logoutput_h

#include "stdafx.h"

class LogOutput {
private:
  THash<TStr, TStr> OutputValues;
public:
  static const TStr OutputDirectory;
  static const TStr PercentEdgesDeleted;
  static const TStr NumOriginalEdges;
  static const TStr NumRemainingEdges;
  static const TStr NumQuotes;
  void LogValue(const TStr Key, TStr Value);
  void WriteOutputToFile();
};

#endif
