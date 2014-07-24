#include "stdafx.h"
#include "lsh.h"

const int LSH::BandSize = 2;
const int LSH::NumBands = 20;
const int LSH::ShingleLen = 4; // In characters
const int LSH::ShingleWordLen = 2;
//const int LSH::WordWindow = 4;
const int LSH::WordWindow = 4;

void LSH::WordHashing(TQuoteBase* QuoteBase, THashSet<TMd5Sig>& Shingles) {
  Err("Hashing shingles using words...\n");
  TIntV QuoteIds;
  QuoteBase->GetAllQuoteIds(QuoteIds);
  for (int qt = 0; qt < QuoteIds.Len(); qt++) {
    if (qt % 1000 == 0) {
      Err("%d out of %d completed\n", qt, QuoteIds.Len());
    }
    TQuote Q;
    QuoteBase->GetQuote(QuoteIds[qt], Q);

    TStrV Content;
    Q.GetParsedContent(Content);

    int ContentLen = Content.Len();
    for (int i = 0; i < ContentLen; i++) {
      const TMd5Sig ShingleMd5(Content[i]);
      Shingles.AddKey(ShingleMd5);
    }
  }
  Err("Done with word hashing! Number of shingles: %d\n", Shingles.Len());
}

void LSH::WordHashing(TQuoteBase *QuoteBase,
    THash<TMd5Sig, TIntSet>& ShingleToQuoteIds) {
  fprintf(stderr, "Hashing shingles using words...\n");
  TIntV QuoteIds;
  QuoteBase->GetAllQuoteIds(QuoteIds);

  THash<TStr, TIntSet> Temp;

  for (int qt = 0; qt < QuoteIds.Len(); qt++) {
    if (qt % 1000 == 0) {
      fprintf(stderr, "%d out of %d completed\n", qt, QuoteIds.Len());
    }
    TQuote Q;
    QuoteBase->GetQuote(QuoteIds[qt], Q);

    TStrV Content;
    Q.GetParsedContent(Content);

    int ContentLen = Content.Len();
    for (int i = 0; i < ContentLen; i++) {
      const TMd5Sig ShingleMd5(Content[i]);
      TIntSet ShingleQuoteIds;
      ShingleToQuoteIds.IsKeyGetDat(ShingleMd5, ShingleQuoteIds);
      ShingleQuoteIds.AddKey(QuoteIds[qt]);
      ShingleToQuoteIds.AddDat(ShingleMd5, ShingleQuoteIds);

      ///// COMMENT OUT LATER
      TIntSet TempSet;
      Temp.IsKeyGetDat(Content[i], TempSet);
      TempSet.AddKey(QuoteIds[qt]);
      Temp.AddDat(Content[i], TempSet);
    }
  }

  TVec<TStr> ShingleKeys;
  Temp.GetKeyV(ShingleKeys);
  ShingleKeys.SortCmp(TCmpSetByLen(false, &Temp));
  for (int i = 0; i < 100; i++) {
    TIntSet TempSet = Temp.GetDat(ShingleKeys[i]);
    Err("%d: %s - %d \n", i, ShingleKeys[i].CStr(), TempSet.Len());
  }

  Err("Done with word hashing!\n");
}

void LSH::ElCheapoHashing(TQuoteBase *QuoteBase, TInt ShingleLen,
    THash<TMd5Sig, TIntSet>& ShingleToQuoteIds) {
  fprintf(stderr, "Hashing shingles the el cheapo way...\n");
  TIntV QuoteIds;
  QuoteBase->GetAllQuoteIds(QuoteIds);
  for (int qt = 0; qt < QuoteIds.Len(); qt++) {
    if (qt % 1000 == 0) {
      fprintf(stderr, "%d out of %d completed\n", qt, QuoteIds.Len());
    }
    TQuote Q;
    QuoteBase->GetQuote(QuoteIds[qt], Q);

    // Put x-character (or x-word) shingles into hash table; x is specified by ShingleLen parameter
    TStr QContentStr;
    Q.GetParsedContentString(QContentStr);
    TChA QContentChA = TChA(QContentStr);

    for (int i = 0; i < QContentChA.Len() - ShingleLen + 1; i++) {
      TChA ShingleChA = TChA();
      for (int j = 0; j < ShingleLen; j++) {
        ShingleChA.AddCh(QContentChA.GetCh(i + j));
      }
      TStr Shingle = TStr(ShingleChA);
      const TMd5Sig ShingleMd5(Shingle);
      TIntSet ShingleQuoteIds;
      if (ShingleToQuoteIds.IsKey(ShingleMd5)) {
        ShingleQuoteIds = ShingleToQuoteIds.GetDat(ShingleMd5);
      }

      ShingleQuoteIds.AddKey(QuoteIds[qt]);
      ShingleToQuoteIds.AddDat(ShingleMd5, ShingleQuoteIds);
    }
  }
  Err("Done with el cheapo hashing!\n");
}

