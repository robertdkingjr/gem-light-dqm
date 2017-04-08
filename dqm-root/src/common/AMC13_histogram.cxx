#include "AMC_histogram.cxx"
#include "TH1.h"

//!A class that creates histograms for AMC13 data
class AMC13_histogram: public Hardware_histogram
{
  public:
    //!Constructor calls the base contructor of Hardware_histogram. Requires a string for filename, directory, and another string.
    AMC13_histogram(const std::string & filename, TDirectory * dir, const std::string & hwid):Hardware_histogram(filename, dir, hwid){}//call base constructor

    //!Books the histograms for AMC13 data
    /*!
     This books histograms for the following data: control_bit5, control_bitA, Evt_ty, LV1_id, BX_id, Source_id, CalTyp, nAMC, OrN, CRC_amc13, Blk_Not, LV1_idT, BX_idT, EvtLength, and CRC_cdf
    */
    void bookHistograms()
    {
      m_dir->cd();
      control_bit5 = new TH1F("Control_Bit5", "Control Bit 5", 15,  0x0 , 0xf);
      control_bitA = new TH1F("Control_BitA", "Control Bit A", 15,  0x0 , 0xf); 
      Evt_ty       = new TH1F("Evt_ty", "Evt_ty", 15, 0x0, 0xf);
      //LV1_id       = new TH1F("LV1_id", "LV1_id", 0xffffff, 0x0, 0xffffff);
      BX_id        = new TH1F("Bx_id", "Bx_id", 4095, 0x0, 0xfff);
      Source_id    = new TH1F("Source_id", "Source_id", 4095, 0x0, 0xfff);
      CalTyp       = new TH1F("CalTyp", "CalTyp", 15, 0x0, 0xf);
      nAMC         = new TH1F("nAMC", "nAMC", 12, 0, 12);
      //OrN          = new TH1F("OrN", "OrN", 0xffffffff, 0x0, 0xffffffff);// N bins overflow!!
      //CRC_amc13    = new TH1F("CRC_amc13", "CRC_amc13", 0xffffffff, 0x0, 0xffffffff);// N bins overflow!!
      Blk_NoT      = new TH1F("Blk_NoT", "Blk_NoT", 255, 0x0, 0xff);
      LV1_idT      = new TH1F("LV1_idT", "LV1_idT", 255, 0x0, 0xff);
      BX_idT       = new TH1F("BX_idT", "BX_idT", 4095, 0x0, 0xfff);
      BX_diff      = new TH1F("BX_diff", "BX_diff", 8191, -4095, 4095);
      //EvtLength    = new TH1F("EvtLength", "EvtLength", 0xffffff, 0x0, 0xffffff);
      //CRC_cdf      = new TH1F("CRC_cdf", "CRC_cdf", 0xffff, 0x0, 0xffff);
    }
    
    //!Fills the histograms for AMC13 data
    /*!
     This fills histograms for the following data: control_bit5, control_bitA, Evt_ty, LV1_id, BX_id, Source_id, CalTyp, nAMC, Blk_Not, LV1_idT, BX_idT, EvtLength, and CRC_cdf
    */
    void fillHistograms(AMC13Event *amc13){
      nAMC->Fill(amc13->nAMC());
      control_bit5->Fill(amc13->cb_5());
      Evt_ty->Fill(amc13->Evt_ty());
      //LV1_id->Fill(amc13->LV1_id());
      BX_id->Fill(amc13->BX_id());
      Source_id->Fill(amc13->Source_id());
      CalTyp->Fill(amc13->CalTyp());
      Blk_NoT->Fill(amc13->Blk_NoT());
      LV1_idT->Fill(amc13->LV1_idT());
      BX_idT->Fill(amc13->BX_idT());
      control_bitA->Fill(amc13->cbA());
      BX_diff->Fill(amc13->BX_idT()-amc13->BX_id());

    }

     
    //!Adds an AMC_histogram object to the m_amcsH vector
    void addAMCH(AMC_histogram amcH){m_amcsH.push_back(amcH);}

    //!Returns the m_amcsH vector
    std::vector<AMC_histogram> amcsH(){return m_amcsH;}
  private:

    std::vector<AMC_histogram> m_amcsH;   ///<A vector of AMC_histogram
    TH1F* control_bit5;                   ///<Histogram for control bit 5
    TH1F* control_bitA;                   ///<Histogram for control bit A
    TH1F* Evt_ty;                         ///<Histogram for Evt_ty
    //TH1F* LV1_id;                         ///<Histogram for LV1_id
    TH1F* BX_id;                          ///<Histogram for BX_id
    TH1F* Source_id;                      ///<Histogram for Source_id
    TH1F* CalTyp;                         ///<Histogram for CalTyp
    TH1F* nAMC;                           ///<Histogram for nAMC
    //TH1F* OrN;                            ///<Histogram for OrN
    //TH1F* CRC_amc13;                      ///<Histogram for CRC_amc13
    TH1F* Blk_NoT;                        ///<Histogram for Blk_Not
    TH1F* LV1_idT;                        ///<Histogram for LV1_idT
    TH1F* BX_idT;                         ///<Histogram for BX_idT
    TH1F* BX_diff;
    //TH1F* EvtLength;                      ///<Histogram for EvtLength
    //TH1F* CRC_cdf;                        ///<Histogram for CRC_cdf
};
