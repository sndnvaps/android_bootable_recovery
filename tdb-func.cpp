/*
 *
 *
 *
 */



#include <string>
#include <vector>
#include <map>
#include "twrp-functions.hpp"
#include "partitions.hpp"
#include "tdb-func.hpp"
#include "twcommon.h"
#include "data.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/limits.h>


using namespace std;

map<std::string,struct stat>     TDBFunc::partst;//初始化变量

TDBFunc::TDBFunc() {
    //do nothing
}

TDBFunc::~TDBFunc() {
    //do nothing
}

/*
 /dev/block/mmcblk0p1     /boot
 /dev/block/mmcblk0p2     /boot1
 /dev/block/mmcblk0p18    /modem
 /dev/block/mmcblk0p19    /modem1
 /dev/block/mmcblk0p23    /system
 /dev/block/mmcblk0p24    /system1
*/
/*
 int main() {
void dualboot_prepare_env(void);
void dualboot_setup_env(void);
 */


bool TDBFunc::GetTDBState(){
    bool tdbstate = false;
    PartitionManager.Mount_By_Path("/data",false);
    if(TWFunc::Path_Exists("/data/.truedualboot")) {
        LOGINFO("/data/.truedualboot is exists \n");
        LOGINFO("TDB state is on\n");
        tdbstate = true;
    } else {
        LOGINFO("TDB state is off");

    }
    PartitionManager.UnMount_By_Path("/data",false);
    return tdbstate;
}


bool TDBFunc::SetUpTDB() {
    //先格式化/data分区
   //删除 /system/bin/mount_ext4.sh , /system1/bin/mount_ext4.sh
   //复制 /res/dualboot/mount_ext4_tdb.sh 到/system/bin/mount_ext4.sh , /system1/bin/mount_ext4.sh
   //生成/data/.truedualboot 空文件
    gui_print("Setup TrueDualBoot ...\n");

    PartitionManager.Mount_By_Path("/data",false);
    if (TWFunc::Path_Exists("/data/.truedualboot")) {//如果文件存在，直接返回true
        return true;
    }
        PartitionManager.Wipe_By_Path("/data");//对data分区进行格式化
        PartitionManager.Mount_By_Path("/data",true);

        if(!GenTDBFile())
            return false;


    PartitionManager.Mount_By_Path("/system",false);
    PartitionManager.Mount_By_Path("/system1",false);
    unlink("/system/bin/mount_ext4.sh");
    unlink("/system1/bin/mount_ext4.sh");
    TWFunc::copy_file("/res/dualboot/mount_ext4_tdb.sh", "/system/bin/mount_ext4.sh",0755);
    TWFunc::copy_file("/res/dualboot/mount_ext4_tdb.sh", "/system1/bin/mount_ext4.sh",0755);

    PartitionManager.UnMount_By_Path("/data",false);
    PartitionManager.UnMount_By_Path("/system",false);
    PartitionManager.UnMount_By_Path("/system1",false);

    gui_print("Setup TrueDualBoot finish ...\n");

    return true;


}
bool TDBFunc::GenTDBFile() {
    struct stat st;
    int ret = 0;
    string tdbfile = "/data/.truedualboot";
    FILE* tdbp = fopen(tdbfile.c_str(),"w");
    fprintf(tdbp, "twrp");
    fclose(tdbp);
    mkdir("/data/system0",0755);
    mkdir("/data/system1",0755);

    if (TWFunc::Path_Exists("/data/system0") &&
            TWFunc::Path_Exists("/data/system1") &&
            TWFunc::Path_Exists(tdbfile)) {
        return true;
    }

    return false;
}

bool TDBFunc::DisableTDB() {
    //格式化/data分区
    //替换/system/bin/mount_ext4.sh  为 /res/dualboot/mount_ext4_default.sh
    //替换/system1/bin/mount_ext4.sh 为 /res/dualboot/mount_ext4_default.sh

    gui_print("Disable TrueDualBoot ...\n");
    if (!GetTDBState()) //当文件不存在的时候，即为不是tdb模式，直接返回true
        return true;
    PartitionManager.Wipe_By_Path("/data"); //格式化/data分区
    PartitionManager.Mount_By_Path("/system",false);
    PartitionManager.Mount_By_Path("/system1",false);


    unlink("/system/bin/mount_ext4.sh");
    unlink("/system1/bin/mount_ext4.sh");
    TWFunc::copy_file("/res/dualboot/mount_ext4_default.sh", "/system/bin/mount_ext4.sh",0755);
    TWFunc::copy_file("/res/dualboot/mount_ext4_default.sh", "/system1/bin/mount_ext4.sh",0755);

    PartitionManager.UnMount_By_Path("/system",false);
    PartitionManager.UnMount_By_Path("/system1",false);

    return true;
}

std::string TDBFunc::GetCurrentSystem() {
    std::string bootmode;
   // bootmode = TDBManager.GetBootmode();
    TDBFunc *tdb = new TDBFunc();
    bootmode = tdb->GetBootmode();
    if (bootmode.compare("boot-system0") == 0)
        return "system0";
    if (bootmode.compare("boot-system1") == 0)
        return "system1";
    return "";
}

std::string TDBFunc::GetBootmode() {
       std::string bootmode_s;
       char bootmode[13];
       //open misc partition
        FILE *misc = fopen("/dev/block/platform/msm_sdcc.1/by-name/misc", "rb");
        if (misc == NULL) {
            printf("Error opening misc partition .\n");
            return "";
        }

        //write bootmode
        fseek(misc, 0x1000, SEEK_SET);
        if (fgets(bootmode, 13, misc) == NULL) {
            printf("Error reading bootmode from misc partition. \n");
            return "";
        }

        //close
        fclose(misc);
        bootmode_s = bootmode;

        return bootmode_s;
}

