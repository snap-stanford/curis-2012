#ifndef argumentparser_h
#define argumentparser_h

#include "stdafx.h"
#include "logoutput.h"



class ArgumentParser {
public:
  static THash<TStr, TStr> Arguments;
  static void ParseArguments(int argc, char *argv[], THash<TStr, TStr>& Arguments, LogOutput& Log);
  static TStr GetArgument(THash<TStr, TStr>& Arguments, TStr Key, TStr Default);
  static bool Contains(TStr Key);
};

#endif
