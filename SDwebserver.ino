#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};//mac地址
IPAddress ip(192, 168, 31, 177);//ip地址
EthernetServer server(80);//访问端口
String fakename = "Nginx/1.8.0 (ATmega328p/Ubuntu 12.04 LTS)"; //装逼参数(伪装服务器）
String res = "", path = "";
EthernetClient client;
void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  //启动sd
  Serial.print("init sd:");
  if (!SD.begin(4)) {
    Serial.println("failed");
    return;
  }
  Serial.println("done");
  //启动eth
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("ip:");
  Serial.println(Ethernet.localIP());

}


void loop() {
  client = server.available();
  if (client) {
    Serial.println("new client");
    
    delay(200);
    res = ""; path = ""; /*httpget = "", */
    int resend = 1;
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //Serial.write(c);
        //只读取一行
        if (c != '\n' && resend) {
          res += c;
        } else {
          resend = 0;
        }

        if (c == '\n' && currentLineIsBlank) {
          //判断GET头完整性
          if ((res.indexOf("GET ") != -1) && (res.indexOf(" HTTP") != -1)) {
            //判断是否存在get参数
            if (res.indexOf('?') != -1) {
              //httpget = res.substring(res.indexOf('?') + 1, res.indexOf(" HTTP"));
              path = res.substring(res.indexOf("GET ") + 4, res.indexOf('?'));
            } else {
              path = res.substring(res.indexOf("GET ") + 4, res.indexOf(" HTTP"));
            }
            Serial.println(res);
            //Serial.println("GET:" + httpget);
            Serial.println("path:" + path);
            //delay(100);
            File s = SD.open(path);
            if (s) {
              webprintDirectory(s);
            } else {
              client.println(s);
              s.rewindDirectory();
              s.close();
            }
            //client.println("<p>" + fakename + "</p></body></html>");
            break;
          }
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");

  }
}

void webprintDirectory(File dir) {
  //判断是否为文件
  if (!dir.isDirectory()) {

    if (path.indexOf('.') != -1) {
      //获取后缀名
      http_header("200 OK", path.substring(path.indexOf('.') + 1));
    } else {
      http_header("200 OK", "txt");//文件没有后缀名 默认显示文本格式
    }
    //读文件
    while (dir.available()) {
      char sc = dir.read();
      client.print(sc);
    }
    dir.close();
    dir.rewindDirectory();
    return;//退出
  }
  //判断不是文件
  http_header("200 OK", "htm");
  client.print("<!DOCTYPE HTML><html><body><h1>200 Success</h1><br />PATH:" + path + "<hr />");
  dir.rewindDirectory();//索引回到第一个位置
  while (true) {
    File entry =  dir.openNextFile();
    if (!entry) {
      //没有下一个索引了
      client.print("<br />No more files");
      dir.rewindDirectory();
      break;
    }
    if (path == "/") {
      client.print("<br /><a href=\"" + path);
    } else {
      client.print("<br /><a href=\"" + path + "/");
    }
    client.print(entry.name());
    client.print("\"target=\"_self\">");
    client.print(entry.name());
    client.print("</a>");
    if (!entry.isDirectory()) {
      // files have sizes, directories do not
      client.print("&nbsp;&nbsp;");
      client.println(entry.size(), DEC);
    }

    entry.close();
  }
  client.println("<p>" + fakename + "</p></body></html>");
  dir.close();
  delay(200);
}

void http_header(String statuscode, String filetype) {
  filetype.toLowerCase();//把后缀名变小写
  client.println("HTTP/1.1 " + statuscode);
  client.print("Content-Type: ");
  //判断文件mime类型
  if (filetype == "htm" || filetype == "html") {
    client.println("text/html");
  }
  if (filetype == "png" || filetype == "jpg" || filetype == "bmp" || filetype == "gif") {
    client.println("image/" + filetype);
  } else {
    client.println("text/plain");
  }
  client.println("Connection: close");
  client.println();
}