int TDBFunc::SetBootmode(string bootmode) {
   // open misc-partition
   char *bm =(char*)bootmode.c_str();
   LOGINFO("bootmode = %s\n", bm);
   FILE* misc = fopen("/dev/block/platform/msm_sdcc.1/by-name/misc", "wb");
   if (misc == NULL) {
        LOGERR("Open misc partitions failed.\n");
      return -1;
   }

   // write bootmode
   fseek(misc, 0x1000, SEEK_SET);
   if(fputs(bm, misc)<0) {
      LOGERR("Write bootmode failed.\n");
      return 0;
   }

   // close
   fclose(misc);
   return 0;
}

bool TDBFunc::Replace_Device_Node(std::string part,struct stat* st){
    if (st == NULL) {
        LOGERR("partiion stat is null !!!!\n");
        return false;
    }

    TWPartition* part_ = PartitionManager.Find_Partition_By_Path(part);
    std::string block = part_->Actual_Block_Device;
    if(!part_->UnMount(true)) {
        LOGERR("Unmount %s failed ..\n",block.c_str());
        return false;
    }

    unlink(block.c_str());
    if(mknod(block.c_str(),st->st_mode,st->st_rdev)!=0) {
        LOGERR("Could not create node %s\n",block.c_str());
        return false;
    }

    return true;

}

void TDBFunc::dualboot_init_part(std::string part) {
    //part == "/system";
    //part_backup == "/dev/part_backup_system";
    std::string part_name;
    std::string part_backup;
    struct stat st;
    part_name = part.substr(1,std::string::npos);//part_name == "system"
    TWPartition* partition;
    partition = PartitionManager.Find_Partition_By_Path(part);
    std::string block;
    block = partition->Actual_Block_Device;
    part_backup = "/dev/part_backup_" + part_name;
    stat(block.c_str(),&st);
    //st = partition->GetStat();

    // check if moved node already exist
    if(stat(part_backup.c_str(), &st) == 0) {
            return;
       // check for original otherwise
     } else if(stat(block.c_str(), &st)==0) {
            // move
        if(rename(block.c_str(),part_backup.c_str()) != 0) {
            LOGERR("rename %s ->  %s\n",block.c_str(),part_backup.c_str());
            }
        }
}

void TDBFunc::dualboot_prepare_env(void) {
    dualboot_init_part("/system");
    dualboot_init_part("/system1");
    dualboot_init_part("/boot");
    dualboot_init_part("/boot1");
    dualboot_init_part("/modem");
    dualboot_init_part("/modem1");


    //TWPartition* part;//  PartitionManager.Find_Partition_By_Path("/boot");
    //std::vector<string> partlist;
    std::map<string,string> partlist;
    partlist.insert(make_pair("/boot","/dev/block/mmcblk0p1"));
    partlist.insert(make_pair("/boot1","/dev/block/mmcblk0p2"));
    partlist.insert(make_pair("/modem","/dev/block/mmcblk0p18"));
    partlist.insert(make_pair("/modem1","/dev/block/mmcblk0p19"));
    partlist.insert(make_pair("/system","/dev/block/mmcblk0p23"));
    partlist.insert(make_pair("/system1","/dev/block/mmcblk0p24"));

    //std::vector<string>::iterator it;
    std::map<string,string>::iterator it;
    //map<std::string,struct stat> partst;
    for (it = partlist.begin(); it != partlist.end(); it++) {
         struct stat st;
         stat(it->second.c_str(),&st);
            partst.insert(make_pair(it->first,st));


    }

}

void TDBFunc::dualboot_setup_env(void) {

    if (PartitionManager.UnMount_By_Path("/data",true) == 0){//false == 0
        LOGERR("Error unmounting /data!\n");
        return;
    }

    std::string system;
    //DataManager::GetValue()
    system = TDBFunc::GetCurrentSystem();
    map<string, struct stat>::iterator system0;
    map<string, struct stat>::iterator system1;
    map<string, struct stat>::iterator boot0;
    map<string, struct stat>::iterator boot1;
    map<string, struct stat>::iterator modem0;
    map<string, struct stat>::iterator modem1;

    system0 = partst.find("/system");
    system1 = partst.find("/system1");
    boot0 = partst.find("/boot");
    boot1 = partst.find("/boot1");
    modem0 = partst.find("/modem");
    modem1 = partst.find("/modem1");



      if (system.compare("system0") == 0) {
        Replace_Device_Node(system0->first, &system0->second);
        Replace_Device_Node(system1->first, &system0->second);
        Replace_Device_Node(boot0->first, &boot0->second);
        Replace_Device_Node(boot1->first, &boot0->second);
        Replace_Device_Node(modem0->first, &modem0->second);
        Replace_Device_Node(modem1->first, &modem0->second);
    } else if(system.compare("system1") == 0) {
        Replace_Device_Node(system0->first, &system1->second);
        Replace_Device_Node(system1->first, &system1->second);
        Replace_Device_Node(boot0->first, &boot1->second);
        Replace_Device_Node(boot1->first, &boot1->second);
        Replace_Device_Node(modem0->first, &modem1->second);
        Replace_Device_Node(modem1->first, &modem1->second);
    } else {
          LOGERR("Invalid system '%s'\n", system.c_str());
    }
}

void TDBFunc::dualboot_init() {
    dualboot_prepare_env();//初始化变量
    dualboot_setup_env();//环境配置
}
