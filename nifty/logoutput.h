#ifndef logoutput_h
#define logoutput_h

#include <sys/types.h>
#include <sys/stat.h>

class LogOutput {
private:
  TStr OutputDirectory = "/lfs/1/tmp/curis/output/";
  THash<TStr, TStr> OutputValues;
public:
  const TStr PercentEdgesDeleted = "Percent of Edges Deleted";
  const TStr NumOriginalEdges = "Number of Edges Prior to Deletion";
  const TStr NumRemainingEdges = "Number of Remaining Edges";
  void LogOutput(TStr Key, TStr Value);
  void WriteOutputToFile();
};

#endif
