#include <iostream>

#include <TH1F.h>
#include <TFile.h>
#include <TROOT.h>
#include <TString.h>
#include <TSystem.h>
#include <Rtypes.h>

#include <TMath.h>
#include <TAxis.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TAttLine.h>
#include <TPaveText.h>

#include "$CMSSW_BASE/src/HiggsAnalysis/HiggsToTauTau/interface/HttStyles.h"
#include "$CMSSW_BASE/src/HiggsAnalysis/HiggsToTauTau/src/HttStyles.cc"

static const float SIGNAL_SCALE = 1.;

$DEFINE_ASIMOV
$DEFINE_DROP_SIGNAL
$DEFINE_EXTRA_SAMPLES
$DEFINE_MSSM

/**
   \class   HTT_MT_X_template HTT_MT_X_template.C "HiggsAnalysis/HiggsToTauTau/postfit/tamplates/HTT_MT_X_template.C"

   \brief   macro template to create pre-/postfit plots of the inputs to the limit calculation

   This is a macro template to create pre-/postfit plots of the inputs tp the limit calculation.
   This macro is picked up from the produce_macros.py script in the HiggsAnalysis/HiggsToTauTau/postfit
   directory. The key words in the replace(...) function are replaced by proper values that have
   been calculated from the uncertainties as picked up from the datacards in the postfit/datacards 
   directory of the package and the pulls of the fit as picked up from the maximum likelihood fit 
   results file in the postfit/fitresults directory of the package.

   
   In the headline of the main macro the keywords HTT_MT_X, $HISTFILE and $CATEGORY will be 
   replaced by proper names according to the inputfile and event category, for which the polts 
   are supposed to be made.
*/

static const bool BLIND_DATA = true; //false;
static const bool FULLPLOTS = false; //true;
float blinding_SM(float mass){ return (100<mass && mass<150); }
float blinding_MSSM(float mass){ return (100<mass); }
float maximum(TH1F* h, bool LOG=false){
  if(LOG){
    if(h->GetMaximum()>1000){ return 1000.*TMath::Nint(500*h->GetMaximum()/1000.); }
    if(h->GetMaximum()>  10){ return   10.*TMath::Nint( 50*h->GetMaximum()/  10.); }
    return 50*h->GetMaximum(); 
  }
  else{
    if(h->GetMaximum()>  12){ return 10.*TMath::Nint((1.3*h->GetMaximum()/10.)); }
    if(h->GetMaximum()> 1.2){ return TMath::Nint((1.6*h->GetMaximum())); }
    return 1.6*h->GetMaximum(); 
  }
}


TH1F* refill(TH1F* hin, const char* sample, bool data=false)
/*
  refill histograms, divide by bin width and correct bin errors. For MC histograms set 
  bin errors to zero.
*/
{
  if(hin==0){
    std::cout << "hist not found: " << sample << " -- close here" << std::endl;
    exit(1);  
  }
  TH1F* hout = (TH1F*)hin->Clone(); hout->Clear();
  for(int i=0; i<hout->GetNbinsX(); ++i){
    if(data){
#if defined MSSM
      hout->SetBinContent(i+1, BLIND_DATA && blinding_MSSM(hin->GetBinCenter(i+1)) ? 0. : hin->GetBinContent(i+1)/hin->GetBinWidth(i+1));
      hout->SetBinError  (i+1, BLIND_DATA && blinding_MSSM(hin->GetBinCenter(i+1)) ? 0. : hin->GetBinError(i+1)/hin->GetBinWidth(i+1));
#else
      hout->SetBinContent(i+1, BLIND_DATA && blinding_SM(hin->GetBinCenter(i+1)) ? 0. : hin->GetBinContent(i+1)/hin->GetBinWidth(i+1));
      hout->SetBinError  (i+1, BLIND_DATA && blinding_SM(hin->GetBinCenter(i+1)) ? 0. : hin->GetBinError(i+1)/hin->GetBinWidth(i+1));
#endif
    }
    else{
      hout->SetBinContent(i+1, hin->GetBinContent(i+1)/hin->GetBinWidth(i+1));
      hout->SetBinError(i+1, 0.);
    }
  }
  return hout;
}