/// For every quote, add it to corresponding bucket for each hashed x-character shingle of the quote
// (Shingles by characters)
void LSH::HashShingles(TQuoteBase *QuoteBase, TClusterBase *CB, TInt ShingleLen,
    THash<TMd5Sig, TShingleIdSet>& ShingleToQuoteIds) {
  Err("Hashing shingles...\n");
  TIntV QuoteIds;
  QuoteBase->GetAllQuoteIds(QuoteIds);
  for (int qt = 0; qt < QuoteIds.Len(); qt++) {
    if (qt % 1000 == 0) {
      fprintf(stderr, "%d out of %d completed\n", qt, QuoteIds.Len());
    }

    if (CB->IsQuoteInArchivedCluster(QuoteIds[qt]))
      continue;
    TQuote Q;
    QuoteBase->GetQuote(QuoteIds[qt], Q);

    // Put x-character (or x-word) shingles into hash table; x is specified by ShingleLen parameter
    TStr QContentStr;
    Q.GetParsedContentString(QContentStr);
    TChA QContentChA = TChA(QContentStr);

    int CurWord = 0;

    for (int i = 0; i < QContentChA.Len() - ShingleLen + 1; i++) {
      TChA ShingleChA = TChA();
      for (int j = 0; j < ShingleLen; j++) {
        ShingleChA.AddCh(QContentChA.GetCh(i + j));
      }
      TStr Shingle = TStr(ShingleChA);
      const TMd5Sig ShingleMd5(Shingle);
      TShingleIdSet ShingleQuoteIds;
      if (ShingleToQuoteIds.IsKey(ShingleMd5)) {
        ShingleQuoteIds = ShingleToQuoteIds.GetDat(ShingleMd5);
      }

      for (int j = CurWord; j > CurWord - WordWindow && j >= 0; j--) {
        ShingleQuoteIds.AddKey(TShingleId(QuoteIds[qt], j));
      }

      ShingleToQuoteIds.AddDat(ShingleMd5, ShingleQuoteIds);

      // up the current word index if we see a space
      if (QContentChA.GetCh(i + ShingleLen - 1) == ' ') {
        CurWord++;
      }
    }
  }
  Err("Done hashing!\n");
}

void LSH::GetHashedShinglesOfCluster(TQuoteBase *QuoteBase, TCluster& C,
    TInt ShingleLen, THashSet<TMd5Sig>& HashedShingles) {
  TIntV QuoteIds;
  C.GetQuoteIds(QuoteIds);
  for (int qt = 0; qt < QuoteIds.Len(); qt++) {
    TQuote Q;
    QuoteBase->GetQuote(QuoteIds[qt], Q);
    TStr QContentStr;
    Q.GetContentString(QContentStr);
    TStr QContentStrNoPunc;
    TStringUtil::RemovePunctuation(QContentStr, QContentStrNoPunc);
    TStrV QContentV;
    QContentStrNoPunc.SplitOnWs(QContentV);
    for (int i = 0; i < QContentV.Len() - ShingleLen + 1; i++) {
      TStr Shingle;
      for (int j = 0; j < ShingleLen; j++) {
        if (j > 0) {
          Shingle.InsStr(Shingle.Len(), " ");
        }
        Shingle.InsStr(Shingle.Len(), QContentV[i + j]);
      }
      TMd5Sig ShingleMd5(Shingle);
      HashedShingles.AddKey(ShingleMd5);
    }
  }
}

