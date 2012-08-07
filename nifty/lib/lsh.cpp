#include "stdafx.h"
#include "lsh.h"

const int LSH::BandSize = 3;
const int LSH::NumBands = 20;
const int LSH::ShingleLen = 4;  // In characters
const int LSH::ShingleWordLen = 2;
const int LSH::WordWindow = 5;

/// For every quote, add it to corresponding bucket for each hashed x-character shingle of the quote
// (Shingles by characters)
void LSH::HashShingles(TQuoteBase *QuoteBase, TInt ShingleLen, THash<TMd5Sig, TShingleIdSet>& ShingleToQuoteIds) {
  fprintf(stderr, "Hashing shingles...\n");
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
    
    int CurWord = 0;

    for (int i = 0; i < QContentChA.Len()-ShingleLen+1; i++) {
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
  fprintf(stderr, "Done hashing!\n");
}

void LSH::GetHashedShinglesOfCluster(TQuoteBase *QuoteBase, TCluster& C, TInt ShingleLen, THashSet<TMd5Sig>& HashedShingles) {
  TIntV QuoteIds;
  C.GetQuoteIds(QuoteIds);
  for (int qt = 0; qt < QuoteIds.Len(); qt++) {
    TQuote Q;
    QuoteBase->GetQuote(QuoteIds[qt], Q);
    TStr QContentStr;
    Q.GetContentString(QContentStr);
    TStr QContentStrNoPunc;
    TQuote::RemovePunctuation(QContentStr, QContentStrNoPunc);
    TStrV QContentV;
    QContentStrNoPunc.SplitOnWs(QContentV);
    for (int i = 0; i < QContentV.Len()-ShingleLen+1; i++) {
      TStr Shingle;
      for (int j = 0; j < ShingleLen; j++) {
        if (j > 0) { Shingle.InsStr(Shingle.Len(), " "); }
        Shingle.InsStr(Shingle.Len(), QContentV[i + j]);
      }
      TMd5Sig ShingleMd5(Shingle);
      HashedShingles.AddKey(ShingleMd5);
    }
  }
}

/// Shingles by words
void LSH::HashShinglesOfClusters(TQuoteBase *QuoteBase, TClusterBase *ClusterBase, TIntV& ClusterIds, TInt ShingleLen, THash<TMd5Sig, TIntV>& ShingleToClusterIds) {
  fprintf(stderr, "Hashing shingles of clusters...\n");
  for (int i = 0; i < ClusterIds.Len(); i++) {
    if (i % 1000 == 0) {
      fprintf(stderr, "%d out of %d completed\n", i, ClusterIds.Len());
    }
    TCluster C;
    ClusterBase->GetCluster(ClusterIds[i], C);
    //fprintf(stderr, "%d vs. %d\n", ClusterIds[i].Val, C.GetId().Val);

    // Put x-word shingles into hash table; x is specified by ShingleLen parameter
    THashSet<TMd5Sig> CHashedShingles;
    GetHashedShinglesOfCluster(QuoteBase, C, ShingleLen, CHashedShingles);
    for (THashSet<TMd5Sig>::TIter Hash = CHashedShingles.BegI(); Hash < CHashedShingles.EndI(); Hash++) {
      TIntV ShingleClusterIds;
      if (ShingleToClusterIds.IsKey(*Hash)) {
        ShingleClusterIds = ShingleToClusterIds.GetDat(*Hash);
      }
      ShingleClusterIds.Add(ClusterIds[i]);
      ShingleToClusterIds.AddDat(*Hash, ShingleClusterIds);
    }
  }
  fprintf(stderr, "Done hashing!\n");
}

void  LSH::MinHash(THash<TMd5Sig, TShingleIdSet>& ShingleToQuoteIds, TVec<THash<TIntV, TIntSet> >& SignatureBandBuckets) {
  TRnd RandomGenerator; // TODO: make this "more random" by incorporating time
  for(int i = 0; i < NumBands; ++i) {
    THash<TShingleId, TIntV> Inverted; // (QuoteID, QuoteSignatureForBand)
    THash<TIntV, TIntSet> BandBuckets; // (BandSignature, QuoteIDs)
    for (int j = 0; j < BandSize; ++j) {
      // Create new signature
      TVec<TMd5Sig> Signature;
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
    printf("%d out of %d band signatures computed\n", i+1, NumBands);
  }
  printf("Minhash step complete!\n");
}
