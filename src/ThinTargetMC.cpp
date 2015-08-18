
#include <cstdlib>
#include <iostream>
#include "ThinTargetMC.h"


namespace NeutrinoFluxReweight{ 
  
  ThinTargetMC* ThinTargetMC::instance = 0;
  
  ThinTargetMC::ThinTargetMC(){
    const char* ppfxDir = getenv("PPFX_DIR");
    char dirData[400]= "data"; 
    sprintf(dirData,"%s/data",ppfxDir);
    spart_prod.push_back("pip");
    spart_prod.push_back("pim");
    spart_prod.push_back("kap");
    spart_prod.push_back("kam");
    spart_prod.push_back("prt");
    spart_prod.push_back("neu");
    mom_inc.push_back(12);mom_inc.push_back(20);
    mom_inc.push_back(31);mom_inc.push_back(40);
    mom_inc.push_back(50);mom_inc.push_back(60);
    mom_inc.push_back(70);mom_inc.push_back(80);
    mom_inc.push_back(90);mom_inc.push_back(100);
    mom_inc.push_back(110);mom_inc.push_back(120);
    
    for(int i=0;i<spart_prod.size();i++){
      if(i<spart_prod.size()-1){
	fpC_x[i] = new TFile(Form("%s/MC/FTFP/invxs_%s_FTFP_BERT.root",dirData,spart_prod[i].c_str()),"read");
	for(int j=0;j<mom_inc.size();j++)vpC_x[i].push_back((TH2D*)fpC_x[i]->Get(Form("xF_pT_%dGeV",mom_inc[j])));
      }
      else if(i==spart_prod.size()-1){
	fpC_x[i] = new TFile(Form("%s/MC/FTFP/yield_%s_FTFP_BERT.root",dirData,spart_prod[i].c_str()),"read");
	for(int j=0;j<mom_inc.size();j++)vpC_n.push_back((TH1D*)fpC_x[i]->Get(Form("dndxf_%dGeV",mom_inc[j])));
      }
    }
    
    //QEL fraction:
    spart_qe_corr.push_back("prt");
    spart_qe_corr.push_back("neu");
    for(int i=0;i<spart_qe_corr.size();i++){
      if(i<spart_qe_corr.size()-1){
	fqe_corr[i] = new TFile(Form("%s/MC/FTFP/invxs_qe_corr_%s.root",dirData,spart_qe_corr[i].c_str()),"read");
	for(int j=0;j<mom_inc.size();j++)vqe_corr_p.push_back((TH2D*)fqe_corr[i]->Get(Form("frac_prod_xF_pT_%dGeV",mom_inc[j])));
      }
      else if(i==spart_qe_corr.size()-1){
	fqe_corr[i] = new TFile(Form("%s/MC/FTFP/yield_qe_corr_%s.root",dirData,spart_qe_corr[i].c_str()),"read");
	for(int j=0;j<mom_inc.size();j++)vqe_corr_n.push_back((TH1D*)fqe_corr[i]->Get(Form("frac_prod_xf_%dGeV",mom_inc[j])));
      }      
    }
    
  }
  
  double ThinTargetMC::getMCval_pC_X(double incP, double xf,double pt, int pdgcode){

    //check:
    if(incP<12)return -1;    
    if(pdgcode!=211 && pdgcode!=-211 && pdgcode!=321 && pdgcode!=-321 && pdgcode!=2212 && pdgcode!=2112)return -1;    
    //idx:
    int idx_part = -1;
    int idx_qe_corr = -1;
    int idx_lowp = -1;
    int idx_hip  = -1;
    for(int i=0;i<mom_inc.size()-1;i++){
      if(incP>=double(mom_inc[i]) && incP<=double(mom_inc[i+1])){
	idx_lowp=i;
	idx_hip =i+1;
      }
    }
    if(idx_lowp==-1){
      idx_lowp = mom_inc.size()-2;
      idx_hip  = mom_inc.size()-1;
    }
    if(pdgcode ==  211)idx_part=0;
    if(pdgcode == -211)idx_part=1;
    if(pdgcode ==  321)idx_part=2;
    if(pdgcode == -321)idx_part=3;
    if(pdgcode == 2212){
      idx_part    =4;
      idx_qe_corr = 0;
    }
     if(pdgcode == 2112){
      idx_part    = 5;
      idx_qe_corr = 1;
    }
 
     double mcval   = 0.0;
     double qe_corr = 1.0;

     if(idx_part<=4){
       int binp   = vpC_x[idx_part][idx_lowp]->FindBin(xf,pt);
       double mclow = vpC_x[idx_part][idx_lowp]->GetBinContent(binp);
       double mchi  = vpC_x[idx_part][idx_hip]->GetBinContent(binp);
       mcval = mclow + (incP-double(mom_inc[idx_lowp]))*(mchi-mclow)/(double(mom_inc[idx_hip])-double(mom_inc[idx_lowp]));
       if(idx_qe_corr==0){
	 int binqe       = vqe_corr_p[idx_lowp]->FindBin(xf,pt);
	 double qelow    = vqe_corr_p[idx_lowp]->GetBinContent(binqe);
	 double qehi     = vqe_corr_p[idx_hip]->GetBinContent(binqe);
	 qe_corr  = qelow + (incP-double(mom_inc[idx_lowp]))*(qehi-qelow)/(double(mom_inc[idx_hip])-double(mom_inc[idx_lowp]));
       }
     }
     else if(idx_part==5){
       int binp     = vpC_n[idx_lowp]->FindBin(xf);
       double mclow = vpC_n[idx_lowp]->GetBinContent(binp);
       double mchi  = vpC_n[idx_hip]->GetBinContent(binp);
       mcval = mclow + (incP-double(mom_inc[idx_lowp]))*(mchi-mclow)/(double(mom_inc[idx_hip])-double(mom_inc[idx_lowp]));
       int binqe    = vqe_corr_n[idx_lowp]->FindBin(xf);
       double qelow = vqe_corr_n[idx_lowp]->GetBinContent(binqe);
       double qehi  = vqe_corr_n[idx_hip]->GetBinContent(binqe);
       qe_corr  = qelow + (incP-double(mom_inc[idx_lowp]))*(qehi-qelow)/(double(mom_inc[idx_hip])-double(mom_inc[idx_lowp]));
     }
     
     mcval /=qe_corr;
     
    //check:
    if(mcval<1.e-12 || mcval!=mcval)return -1;
    
    return mcval;
    
  }
  
  
  ThinTargetMC* ThinTargetMC::getInstance(){
    if (instance == 0) instance = new ThinTargetMC;
    return instance;
  }
  
}
