/*
 * @Descripttion: 
 * @version: 
 * @Author: li
 * @Date: 2021-02-22 13:21:17
 * @LastEditors: li
 * @LastEditTime: 2021-05-12 14:43:09
 */
#pragma once
#ifndef __GLOG_EXTENSION_H__
#define __GLOG_EXTENSION_H__

#include <dirent.h>
#include <iostream>
#include <boost/filesystem.hpp>

static std::string checkDir(const std::string& LOGPATH)
{
    if (!boost::filesystem::exists(LOGPATH))
    {
        boost::filesystem::create_directory(LOGPATH);
    }
    time_t raw_time;
    struct tm* tm_info;

    time(&raw_time);
    tm_info = localtime(&raw_time);

    //printf("time_now:%d%d%d%d%d%d.%ld(us)\n", 1900+tm_info->tm_year, 1+tm_info->tm_mon, tm_info->tm_mday, tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
    char day[128] = {0};
    sprintf(day, "%s%04d%02d%02d", LOGPATH.c_str(), 1900+tm_info->tm_year, 1+tm_info->tm_mon, tm_info->tm_mday);
    
    std::string log_path = day;
    if (!boost::filesystem::exists(log_path))
    {
        boost::filesystem::create_directory(log_path);
    }
    return log_path;
}

static bool readNewFile(const std::string& LOGPATH,std::string &target,std::string suffix = ".pcd"){
    if (!boost::filesystem::exists(LOGPATH))
    {
        printf("current path dont exits");
        return false;
    }
    if(!boost::filesystem::is_directory(LOGPATH)){
        printf("current path is not directory");
        return false;
    }
    std::multimap<std::time_t, boost::filesystem::path> result_set;
    boost::filesystem::directory_iterator end_iter;
    std::time_t last_time;
    for( boost::filesystem::directory_iterator dir_iter(LOGPATH) ; dir_iter != end_iter ; ++dir_iter)
    {
        if (boost::filesystem::is_regular_file(dir_iter->status()))
        {
            boost::filesystem::path cpath = *dir_iter;
            if(cpath.extension().compare(suffix)==0){
                std::cout <<"file name:" <<cpath.filename() << " extension:" << cpath.extension() << std::endl;
                last_time = boost::filesystem::last_write_time(dir_iter->path());
                result_set.insert(std::multimap<std::time_t, boost::filesystem::path>::value_type(last_time,cpath));
            }
        }
    }
    int count = result_set.size();
    if(count){
        auto iter = result_set.find(last_time);
        if (iter != std::end (result_set)){
            boost::filesystem::path cpath = iter->second;
            target = cpath.filename().string();
            return true;
        }
    }
    return false;
}

static int readFilesInDir(const char *basePath)
{
    DIR *dir;
    struct dirent *ptr;

    if ((dir=opendir(basePath)) == NULL){
        perror("Open dir error...");
        return -1;
    }

    while ((ptr=readdir(dir)) != NULL)
    {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
            continue;
        else{

            printf("rm file:%s\n", ptr->d_name);
            char file[128] = {0};
            sprintf(file, "%s/%s", basePath, ptr->d_name);
            int ret = unlink(file);
            if(ret != 0){
                printf("unlink %s failed!\n", file);
                closedir(dir);
                return -1;
            }
        }
    }
    closedir(dir);
    return 0;
}

static int rmdir2(const char *path)
{
    int ret = readFilesInDir(path);
    if(ret != 0)
        return -1;

    ret = rmdir(path);
    if(ret != 0){
        printf("rm %s failed!\n", path);
        return -1;
    }

    return 0;
}

static bool rmOldDay(const std::string& LOGPATH,int MAX_SAVE_DAYS = 7)
{
    struct dirent **namelist;
    int n = 0, i = 0;

    n = scandir(LOGPATH.c_str(), &namelist, 0, alphasort);
    printf("scandir %i \n",n);
    if(n < 1) return false;
    for (size_t i = 0; i < n; i++)
    {
        std::cout << " namelist " << namelist[i]->d_name << std::endl;
    }
    
        int dir_index = 0;
        int max_index = n-2-MAX_SAVE_DAYS;
        while(dir_index < max_index){
            printf("dir index:%i max index:%i",dir_index,max_index);
            char first[128] = {0};
            memcpy(first, namelist[2+dir_index]->d_name, sizeof(first));
            sprintf(first, "%s%s", LOGPATH.c_str(), namelist[2+dir_index]->d_name);
            printf("rm old:%s\n", first);
            int ret = rmdir2(first);
            if(ret != 0){
                printf("rm %s failed!\n", first);
            }
            dir_index += 1;
        }

        for (i = 0; i < n; i++)
        {
            free(namelist[i]);
        }
        free(namelist);

    return true;
}
#endif
