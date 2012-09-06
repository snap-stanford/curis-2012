#ifndef argumentparser_h
#define argumentparser_h

#include "stdafx.h"
#include "logoutput.h"

class ArgumentParser {
public:
  static void ParseArguments(int argc, char *argv[], THash<TStr, TStr>& Arguments, LogOutput& Log, TStr& BaseString);
  static TStr GetArgument(THash<TStr, TStr>& Arguments, TStr Key, TStr Default);
};

#endif
