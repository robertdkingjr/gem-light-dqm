#define NVFAT 24
#define NSLOTS 12
#define NGTX 2
#define NETA 8

#define PORT 3306
#include <mysql/mysql.h>
#include <Python.h>

#include <iomanip> 
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include <array>
#include <TFile.h>
#include <TKey.h>
#include <TDirectory.h>
#include <TNtuple.h>
#include <TH2.h>
#include <TProfile.h>
#include <TCanvas.h>
#include <TFrame.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TRandom3.h>
#include <TBenchmark.h>
#include <TInterpreter.h>
#include <TApplication.h>
#include <TString.h>
#include <Event.h>
#include <TObject.h>
#include <TClonesArray.h>
#include <TRefArray.h>
#include <TRef.h>
#include <TH1.h>
#include <TBits.h>
#include <TMath.h>
#include <TFile.h>
#include <TClassTable.h>
#include <TTree.h>
#include <TBranch.h>
#include <TError.h>
#include <TBufferJSON.h>
#include <memory>
#include <unordered_map>

#include "GEMClusterization/GEMStrip.h"
#include "GEMClusterization/GEMStripCollection.h"
#include "GEMClusterization/GEMClusterContainer.h"
#include "GEMClusterization/GEMClusterizer.h"
#include "plotter.cxx"
#include "logger.cxx"
#include "integrity_checker.cxx"
#include "GEMDQMerrors.cxx"
#include "AMC13_histogram.cxx"

using namespace std;

//!A class that creates the subdirectories and histograms from VFAT, GEM, AMC and AMC13 data
class treeReader
{
public:

  //!The constructor requires a file name ending with .raw.root
  treeReader(const std::string &ifilename)
  {
    if(ifilename.find("chunk") == string::npos)
      RunName = ifilename.substr(ifilename.find("run"),ifilename.size()-9);
    else
      RunName = ifilename.substr(ifilename.find("run"),ifilename.find("chunk")-6);
    
    std::string tmp = ifilename.substr(ifilename.size()-9, ifilename.size());
    if (tmp != ".raw.root") throw std::runtime_error("Wrong input filename (should end with '.raw.root'): "+ifilename);
    ifile = new TFile(ifilename.c_str(), "READ");
    ofilename = ifilename.substr(0,ifilename.size()-9);
    ofilename += ".analyzed.root";
    ofile = new TFile(ofilename.c_str(), "RECREATE");
    if (DEBUG) std::cout << std::dec << "[gemTreeReader]: File for histograms created" << std::endl;   

    if (DEBUG) std::cout << std::dec << "[gemTreeReader]: Connecting to database on port " << PORT << std::endl;
    Database = connectDB();
    if (DEBUG) std::cout << std::dec << "[gemTreeReader]: Fetching hardware" << std::endl;   
    this->fetchHardwareDB();

    // if (DEBUG) std::cout << std::dec << "[gemTreeReader]: Booking histograms" << std::endl;   
    // this->bookAllHistograms();
    this->fillAllHistograms();
  }
  ~treeReader(){}

private:
  TFile *ifile;   ///<Input File, assigned in the constructor
  TFile *ofile;   ///<Output File, assigned in the constructor, where all the histograms will go
  std::string ofilename;  ///<Name of output file, same as input file, but is .analyzed.root instead of .raw.root

  std::string RunName;

  vector<AMC13Event> v_amc13;    ///<Vector of AMC13Event
  vector<AMCdata> v_amc;         ///<Vector of AMCdata
  vector<GEBdata> v_geb;         ///<Vector of GEBdata
  vector<VFATdata> v_vfat;       ///Vector of VFATdata

  //vector<AMC13_histogram> v_amc13H;   //unused
  AMC_histogram * v_amcH;        ///<Vector of AMC_histogram
  GEB_histogram * v_gebH;        ///<Vector of GEB_histogram
  VFAT_histogram * v_vfatH;      ///<Vector of VFAT_histogram

  int VFATMap[12][2][24];

  AMC13_histogram * m_amc13H;
  AMC_histogram * m_amcH;
  GEB_histogram * m_gebH;
  VFAT_histogram * m_vfatH;

  int m_RunType;
  int m_deltaV;
  int m_Latency;

  MYSQL *Database;

