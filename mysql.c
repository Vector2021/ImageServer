//MySQL接口使用用例编写
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<mysql/mysql.h>

int main()
{

  //1.初始化句柄
  MYSQL *mysql = mysql_init(NULL);
  if(mysql == NULL)
  {
    printf("mysql init error\n");
    return -1;
  }
  //2.连接服务器
  //mysql_real_connect(句柄，主机地址，用户名，密码，库名称，端口号，套接字，标志位)
 if(mysql_real_connect(mysql,"127.0.0.1", "root", "2415357472", "image_server", 0, NULL, 0) == 0)
 {
   printf("connect mysql server failed:%s\n",mysql_error(mysql));
   mysql_close(mysql);
   return -1;
 }
  //3.选择数据库
  
  //4.设置字符集
 if(mysql_set_character_set(mysql,"utf8")!=0){
   printf("mysql character failed! : %s\n",mysql_error(mysql));
   mysql_close(mysql);
   return -1;
 }
 
 //5.执行语句，获取结果集，获取条数与列表，遍历结果，释放结果集
  
 // (1)增添数据：(经过验证，插入功能正常)
 // char *sql_insert = "insert table_image values(null,'枫叶', 2048, './a.jpg', '/image/a.jpg', '132qkdjaaweq2123kasd', now());";
 // if(mysql_query(mysql,sql_insert) != 0){
 //   printf("query sql failed: %s\n",mysql_error(mysql));
 //   mysql_close(mysql);
 //   return -1;
 // }
 

 
 //6.关闭数据库
  mysql_close(mysql);
  return 0;

}

