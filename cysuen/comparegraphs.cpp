#include "stdafx.h"

int main(int argc, char *argv[]) {
  if (argc < 3) {
    Err("Error: needs 2 arguments: baseGraph, compareGraph");
  }

  FILE *A = fopen(argv[1], "r");
  FILE *B = fopen(argv[2], "r");

  THashSet<TIntPr> Edges;
  int i1 = 0, i2 = 0;
  int edgesA = 0;
  fscanf (A, "%d %d\n", &i1, &i2);
  while (!feof (A))
  {
    Edges.AddKey(TIntPr(i1, i2));
    edgesA++;
    fscanf (A, "%d %d\n", &i1, &i2);
  }

  int count = 0;
  int edgesB = 0;

  fscanf (B, "%d %d\n", &i1, &i2);
  while (!feof (B))
  {
    if (Edges.IsKey(TIntPr(i1, i2))) {
      count++;
    }
    edgesB++;
    fscanf (B, "%d %d\n", &i1, &i2);
  }

  double P = count * 1.0 / edgesB;
  double R = count * 1.0 / edgesA;
  double F1 = 2 * P * R / (P + R);
  Err("P: %f (%d/%d)\n", P, count, edgesB);
  Err("R: %f (%d/%d)\n", R, count, edgesA);
  Err("F1: %f\n", F1);

  fclose(A);
  fclose(B);
  return 0;
}
