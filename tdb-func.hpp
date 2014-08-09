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

#ifndef TDB_FUNC_HPP
#define TDB_FUNC_HPP

#include <string>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>


#define PATH_BOOT_NODE    "/dev/block/mmcblk0p18"
#define PATH_BOOT_BACKUP  "/dev/part_backup_boot"
#define PATH_BOOT1_NODE   "/dev/block/mmcblk0p19"
#define PATH_BOOT1_BACKUP "/dev/part_backup_boot1"

#define PATH_MODEM_NODE   "/dev/block/mmcblk0p1"
#define PATH_MODEM_BACKUP "/dev/part_backup_modem"
#define PATH_MODEM1_NODE  "/dev/block/mmcblk0p2"
#define PATH_MODEM1_BACKUP "/dev/part_backup_modem1"

#define PATH_SYSTEM_NODE     "/dev/block/mmcblk0p23"
#define PATH_SYSTEM_BACKUP   "/dev/part_backup_system"
#define PATH_SYSTEM1_NODE    "/dev/block/mmcblk0p24"
#define PATH_SYSTEM1_BACKUP  "/dev/part_backup_system1"

#define PATH_USERDATA_NODE "/dev/block/mmcblk0p26"
#define PATH_USERDATA_BACKUP "/dev/part_backup_data"

/*
/dev/block/mmcblk0p1     /modem
/dev/block/mmcblk0p2     /modem1
/dev/block/mmcblk0p18    /boot
/dev/block/mmcblk0p19    /boot1
/dev/block/mmcblk0p23    /system
/dev/block/mmcblk0p24    /system1

*/

class TDBFunc {
public:
    TDBFunc();
    ~TDBFunc();
public:

    int SetBootmode(std::string bootmode);//set active system  , bootmode == system0 / system1
    std::string GetBootmode();//return system0 || system1
    bool GetTDBState(); //用于判断/data/.truedualboot文件的状态，当存在时返回true,不存在返回false
    std::string GetCurrentSystem(); // 用于获取当前激活的系统，system0 | system1
    bool SetUpTDB();   //用于开启真实双系统
    bool DisableTDB();  //用于关闭真实双系统
    bool GenTDBFile();//用于生成/data/.truedualboot 文件
    bool TDB_before_update();
    bool Replace_Device_Node(std::string part,struct stat* st);//用于切换设备的device node
    void dualboot_init_part(std::string part);//用于初始化设备
    void dualboot_prepare_env(void);//初始化变量
    void dualboot_setup_env(void);//环境配置
    void dualboot_init(void);//包含 初始化变量 && 环境配置
    bool dualboot_restore_node(void);// restore the node for change active system
    static void dualboot_partinit(); //init the partition info map<string,string> partst;

private:
    static std::map<std::string,struct stat> partst;

};
extern TDBFunc TDBManager;

#endif