/// Shingles by words
void LSH::HashShinglesOfClusters(TQuoteBase *QuoteBase,
    TClusterBase *ClusterBase, TIntV& ClusterIds, TInt ShingleLen,
    THash<TMd5Sig, TIntV>& ShingleToClusterIds) {
  Err("Hashing shingles of clusters...\n");
  for (int i = 0; i < ClusterIds.Len(); i++) {
    if (i % 1000 == 0) {
      fprintf(stderr, "%d out of %d completed\n", i, ClusterIds.Len());
    }
    TCluster C;
    ClusterBase->GetCluster(ClusterIds[i], C);
    //fprintf(stderr, "%d vs. %d\n", ClusterIds[i].Val, C.GetId().Val);

    // Put x-word shingles into hash table; x is specified by ShingleLen parameter
    THashSet < TMd5Sig > CHashedShingles;
    GetHashedShinglesOfCluster(QuoteBase, C, ShingleLen, CHashedShingles);
    for (THashSet<TMd5Sig>::TIter Hash = CHashedShingles.BegI();
        Hash < CHashedShingles.EndI(); Hash++) {
      TIntV ShingleClusterIds;
      if (ShingleToClusterIds.IsKey(*Hash)) {
        ShingleClusterIds = ShingleToClusterIds.GetDat(*Hash);
      }
      ShingleClusterIds.Add(ClusterIds[i]);
      ShingleToClusterIds.AddDat(*Hash, ShingleClusterIds);
    }
  }
  Err("Done hashing!\n");
}

void LSH::MinHash(THash<TMd5Sig, TShingleIdSet>& ShingleToQuoteIds,
    TVec<THash<TIntV, TIntSet> >& SignatureBandBuckets) {
  TRnd RandomGenerator; // TODO: make this "more random" by incorporating time
  for (int i = 0; i < NumBands; ++i) {
    THash<TShingleId, TIntV> Inverted; // (QuoteID, QuoteSignatureForBand)
    THash < TIntV, TIntSet > BandBuckets; // (BandSignature, QuoteIDs)
    for (int j = 0; j < BandSize; ++j) {
      // Create new signature
      TVec < TMd5Sig > Signature;
      ShingleToQuoteIds.GetKeyV(Signature);
      Signature.Shuffle(RandomGenerator);

      // Place in bucket - not very efficient
      int SigLen = Signature.Len();
      for (int k = 0; k < SigLen; ++k) {
        TShingleIdSet CurSet = ShingleToQuoteIds.GetDat(Signature[k]);
        for (TShingleIdSet::TIter l = CurSet.BegI(); l < CurSet.EndI(); l++) {
          TShingleId Key = l.GetKey();
          if (Inverted.IsKey(Key)) {
            TIntV CurSignature = Inverted.GetDat(Key);
            if (CurSignature.Len() <= j) {
              CurSignature.Add(k);
              Inverted.AddDat(Key, CurSignature);
            }
          } else {
            TIntV NewSignature;
            NewSignature.Add(k);
            Inverted.AddDat(Key, NewSignature);
          }
        }
      }
    }

    TVec<TShingleId> InvertedKeys;
    Inverted.GetKeyV(InvertedKeys);
    TInt InvertedLen = InvertedKeys.Len();
    for (int k = 0; k < InvertedLen; ++k) {
      TIntSet Bucket;
      TIntV Signature = Inverted.GetDat(InvertedKeys[k]);
      if (BandBuckets.IsKey(Signature)) {
        Bucket = BandBuckets.GetDat(Signature);
      }
      Bucket.AddKey(InvertedKeys[k].Val1);
      BandBuckets.AddDat(Signature, Bucket);
    }

    SignatureBandBuckets.Add(BandBuckets);
    Err("%d out of %d band signatures computed\n", i + 1, NumBands);
  }
  Err("Minhash step complete!\n");
}

