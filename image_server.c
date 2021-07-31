#include"db.hpp"
#include"httplib.h"
#include<regex>
#include<openssl/md5.h>

#define WWW_ROOT "./www"
#define IMAGE_PATH "/images/"

//void db_test()
//{
  //实例化一个数据库表，实现数据的增删改查操作进行局部功能测试
 // MYSQL *mysql = image_server::MysqlInit();
 // image_server::TableImage table_image(mysql);

//  Json::Value image;
  //image["name"] = "阴天";
  //image["fsize"] = 4096;
  //image["fpath"] = "./www/test.png";
  //image["furl"] = "./www/test.png";
  //image["fmd5"] = "12afsad1afaw31fasf";
  //table_image.Insert(image);
  
  //image["name"] = "阴雨绵绵";
  //table_image.Update(5,image);
  
  //table_image.GetAll(&image);
  //Json::StyledWriter writer;
  //std::cout << writer.write(image) << std::endl;

 // table_image.GetOne(1,&image);
 // Json::StyledWriter writer;
 // std::cout << writer.write(image) << std::endl;

 // image_server::MysqlRelease(mysql);
//}

image_server::TableImage *image_table;


bool WriteFile(const std::string &filename,const std::string body)
{
  std::ofstream ofs(filename,std::ios::binary); //二进制打开文件
  if(ofs.is_open() == false)
  {
    printf("open file %s failed!\n",filename.c_str());
    return false;
  }
  ofs.write(body.c_str(),body.size());
  if(ofs.good() == false)
  {
    printf("write file body failed!\n");
    ofs.close();
    return false;
  }
  ofs.close();
  return true;
}

void MD5_CALC(const std::string &body,std::string *md5)
{
  unsigned char buf[16] = {0};
  MD5((const unsigned char *)body.c_str(), body.size(), buf);
  for(int i = 0;i < 16;i++)
  {
    char tmp[3] = {0};
    sprintf(tmp, "%02x", buf[i]);
    *md5 += tmp;
  }
}

void GetAllImage(const httplib::Request &req, httplib::Response &rsp)
{
  printf("Process request to get all picture meta information\n");
  Json::Value images;
  bool ret = image_table->GetAll(&images);
  if(ret == false)
  {
    printf("Failed to get all data from databases\n");
    rsp.status = 500;//服务器内部错误 
    return;
  }
  Json::FastWriter writer;
  //rsp.body = writer.write(images);
  //rsp.set_header("Content-Type", "application/json");
  rsp.set_content(writer.write(images), "application/json");
  std::cout << rsp.body << std::endl;

  rsp.status = 200;
  printf("Get all picture meta information request processed successfully\n");
  return ;
}


void GetOneImage(const httplib::Request &req, httplib::Response &rsp)
{
  int image_id = std::stoi(req.matches[1]);
  Json::Value image;
  bool ret = image_table->GetOne(image_id,&image);
  if(ret == false)
  {
    printf("Failed to get one data from database\n");
    rsp.status = 500;
    return;
  }
  Json::FastWriter writer;
  rsp.set_content(writer.write(image), "application/json");
  return ;
}

void DeleteImage(const httplib::Request &req, httplib::Response &rsp)
{
  int image_id = std::stoi(req.matches[1]);
  Json::Value image;
  bool ret = image_table->GetOne(image_id, &image);
  if(ret == false)
  {
    printf("Failed to get image meta information when deleting picture\n");
    rsp.status = 500;
    return;
  }
  std::string real_path = image["fpath"].asString();
  unlink(real_path.c_str());//删除文件
  //从数据库中删除图片元信息；
  ret = image_table->Delete(image_id);
  if(ret == false)
  {
    printf("Failed to delete image meta information when deleting picture\n");
    rsp.status = 500;
    return;
  }
  return ;
}

void UpdateImage(const httplib::Request &req, httplib::Response &rsp)
{
  int image_id = std::stoi(req.matches[1]);
  Json::Value image;
  Json::Reader reader;
  bool ret = reader.parse(req.body,image);
  if(ret == false)
  {
    printf("The modified picture information is incorrecr\n");
    rsp.status = 400;
    return;
  }

  ret = image_table->Update(image_id, image);  
  if(ret == false)
  {
    printf("Failed to modify image information from database\n");
    rsp.status = 500;
    return;
  }
  return ;
}

void AppendImage(const httplib::Request &req, httplib::Response &rsp)
{
  bool ret = req.has_file("imagefile");//判断是否存在该文件
  if(ret == false)
  {
    printf("The name of the uploaded file was not found\n");
    rsp.status = 400;
    return;
  }
  auto file = req.get_file_value("imagefile");//获取文件信息
  int fsize = file.content.size();
  std::string md5_str;
  MD5_CALC(file.content, &md5_str);
  std::string image_url = IMAGE_PATH + file.filename;
  std::string real_path = WWW_ROOT + image_url;

  Json::Value image;
  image["name"] = file.filename;
  image["fsize"] = (Json::Value::Int64)file.content.size();
  image["fpath"] = real_path;
  image["furl"] = image_url;
  image["fmd5"] = md5_str;

  ret = image_table->Insert(image);
  if(ret == false)
  {
    printf("Failed to append image information into database\n");
    rsp.status = 500;
    return;
  }

  WriteFile(real_path, file.content);

  rsp.status = 200;
  return;
}




int main()
{
  //db_test();
  MYSQL *mysql = image_server::MysqlInit();
  image_table = new image_server::TableImage(mysql);
  httplib::Server srv;
  //srv.set_base_dir(WWW_ROOT);//设置静态资源根目录
  //注册请求路由信息
  //获取图片元信息请求
  srv.Get("/image", GetAllImage);
  //获取指定图片数据请求(可以不用处理，httplib会自动处理)
  //获取指定图片元信息请求
  srv.Get(R"(/image/(\d+))", GetOneImage);
  //删除图片请求
  srv.Delete(R"(/image/(\d+))", DeleteImage);
  //上传图片请求
  srv.Post("/image",AppendImage);
  //修改图片元信息请求
  srv.Put(R"(/image/(\d+))", UpdateImage);

  srv.listen("0.0.0.0", 9000);

  image_server::MysqlRelease(mysql);
  return 0;
}
