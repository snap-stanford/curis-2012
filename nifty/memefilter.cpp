#include "stdafx.h"
#include <stdio.h>

const int MinMemeFreq = 5;
const int MinQtWrdLen = 3;
const int MaxQtWrdLen = 30;
const double MinCommonEnglishRatio = 0.25;

THashSet<TMd5Sig> SeenUrlSet(Mega(100), true);
THashSet<TStr> URLBlackList;
THashSet<TStr> CommonEnglishWordsList;

void LoadCommonEnglishWords() {
  PSIn EnglishWords = TFIn::New("resources/common_english_words.txt");
  TStr Word;
  while (!EnglishWords->Eof() && EnglishWords->GetNextLn(Word)) {
    CommonEnglishWordsList.AddKey(Word);
  }
}

bool IsEnglish(const TChA &quote) {
  return quote.CountCh('?') <= quote.Len()/2;
}

/// Assumes lower case characters only format
bool IsRobustlyEnglish(TStr Quote) {
  TStrV Parsed;
  TQuote::ParseContentString(Quote, Parsed);
  TInt EnglishCount = 0;
  for (int i = 0; i < Parsed.Len(); ++i) {
    if (CommonEnglishWordsList.IsKey(Parsed[i])) {
      EnglishCount++;
    }
  }
  //printf("%f: %s\n", EnglishCount * 1.0 / Parsed.Len(), Quote.CStr());
  return EnglishCount * 1.0 / Parsed.Len() >= MinCommonEnglishRatio;
}

void LoadURLBlackList() {
  PSIn BlackListFile = TFIn::New("resources/URLBlacklist");
  TStr BadURL;
  while (!BlackListFile->Eof() && BlackListFile->GetNextLn(BadURL)) {
    URLBlackList.AddKey(BadURL);
  }
}

bool IsUrlInBlackList(const TChA &Url) {
  TStr UrlStr(Url);
  TStrV PeriodVector;
  UrlStr.SplitOnAllAnyCh(".", PeriodVector);

  if (PeriodVector.Len() >= 2) {
    TStrV SlashVector;
    PeriodVector[PeriodVector.Len() - 1].SplitOnAllAnyCh("/", SlashVector);
    if (SlashVector.Len() >= 1) {
      TStr DomainName = PeriodVector[PeriodVector.Len() - 2] + "." + SlashVector[0];
      if (URLBlackList.IsKey(DomainName)) {
        return true;
      }
    }
  }

  return false;
}


// Removes all punctuation in the quotes and replace with spaces.
// Also converts upper case to lower case.
// Adapted (but modified) from memes.h because I want a white list, not a blacklist.
void FilterSpacesAndSetLowercase(TStr &QtStr) {
  // Three passes...hopefully this isn't too slow.
  TChA QtChA(QtStr);
  for (int i = 0; i < QtChA.Len(); i++) {
    if (!(isalpha(QtChA[i]) || QtChA[i] == '\'')) {
      QtChA[i] = ' ';
    } else {
      QtChA[i] = tolower(QtChA[i]);
    }
  }
  QtStr = TStr(QtChA);
  TStrV WordV;
  QtStr.SplitOnAllAnyCh(" ", WordV);
  QtStr.Clr();
  for (int i = 0; i < WordV.Len(); ++i) {
    if (i > 0)  QtStr.InsStr(QtStr.Len()," ");
    QtStr.InsStr(QtStr.Len(), WordV[i]);
  }
}

bool IsDuplicateUrl(const TChA &Url) {
  TMd5Sig UrlSig = TMd5Sig(Url);
  if (SeenUrlSet.IsKey(UrlSig)) { return true; }
    SeenUrlSet.AddKey(UrlSig);
  return false;
}

bool IsPostTimeValid(TSecTm PostTm, TSecTm FileTm) {
  return (PostTm <= FileTm + 12 * 3600) && (PostTm >= FileTm - 12 * 3600);
}

void OutputQuoteInformation(TQuoteBase* QuoteBase, TStr FileName) {
  FILE *F = fopen(FileName.CStr(), "w");
  TFOut QuotesFile(FileName);
  TIntV QuoteIds;
  QuoteBase->GetAllQuoteIds(QuoteIds);
  for (int i = 0; i < QuoteIds.Len(); ++i) {
    TQuote Quote;
    bool IsInQB = QuoteBase->GetQuote(QuoteIds[i], Quote);
    if (IsInQB) {
      TStr ContentString;
      Quote.GetContentString(ContentString);
      fprintf(F, "%d\t%s\n", Quote.GetNumSources().Val, ContentString.CStr());
    }
  }
  fclose(F);
}

