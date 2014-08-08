#ifndef TDB_FUNC_HPP
#define TDB_FUNC_HPP
/*


*/

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


/*
/dev/block/mmcblk0p1     /modem
/dev/block/mmcblk0p2     /modem1
/dev/block/mmcblk0p18    /boot
/dev/block/mmcblk0p19    /boot1
/dev/block/mmcblk0p23    /system
/dev/block/mmcblk0p24    /system1

*/

typedef std::map<std::string,struct stat> part_node;
typedef std::map<std::string, struct stat> part_backup;
typedef  std::map<part_node,part_backup> node_backup;


using namespace std;
class TDBFunc {
public:
    TDBFunc();
    ~TDBFunc();
public:

 int SetBootmode(string bootmode);//set active system  , bootmode == system0 / system1
 string GetBootmode();//return system0 || system1

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
 static void dualboot_partinit();


private:
static std::map<std::string,struct stat> partst;



static  std::map<std::string, node_backup> Node_Part;

};
extern TDBFunc TDBManager;

#endif
