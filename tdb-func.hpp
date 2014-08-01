#ifndef TDB_FUNC_HPP
#define TDB_FUNC_HPP
/*


*/

#include <string>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;
class TDBFunc {
public:
    TDBFunc();
    virtual ~TDBFunc();
public:

 int SetBootmode(string bootmode);//set active system  , bootmode == system0 / system1
 string GetBootmode();//return system0 || system1

 static bool GetTDBState(); //用于判断/data/.truedualboot文件的状态，当存在时返回true,不存在返回false
 static std::string GetCurrentSystem(); // 用于获取当前激活的系统，system0 | system1
 bool SetUpTDB();   //用于开启真实双系统
 bool DisableTDB();  //用于关闭真实双系统
 bool GenTDBFile();//用于生成/data/.truedualboot 文件
 bool Replace_Device_Node(std::string part,struct stat* st);//用于切换设备的device node
 void dualboot_init_part(std::string part);//用于初始化设备
 void dualboot_prepare_env(void);//初始化变量
 void dualboot_setup_env(void);//环境配置
 void dualboot_init(void);//包含 初始化变量 && 环境配置

private:
static std::map<std::string,struct stat> partst;

};
extern TDBFunc TDBManager;

#endif
