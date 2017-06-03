#include <direct.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <time.h>
#include <windows.h>
#include "dirent.h"
#include "nedmalloc.h"
#define AVERAGE(a, b) (unsigned char)(((a) + (b)) >> 1)

typedef char *NM;
typedef struct INFILES {
  int nts;
  int nnet;
  NM *TS;
  NM *NET;
} inFiles;

void ScaleLine(char *Target, char *Source, int SrcWidth, int TgtWidth) {
  int NumPixels = TgtWidth;
  int IntPart = SrcWidth / TgtWidth;
  int FractPart = SrcWidth % TgtWidth;
  int E = 0;

  while (NumPixels-- > 0) {
    *Target++ = *Source;
    Source += IntPart;
    E += FractPart;
    if (E >= TgtWidth) {
      E -= TgtWidth;
      Source++;
    } /* if */
  }   /* while */
}

void ScaleRect(char *Target, char *Source, int SrcWidth, int SrcHeight,
               int TgtWidth, int TgtHeight) {
  int NumPixels = TgtHeight;
  int IntPart = (SrcHeight / TgtHeight) * SrcWidth;
  int FractPart = SrcHeight % TgtHeight;
  int E = 0;
  char *PrevSource = NULL;

  while (NumPixels-- > 0) {
    if (Source == PrevSource) {
      memcpy(Target, Target - TgtWidth, TgtWidth * sizeof(*Target));
    } else {
      ScaleLine(Target, Source, SrcWidth, TgtWidth);
      PrevSource = Source;
    } /* if */
    Target += TgtWidth;
    Source += IntPart;
    E += FractPart;
    if (E >= TgtHeight) {
      E -= TgtHeight;
      Source += SrcWidth;
    } /* if */
  }   /* while */
}

int main() {
  int i = 1, j, k, w, h, capo = 5, H, L, zeri = 0, uni = 0, under = 2;
  double pixel[225];
  char Buffer[1000], *Buff_ts, prova[1000], c, ncerchi[20];
  // unsigned char** mat;
  unsigned char *mat, *matr_imm5;
  WIN32_FIND_DATA FindFileData;
  HANDLE hFind;
  size_t origsize;
  DIR *dir;
  WCHAR Buffer2[1000];
  inFiles *INNF;
  FILE *file, *file1;
  errno_t err;

  H = 256;
  L = 256;
  memset(pixel, 0, 225);
  mat = (unsigned char *)calloc(H * L, sizeof(unsigned char));
  matr_imm5 = malloc(sizeof(unsigned char) * 16 * 16);
  i = 1;
  INNF = nedmalloc(sizeof(inFiles));
  /*procedura per la cattura della directory corrente*/
  _getcwd(Buffer, 1000 * sizeof(char));
  strcat_s(Buffer, 1000, "\\input_files_test_sets\\");
  _chdir(Buffer);  // cambia la posizione della cartella di lavoro
  /*apre lo stream della directory input */
  if ((dir = opendir(Buffer)) == NULL) {
    printf("\n**errore cartella input**\n");
    system("pause");
    exit(1);
  }
  /*routine ricerca file .ts nella directory*/
  Buff_ts = _strdup(Buffer);
  realloc(Buff_ts, strlen(Buff_ts) + 6);
  strcat_s(Buff_ts, strlen(Buff_ts) + 6, "*.PGM");
  MultiByteToWideChar(CP_ACP, 0, Buff_ts, -1, Buffer2, 1000);
  if ((hFind = FindFirstFile(Buffer2, &FindFileData)) == INVALID_HANDLE_VALUE) {
    printf("**files .ts non trovati nella cartella input**\n");
    FindClose(hFind);
    system("pause");
    exit(1);
  }

  /*mi salvo il primo file trovato*/
  origsize = wcslen(FindFileData.cFileName) + 1;
  wcstombs_s(NULL, prova, origsize, FindFileData.cFileName, 1000);
  INNF->TS = nedmalloc(sizeof(NM *));
  INNF->TS[0] = _strdup(prova);
  /*mi salvo gli altri file trovati*/
  while (FindNextFile(hFind, &FindFileData) != 0) {
    origsize = wcslen(FindFileData.cFileName) + 1;
    wcstombs_s(NULL, prova, origsize, FindFileData.cFileName, 1000);
    nedrealloc(INNF->TS, (++i) * sizeof(NM *));
    INNF->TS[i - 1] = _strdup(prova);
  }
  INNF->nts = i; /*numero file trovati*/
  printf("\n**Trovati %d Files con estensione .PGM**\n", i);
  FindClose(hFind);
  if ((err = fopen_s(&file1, "train.ts", "w")) != 0) {
    printf("ERROR: file open failed\n");
    system("pause");
    return (-1);
  }
  fprintf(file1, "<dim>256</dim>\n<\n");
  // ciclo su ogni file pgm
  for (h = 0; h < INNF->nts; h++) {
    if (h % 100 == 0) printf("\n**analizzati %d files**\n", h);
    if ((err = fopen_s(&file, INNF->TS[h], "r")) != 0) {
      printf("ERROR: file open failed\n");
      system("pause");
      return (-1);
    }
    capo = 5;
    memset(ncerchi, 0, 8);
    j = 0;
    while (capo) {
      c = fgetc(file);
      if (c == '#') {
        while ((c = fgetc(file)) != '\n') {
          ncerchi[j++] = c;
        }
        ncerchi[j] = '\0';
      }
      if (c == '\n') capo--;
    }
    for (k = 0; k < L; k++) {
      for (w = 0; w < H; w++) {
        mat[w + k * L] = fgetc(file);
        fgetc(file);
      }
    }
    ScaleRect(matr_imm5, mat, 256, 256, 16, 16);

    /*//tecnica sperimentale
             for(k=0;k<L;k++){
                            for(w=0;w<H;w++){
                      mat[k][w]=fgetc(file);
                             fgetc(file);
                     }
                    }

    for(j=0;j<15;j++){
            for(i=0;i<15;i++){
                    uni=zeri=0;
      for(k=j*16;k<32+(j*16);k++){
                     for(w=i*16;w<32+(i*16);w++){
        if(mat[k][w]=='1')uni++;
                                    else zeri++;
                            //printf("%c",mat[k][w]);
                            }
                            //printf("\n");
                    }
            //	system("pause");
            //	printf("\nzeri:%d\nuni:%d",zeri,uni);
                    pixel[i+15*j]=(uni>zeri)? 1 : 0;
                    //pixel[i+15*j]=((double)uni)/1024.0;
     }
    }*/

    for (i = 0; i < 256; i++) {
      fprintf(file1, "%c ", matr_imm5[i]);
    }
    fprintf(file1, "  ");
    // inserire numero cerchi
    fprintf(file1, "%s", ncerchi);
    fprintf(file1, "\n");
    fclose(file);
  }
  fprintf(file1, ">");
  closedir(dir);
  system("pause");
  return 0;
}