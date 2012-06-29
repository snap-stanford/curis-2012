#include "stdafx.h"
#include "memes.h"

void BigMain(int argc, char* argv[]);

int main(int argc, char* argv[]) {
  printf("MemeWorks. build: %s, %s. Start time: %s\n\n", __TIME__, __DATE__, TExeTm::GetCurTm());

  TExeTm ExeTm;  TInt::Rnd.PutSeed(0);  Try
  TSecTm BegTm = TSecTm::GetCurTm();

//	char *ToDo [] = {"memeclust", "-do:memestoqtbs", "-i:201101.txt", "-w:F", "-o:1101", "-mint:20110101", "-maxt:20110106"};  BigMain(7, ToDo);   
//  char *ToDo [] = {"memeclust", "-do:memestoqtbs", "-i:201101_201103.txt", "-w:F", "-o:11011103"};  BigMain(5, ToDo);   
//  char *ToDo [] = {"memeclust", "-do:memestoqtbs", "-i:201104_201106.txt", "-w:F", "-o:11041106", "-mint:20110401", "-maxt:20110701"};  BigMain(7, ToDo);   
//  char *ToDo [] = {"memeclust", "-do:memestoqtbs", "-i:201007_201107.txt", "-w:F", "-o:10071107"};  BigMain(5, ToDo);
//  char *ToDo [] = {"memeclust", "-do:memestoqtbs", "-i:201007_201107.txt", "-o:10071107", "-mint:20100714", "-maxt:20110728"};  BigMain(6, ToDo); 
//	char *ToDo [] = {"memeclust", "-do:memestoqtbs", "-i:201101.txt", "-o:1101", "-mint:20110101", "-maxt:20110106"};  BigMain(6, ToDo);   

//	char *ToDo [] = {"memeclust", "-do:mkclustnet", "-i:1101-w4mfq5.QtBs", "-o:1101", "-shglready:F", "-netready:F"};  BigMain(6, ToDo);  
//  char *ToDo [] = {"memeclust", "-do:mkclustnet", "-i:qt08080902-w4mfq5.QtBs", "-o:0808", "-shglready:F", "-netready:F"};  BigMain(6, ToDo);  
//  char *ToDo [] = {"memeclust", "-do:mkclustnet", "-i:11011103-w4mfq5.QtBs", "-o:11011103", "-shglready:F", "-netready:F"};  BigMain(6, ToDo);  
//	char *ToDo [] = {"memeclust", "-do:mkclustnet", "-i:11041106-w4mfq5.QtBs", "-o:11041106", "-shglready:F", "-netready:F"};  BigMain(6, ToDo);  
//  char *ToDo [] = {"memeclust", "-do:mkclustnet", "-i:10071107-w4mfq5.QtBs", "-o:10071107", "-shglready:F", "-netready:F"};  BigMain(6, ToDo);  
  
  //char *ToDo [] = {"memeclust", "-do:memeclustzarya", "-i:201102.txt", "-o:201102", "-shglready:F", "-netready:F", "-mint:20110201", "-maxt:20110301"};  BigMain(8, ToDo);  
  BigMain(argc, argv);

  TSecTm EndTm = TSecTm::GetCurTm();
  double usedTime = EndTm.GetAbsSecs() - BegTm.GetAbsSecs();
  printf("Total execution time : %02dh%02dm%02ds\n", int(usedTime)/3600, (int(usedTime)%3600)/60, int(usedTime)%60);
  return 0;
  
  CatchFull
  printf("\nrun time: %s (%s)\n", ExeTm.GetTmStr(), TSecTm::GetCurTm().GetTmStr().CStr());
  return 0;
}

