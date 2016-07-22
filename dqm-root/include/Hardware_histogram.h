#include "TDirectory.h"
#include "TFile.h"
#include <string>
#include "db_interface.cxx"

class Hardware_histogram
{
 public:
  Hardware_histogram(const std::string & filename, TDirectory * dir, const std::string & hwid){
    //m_file = new TFile(filename.c_str(), "UPDATE");
    m_dir = dir;
    m_HWID = hwid;
  }
  virtual ~Hardware_histogram(){}//m_file->Close();}
  void bookHistograms(){}
  TDirectory * getTDir(){return m_dir;}
  std::string getHWID(){return m_HWID;}
  void readMap(int sn, int* t_strip_map){
    path = std::getenv("BUILD_HOME");
    if (sn < 2) {
      path += "/gem-light-dqm/dqm-root/data/v2b_schema_chips0-1.csv";
    } else if (sn < 16) {
      path += "/gem-light-dqm/dqm-root/data/v2b_schema_chips2-15.csv";
    } else if (sn < 18) {
      path += "/gem-light-dqm/dqm-root/data/v2b_schema_chips16-17.csv";
    } else {
      path += "/gem-light-dqm/dqm-root/data/v2b_schema_chips18-23.csv";
    }
    std::ifstream icsvfile_;
    icsvfile_.open(path);
    if(!icsvfile_.is_open()) {
      std::cout << "\nThe file: " << icsvfile_ << " is missing.\n" << std::endl;
      return;
    }  
    for (int il = 0; il < 128; il++) {
      std::string line;
      std::getline(icsvfile_, line);
      std::istringstream iss(line);
      std::string val;
      std::getline(iss,val,',');
      std::stringstream convertor(val);
      int strip;
      convertor >> std::dec >> strip;
      std::getline(iss,val,',');
      convertor.str("");
      convertor.clear();
      convertor << val;
      int channel;
      convertor >> std::dec >> channel;
      t_strip_map[channel] = strip;
    }
    icsvfile_.close();
  }

 protected:
  TDirectory *m_dir;
  //TFile *m_file;
  std::string m_HWID;
  std::string path;
};