// YES I COPIED AND PASTED CODE my section leader would be so ashamed :D
void LSH::MinHash(THash<TMd5Sig, TIntSet>& ShingleToQuoteIds,
    TVec<THash<TIntV, TIntSet> >& SignatureBandBuckets) {
  TRnd RandomGenerator; // TODO: make this "more random" by incorporating time
  for (int i = 0; i < NumBands; ++i) {
    THash < TInt, TIntV > Inverted; // (QuoteID, QuoteSignatureForBand)
    THash < TIntV, TIntSet > BandBuckets; // (BandSignature, QuoteIDs)
    for (int j = 0; j < BandSize; ++j) {
      // Create new signature
      TVec < TMd5Sig > Signature;
      ShingleToQuoteIds.GetKeyV(Signature);
      Signature.Shuffle(RandomGenerator);

      // Place in bucket - not very efficient
      int SigLen = Signature.Len();
      for (int k = 0; k < SigLen; ++k) {
        TIntSet CurSet = ShingleToQuoteIds.GetDat(Signature[k]);
        for (TIntSet::TIter l = CurSet.BegI(); l < CurSet.EndI(); l++) {
          TInt Key = l.GetKey();
          if (Inverted.IsKey(Key)) {
            TIntV CurSignature = Inverted.GetDat(Key);
            if (CurSignature.Len() <= j) {
              CurSignature.Add(k);
              Inverted.AddDat(Key, CurSignature);
            }
          } else {
            TIntV NewSignature;
            NewSignature.Add(k);
            Inverted.AddDat(Key, NewSignature);
          }
        }
      }
    }

    TIntV InvertedKeys;
    Inverted.GetKeyV(InvertedKeys);
    TInt InvertedLen = InvertedKeys.Len();
    for (int k = 0; k < InvertedLen; ++k) {
      TIntSet Bucket;
      TIntV Signature = Inverted.GetDat(InvertedKeys[k]);
      if (BandBuckets.IsKey(Signature)) {
        Bucket = BandBuckets.GetDat(Signature);
      }
      Bucket.AddKey(InvertedKeys[k]);
      BandBuckets.AddDat(Signature, Bucket);
    }

    SignatureBandBuckets.Add(BandBuckets);
    Err("%d out of %d band signatures computed\n", i + 1, NumBands);
  }
  Err("Minhash step complete!\n");
}

void LSH::ComputeSignatures(THashSet<TMd5Sig>& Shingles,
    THash<TMd5Sig, TIntV>& Signatures, int NumSignatures) {
  if (NumSignatures < 1)
    return;
  TRnd RandomGenerator; // TODO: make this "more random" by incorporating time
  TInt NumShingles = Shingles.Len();

  for (int i = 0; i < NumSignatures; ++i) {
    // Create new signature
    TVec < TMd5Sig > Shuffle;
    Shingles.GetKeyV(Shuffle);
    Shuffle.Shuffle(RandomGenerator);

    for (int j = 0; j < NumShingles; j++) {
      TIntV Signature;
      Signatures.IsKeyGetDat(Shuffle[j], Signature);
      Signature.Add(j);
      Signatures.AddDat(Shuffle[j], Signature);
    }
  }
  Err("Computed %d signatures!\n", NumSignatures);
}

void LSH::MinHash(TQuoteBase *QB, THashSet<TMd5Sig>& Shingles,
    TVec<THash<TMd5Sig, TIntSet> >& SignatureBandBuckets) {
  Err("Creating buckets...\n");
  THash < TMd5Sig, TIntV > Signatures;
  ComputeSignatures(Shingles, Signatures, NumBands * BandSize);

  // bucket creation
  for (int i = 0; i < NumBands; ++i) {
    SignatureBandBuckets.Add(THash<TMd5Sig, TIntSet>());
  }


  // bucket filling
  int NumShingles = Shingles.Len();
  THash<TInt, TQuote> Quotes;
  QB->GetIdToTQuotes(Quotes);

  THash<TInt, TQuote>::TIter CurI = Quotes.BegI();
  THash<TInt, TQuote>::TIter EndI = Quotes.EndI();
  TQuote Q; // SKYFALL

  for (; CurI < EndI; CurI++) {
    Q = CurI.GetDat();

    TStrV Content;
    Q.GetParsedContent(Content);
    TInt Id = Q.GetId();

    // signature for quote
    int ContentLen = Content.Len();
    TVec < TIntV > Signature;
    for (int i = 0; i < ContentLen; i++) {
      const TMd5Sig ShingleMd5(Content[i]);
      Signature.Add(Signatures.GetDat(ShingleMd5));
    }

    // place in bucket
    if (ContentLen < WordWindow) {
      for (int i = 0; i < NumBands; ++i) {
        TStr Sig;
        for (int j = 0; j < BandSize; ++j) {
          int CurSig = i * BandSize + j;

          TInt min = NumShingles;
          for (int k = 0; k < ContentLen; k++) {
            if (Signature[k][CurSig] < min) {
              min = Signature[k][CurSig];
            }
          }
          Sig += min.GetStr() + "-";
        }
        //Err(Sig.CStr());

        const TMd5Sig SigMd5(Sig);
        TIntSet Bucket;
        SignatureBandBuckets[i].IsKeyGetDat(SigMd5, Bucket);
        Bucket.AddKey(Id);
        SignatureBandBuckets[i].AddDat(SigMd5, Bucket);
      }
    } else {

    }

  }
  Err("Minhash step complete!\n");
}