void BigMain(int argc, char* argv[]) {
  TExeTm ExeTm;
  Env = TEnv(argc, argv, TNotify::StdNotify);
  Env.PrepArgs("QuotesApp");
  const TStr ToDo = Env.GetIfArgPrefixStr("-do:", "", "To do").GetLc();
  if (Env.IsEndOfRun()) {
    printf("To do:\n");
    printf("    MkDataset         : Make memes dataset (extract quotes and save txt)\n");
    printf("    ExtractSubset     : Extract a subset of memes containing particular words\n");
    printf("    MemesToQtBs       : Load memes dataset and create quote base\n");
    printf("    MkClustNet        : Build cluster network from the quote base\n");
    return;
  }	
#pragma region mkdataset
  // extract quotes and links and make them into a single file
  if (ToDo == "mkdataset") {
    const TStr InFNm = Env.GetIfArgPrefixStr("-i:", "files.txt", "Spinn3r input files (one file per line)");
    const TStr OutFNm = Env.GetIfArgPrefixStr("-o:", "Spinn3r-dataset.txt", "Output file");
    const int MinQtWrdLen = Env.GetIfArgPrefixInt("-w:", 3, "Minimum quote word length");
    const TStr UrlFNm = Env.GetIfArgPrefixStr("-u:", "", "Seen url set (THashSet<TMd5Sig>) file name");
    const bool UrlOnlyOnce = Env.GetIfArgPrefixBool("-q:", true, "Only keep unique Urls");
    //// parse directly from Spinn3r
    TStr Spinn3rFNm;
    THashSet<TMd5Sig> SeenUrlSet;
    if (UrlOnlyOnce && ! UrlFNm.Empty()) {  // keep track of already seen urls (so that there are no duplicate urls)
      TFIn FIn(UrlFNm);  SeenUrlSet.Load(FIn);
    }
    FILE *F = fopen(OutFNm.CStr(), "wt");
    TFIn FIn(InFNm);
    int Items=0;
    for (int f=0; FIn.GetNextLn(Spinn3rFNm); f++) {
      TQuoteExtractor QE(Spinn3rFNm.ToTrunc());
      printf("Processing %02d: %s [%s]\n", f+1, Spinn3rFNm.CStr(), TExeTm::GetCurTm());
      fflush(stdout);
      for (int item = 0; QE.Next(); item++) {
        const TMd5Sig PostMd5(QE.PostUrlStr);
        if (QE.QuoteV.Empty() && QE.LinkV.Empty()) { continue; } // no quotes, no links
        if (UrlOnlyOnce) {
          if (SeenUrlSet.IsKey(PostMd5)) { continue; }
          SeenUrlSet.AddKey(PostMd5);
        }
        fprintf(F, "P\t%s\n", QE.PostUrlStr.CStr());
        //if (QE.PubTm > TSecTm(2008,8,30) || QE.PubTm < TSecTm(2008,7,25)) { printf("%s\n", QE.PubTm.GetStr().CStr()); }
        fprintf(F, "T\t%s\n", QE.PubTm.GetYmdTmStr().CStr());
        for (int q = 0; q < QE.QuoteV.Len(); q++) {
          if (TStrUtil::CountWords(QE.QuoteV[q]) >= MinQtWrdLen) {
            fprintf(F, "Q\t%s\n", QE.QuoteV[q].CStr()); }
        }
        for (int l = 0; l < QE.LinkV.Len(); l++) {
          fprintf(F, "L\t%s\n", QE.LinkV[l].CStr()); }
        fprintf(F, "\n");
        if (item>0 && item % Kilo(100) == 0) {
          QE.DumpStat();  QE.ExeTm.Tick(); }
        Items++;
      }
      printf("file done. Total %d all posts, %d all items\n", SeenUrlSet.Len(), Items);
      fflush(stdout);
    }
    printf("all done. Saving %d post urls\n", SeenUrlSet.Len());  fflush(stdout);
    if (! SeenUrlSet.Empty()) {
      TFOut FOut(OutFNm.GetFMid()+".SeenUrlSet");
      SeenUrlSet.Save(FOut);
    }
    fclose(F);
  }
#pragma endregion mkdataset

#pragma region extractsubset
  // save posts with memes containing particular words
  else if (ToDo == "extractsubset") {
    const TStr InFNmWc = Env.GetIfArgPrefixStr("-i:", "memes_*.rar", "Input file prefix");
    const bool IsInFNmWc = Env.GetIfArgPrefixBool("-w:", true, "Input is wildcard (else a file with list of input files)");
    const TStr OutFNm = Env.GetIfArgPrefixStr("-o:", "memes-subset.txt", "Output memes file");
    const TStr WordsFNm = Env.GetIfArgPrefixStr("-p:", "phrases-in.txt", "Phrases that memes have to contain");

    TChAV CatchMemeV;// = TStr::GetV("great depression", "economic meltdown", "recession had bottomed out", "green shoots", "slow recovery", "gradual recovery");
    printf("Loading %s\n", WordsFNm.CStr());
    { TFIn FIn(WordsFNm);
    for (TStr Ln; FIn.GetNextLn(Ln); ) {
      printf("  %s\n", Ln.GetLc().CStr());
      CatchMemeV.Add(Ln.GetLc()); }
    }
    printf("%d strings loaded\n", CatchMemeV.Len());
    TFOut FOut(OutFNm);
    TMemesDataLoader Memes(InFNmWc, IsInFNmWc);
    for (int posts = 0, nsave=0; Memes.LoadNext(); posts++) {
      bool DoSave = false;
      for (int m = 0; m < Memes.MemeV.Len(); m++) {
        for (int i = 0; i < CatchMemeV.Len(); i++) {
          if (Memes.MemeV[m].SearchStr(CatchMemeV[i]) != -1) {
            DoSave=true; break; }
        }
        if (DoSave) { break; }
      }
      if (DoSave) { Memes.SaveTxt(FOut); nsave++; }
      if (posts % Mega(1) == 0) {
        printf("%dm posts, %d saved\n", posts/Mega(1), nsave);
        FOut.Flush();
      }
    }
  }
#pragma endregion extractsubset

#pragma region memestoqtbs
  // load memes dataset (MkDataset) and create quote base
  else if (ToDo == "memestoqtbs") {
    const TStr InFNm = Env.GetIfArgPrefixStr("-i:", "201007_201107.txt", "Input Memes dataset files");
    const TStr MediaUrlFNm = Env.GetIfArgPrefixStr("-u:", "news_media.txt", "Fule with news media urls");
    const TStr Pref = Env.GetIfArgPrefixStr("-o:", "qt", "Output file name prefix");
    const int MinWrdLen = Env.GetIfArgPrefixInt("-l:", 4, "Min quote word length");
    const int MinMemeFq = Env.GetIfArgPrefixInt("-f:", 5, "Min meme frequency");
		const TStr MinTmStr = Env.GetIfArgPrefixStr("-mint:", "20100714", "Min time of quotes, format = YYYYMMDD");
		const TStr MaxTmStr = Env.GetIfArgPrefixStr("-maxt:", "20110728", "Max time of quotes, format = YYYYMMDD");
		TSecTm MinTm(atoi(MinTmStr.GetSubStr(0,3).CStr()),atoi(MinTmStr.GetSubStr(4,5).CStr()),atoi(MinTmStr.GetSubStr(6,7).CStr()));
		TSecTm MaxTm(atoi(MaxTmStr.GetSubStr(0,3).CStr()),atoi(MaxTmStr.GetSubStr(4,5).CStr()),atoi(MaxTmStr.GetSubStr(6,7).CStr()));

		PQuoteBs QtBs = TQuoteBs::New();
		int HashTableSize = 100; // 100 for each quarter, for one year data, use 400
		int UrlSetSize = 4 * HashTableSize;
		QtBs->ConstructQtBsZarya(InFNm, Pref, MediaUrlFNm, MinTm, MaxTm, MinWrdLen, MinMemeFq, HashTableSize, UrlSetSize);
		}
#pragma endregion memestoqtbs

#pragma region mkclustnet
  // make cluster network
  else if (ToDo == "mkclustnet") {
    TStr InQtBsNm = Env.GetIfArgPrefixStr("-i:", "", "Input quote base file name");
    TStr Pref = Env.GetIfArgPrefixStr("-o:", "qt", "Output network/updated QtBs filename");
		TStr BlackListFNm = Env.GetIfArgPrefixStr("-b:", "quote_blacklist.txt", "Blacklist file name");
		bool IsShglReady = Env.GetIfArgPrefixBool("-shglready:", false, "Indicate whether shingle hash table is ready");
		bool IsNetReady = Env.GetIfArgPrefixBool("-netready:", false, "Indicate whether cluster net is ready");
		double BktThresh = Env.GetIfArgPrefixFlt("-bktthresh:", 0.4, "Threshold for bad shingle bucket elimination");
		double MxTmDelay = Env.GetIfArgPrefixFlt("-delaythresh:", 5, "Max time delay between two quotes in the same cluster");
		double MxTmDev = Env.GetIfArgPrefixFlt("-devthresh:", 3, "Max time deviation for a quote to be specific rather than general");
		double RefineThresh = Env.GetIfArgPrefixFlt("-refinethresh:", 0.2, "Threshold for merging quote cluster in refining process");
    const int MinWrdLen = Env.GetIfArgPrefixInt("-minl:", 4, "Min quote word length");
		const int MaxWrdLen = Env.GetIfArgPrefixInt("-maxl:", 200, "Max quote word length");
    const int MinMemeFq = Env.GetIfArgPrefixInt("-mf:", 5, "Min meme frequency");
		const int MinClustFq = Env.GetIfArgPrefixInt("-cf:", 50, "Min quote cluster frequency");

		// Load quote base
    PQuoteBs QtBs;
    if (TZipIn::IsZipFNm(InQtBsNm)) { TZipIn ZipIn(InQtBsNm);  QtBs = TQuoteBs::Load(ZipIn); }
    else { TFIn FIn(InQtBsNm);  QtBs = TQuoteBs::Load(FIn); }

		// Cluster the quotes
    QtBs->ClusterQuotes(MinMemeFq, MinWrdLen, MaxWrdLen, BlackListFNm, Pref, IsShglReady, IsNetReady, BktThresh, MxTmDelay, MxTmDev, RefineThresh);

		// Dump the clusters
		bool SkipUrl = true, FlashDisp = true;
		QtBs->DumpQuoteClusters(MinWrdLen, MinClustFq, SkipUrl, FlashDisp, Pref);
  }
#pragma endregion mkclustnet

#pragma region memeclust
	else if (ToDo.SearchStr(TStr("memeclust")) >= 0) {
    const TStr InFNm = Env.GetIfArgPrefixStr("-i:", "201101.txt", "Input Memes dataset files");
    const TStr MediaUrlFNm = Env.GetIfArgPrefixStr("-u:", "news_media.txt", "Fule with news media urls");
    TStr Pref = Env.GetIfArgPrefixStr("-o:", "qt", "Output file name prefix");
    const int MinWrdLen = Env.GetIfArgPrefixInt("-minl:", 4, "Min quote word length");
		const int MaxWrdLen = Env.GetIfArgPrefixInt("-maxl:", 200, "Max quote word length");
		const int MinMemeFq = Env.GetIfArgPrefixInt("-f:", 5, "Min meme frequency");

		const int MinClustFq = Env.GetIfArgPrefixInt("-cf:", 50, "Min quote cluster frequency");
		TStr BlackListFNm = Env.GetIfArgPrefixStr("-b:", "quote_blacklist.txt", "Blacklist file name");
		bool IsQtBsReady = Env.GetIfArgPrefixBool("-qtbsready:", false, "Indicate whether quote base is ready and can be loaded readily");
		bool IsShglReady = Env.GetIfArgPrefixBool("-shglready:", false, "Indicate whether shingle hash table is ready");
		bool IsNetReady = Env.GetIfArgPrefixBool("-netready:", false, "Indicate whether cluster net is ready");
		double BktThresh = Env.GetIfArgPrefixFlt("-bktthresh:", 0.4, "Threshold for bad shingle bucket elimination");
		double MxTmDelay = Env.GetIfArgPrefixFlt("-delaythresh:", 5, "Max time delay between two quotes in the same cluster");
		double MxTmDev = Env.GetIfArgPrefixFlt("-devthresh:", 3, "Max time deviation for a quote to be specific rather than general");
		double RefineThresh = Env.GetIfArgPrefixFlt("-refinethresh:", 0.2, "Threshold for merging quote cluster in refining process");

		const TStr MinTmStr = Env.GetIfArgPrefixStr("-mint:", "20010101", "Min time of quotes, format = YYYYMMDD");
		const TStr MaxTmStr = Env.GetIfArgPrefixStr("-maxt:", "20200101", "Max time of quotes, format = YYYYMMDD");
		TSecTm MinTm(atoi(MinTmStr.GetSubStr(0,3).CStr()),atoi(MinTmStr.GetSubStr(4,5).CStr()),atoi(MinTmStr.GetSubStr(6,7).CStr()));
		TSecTm MaxTm(atoi(MaxTmStr.GetSubStr(0,3).CStr()),atoi(MaxTmStr.GetSubStr(4,5).CStr()),atoi(MaxTmStr.GetSubStr(6,7).CStr()));

		// Construct the quote base from Zarya data
		PQuoteBs QtBs = TQuoteBs::New();

		if (!IsQtBsReady) {
			int HashTableSize = 100; // 100 for each quarter, for one year data, use 400
			if (ToDo == "memeclustzarya") {
				int UrlSetSize = 4 * HashTableSize;
				QtBs->ConstructQtBsZarya(InFNm, Pref, MediaUrlFNm, MinTm, MaxTm, MinWrdLen, MinMemeFq, HashTableSize, UrlSetSize);
			}	else if (ToDo == "memeclustqtonly") {
				QtBs->ConstructQtBsQtOnly(InFNm, Pref, MediaUrlFNm, MinWrdLen, MinMemeFq, HashTableSize);
			} else if (ToDo == "memeclustqttime") {
				QtBs->ConstructQtBsQtTime(InFNm, Pref, MediaUrlFNm, MinWrdLen, MinMemeFq, HashTableSize);
			} else {
				printf("Please specify one of the three options for -do : memeclustzarya, memeclustqtonly, memeclustqttime!\n");
				return;
			}
		} else {
			TStr InQtBsNm = TStr::Fmt("%s-w%dmfq%d.QtBs", Pref.CStr(), MinWrdLen, MinMemeFq);
			if (TZipIn::IsZipFNm(InQtBsNm)) { TZipIn ZipIn(InQtBsNm);  QtBs = TQuoteBs::Load(ZipIn); }
			else { TFIn FIn(InQtBsNm);  QtBs = TQuoteBs::Load(FIn); }
		}

		// Cluster the quotes
    QtBs->ClusterQuotes(MinMemeFq, MinWrdLen, MaxWrdLen, BlackListFNm, Pref, IsShglReady, IsNetReady, BktThresh, MxTmDelay, MxTmDev, RefineThresh);

		// Dump the clusters
		bool SkipUrl = true, FlashDisp = true;
		QtBs->DumpQuoteClusters(MinWrdLen, MinClustFq, SkipUrl, FlashDisp, Pref);
	}
#pragma endregion memeclust
}
