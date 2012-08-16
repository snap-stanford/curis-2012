#include "stdafx.h"
#include <omp.h>
#include <stdio.h>

const int MinMemeFreq = 5;
const int MinQtWrdLen = 3;
const int MaxQtWrdLen = 30;

THashSet<TStr> URLBlackList;

void LoadURLBlackList() {
  PSIn BlackListFile = TFIn::New("resources/URLBlacklist");
  TStr BadURL;
  while (!BlackListFile->Eof() && BlackListFile->GetNextLn(BadURL)) {
    URLBlackList.AddKey(BadURL);
  }
}

void LoadDateList(const TStr& InFileName, TStrV &DateList) {
  PSIn InFileNameF = TFIn::New(InFileName);
  TStr Date;
  while (!InFileNameF->Eof() && InFileNameF->GetNextLn(Date))
    DateList.Add(Date);
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

bool IsPostTimeValid(TSecTm PostTm, TSecTm FileTm) {
  return (PostTm <= FileTm + 12 * 3600) && (PostTm >= FileTm - 12 * 3600);
}

void OutputQuoteInformation(TQuoteBase &QuoteBase, TStr FileName) {
  FILE *F = fopen(FileName.CStr(), "w");
  TFOut QuotesFile(FileName);
  TIntV QuoteIds;
  QuoteBase.GetAllQuoteIds(QuoteIds);
  for (int i = 0; i < QuoteIds.Len(); ++i) {
    TQuote Quote;
    bool IsInQB = QuoteBase.GetQuote(QuoteIds[i], Quote);
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
  TStr OutputDirectory = "/lfs/1/tmp/curis/output/filtering/";

  // Initialize
  LoadURLBlackList();

  if (argc != 2) {
    printf("Please specify date in the format yyyy-mm-dd\n");
    return 0;
  }
  TStr Date = argv[1];
  TStr MonDir = Date.GetSubStr(0, 6);
  TSecTm CurrentDate = TSecTm::GetDtTmFromYmdHmsStr(Date + " 08:00:00");

  printf("Loading data from Spinn3r dataset to QuoteBase...\n");

  TQuoteBase TmpQB;
  TDocBase TmpDB;

  int NSkipBlackList = 0, NSkipDuplicate = 0, NSkipInvalidTime = 0, NSkipNoValidQuote = 0;
  int NSkipNonEnglish = 0, NSkipTooShort = 0, NSkipTooLong = 0;

  THashSet<TMd5Sig> SeenUrlSet(Mega(100), true);

  FILE *FLog = fopen((OutputDirectory + "FILTER" + Date + ".log").CStr(), "w");
  fprintf(FLog, "Initial Filtering:\n");
  TDataLoader Memes;
  for (int j = 0; j < 24; j++) {
    TStr CurFile = "web-" + CurrentDate.GetDtYmdStr() + TStr::Fmt("T%02d-00-00Z.rar", CurrentDate.GetHourN());
    if(!Memes.LoadFile("/lfs/1/tmp/curis/spinn3r/" + MonDir + "/", CurFile)) {continue;}
    while (Memes.LoadNextEntry()) {
      if (IsUrlInBlackList(Memes.PostUrlStr)) { NSkipBlackList++;continue; }
      TMd5Sig UrlSig = TMd5Sig(Memes.PostUrlStr);
      if (SeenUrlSet.IsKey(UrlSig)) { NSkipDuplicate++;continue; }
      SeenUrlSet.AddKey(UrlSig);
      if (!IsPostTimeValid(Memes.PubTm, CurrentDate)) { NSkipInvalidTime++;continue; }
      bool ContainValidQuote = false;
      for (int m = 0; m < Memes.MemeV.Len(); m++) {
        // Change Memes.MemeV[m] to a space separated sequence of words, so CountWords works correctly
        TStringUtil::RemoveEndPunctuations(Memes.MemeV[m]);
        Memes.MemeV[m] = TStrUtil::GetCleanStr(Memes.MemeV[m]);
        if (TStringUtil::IsEnglish(Memes.MemeV[m]) &&
            TStrUtil::CountWords(Memes.MemeV[m]) >= MinQtWrdLen &&
            TStrUtil::CountWords(Memes.MemeV[m]) <= MaxQtWrdLen) {
          ContainValidQuote = true;
        } else {
          if (!TStringUtil::IsEnglish(Memes.MemeV[m])) {
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
        TInt CurrentDocId = TmpDB.AddDoc(Memes.PostUrlStr, Memes.PubTm, Memes.ContentStr, Memes.LinkV);
        for (int m = 0; m < Memes.MemeV.Len(); m++) {
          if (!Memes.MemeV[m].Len() < 1) {
            TmpQB.AddQuote(Memes.MemeV[m], CurrentDocId);
          }
        }
      } else {
        NSkipNoValidQuote++;
      }
    }
    CurrentDate.AddHours(1);
  }

  fprintf(FLog, "Number of quotes inserted into QuoteBase: %d\n", TmpQB.Len());
  fprintf(FLog, "Skipped %d quotes\n", NSkipNonEnglish + NSkipTooShort + NSkipTooLong);
  fprintf(FLog, "\tbecause quote is in not English:\t%d\n", NSkipNonEnglish);
  fprintf(FLog, "\tbecause quote is too short:\t\t%d\n", NSkipTooShort);
  fprintf(FLog, "\tbecause quote is too long:\t\t%d\n", NSkipTooLong);

  fprintf(FLog, "Number of documents inserted into DocBase: %d\n", TmpDB.Len());
  fprintf(FLog, "Skipped %d documents\n", NSkipBlackList + NSkipDuplicate + NSkipInvalidTime + NSkipNoValidQuote);
  fprintf(FLog, "\tbecause URL is in blacklist:\t\t%d\n", NSkipBlackList);
  fprintf(FLog, "\tbecause URL is duplicated:\t\t%d\n", NSkipDuplicate);
  fprintf(FLog, "\tbecause URL has invalid time:\t\t%d\n", NSkipInvalidTime);
  fprintf(FLog, "\tbecause URL has no valid quotes:\t%d\n", NSkipNoValidQuote);

  fprintf(FLog, "\nSecond Filtering:\n");
  TQuoteBase QB;
  TDocBase DB;
  int NSkipInfrequent = 0, NSkipBadDomainRatio = 0, NSkipRobustEnglish = 0;
  TIntV QuoteIds;
  TmpQB.GetAllQuoteIds(QuoteIds);
  for (int j = 0; j < QuoteIds.Len(); j++) {
    TQuote Q;
    TmpQB.GetQuote(QuoteIds[j], Q);
    TStr QContentString;
    Q.GetContentString(QContentString);
    if (Q.GetNumSources() >= MinMemeFreq &&
       (Q.GetNumSources() <= 20 || (Q.GetNumSources() <= 6 * Q.GetNumDomains(TmpDB) && Q.GetNumDomains(TmpDB) >= 2)) &&
       TStringUtil::IsRobustlyEnglish(QContentString)) {
      TIntV Sources;
      Q.GetSources(Sources);
      for (int k = 0; k < Sources.Len(); k++) {
        TDoc D;
        TmpDB.GetDoc(Sources[k], D);
        TInt NewSourceId = DB.AddDoc(D);
        QB.AddQuote(QContentString, NewSourceId);
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
  }

  QB.RemoveQuoteDuplicateSources();

  fprintf(FLog, "Number of quotes: %d\n", QB.Len());
  fprintf(FLog, "Skipped %d quotes\n", NSkipInfrequent + NSkipBadDomainRatio + NSkipRobustEnglish);
  fprintf(FLog, "\tbecause quote is infrequent:\t\t\t%d\n", NSkipInfrequent);
  fprintf(FLog, "\tbecause quote is mainly from the same domains:\t%d\n", NSkipBadDomainRatio);
  fprintf(FLog, "\tbecause quote is not English:\t\t\t%d\n", NSkipRobustEnglish);
  fprintf(FLog, "Number of documents: %d\n", DB.Len());
  printf("\nLOADING DATA TO QUOTE BASE DONE!\n");
  printf("Writing quote frequencies...\n");
  OutputQuoteInformation(QB, OutputDirectory + "QFREQ" + Date + ".txt");
  printf("Done!\n");
  printf("Writing QuoteBase and DocBase\n");
  TFOut FOut(OutputDirectory + "QBDB" + Date + ".bin");
  QB.Save(FOut);
  DB.Save(FOut);
  printf("Done!\n");
  fclose(FLog);
  return 0;
}
