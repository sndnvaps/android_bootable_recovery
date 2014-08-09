/*
    Copyright 2014 (C) sndnvaps<sndnvaps@yahoo.com>

    This file is create by sndnvaps.
    Support TrueDualBoot feature in XiaoMi Devices.

    Devices:: XiaoMi Mi2S

    This is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with it.  If not, see <http://www.gnu.org/licenses/>.
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


bool TDBFunc::TDB_before_update(){
    //HACK:: delete node to userdata partition so update_script
    //will not be able to mount it (the wrong way)
    //we'll mount it now so files won't land on ramdisk


    if(!GetTDBState()){
        unlink(PATH_USERDATA_NODE);
        PartitionManager.UnMount_By_Path("/data",true);
    } else {
        struct stat st;
        if (stat(PATH_USERDATA_BACKUP,&st) != 0) {
            LOGERR("Could not stat userdata !\n");
        }
        unlink(PATH_USERDATA_NODE);
        if(mknod(PATH_USERDATA_NODE,st.st_mode,st.st_rdev) != 0) {
            LOGERR("could not create node!\n");
            return false;
        }
        PartitionManager.UnMount_By_Path("/data",true);
    }

    return true;
}

bool TDBFunc::GetTDBState(){
    bool tdbstate;
    PartitionManager.UnMount_By_Path("/data_root",true);//if /data_root is mount, if will case problem
    PartitionManager.Mount_By_Path("/data",false);
    if(TWFunc::Path_Exists("/data/.truedualboot")) {
        LOGINFO("/data/.truedualboot is exists \n");
        LOGINFO("TDB state is on\n");
        tdbstate = true;
    } else {
        LOGINFO("TDB state is off");
        tdbstate = false;

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
     bootmode=  GetBootmode();
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

bool TDBFunc::dualboot_restore_node(void) {
  /*
   * boot,boot1,modem,modem1,system,system1
   * This func just for XiaoMi MI2S
   * Need to modify
   *     struct rpart_list {
   *      ...
   *      } rpl[6] {
   *        .......
   *      };
   *      change the third param;
   *
   */
  struct stat r_st;
  struct rpart_list {
    std::string mount_point;
    std::string backup_node;
    std::string origin_node;

  } rpl[6] {
  {"boot","/dev/part_backup_boot","/dev/block/mmcblk0p18"},
  {"boot1","/dev/part_backup_boot1","/dev/block/mmcblk0p19"},
  {"modem","/dev/part_backup_modem","/dev/block/mmcblk0p1"},
  {"modem1","/dev/part_backup_modem1","/dev/block/mmcblk0p2"},
  {"system","/dev/part_backup_system","/dev/block/mmcblk0p23"},
  {"system1","/dev/part_backup_system1","/dev/block/mmcblk0p24"},
  };

  for (int i = 0; i < 6; i++ ) {
      PartitionManager.UnMount_By_Path(rpl[i].mount_point,true);
      //LOGINFO("mount_point = %s\nbackup_node = %s\norigin_node = %s\n",rpl[i].mount_point.c_str(),rpl[i].backup_node.c_str(),rpl[i].origin_node.c_str());



      if(stat(rpl[i].backup_node.c_str(),&r_st) == 0) {
          //LOGINFO("backup_node '%s' exists \n",rpl[i].backup_node.c_str());
          unlink(rpl[i].origin_node.c_str());

          if(mknod(rpl[i].origin_node.c_str(), r_st.st_mode, r_st.st_rdev)!=0) {
              LOGERR("Could not restore device node '%s'\n",rpl[i].origin_node.c_str());
               return false;

                }
          unlink(rpl[i].backup_node.c_str());//unlink the backup_node for next dualboot_init();
        } else {
          LOGERR("backup_node '%s' not exists \n",rpl[i].backup_node.c_str());
          LOGINFO("First init /dev/part_backup_$NAME\n");
          return false;
        }
   }

      return true;
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

    // check if moved node already exist
    if(stat(part_backup.c_str(), &st) == 0) {
             LOGINFO(" node = %s already exists \n",part_backup.c_str());
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
    //dualboot_init_part("/data");//backup userdata partition
    dualboot_init_part("/system");
    dualboot_init_part("/system1");
    dualboot_init_part("/boot");
    dualboot_init_part("/boot1");
    dualboot_init_part("/modem");
    dualboot_init_part("/modem1");

}

void TDBFunc::dualboot_partinit() {

  LOGINFO("dualboot init partitions\n");
  std::map<string,string> partlist;
  //// \if you want to make it support another device(default is XiaoMi MI2S)
  ///  \ Must change the partition info below , define the  Actual_Block_Device
  ///
  partlist.insert(make_pair("/boot","/dev/block/mmcblk0p18"));
  partlist.insert(make_pair("/boot1","/dev/block/mmcblk0p19"));
  partlist.insert(make_pair("/modem","/dev/block/mmcblk0p1"));
  partlist.insert(make_pair("/modem1","/dev/block/mmcblk0p2"));
  partlist.insert(make_pair("/system","/dev/block/mmcblk0p23"));
  partlist.insert(make_pair("/system1","/dev/block/mmcblk0p24"));

  std::map<string,string>::iterator it;
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
    system = GetCurrentSystem();
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
