#ifndef argumentparser_h
#define argumentparser_h

#include "stdafx.h"
#include "logoutput.h"

class ArgumentParser {
public:
  static void ParseArguments(int argc, char *argv[], THash<TStr, TStr>& Arguments, LogOutput& Log, TStr& BaseString, bool& DoIncrementalClustering);
};

#endif