void rescale(TH1F* hin, unsigned int idx)
/*
  rescale histograms according to fit results. The keywords like $Ztt will be replaced 
  by a cout statement and a scaling command.
*/
{
  switch(idx){
  case  1: // ZTT 
  $ZTT
  case  2: // TT
  $TT
  case  3: // W  [EWK1]
  $W
#if defined EXTRA_SAMPLES
  case  4: // ZJ [EWK2]
  $ZJ
  case  5: // ZL [EWK3]
  $ZL
#else
  case  4: // ZLL [EWK2]
  $ZLL
#endif
  case  6: // VV [EWK ]
  $VV
  case  7: // QCD
  $QCD
#if defined MSSM
  case  8: // ggH
  ${MSSM}ggH$MA
  case  9: // bbH
  ${MSSM}bbH$MA
#else
#ifndef DROP_SIGNAL
  case  8: // ggH
  ${SM}ggH125
  case  9: // qqH
  ${SM}qqH125
  case 10: // VH
  ${SM}VH125
#endif
#endif
  default :
    std::cout << "error histograms not known?!?" << std::endl;
  }
}

void 
HTT_MT_X(bool scaled=true, bool log=true, float min=0.1, float max=-1., string inputfile="root/$HISTFILE", const char* directory="muTau_$CATEGORY")
{
  // defining the common canvas, axes pad styles
  SetStyle(); gStyle->SetLineStyleString(11,"20 10");

  // determine category tag
  const char* category_extra = "";
  if(std::string(directory) == std::string("muTau_0jet_low"  )){ category_extra = "0 jet, low p_{T}";  }
  if(std::string(directory) == std::string("muTau_0jet_high" )){ category_extra = "0 jet, high p_{T}"; }
  if(std::string(directory) == std::string("muTau_1jet_low"  )){ category_extra = "1 jet, low p_{T}";  }
  if(std::string(directory) == std::string("muTau_1jet_high" )){ category_extra = "1 jet, high p_{T}"; }
  if(std::string(directory) == std::string("muTau_vbf"       )){ category_extra = "2 jet (VBF)";       }
  if(std::string(directory) == std::string("muTau_nobtag"    )){ category_extra = "No B-Tag";          }
  if(std::string(directory) == std::string("muTau_btag"      )){ category_extra = "B-Tag";             }

  const char* dataset;
  if(std::string(inputfile).find("7TeV")!=std::string::npos){dataset = "CMS Preliminary,  H#rightarrow#tau#tau, 4.9 fb^{-1} at 7 TeV";}
  if(std::string(inputfile).find("8TeV")!=std::string::npos){dataset = "CMS Preliminary,  H#rightarrow#tau#tau, 19.8 fb^{-1} at 8 TeV";}
#ifdef MSSM
  if(std::string(inputfile).find("8TeV")!=std::string::npos){dataset = "CMS Preliminary,  H#rightarrow#tau#tau, 19.8 fb^{-1} at 8 TeV";}
#endif
 
  // open example histogram file
  TFile* input = new TFile(inputfile.c_str());
#ifdef MSSM
  TFile* input2 = new TFile((inputfile+"_$MA_$TANB").c_str());
#endif
  TH1F* Fakes  = refill((TH1F*)input->Get(TString::Format("%s/QCD"   , directory)), "QCD"); InitHist(Fakes, "", "", kMagenta-10, 1001);
  TH1F* EWK1   = refill((TH1F*)input->Get(TString::Format("%s/W"     , directory)), "W"  ); InitHist(EWK1 , "", "", kRed    + 2, 1001);
#ifdef EXTRA_SAMPLES
  TH1F* EWK2   = refill((TH1F*)input->Get(TString::Format("%s/ZJ"    , directory)), "ZJ" ); InitHist(EWK2 , "", "", kRed    + 2, 1001);
  TH1F* EWK3   = refill((TH1F*)input->Get(TString::Format("%s/ZL"    , directory)), "ZL" ); InitHist(EWK3 , "", "", kRed    + 2, 1001);
#else
  TH1F* EWK2   = refill((TH1F*)input->Get(TString::Format("%s/ZLL"   , directory)), "ZLL"); InitHist(EWK2 , "", "", kRed    + 2, 1001);
#endif
  TH1F* EWK    = refill((TH1F*)input->Get(TString::Format("%s/VV"    , directory)), "VV" ); InitHist(EWK  , "", "", kRed    + 2, 1001);
  TH1F* ttbar  = refill((TH1F*)input->Get(TString::Format("%s/TT"    , directory)), "TT" ); InitHist(ttbar, "", "", kBlue   - 8, 1001);
  TH1F* Ztt    = refill((TH1F*)input->Get(TString::Format("%s/ZTT"   , directory)), "ZTT"); InitHist(Ztt  , "", "", kOrange - 4, 1001);
#ifdef MSSM
  // float ggHScale = 1., bbHScale = 1.;
//   ggHScale = ($MSSM_SIGNAL_ggH_xseff_A + $MSSM_SIGNAL_ggH_xseff_hH);
//   bbHScale = ($MSSM_SIGNAL_bbH_xseff_A + $MSSM_SIGNAL_bbH_xseff_hH);
  TH1F* ggH    = refill((TH1F*)input2->Get(TString::Format("%s/ggH$MA", directory)), "ggH"); InitSignal(ggH); ggH->Scale($TANB); //ggH ->Scale(ggHScale);
  TH1F* bbH    = refill((TH1F*)input2->Get(TString::Format("%s/bbH$MA", directory)), "bbH"); InitSignal(bbH); bbH->Scale($TANB); //bbH ->Scale(bbHScale);
#else
#ifndef DROP_SIGNAL
  TH1F* ggH    = refill((TH1F*)input->Get(TString::Format("%s/ggH125", directory)), "ggH"); InitSignal(ggH); ggH ->Scale(SIGNAL_SCALE);
  TH1F* qqH    = refill((TH1F*)input->Get(TString::Format("%s/qqH125", directory)), "qqH"); InitSignal(qqH); qqH ->Scale(SIGNAL_SCALE);
  TH1F* VH     = refill((TH1F*)input->Get(TString::Format("%s/VH125" , directory)), "VH" ); InitSignal(VH ); VH  ->Scale(SIGNAL_SCALE);
#endif
#endif
#ifdef ASIMOV
  TH1F* data   = refill((TH1F*)input->Get(TString::Format("%s/data_obs_asimov", directory)), "data", true);
#else
  TH1F* data   = refill((TH1F*)input->Get(TString::Format("%s/data_obs", directory)), "data", true);
#endif
  InitHist(data, "#bf{m_{#tau#tau} [GeV]}", "#bf{dN/dm_{#tau#tau} [1/GeV]}"); InitData(data);

  TH1F* ref=(TH1F*)Fakes->Clone("ref");
  ref->Add(EWK1 );
  ref->Add(EWK2 );
#ifdef EXTRA_SAMPLES
  ref->Add(EWK3 );
#endif
  ref->Add(EWK  );
  ref->Add(ttbar);
  ref->Add(Ztt  );

  double unscaled[7];
  unscaled[0] = Fakes->Integral();
  unscaled[1] = EWK  ->Integral();
  unscaled[1]+= EWK1 ->Integral();
  unscaled[1]+= EWK2 ->Integral();
#ifdef EXTRA_SAMPLES
  unscaled[1]+= EWK3 ->Integral();
#endif
  unscaled[2] = ttbar->Integral();
  unscaled[3] = Ztt  ->Integral();
#ifdef MSSM
  unscaled[4] = ggH  ->Integral();
  unscaled[5] = bbH  ->Integral();
  unscaled[6] = 0;
#else
#ifndef DROP_SIGNAL
  unscaled[4] = ggH  ->Integral();
  unscaled[5] = qqH  ->Integral();
  unscaled[6] = VH   ->Integral();
#endif
#endif

  if(scaled){
    rescale(Fakes, 7); 
    rescale(EWK1 , 3); 
    rescale(EWK2 , 4); 
#ifdef EXTRA_SAMPLES
    rescale(EWK3 , 5);
#endif 
    rescale(EWK  , 6); 
    rescale(ttbar, 2); 
    rescale(Ztt  , 1);
#ifdef MSSM
    rescale(ggH  , 8); 
    rescale(bbH  , 9);  
#else
#ifndef DROP_SIGNAL
    rescale(ggH  , 8); 
    rescale(qqH  , 9);  
    rescale(VH   ,10);  
#endif
#endif
  }

  TH1F* scales[7];
  scales[0] = new TH1F("scales-Fakes", "", 7, 0, 7);
  scales[0]->SetBinContent(1, unscaled[0]>0 ? (Fakes->Integral()/unscaled[0]-1.) : 0.);
  scales[1] = new TH1F("scales-EWK"  , "", 7, 0, 7);
  scales[1]->SetBinContent(2, unscaled[1]>0 ? ((EWK  ->Integral()
					       +EWK1 ->Integral()
					       +EWK2 ->Integral()
#ifdef EXTRA_SAMPLES
					       +EWK3 ->Integral()
#endif
						)/unscaled[1]-1.) : 0.);
  scales[2] = new TH1F("scales-ttbar", "", 7, 0, 7);
  scales[2]->SetBinContent(3, unscaled[2]>0 ? (ttbar->Integral()/unscaled[2]-1.) : 0.);
  scales[3] = new TH1F("scales-Ztt"  , "", 7, 0, 7);
  scales[3]->SetBinContent(4, unscaled[3]>0 ? (Ztt  ->Integral()/unscaled[3]-1.) : 0.);
#ifdef MSSM
  scales[4] = new TH1F("scales-ggH"  , "", 7, 0, 7);
  scales[4]->SetBinContent(5, unscaled[4]>0 ? (ggH  ->Integral()/unscaled[4]-1.) : 0.);
  scales[5] = new TH1F("scales-bbH"  , "", 7, 0, 7);
  scales[5]->SetBinContent(6, unscaled[5]>0 ? (bbH  ->Integral()/unscaled[5]-1.) : 0.);
  scales[6] = new TH1F("scales-NONE" , "", 7, 0, 7);
  scales[6]->SetBinContent(7, 0.);
#else
#ifndef DROP_SIGNAL
  scales[4] = new TH1F("scales-ggH"  , "", 7, 0, 7);
  scales[4]->SetBinContent(5, unscaled[4]>0 ? (ggH  ->Integral()/unscaled[4]-1.) : 0.);
  scales[5] = new TH1F("scales-qqH"  , "", 7, 0, 7);
  scales[5]->SetBinContent(6, unscaled[5]>0 ? (qqH  ->Integral()/unscaled[5]-1.) : 0.);
  scales[6] = new TH1F("scales-VH"   , "", 7, 0, 7);
  scales[6]->SetBinContent(7, unscaled[6]>0 ? (VH   ->Integral()/unscaled[6]-1.) : 0.);
#endif
#endif

  EWK1 ->Add(Fakes);
  EWK2 ->Add(EWK1 );
#ifdef EXTRA_SAMPLES
  EWK3 ->Add(EWK2 );
  EWK  ->Add(EWK3 );
#else
  EWK  ->Add(EWK2 );
#endif
  ttbar->Add(EWK  );
  Ztt  ->Add(ttbar);
  if(log){
#ifdef MSSM
    ggH  ->Add(bbH);
#else
#ifndef DROP_SIGNAL
    qqH  ->Add(VH );
    ggH  ->Add(qqH);
#endif
#endif
  }
  else{
#ifdef MSSM    
    bbH  ->Add(Ztt);
    ggH  ->Add(bbH);
#else
#ifndef DROP_SIGNAL
    VH   ->Add(Ztt);
    qqH  ->Add(VH );
    ggH  ->Add(qqH);
#endif
#endif
  }

  /*
    Mass plot before and after fit
  */
  TCanvas *canv = MakeCanvas("canv", "histograms", 600, 600);

  canv->cd();
  if(log){ canv->SetLogy(1); }
#if defined MSSM
  if(!log){ data->GetXaxis()->SetRange(0, data->FindBin(350)); } else{ data->GetXaxis()->SetRange(0, data->FindBin(1000)); };
#else
  data->GetXaxis()->SetRange(0, data->FindBin(350));
#endif
  data->SetNdivisions(505);
  data->SetMinimum(min);
  data->SetMaximum(max>0 ? max : std::max(maximum(data, log), maximum(Ztt, log)));
  data->Draw("e");

  TH1F* errorBand = (TH1F*)Ztt ->Clone();
  errorBand  ->SetMarkerSize(0);
  errorBand  ->SetFillColor(1);
  errorBand  ->SetFillStyle(3013);
  errorBand  ->SetLineWidth(1);
  for(int idx=0; idx<errorBand->GetNbinsX(); ++idx){
    if(errorBand->GetBinContent(idx)>0){
      std::cout << "Uncertainties on summed background samples: " << errorBand->GetBinError(idx)/errorBand->GetBinContent(idx) << std::endl;
      break;
    }
  }
  if(log){
    Ztt  ->Draw("histsame");
    ttbar->Draw("histsame");
    EWK  ->Draw("histsame");
    Fakes->Draw("histsame");
    $DRAW_ERROR
#ifndef DROP_SIGNAL
    ggH  ->Draw("histsame");
#endif
  }
  else{
#ifndef DROP_SIGNAL
    ggH  ->Draw("histsame");
#endif
    Ztt  ->Draw("histsame");
    ttbar->Draw("histsame");
    EWK  ->Draw("histsame");
    Fakes->Draw("histsame");
    $DRAW_ERROR
  }
  data->Draw("esame");
  canv->RedrawAxis();

  //CMSPrelim(dataset, "#tau_{#mu}#tau_{h}", 0.17, 0.835);
  CMSPrelim(dataset, "", 0.16, 0.835);  
  TPaveText* chan     = new TPaveText(0.20, 0.74+0.061, 0.32, 0.74+0.161, "NDC");
  chan->SetBorderSize(   0 );
  chan->SetFillStyle(    0 );
  chan->SetTextAlign(   12 );
  chan->SetTextSize ( 0.05 );
  chan->SetTextColor(    1 );
  chan->SetTextFont (   62 );
  chan->AddText("#mu#tau_{h}");
  chan->Draw();

  TPaveText* cat      = new TPaveText(0.20, 0.68+0.061, 0.32, 0.68+0.161, "NDC");
  cat->SetBorderSize(   0 );
  cat->SetFillStyle(    0 );
  cat->SetTextAlign(   12 );
  cat->SetTextSize ( 0.05 );
  cat->SetTextColor(    1 );
  cat->SetTextFont (   62 );
  cat->AddText(category_extra);
  cat->Draw();

#ifdef MSSM
  TPaveText* massA      = new TPaveText(0.75, 0.48+0.061, 0.85, 0.48+0.161, "NDC");
  massA->SetBorderSize(   0 );
  massA->SetFillStyle(    0 );
  massA->SetTextAlign(   12 );
  massA->SetTextSize ( 0.03 );
  massA->SetTextColor(    1 );
  massA->SetTextFont (   62 );
  massA->AddText("m_{A}=$MA GeV");
  massA->Draw();

  TPaveText* tanb      = new TPaveText(0.75, 0.44+0.061, 0.85, 0.44+0.161, "NDC");
  tanb->SetBorderSize(   0 );
  tanb->SetFillStyle(    0 );
  tanb->SetTextAlign(   12 );
  tanb->SetTextSize ( 0.03 );
  tanb->SetTextColor(    1 );
  tanb->SetTextFont (   62 );
  tanb->AddText("tan#beta=$TANB");
  tanb->Draw();

  TPaveText* scen      = new TPaveText(0.75, 0.40+0.061, 0.85, 0.40+0.161, "NDC");
  scen->SetBorderSize(   0 );
  scen->SetFillStyle(    0 );
  scen->SetTextAlign(   12 );
  scen->SetTextSize ( 0.03 );
  scen->SetTextColor(    1 );
  scen->SetTextFont (   62 );
  scen->AddText("m^{h}_{max}");
  scen->Draw();
#endif

#ifdef MSSM
  TLegend* leg = new TLegend(0.45, 0.65, 0.95, 0.90);
  SetLegendStyle(leg);
  leg->AddEntry(ggH  , "#phi#rightarrow#tau#tau" , "L" );
#else
  TLegend* leg = new TLegend(0.50, 0.65, 0.95, 0.90);
  SetLegendStyle(leg);
#ifndef DROP_SIGNAL
  if(SIGNAL_SCALE!=1){
    leg->AddEntry(ggH  , TString::Format("%.0f#timesH(125 GeV)#rightarrow#tau#tau", SIGNAL_SCALE) , "L" );
  }
  else{
    leg->AddEntry(ggH  , "H(125 GeV)#rightarrow#tau#tau" , "L" );
  }
#endif
#endif
#ifdef ASIMOV
  leg->AddEntry(data , "sum(bkg) + SM125 GeV signal"    , "LP");
#else
  leg->AddEntry(data , "observed"                       , "LP");
#endif
  leg->AddEntry(Ztt  , "Z#rightarrow#tau#tau"           , "F" );
  leg->AddEntry(ttbar, "t#bar{t}"                       , "F" );
  leg->AddEntry(EWK  , "electroweak"                    , "F" );
  leg->AddEntry(Fakes, "QCD"                            , "F" );
  $ERROR_LEGEND
  leg->Draw();

//#ifdef MSSM
//  TPaveText* mssm  = new TPaveText(0.69, 0.85, 0.90, 0.90, "NDC");
//  mssm->SetBorderSize(   0 );
//  mssm->SetFillStyle(    0 );
//  mssm->SetTextAlign(   12 );
//  mssm->SetTextSize ( 0.03 );
//  mssm->SetTextColor(    1 );
//  mssm->SetTextFont (   62 );
//  mssm->AddText("(m_{A}=120, tan#beta=10)");
//  mssm->Draw();
//#else
//  TPaveText* mssm  = new TPaveText(0.83, 0.85, 0.95, 0.90, "NDC");
//  mssm->SetBorderSize(   0 );
//  mssm->SetFillStyle(    0 );
//  mssm->SetTextAlign(   12 );
//  mssm->SetTextSize ( 0.03 );
//  mssm->SetTextColor(    1 );
//  mssm->SetTextFont (   62 );
//  mssm->AddText("m_{H}=125");
//  mssm->Draw();
//#endif

  /*
    Ratio Data over MC
  */
  TCanvas *canv0 = MakeCanvas("canv0", "histograms", 600, 400);
  canv0->SetGridx();
  canv0->SetGridy();
  canv0->cd();

  TH1F* zero = (TH1F*)ref->Clone("zero"); zero->Clear();
  TH1F* rat1 = (TH1F*)data->Clone("rat"); 
  rat1->Divide(Ztt);
  for(int ibin=0; ibin<rat1->GetNbinsX(); ++ibin){
    if(rat1->GetBinContent(ibin+1)>0){
      // catch cases of 0 bins, which would lead to 0-alpha*0-1
      rat1->SetBinContent(ibin+1, rat1->GetBinContent(ibin+1)-1.);
    }
    zero->SetBinContent(ibin+1, 0.);
  }
  rat1->SetLineColor(kBlack);
  rat1->SetFillColor(kGray );
  rat1->SetMaximum(+0.5);
  rat1->SetMinimum(-0.5);
  rat1->GetYaxis()->CenterTitle();
  rat1->GetYaxis()->SetTitle("#bf{Data/MC-1}");
  rat1->GetXaxis()->SetTitle("#bf{m_{#tau#tau} [GeV]}");
  rat1->Draw();
  zero->SetLineColor(kBlack);
  zero->Draw("same");
  canv0->RedrawAxis();

  /*
    Ratio After fit over Prefit
  */
  TCanvas *canv1 = MakeCanvas("canv1", "histograms", 600, 400);
  canv1->SetGridx();
  canv1->SetGridy();
  canv1->cd();

  TH1F* rat2 = (TH1F*) Ztt->Clone("rat2");
  rat2->Divide(ref);
  for(int ibin=0; ibin<rat2->GetNbinsX(); ++ibin){
    if(rat2->GetBinContent(ibin+1)>0){
      // catch cases of 0 bins, which would lead to 0-alpha*0-1
      rat2 ->SetBinContent(ibin+1, rat2->GetBinContent(ibin+1)-1.);
    }
  }
  rat2->SetLineColor(kRed+ 3);
  rat2->SetFillColor(kRed-10);
  rat2->SetMaximum(+0.3);
  rat2->SetMinimum(-0.3);
  rat2->GetYaxis()->SetTitle("#bf{Fit/Prefit-1}");
  rat2->GetYaxis()->CenterTitle();
  rat2->GetXaxis()->SetTitle("#bf{m_{#tau#tau} [GeV]}");
  rat2->Draw();
  zero->SetLineColor(kBlack);
  zero->Draw("same");
  canv1->RedrawAxis();

  /*
    Relative shift per sample
  */
  TCanvas *canv2 = MakeCanvas("canv2", "histograms", 600, 400);
  canv2->SetGridx();
  canv2->SetGridy();
  canv2->cd();

  InitHist  (scales[0], "", "", kMagenta-10, 1001);
  InitHist  (scales[1], "", "", kRed    + 2, 1001);
  InitHist  (scales[2], "", "", kBlue   - 8, 1001);
  InitHist  (scales[3], "", "", kOrange - 4, 1001);
#ifndef DROP_SIGNAL
  InitSignal(scales[4]);
  InitSignal(scales[5]);
  InitSignal(scales[6]);
#endif
  scales[0]->Draw();
  scales[0]->GetXaxis()->SetBinLabel(1, "#bf{Fakes}");
  scales[0]->GetXaxis()->SetBinLabel(2, "#bf{EWK}"  );
  scales[0]->GetXaxis()->SetBinLabel(3, "#bf{ttbar}");
  scales[0]->GetXaxis()->SetBinLabel(4, "#bf{Ztt}"  );
#ifdef MSSM
  scales[0]->GetXaxis()->SetBinLabel(5, "#bf{ggH}"  );
  scales[0]->GetXaxis()->SetBinLabel(6, "#bf{bbH}"  );
  scales[0]->GetXaxis()->SetBinLabel(7, "NONE"      );
#else
  scales[0]->GetXaxis()->SetBinLabel(5, "#bf{ggH}"  );
  scales[0]->GetXaxis()->SetBinLabel(6, "#bf{qqH}"  );
  scales[0]->GetXaxis()->SetBinLabel(7, "#bf{VH}"   );
#endif
  scales[0]->SetMaximum(+1.0);
  scales[0]->SetMinimum(-1.0);
  scales[0]->GetYaxis()->CenterTitle();
  scales[0]->GetYaxis()->SetTitle("#bf{Fit/Prefit-1}");
  scales[1]->Draw("same");
  scales[2]->Draw("same");
  scales[3]->Draw("same");
#ifndef DROP_SIGNAL
  scales[4]->Draw("same");
  scales[5]->Draw("same");
  scales[6]->Draw("same");
#endif
  zero->Draw("same");
  canv2->RedrawAxis();

  /*
    prepare output
  */
  bool isSevenTeV = std::string(inputfile).find("7TeV")!=std::string::npos;
  canv ->Print(TString::Format("%s_%sfit_%s_%s.png"       , directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN")); 
  canv ->Print(TString::Format("%s_%sfit_%s_%s.pdf"       , directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN")); 
  canv ->Print(TString::Format("%s_%sfit_%s_%s.eps"       , directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN")); 
  if(!log || FULLPLOTS)
  {
    canv0->Print(TString::Format("%s_datamc_%sfit_%s_%s.png", directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN")); 
    canv0->Print(TString::Format("%s_datamc_%sfit_%s_%s.pdf", directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN"));
    canv0->Print(TString::Format("%s_datamc_%sfit_%s_%s.eps", directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN"));
  }
  if((!log && scaled) || FULLPLOTS)
  {
    canv1->Print(TString::Format("%s_prefit_%sfit_%s_%s.png", directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN")); 
    canv1->Print(TString::Format("%s_prefit_%sfit_%s_%s.pdf", directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN"));
    canv1->Print(TString::Format("%s_prefit_%sfit_%s_%s.eps", directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN"));
    canv2->Print(TString::Format("%s_sample_%sfit_%s_%s.png", directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN")); 
    canv2->Print(TString::Format("%s_sample_%sfit_%s_%s.pdf", directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN"));
    canv2->Print(TString::Format("%s_sample_%sfit_%s_%s.eps", directory, scaled ? "post" : "pre", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN"));
  }

  TFile* output = new TFile(TString::Format("%s_%sfit_%s_%s.root", directory, scaled ? "pre" : "post", isSevenTeV ? "7TeV" : "8TeV", log ? "LOG" : "LIN"), "update");
  output->cd();
  data ->Write("data_obs");
  Fakes->Write("Fakes"   );
  EWK  ->Write("EWK"     );
  ttbar->Write("ttbar"   );
  Ztt  ->Write("Ztt"     );
#ifdef MSSM
  ggH  ->Write("ggH"     );
  bbH  ->Write("bbH"     );
#else
#ifndef DROP_SIGNAL
  ggH  ->Write("ggH"     );
  qqH  ->Write("qqH"     );
  VH   ->Write("VH"      );
#endif
#endif
  if(errorBand){
    errorBand->Write("errorBand");
  }
  output->Close();
}