// usage filelist directory
int main(int argc, char *argv[]) {
  printf("File name must be in the form: web-{year}-{month}-{day}T{hour}-{minute}-{second}Z.rar\n");

  // Setup Output Directory
  TSecTm Tm = TSecTm::GetCurTm();
  TStr TimeStamp = TStr::Fmt("%04d-%02d-%02d",  Tm.GetYearN(), Tm.GetMonthN(), Tm.GetDayN());
  TimeStamp += "_" + Tm.GetTmStr();
  TStr Command = "mkdir -p /lfs/1/tmp/curis/output/filtering/" + TimeStamp;
  system(Command.CStr());
  TStr OutputDirectory = "/lfs/1/tmp/curis/output/filtering/" + TimeStamp + "/";

  // Initialize
  FILE *FLog = fopen((OutputDirectory + "filter.log").CStr(), "w");
  TStr InFileName = "resources/Spinn3rFileList.txt";
  TStr OutFileName = "QuoteFrequencies.txt";
  if (argc >= 2) {
    InFileName = TStr(argv[1]);
  }
  if (argc >= 3) {
    OutFileName = TStr(argv[2]);
  }
  LoadURLBlackList();
  LoadCommonEnglishWords();

  printf("Loading data from Spinn3r dataset to QuoteBase...\n");
  fprintf(FLog, "1: Initial Filtering:\n");
  TQuoteBase *TmpQB = new TQuoteBase;
  TDocBase *TmpDB = new TDocBase;
  FILE *FTime = fopen((OutputDirectory + "InvalidTimeUrl").CStr(), "w");
  int NSkipBlackList = 0, NSkipDuplicate = 0, NSkipInvalidTime = 0, NSkipNoValidQuote = 0;
  int NSkipNonEnglish = 0, NSkipTooShort = 0, NSkipTooLong = 0;
  THash<TStr, TInt> DuplicateUrl(Mega(100), true);
  // Read files and count the quotes
  TDataLoader Memes;
  Memes.LoadFileList(InFileName, "/lfs/1/tmp/curis/spinn3r/2012-01/");
  while (Memes.LoadNextFile()) {
    while (Memes.LoadNextEntry()) {
      if (IsUrlInBlackList(Memes.PostUrlStr)) { NSkipBlackList++;continue; }
      if (IsDuplicateUrl(Memes.PostUrlStr)) {
        TInt counter;
        if (DuplicateUrl.IsKeyGetDat(Memes.PostUrlStr, counter)) {
          counter++;
        } else {
          counter = 1;
        }
        DuplicateUrl.AddDat(Memes.PostUrlStr, counter);
        NSkipDuplicate++;
        continue;
      }
      if (!IsPostTimeValid(Memes.PubTm, Memes.GetCurrentFileTime())) { fprintf(FTime, "%s\n", Memes.PostUrlStr.CStr());NSkipInvalidTime++;continue; }
      bool ContainValidQuote = false;
      for (int m = 0; m < Memes.MemeV.Len(); m++) {
        if (IsEnglish(Memes.MemeV[m]) &&
            TStrUtil::CountWords(Memes.MemeV[m]) >= MinQtWrdLen &&
            TStrUtil::CountWords(Memes.MemeV[m]) <= MaxQtWrdLen) {
          FilterSpacesAndSetLowercase(Memes.MemeV[m]);
          ContainValidQuote = true;
        } else {
          if (!IsEnglish(Memes.MemeV[m])) {
            NSkipNonEnglish++;
          } else if (TStrUtil::CountWords(Memes.MemeV[m]) < MinQtWrdLen) {
            NSkipTooShort++;
          } else if (TStrUtil::CountWords(Memes.MemeV[m]) > MaxQtWrdLen) {
            NSkipTooLong++;
          }
          Memes.MemeV[m].Clr();
        }
      }
      if (ContainValidQuote) {
        TInt CurrentDocId = TmpDB->AddDoc(Memes.PostUrlStr, Memes.PubTm, Memes.ContentStr, Memes.LinkV);
        for (int m = 0; m < Memes.MemeV.Len(); m++) {
          if (!Memes.MemeV[m].Len() < 1) {
            TmpQB->AddQuote(Memes.MemeV[m], CurrentDocId);
          }
        }
      } else {
        NSkipNoValidQuote++;
      }
    }
  }
  fclose(FTime);
  FILE *FDup = fopen((OutputDirectory + "DupUrl").CStr(), "w");
  TVec<TPair<TStr, TInt> > DuplicateUrlV;
  DuplicateUrl.GetKeyDatPrV(DuplicateUrlV);
  fprintf(FDup, "%d\n", DuplicateUrlV.Len());
  for (int i = 0; i < DuplicateUrlV.Len(); i++) {
    fprintf(FDup, "%d\t%s\n", DuplicateUrlV[i].Val2.Val, DuplicateUrlV[i].Val1.CStr());
  }
  fclose(FDup);
  fprintf(FLog, "Number of quotes inserted into QuoteBase: %d\n", TmpQB->Len());
  fprintf(FLog, "Skipped %d quotes\n", NSkipNonEnglish + NSkipTooShort + NSkipTooLong);
  fprintf(FLog, "\tbecause quote is in not English:\t%d\n", NSkipNonEnglish);
  fprintf(FLog, "\tbecause quote is too short:\t\t%d\n", NSkipTooShort);
  fprintf(FLog, "\tbecause quote is too long:\t\t%d\n", NSkipTooLong);

  fprintf(FLog, "Number of documents inserted into DocBase: %d\n", TmpDB->Len());
  fprintf(FLog, "Skipped %d documents\n", NSkipBlackList + NSkipDuplicate + NSkipInvalidTime + NSkipNoValidQuote);
  fprintf(FLog, "\tbecause URL is in blacklist:\t\t%d\n", NSkipBlackList);
  fprintf(FLog, "\tbecause URL is duplicated:\t\t%d\n", NSkipDuplicate);
  fprintf(FLog, "\tbecause URL has invalid time:\t\t%d\n", NSkipInvalidTime);
  fprintf(FLog, "\tbecause URL has no valid quotes:\t%d\n", NSkipNoValidQuote);


  fprintf(FLog, "\n2: Second Filtering:\n");
  TQuoteBase *QB = new TQuoteBase;
  TDocBase *DB = new TDocBase;
  int NSkipRobustEnglish = 0, NSkipInfrequent = 0, NSkipBadDomainRatio = 0;
  TIntV QuoteIds;
  TmpQB->GetAllQuoteIds(QuoteIds);
  for (int i = 0; i < QuoteIds.Len(); i++) {
    TQuote Q;
    TmpQB->GetQuote(QuoteIds[i], Q);
    TStr QContentString;
    Q.GetContentString(QContentString);
    if (Q.GetNumSources() >= MinMemeFreq &&
       (Q.GetNumSources() <= 20 || Q.GetNumSources() <= 4 * Q.GetNumDomains(TmpDB)) &&
       Q.GetNumDomains(TmpDB) >= 2 &&
       IsRobustlyEnglish(QContentString)) {
      TIntV Sources;
      Q.GetSources(Sources);
      for (int j = 0; j < Sources.Len(); j++) {
        TDoc D;
        TmpDB->GetDoc(Sources[j], D);
        TInt NewSourceId = DB->AddDoc(D);
        QB->AddQuote(QContentString, NewSourceId);
      }
    } else {
      if (Q.GetNumSources() < MinMemeFreq) {
        NSkipInfrequent++;
      } else if (Q.GetNumSources() > 20 && Q.GetNumSources() > 4 * Q.GetNumDomains(TmpDB)) {
        NSkipBadDomainRatio++;
      } else if (Q.GetNumDomains(TmpDB) < 2) {
        NSkipBadDomainRatio++;
      } else {
        NSkipRobustEnglish++;
      }
    }
    if (i % 50000 == 49999) {
      printf("%d out of %d quotes processed\n", i + 1, TmpQB->Len());
    }
  }

  fprintf(FLog, "Number of quotes: %d\n", QB->Len());
  fprintf(FLog, "Skipped %d quotes\n", NSkipInfrequent, NSkipBadDomainRatio, NSkipRobustEnglish);
  fprintf(FLog, "\tbecause quote is infrequent:\t\t\t%d\n", NSkipInfrequent);
  fprintf(FLog, "\tbecause quote is mainly from the same domains:\t%d\n", NSkipBadDomainRatio);
  fprintf(FLog, "\tbecause quote is not English:\t\t\t%d\n", NSkipRobustEnglish);
  fprintf(FLog, "Number of documents: %d\n", DB->Len());
  printf("\nLOADING DATA TO QUOTE BASE DONE!\n");
  printf("Writing quote frequencies...\n");
  OutputQuoteInformation(QB, OutputDirectory + OutFileName);
  printf("Done!\n");
  printf("Writing QuoteBase and DocBase\n");
  TFOut FOut(OutputDirectory + "QBDB.bin");
  QB->Save(FOut);
  DB->Save(FOut);
  printf("Done!\n");
  delete QB;
  delete DB;
  delete TmpQB;
  delete TmpDB;
  fclose(FLog);
  return 0;
}