  void fetchHardwareDB()
  {
    VFATMap = {{{0}}};
    char * diramc13 = "AMC13-1";
    
    m_amc13H = new AMC13_histogram(ofilename, gDirectory->mkdir(diramc13), "1");
    m_amc13H->bookHistograms();

    string AMC13Query = "select id from ldqm_db_run where Name like '";
    AMC13Query += RunName;
    AMC13Query += "'";

    string RunIDstr = stringFromChar(simpleDBQuery(Database, AMC13Query));
    
    string AMCQuery = "select amc_id from ldqm_db_run_amcs where run_id like '"+RunIDstr+"'";
    vector<string> AMCs = manyDBQuery(Database,AMCQuery);
    for ( int amc = 0; amc < AMCs.size(); amc++ )
      {
        string currentAMCid = stringFromChar(AMCs.at(amc).c_str());
        string AMCBidQuery = "select BoardID from ldqm_db_amc where id like '"+currentAMCid+"'";
        char* AMCBidchar = simpleDBQuery(Database,AMCBidQuery);
        string AMCBidstr(AMCBidchar);
        string aslot_ch = AMCBidstr.substr(AMCBidstr.find("-")+1,AMCBidstr.size()-2);
        long long int a_slot = atoi(aslot_ch.c_str());
        string amcdir = "AMC-";
        amcdir += to_string(a_slot);
        m_amcH = new AMC_histogram(ofilename, gDirectory->mkdir(amcdir.c_str()), to_string(a_slot));
        m_amcH->bookHistograms();
        string GEBQuery = "select geb_id from ldqm_db_amc_gebs where amc_id like '"+currentAMCid+"'";
        vector<string> GEBs = manyDBQuery(Database,GEBQuery);
        for ( int geb = 0; geb < GEBs.size(); geb++ )
          {
            string currentGEBid = stringFromChar(GEBs.at(geb).c_str());
            string GEBBidQuery = "select ChamberID from ldqm_db_geb where id like '"+currentGEBid+"'";
            char* GEBBidchar = simpleDBQuery(Database,GEBBidQuery);
            string GEBBidstr(GEBBidchar);
            string gslot_ch = GEBBidstr.substr(GEBBidstr.find("-")+1,GEBBidstr.size());
            long long int g_slot = atoi(gslot_ch.c_str());
            string gebdir = "GTX-";
            gebdir += to_string(g_slot);
            m_gebH = new GEB_histogram(ofilename, gDirectory->mkdir(gebdir.c_str()), to_string(g_slot));
            m_gebH->bookHistograms();
            string VFATQuery = "select vfat_id from ldqm_db_geb_vfats where geb_id like '"+currentGEBid+"'";
            vector<string> VFATs = manyDBQuery(Database,VFATQuery);
            for ( int vfat = 0; vfat < VFATs.size(); vfat++ )
              {
                string currentVFATid = stringFromChar(VFATs.at(vfat).c_str());
                string VFATSlotQuery = "select Slot from ldqm_db_vfat where id like '"+currentVFATid+"'";
                char* VFATSlotchar = simpleDBQuery(Database,VFATSlotQuery);
                long long int v_slot = atoi(VFATSlotchar);
                string VFATdirname = "VFAT-";
                VFATdirname += to_string(v_slot);
                string VFATChipQuery = "select ChipID from ldqm_db_vfat where id like '"+currentVFATid+"'";
                char* VFATChipchar = simpleDBQuery(Database,VFATChipQuery);
                int v_chipid = strtol(VFATChipchar, NULL, 16);
                VFATMap[atoi(aslot_ch.c_str())][atoi(gslot_ch.c_str())][v_slot] = v_chipid;
                m_vfatH = new VFAT_histogram(ofilename, gDirectory->mkdir(VFATdirname.c_str()), to_string(v_slot));
                m_vfatH->bookHistograms();
                m_gebH->addVFATH(m_vfatH,v_slot);

                gDirectory->cd("..");   //moves back to previous directory
              } /* END VFAT LOOP */
            gDirectory->cd("..");     //moves back to previous directory
            m_amcH->addGEBH(m_gebH,g_slot);
          } /* END GEB LOOP */
        gDirectory->cd("..");       //moves back to previous directory
        m_amc13H->addAMCH(m_amcH, a_slot);
      } /* END AMC LOOP */
  } /* END AMC13 LOOP */

  int slotFromMap(int a, int g, int cid)
  {
    int res = -1;
    for (int i=0; i<24; i++){
      if (VFATMap[a][g][i] == cid) {res = i;}
    }
    return res;
  }

  //!Fills the histograms that were book from bookAllHistograms
  void fillAllHistograms()
  {
    int a13_c=0;    //counter through AMC13s
    int a_c=0;      //counter through AMCs
    int g_c=0;      //counter through GEBs
    int v_c=0;      //counter through VFATs

    TTree *tree = (TTree*)ifile->Get("GEMtree");
    Event *event = new Event();
    TBranch *branch = tree->GetBranch("GEMEvents");
    branch->SetAddress(&event);
    Int_t nentries = tree->GetEntries();
    /* LOOP THROUGH Events */
    for (int i = 0; i < nentries; i++){
      branch->GetEntry(i);
      v_amc13 = event->amc13s();
      /* LOOP THROUGH AMC13s */
      for(auto a13 = v_amc13.begin(); a13!=v_amc13.end(); a13++){
        v_amc = a13->amcs();
        m_amc13H->fillHistograms(&*a13);
        /* LOOP THROUGH AMCs */
        for(auto a=v_amc.begin(); a!=v_amc.end(); a++){
          v_geb = a->gebs();
          a_c=a->AMCnum();
          v_amcH = m_amc13H->amcsH(a_c);
          v_amcH->fillHistograms(&*a);
          m_RunType = a->Rtype();

          if (m_RunType){
            m_deltaV = a->Param2() - a->Param3();
            m_Latency = a->Param1();
          }
          g_c=0;
          /* LOOP THROUGH GEBs */
          for(auto g=v_geb.begin(); g!=v_geb.end();g++){
            v_vfat = g->vfats();
            int gID = g->InputID();
            v_gebH = v_amcH->gebsH(gID);
            std::map<int,int> slot_map;
            for(auto v=v_vfat.begin(); v!=v_vfat.end();v++){
              int vID = v->ChipID();
              vID = vID | 0xf000;
              int slot = slotFromMap(a_c, gID, vID);
              slot_map.insert(std::make_pair(v->ChipID(), slot));
            }
            v_gebH->fillHistograms(&*g, slot_map);
            /* LOOP THROUGH VFATs */
            for(auto v=v_vfat.begin(); v!=v_vfat.end();v++){
              int slot = slot_map.find(v->ChipID())->second;
              v_vfatH = v_gebH->vfatsH(slot);
              bool final = i == nentries-1;
              v_vfatH->fillHistograms(&*v, final);
              if (m_RunType == 1){
                v_vfatH->fillScanHistograms(&*v, m_RunType, m_deltaV, m_Latency);
              }
            } /* END VFAT LOOP */
          } /* END GEB LOOP */
          a_c++;
        } /* END AMC LOOP */
        a13_c++;
      } /* END AMC13 LOOP */
    } /* END EVENTS LOOP */
    ofile->Write();
  }
};
