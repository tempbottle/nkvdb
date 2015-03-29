#include "storage.h"
#include <boost/filesystem.hpp>
#include <utils/Exception.h>
#include <ctime>
#include <sstream>

using namespace storage;
namespace fs=boost::filesystem;

DataStorage::DataStorage(){

}

DataStorage::~DataStorage(){

}

DataStorage::PDataStorage DataStorage::Create(const std::string& ds_path, u_int64_t page_size){
    DataStorage::PDataStorage result(new DataStorage);
    result->m_default_page_size=page_size;

    if(fs::exists(ds_path)){
        if(!utils::rm(ds_path)){
            throw utils::Exception::CreateAndLog(POSITION, "can`t create. remove error: "+ds_path);
        }
    }

    fs::create_directory(ds_path);
    result->m_path=std::string(ds_path);
    result->createNewPage();
    return result;
}

fs::path getOldesPage(const std::list<fs::path> &pages){
    if(pages.size()==1)
        return pages.front();
    fs::path maxTimePage;
    Time maxTime=0;
    for(auto p:pages){
        storage::Page::Header hdr=storage::Page::ReadHeader(p.string());
        Time cur_time=hdr.maxTime;
        if(maxTime<cur_time || cur_time==0){
            maxTime=cur_time;
            maxTimePage=p;
        }
    }
    if(maxTimePage.string().size()==0){
        throw utils::Exception::CreateAndLog(POSITION, "open error. page not found.");
    }
    return maxTimePage;
}

DataStorage::PDataStorage DataStorage::Open(const std::string& ds_path){
    DataStorage::PDataStorage result(new DataStorage);

    if(!fs::exists(ds_path)){
        throw utils::Exception::CreateAndLog(POSITION, ds_path+" not exists.");
    }

    auto pages=utils::ls(ds_path);
    fs::path maxTimePage=getOldesPage(pages);


    result->m_curpage=storage::Page::Open(maxTimePage.string());
    result->m_path=std::string(ds_path);
    return result;
}

void DataStorage::createNewPage(){
    if(m_curpage!=nullptr){
        m_curpage=nullptr;
    }

    std::stringstream ss;
    ss<<std::time(nullptr);

    fs::path page_path;
    page_path.append(m_path);
    page_path.append(ss.str()+".page");

    m_curpage=Page::Create(page_path.string(), this->m_default_page_size);
}

bool DataStorage::havePage2Write()const{
    return this->m_curpage!=nullptr && !this->m_curpage->isFull();
}

bool DataStorage::append(const Meas::PMeas m){
    if(this->m_curpage->isFull()){
        m_curpage=nullptr;
        this->createNewPage();
    }
    return this->m_curpage->append(m);
}