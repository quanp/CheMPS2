/*
   CheMPS2: a spin-adapted implementation of DMRG for ab initio quantum chemistry
   Copyright (C) 2013, 2014 Sebastian Wouters

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <math.h>
#include <stdlib.h>
#include <algorithm>

#include "Sobject.h"
#include "TensorT.h"
#include "SyBookkeeper.h"
#include "Lapack.h"
#include "Gsl.h"

using std::min;
using std::max;

CheMPS2::Sobject::Sobject(const int indexIn, const int IlocalIn1, const int IlocalIn2, SyBookkeeper * denBKIn){

   index = indexIn;
   Ilocal1 = IlocalIn1;
   Ilocal2 = IlocalIn2;
   denBK = denBKIn;
      
   nKappa = 0;
   
   for (int NL = denBK->gNmin(index); NL<= denBK->gNmax(index); NL++){
      for (int TwoSL = denBK->gTwoSmin(index,NL); TwoSL<= denBK->gTwoSmax(index,NL); TwoSL+=2){
         for (int IL = 0; IL< denBK->getNumberOfIrreps(); IL++){
            int dimL = denBK->gCurrentDim(index,NL,TwoSL,IL);
            if (dimL>0){
               for (int N1=0; N1<=2; N1++){
                  for (int N2=0; N2<=2; N2++){
                     int NR = NL+N1+N2;
                     int IR = ((N1==1)?(Irreps::directProd(IL,Ilocal1)):IL); //IR as intermediary
                         IR = ((N2==1)?(Irreps::directProd(IR,Ilocal2)):IR);
                     for (int TwoJ = ((N1+N2)%2); TwoJ<=(((N1==1)&&(N2==1))?2:((N1+N2)%2)) ; TwoJ+=2){
                        for (int TwoSR = TwoSL-TwoJ; TwoSR <= TwoSL+TwoJ; TwoSR+=2){
                           if (TwoSR>=0){
                              int dimR = denBK->gCurrentDim(index+2,NR,TwoSR,IR); //two boundaries further!
                              if (dimR>0){
                                 nKappa++;
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
   
   sectorNL = new int[nKappa];
   sectorTwoSL = new int[nKappa];
   sectorIL = new int[nKappa];
   sectorN1 = new int[nKappa];
   sectorN2 = new int[nKappa];
   sectorTwoJ = new int[nKappa];
   sectorNR = new int[nKappa];
   sectorTwoSR = new int[nKappa];
   sectorIR = new int[nKappa];
   kappa2index = new int[nKappa+1];
   kappa2index[0] = 0;
   
   nKappa = 0;
   
   for (int NL = denBK->gNmin(index); NL<= denBK->gNmax(index); NL++){
      for (int TwoSL = denBK->gTwoSmin(index,NL); TwoSL<= denBK->gTwoSmax(index,NL); TwoSL+=2){
         for (int IL = 0; IL< denBK->getNumberOfIrreps(); IL++){
            const int dimL = denBK->gCurrentDim(index,NL,TwoSL,IL);
            if (dimL>0){
               for (int N1=0; N1<=2; N1++){
                  for (int N2=0; N2<=2; N2++){
                     const int NR = NL+N1+N2;
                     int IR = ((N1==1)?(Irreps::directProd(IL,Ilocal1)):IL); //IR as intermediary
                         IR = ((N2==1)?(Irreps::directProd(IR,Ilocal2)):IR); //IR final result
                     for (int TwoJ = ((N1+N2)%2); TwoJ<=(((N1==1)&&(N2==1))?2:((N1+N2)%2)) ; TwoJ+=2){
                        for (int TwoSR = TwoSL-TwoJ; TwoSR <= TwoSL+TwoJ; TwoSR+=2){
                           if (TwoSR>=0){
                              const int dimR = denBK->gCurrentDim(index+2,NR,TwoSR,IR); //two boundaries further!
                              if (dimR>0){
                                 sectorNL[nKappa] = NL;
                                 sectorTwoSL[nKappa] = TwoSL;
                                 sectorIL[nKappa] = IL;
                                 sectorN1[nKappa] = N1;
                                 sectorN2[nKappa] = N2;
                                 sectorTwoJ[nKappa] = TwoJ;
                                 sectorNR[nKappa] = NR;
                                 sectorTwoSR[nKappa] = TwoSR;
                                 sectorIR[nKappa] = IR;
                                 nKappa++;
                                 kappa2index[nKappa] = kappa2index[nKappa-1] + dimL*dimR;
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
   
   storage = new double[kappa2index[nKappa]];
   
   reorder = new int[nKappa];
   for (int cnt=0; cnt<nKappa; cnt++){ reorder[cnt] = cnt; }
   bool sorted = false;
   while (!sorted){ //Bubble sort so that blocksize(reorder[i]) >= blocksize(reorder[i+1]), with blocksize(k) = kappa2index[k+1]-kappa2index[k]
      sorted = true;
      for (int cnt=0; cnt<nKappa-1; cnt++){
         const int index1 = reorder[cnt];
         const int index2 = reorder[cnt+1];
         if ((kappa2index[index1+1] - kappa2index[index1]) < (kappa2index[index2+1] - kappa2index[index2])){
            sorted = false;
            reorder[cnt]   = index2;
            reorder[cnt+1] = index1;
         }
      }
   }

}

CheMPS2::Sobject::~Sobject(){

   delete [] sectorNL;
   delete [] sectorTwoSL;
   delete [] sectorIL;
   delete [] sectorN1;
   delete [] sectorN2;
   delete [] sectorTwoJ;
   delete [] sectorNR;
   delete [] sectorTwoSR;
   delete [] sectorIR;
   delete [] kappa2index;
   delete [] storage;
   delete [] reorder;

}

int CheMPS2::Sobject::gNKappa() const { return nKappa; }

double * CheMPS2::Sobject::gStorage() { return storage; }

int CheMPS2::Sobject::gReorder(const int ikappa) const{ return reorder[ikappa]; }
      
int CheMPS2::Sobject::gKappa(const int NL, const int TwoSL, const int IL, const int N1, const int N2, const int TwoJ, const int NR, const int TwoSR, const int IR) const{

   for (int cnt=0; cnt<nKappa; cnt++){
      if ((sectorNL[cnt]==NL)&&(sectorTwoSL[cnt]==TwoSL)&&(sectorIL[cnt]==IL)&&(sectorN1[cnt]==N1)&&(sectorN2[cnt]==N2)&&(sectorTwoJ[cnt]==TwoJ)&&(sectorNR[cnt]==NR)&&(sectorTwoSR[cnt]==TwoSR)&&(sectorIR[cnt]==IR)) return cnt;
   }
   
   return -1;

}
      
int CheMPS2::Sobject::gKappa2index(const int kappa) const{ return kappa2index[kappa]; }
      
double * CheMPS2::Sobject::gStorage(const int NL, const int TwoSL, const int IL, const int N1, const int N2, const int TwoJ, const int NR, const int TwoSR, const int IR){

   int kappa = gKappa(NL,TwoSL,IL,N1,N2,TwoJ,NR,TwoSR,IR);
   if (kappa == -1) return NULL;
   return storage + kappa2index[kappa];

}

int CheMPS2::Sobject::gIndex() const { return index; }

int CheMPS2::Sobject::gNL(const int ikappa) const{ return sectorNL[ikappa]; }
int CheMPS2::Sobject::gTwoSL(const int ikappa) const{ return sectorTwoSL[ikappa]; }
int CheMPS2::Sobject::gIL(const int ikappa) const{ return sectorIL[ikappa]; }
int CheMPS2::Sobject::gN1(const int ikappa) const{ return sectorN1[ikappa]; }
int CheMPS2::Sobject::gN2(const int ikappa) const{ return sectorN2[ikappa]; }
int CheMPS2::Sobject::gTwoJ(const int ikappa) const{ return sectorTwoJ[ikappa]; }
int CheMPS2::Sobject::gNR(const int ikappa) const{ return sectorNR[ikappa]; }
int CheMPS2::Sobject::gTwoSR(const int ikappa) const{ return sectorTwoSR[ikappa]; }
int CheMPS2::Sobject::gIR(const int ikappa) const{ return sectorIR[ikappa]; }

void CheMPS2::Sobject::Join(TensorT * Tleft, TensorT * Tright){

   //PARALLEL
   #pragma omp parallel for schedule(dynamic)
   for (int ikappa=0; ikappa<nKappa; ikappa++){
      int NM = sectorNL[ikappa] + sectorN1[ikappa];
      int IM = ((sectorN1[ikappa]==1)?(Irreps::directProd(sectorIL[ikappa],Ilocal1)):sectorIL[ikappa]);
      
      int nCases = 1; // number of TwoJM possibilities --> most cases 1, in the case of the next line: 2
      if ((sectorTwoSR[ikappa]==sectorTwoSL[ikappa]) && (sectorN1[ikappa]==1) && (sectorN2[ikappa]==1) && (sectorTwoSR[ikappa]>=1)) nCases = 2;
      
      int * TwoJM = new int[nCases];
      if (nCases==2){
         TwoJM[0] = sectorTwoSL[ikappa]-1;
         TwoJM[1] = sectorTwoSL[ikappa]+1;
      } else { // 1 case
         if (((sectorN1[ikappa])%2)==0) TwoJM[0] = sectorTwoSL[ikappa];
         else {
            if (((sectorN2[ikappa])%2)==0) TwoJM[0] = sectorTwoSR[ikappa];
            else { //N1==1 and N2==1
               if ((sectorTwoSR[ikappa]==sectorTwoSL[ikappa]) && (sectorTwoSR[ikappa] == 0)) TwoJM[0] = 1;
               else { //TwoSR != TwoSL
                  if (sectorTwoSR[ikappa]>sectorTwoSL[ikappa]) TwoJM[0] = sectorTwoSL[ikappa] + 1;
                  else TwoJM[0] = sectorTwoSL[ikappa] - 1;
               }
            }
         }
      }
      
      int dimL = denBK->gCurrentDim(index,  sectorNL[ikappa],sectorTwoSL[ikappa],sectorIL[ikappa]); // dimL>0, checked at creation
      int dimR = denBK->gCurrentDim(index+2,sectorNR[ikappa],sectorTwoSR[ikappa],sectorIR[ikappa]); // dimR>0, checked at creation
      int phase = ((((sectorTwoSL[ikappa] + sectorTwoSR[ikappa] + ((sectorN2[ikappa]==1)?1:0) + ((sectorN1[ikappa]==1)?1:0))/2)%2)!=0)?-1:1;
      
      for (int casenr=0; casenr<nCases; casenr++){
      
         int dimM = denBK->gCurrentDim(index+1,NM,TwoJM[casenr],IM);
         if (dimM>0){
            double * BlockLeft =   Tleft->gStorage(sectorNL[ikappa],sectorTwoSL[ikappa],sectorIL[ikappa],NM,TwoJM[casenr],IM);
            double * BlockRight = Tright->gStorage(NM,TwoJM[casenr],IM,sectorNR[ikappa],sectorTwoSR[ikappa],sectorIR[ikappa]);
         
            double prefactor = gsl_sf_coupling_6j(sectorTwoSL[ikappa],sectorTwoSR[ikappa],sectorTwoJ[ikappa],((sectorN2[ikappa]==1)?1:0),((sectorN1[ikappa]==1)?1:0),TwoJM[casenr]) * sqrt((sectorTwoJ[ikappa]+1.0)*(TwoJM[casenr]+1)) * phase;
         
            double beta = (casenr==0)?0.0:1.0; //first time: set, other times: add.
            char notrans = 'N';
            dgemm_(&notrans,&notrans,&dimL,&dimR,&dimM,&prefactor,BlockLeft,&dimL,BlockRight,&dimM,&beta,storage+kappa2index[ikappa],&dimL);
         } else {
            if (casenr==0){ for (int cnt=0; cnt<dimL*dimR; cnt++){ storage[kappa2index[ikappa]+cnt] = 0.0; }}
         }
         
      }
      
      delete [] TwoJM;
      
   }

}

double CheMPS2::Sobject::Split(TensorT * Tleft, TensorT * Tright, const int virtualdimensionD, const bool movingright, const bool change){

   //Get the number of central sectors
   int nCenterSectors = 0;
   for (int NM=denBK->gNmin(index+1); NM<=denBK->gNmax(index+1); NM++){
      for (int TwoSM=denBK->gTwoSmin(index+1,NM); TwoSM<=denBK->gTwoSmax(index+1,NM); TwoSM+=2){
         for (int IM=0; IM<denBK->getNumberOfIrreps(); IM++){
            int dimM = denBK->gFCIdim(index+1,NM,TwoSM,IM); //FCIdim !! Whether possible hence.
            if (dimM>0){
               nCenterSectors++;
            }
         }
      }
   }
   
   //Get the labels of the central sectors
   int * SplitSectNM = new int[nCenterSectors];
   int * SplitSectTwoJM = new int[nCenterSectors];
   int * SplitSectIM = new int[nCenterSectors];
   nCenterSectors = 0;
   for (int NM=denBK->gNmin(index+1); NM<=denBK->gNmax(index+1); NM++){
      for (int TwoSM=denBK->gTwoSmin(index+1,NM); TwoSM<=denBK->gTwoSmax(index+1,NM); TwoSM+=2){
         for (int IM=0; IM<denBK->getNumberOfIrreps(); IM++){
            int dimM = denBK->gFCIdim(index+1,NM,TwoSM,IM); //FCIdim !! Whether possible hence.
            if (dimM>0){
               SplitSectNM[nCenterSectors] = NM;
               SplitSectTwoJM[nCenterSectors] = TwoSM;
               SplitSectIM[nCenterSectors] = IM;
               nCenterSectors++;
            }
         }
      }
   }
   
   //SVD each sector --> Allocate memory
   double ** Lambdas = new double*[nCenterSectors];
   double ** Us = new double*[nCenterSectors];
   double ** VTs = new double*[nCenterSectors];
   int * CenterDims = new int[nCenterSectors];
   int * DimLtotal = new int[nCenterSectors];
   int * DimRtotal = new int[nCenterSectors];
   
   //PARALLEL
   #pragma omp parallel for schedule(dynamic)
   for (int iCenter=0; iCenter<nCenterSectors; iCenter++){
   
      //Determine left and right dimensions contributing to the center block iCenter
      DimLtotal[iCenter] = 0;
      for (int NL=SplitSectNM[iCenter]-2; NL<=SplitSectNM[iCenter]; NL++){
         for (int TwoSL=SplitSectTwoJM[iCenter]-((NL==SplitSectNM[iCenter]-1)?1:0); TwoSL<SplitSectTwoJM[iCenter]+2; TwoSL+=2){
            if (TwoSL>=0){
               int IL = ((NL==SplitSectNM[iCenter]-1)?Irreps::directProd(Ilocal1,SplitSectIM[iCenter]):SplitSectIM[iCenter]);
               int dimL = denBK->gCurrentDim(index,NL,TwoSL,IL);
               if (dimL>0){
                  DimLtotal[iCenter] += dimL;
               }
            }
         }
      }
      DimRtotal[iCenter] = 0;
      for (int NR=SplitSectNM[iCenter]; NR<=SplitSectNM[iCenter]+2; NR++){
         for (int TwoSR=SplitSectTwoJM[iCenter]-((NR==SplitSectNM[iCenter]+1)?1:0); TwoSR<SplitSectTwoJM[iCenter]+2; TwoSR+=2){
            if (TwoSR>=0){
               int IR = ((NR==SplitSectNM[iCenter]+1)?Irreps::directProd(Ilocal2,SplitSectIM[iCenter]):SplitSectIM[iCenter]);
               int dimR = denBK->gCurrentDim(index+2,NR,TwoSR,IR);
               if (dimR>0){
                  DimRtotal[iCenter] += dimR;
               }
            }
         }
      }
      CenterDims[iCenter] = min(DimLtotal[iCenter],DimRtotal[iCenter]); //CenterDims contains the min. amount

      //Allocate memory to store the SVD in.
      Lambdas[iCenter] = new double[CenterDims[iCenter]];
      Us[iCenter] = new double[DimLtotal[iCenter]*CenterDims[iCenter]];
      VTs[iCenter] = new double[CenterDims[iCenter]*DimRtotal[iCenter]];
      
      //Allocate memory to copy the different parts of the S-object. Use prefactor sqrt((2jR+1)/(2jM+1) * (2jM+1) * (2j+1)) W6J (-1)^(jL+jR+s1+s2) and sum over j.
      if (CenterDims[iCenter]>0){
         double * mem = new double[DimLtotal[iCenter]*DimRtotal[iCenter]];
         for (int cnt=0; cnt<DimLtotal[iCenter]*DimRtotal[iCenter]; cnt++){ mem[cnt] = 0.0; }

         int dimLtotal2 = 0;
         int dimRtotal2;
         for (int NL=SplitSectNM[iCenter]-2; NL<=SplitSectNM[iCenter]; NL++){
            for (int TwoSL=SplitSectTwoJM[iCenter]-((NL==SplitSectNM[iCenter]-1)?1:0); TwoSL<SplitSectTwoJM[iCenter]+2; TwoSL+=2){
               if (TwoSL>=0){
                  int IL = ((NL==SplitSectNM[iCenter]-1)?Irreps::directProd(Ilocal1,SplitSectIM[iCenter]):SplitSectIM[iCenter]);
                  int dimL = denBK->gCurrentDim(index,NL,TwoSL,IL);
                  if (dimL>0){
                     dimRtotal2 = 0;
                     for (int NR=SplitSectNM[iCenter]; NR<=SplitSectNM[iCenter]+2; NR++){
                        for (int TwoSR=SplitSectTwoJM[iCenter]-((NR==SplitSectNM[iCenter]+1)?1:0); TwoSR<SplitSectTwoJM[iCenter]+2; TwoSR+=2){
                           if (TwoSR>=0){
                              int IR = ((NR==SplitSectNM[iCenter]+1)?Irreps::directProd(Ilocal2,SplitSectIM[iCenter]):SplitSectIM[iCenter]);
                              int dimR = denBK->gCurrentDim(index+2,NR,TwoSR,IR);
                              if (dimR>0){
                                 //Loop over contributing TwoJ's
                                 int TwoS2 = (NR==SplitSectNM[iCenter]+1)?1:0;
                                 int TwoS1 = (NL==SplitSectNM[iCenter]-1)?1:0;
                                 int fase = ((((TwoSL + TwoSR + TwoS1 + TwoS2)/2)%2)!=0)?-1:1;
                                 for (int TwoJ = max(abs(TwoSR-TwoSL),abs(TwoS2-TwoS1)); TwoJ<=min(TwoS1+TwoS2,TwoSL+TwoSR); TwoJ+=2){
                                    //calc prefactor
                                    double prefact = gsl_sf_coupling_6j(TwoSL,TwoSR,TwoJ,TwoS2,TwoS1,SplitSectTwoJM[iCenter]) * sqrt((TwoJ+1.0)*(TwoSR+1)) * fase;
                              
                                    //add them to mem.
                                    double * Block = gStorage(NL,TwoSL,IL,SplitSectNM[iCenter]-NL,NR-SplitSectNM[iCenter],TwoJ,NR,TwoSR,IR);
                                    for (int l=0; l<dimL; l++){
                                       for (int r=0; r<dimR; r++){
                                          mem[dimLtotal2 + l + DimLtotal[iCenter] * (dimRtotal2 + r)] += prefact * Block[l + dimL * r]; //+= because several TwoJ
                                       }
                                    }
                                 }
                                 dimRtotal2 += dimR;
                              }
                           }
                        }
                     }
                     dimLtotal2 += dimL;
                  }
               }
            }
         }
      
         //Now mem contains sqrt((2jR+1)/(2jM+1)) * (TT)^{jM nM IM) --> SVD per central symmetry
         char jobz = 'S'; //M x min(M,N) in U and min(M,N) x N in VT
         int lwork = 3*CenterDims[iCenter] + max(max(DimLtotal[iCenter],DimRtotal[iCenter]),4*CenterDims[iCenter]*(CenterDims[iCenter]+1));
         double * work = new double[lwork];
         int * iwork = new int[8*CenterDims[iCenter]];
         int info;

         //dgesdd is not thread-safe in every implementation (intel MKL is safe, Atlas is not safe)
         #pragma omp critical
         dgesdd_(&jobz, DimLtotal + iCenter, DimRtotal + iCenter, mem, DimLtotal + iCenter, Lambdas[iCenter], Us[iCenter], DimLtotal + iCenter, VTs[iCenter], CenterDims + iCenter, work, &lwork, iwork, &info);

         delete [] work;
         delete [] iwork;
         delete [] mem;
      }
   }
   
   double discardedWeight = 0.0; //Only if change==true; will the discardedWeight be meaningful and different from zero.

   //If change: determine new virtual dimensions.
   if (change){
   
      int * NewDims = new int[nCenterSectors];
      //First determine the total number of singular values
      int totalDimSVD = 0;
      int totalDimMPS = 0;
      for (int iCenter=0; iCenter<nCenterSectors; iCenter++){
         NewDims[iCenter] = CenterDims[iCenter];
         totalDimSVD += NewDims[iCenter];
         totalDimMPS += denBK->gCurrentDim(index+1,SplitSectNM[iCenter],SplitSectTwoJM[iCenter],SplitSectIM[iCenter]);
      }
      
      //If larger then the required virtualdimensionD, new virtual dimensions will be set in NewDims.
      if (totalDimSVD>virtualdimensionD){
         //Copy them all in 1 array
         double * values = new double[totalDimSVD];
         totalDimSVD = 0;
         int inc = 1;
         for (int iCenter=0; iCenter<nCenterSectors; iCenter++){
            if (NewDims[iCenter]>0){
               dcopy_(NewDims + iCenter, Lambdas[iCenter], &inc, values + totalDimSVD, &inc);
               totalDimSVD += NewDims[iCenter];
            }
         }
         
         //Sort them in decreasing order
         char ID = 'D';
         int info;
         dlasrt_(&ID,&totalDimSVD,values,&info); //quicksort
      
         //The D+1'th value becomes the lower bound Schmidt value. Every value smaller than or equal to the D+1'th value is thrown out (hence Dactual <= Ddesired).
         const double lowerBound = values[virtualdimensionD];
         for (int iCenter=0; iCenter<nCenterSectors; iCenter++){
            for (int cnt=0; cnt<NewDims[iCenter]; cnt++){
               if (Lambdas[iCenter][cnt]<=lowerBound) NewDims[iCenter] = cnt;
            }
         }
         
         //Discarded weight
         double totalSum = 0.0;
         double discardedSum = 0.0;
         for (int iCenter=0; iCenter<nCenterSectors; iCenter++){
            for (int iLocal=0; iLocal<CenterDims[iCenter]; iLocal++){
               double temp = (SplitSectTwoJM[iCenter]+1) * Lambdas[iCenter][iLocal] * Lambdas[iCenter][iLocal];
               totalSum += temp;
               if (Lambdas[iCenter][iLocal] <= lowerBound){ discardedSum += temp; }
            }
         }
         discardedWeight = discardedSum / totalSum;
         
         //Clean-up
         delete [] values;
      }
      
      //Set NewDims only if relevant --> if totalDimSVD == totalDimMPS <= virtualdimensionD, no new symm sect virt D.
      if ((totalDimSVD>virtualdimensionD) || (totalDimSVD!=totalDimMPS)){
         for (int iCenter=0; iCenter<nCenterSectors; iCenter++){
            denBK->SetDim(index+1,SplitSectNM[iCenter],SplitSectTwoJM[iCenter],SplitSectIM[iCenter],NewDims[iCenter]);
         }
         Tleft ->Reset();
         Tright->Reset();
      }
      
      delete [] NewDims;
      
   }
   
   //copy first gCurrentDimM per central symmetry sector to the relevant parts.
   //PARALLEL
   #pragma omp parallel for schedule(dynamic)
   for (int iCenter=0; iCenter<nCenterSectors; iCenter++){
      int dimM = denBK->gCurrentDim(index+1,SplitSectNM[iCenter],SplitSectTwoJM[iCenter],SplitSectIM[iCenter]);
      if (dimM>0){
         //U-part: copy
         int dimLtotal2 = 0;
         for (int NL=SplitSectNM[iCenter]-2; NL<=SplitSectNM[iCenter]; NL++){
            for (int TwoSL=SplitSectTwoJM[iCenter]-((NL==SplitSectNM[iCenter]-1)?1:0); TwoSL<SplitSectTwoJM[iCenter]+2; TwoSL+=2){
               if (TwoSL>=0){
                  int IL = ((NL==SplitSectNM[iCenter]-1)?Irreps::directProd(Ilocal1,SplitSectIM[iCenter]):SplitSectIM[iCenter]);
                  int dimL = denBK->gCurrentDim(index,NL,TwoSL,IL);
                  if (dimL>0){
                     double * TleftBlock = Tleft->gStorage(NL,TwoSL,IL,SplitSectNM[iCenter],SplitSectTwoJM[iCenter],SplitSectIM[iCenter]);
                     for (int r=0; r<min(dimM,CenterDims[iCenter]); r++){
                        double fact = (movingright)?1.0:Lambdas[iCenter][r];
                        for (int l=0; l<dimL; l++){
                           TleftBlock[l + dimL * r] = fact * Us[iCenter][dimLtotal2 + l + DimLtotal[iCenter]*r]; //l 0-->dimL and r 0-->dimM
                        }
                     }
                     for (int r=min(dimM,CenterDims[iCenter]); r<dimM; r++){
                        for (int l=0; l<dimL; l++){
                           TleftBlock[l + dimL * r] = 0.0;
                        }
                     }
                     dimLtotal2 += dimL;
                  }
               }
            }
         }
         
         //VT-part: copy
         int dimRtotal2 = 0;
         for (int NR=SplitSectNM[iCenter]; NR<=SplitSectNM[iCenter]+2; NR++){
            for (int TwoSR=SplitSectTwoJM[iCenter]-((NR==SplitSectNM[iCenter]+1)?1:0); TwoSR<SplitSectTwoJM[iCenter]+2; TwoSR+=2){
               if (TwoSR>=0){
                  int IR = ((NR==SplitSectNM[iCenter]+1)?Irreps::directProd(Ilocal2,SplitSectIM[iCenter]):SplitSectIM[iCenter]);
                  int dimR = denBK->gCurrentDim(index+2,NR,TwoSR,IR);
                  if (dimR>0){
                     double * TrightBlock = Tright->gStorage(SplitSectNM[iCenter],SplitSectTwoJM[iCenter],SplitSectIM[iCenter],NR,TwoSR,IR);
                     for (int l=0; l<min(dimM,CenterDims[iCenter]); l++){
                        double fact = ((movingright)?Lambdas[iCenter][l]:1.0) * sqrt((SplitSectTwoJM[iCenter] + 1.0)/(TwoSR + 1.0));
                        for (int r=0; r<dimR; r++){
                           TrightBlock[l + dimM * r] = fact * VTs[iCenter][l + CenterDims[iCenter] * (dimRtotal2 + r)]; //l 0-->dimM and r 0-->dimR
                        }
                     }
                     for (int l=min(dimM,CenterDims[iCenter]); l<dimM; l++){
                        for (int r=0; r<dimR; r++){
                           TrightBlock[l + dimM * r] = 0.0;
                        }
                     }
                     dimRtotal2 += dimR;
                  }
               }
            }
         }
      }
   }
   
   //Clean up
   delete [] SplitSectNM;
   delete [] SplitSectTwoJM;
   delete [] SplitSectIM;
   for (int iCenter=0; iCenter<nCenterSectors; iCenter++){
      delete [] Us[iCenter];
      delete [] Lambdas[iCenter];
      delete [] VTs[iCenter];
   }
   delete [] Us;
   delete [] Lambdas;
   delete [] VTs;
   delete [] CenterDims;
   delete [] DimLtotal;
   delete [] DimRtotal;
   
   return discardedWeight;
   
}

void CheMPS2::Sobject::prog2symm(){

   //PARALLEL
   #pragma omp parallel for schedule(dynamic)
   for (int ikappa=0; ikappa<nKappa; ikappa++){
   
      int dim = kappa2index[ikappa+1]-kappa2index[ikappa];
      double alpha = sqrt(sectorTwoSR[ikappa]+1.0);
      int inc = 1;
      dscal_(&dim,&alpha,storage+kappa2index[ikappa],&inc);
   
   }

}
      
void CheMPS2::Sobject::symm2prog(){

   //PARALLEL
   #pragma omp parallel for schedule(dynamic)
   for (int ikappa=0; ikappa<nKappa; ikappa++){
   
      int dim = kappa2index[ikappa+1]-kappa2index[ikappa];
      double alpha = 1.0/sqrt(sectorTwoSR[ikappa]+1.0);
      int inc = 1;
      dscal_(&dim,&alpha,storage+kappa2index[ikappa],&inc);
   
   }

}

void CheMPS2::Sobject::addNoise(const double NoiseLevel){
   
   for (int cnt=0; cnt<gKappa2index(gNKappa()); cnt++){
      double RN = ((double) rand())/RAND_MAX - 0.5;
      gStorage()[cnt] += RN * NoiseLevel;
   }

}



