#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};//mac地址
IPAddress ip(192, 168, 1, 177);//ip地址
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

            //client.println("<!DOCTYPE HTML><html><body><h1>200 Success</h1><br />PATH:" + path + "<hr />");
            delay(200);
            File s = SD.open(path);
            if (s) {
              webprintDirectory(s);
              s.rewindDirectory();
              //s.close();
              //s.close();
            } else {
              client.println(s);
              s.rewindDirectory();
              s.close();

            }
            // q.close();
            delay(100);
            //client.println("<p>" + fakename + "</p></body></html>");
            break;
          } else {
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
  //File entry;
  //client.print("enter webp");
  if (!dir.isDirectory()) {
    // files have sizes, directories do not
    http_header("200 OK","image/jpeg");
    //client.print(dir.name());
    //client.print("read file");
    while (dir.available()) {
      //client.print("reading");
      if(dir.peek()!=-1){
      client.print(dir.peek());
      //client.print("read file end");
      }
    }
    dir.rewindDirectory();

    dir.close();
    return;
  }
  client.print("<!DOCTYPE HTML><html><body><h1>200 Success</h1><br />PATH:" + path + "<hr />");
  client.print("exit diris");
  dir.rewindDirectory();
  while (true) {
    File entry =  dir.openNextFile();
    Serial.println("opennextfiles");
    Serial.print(entry);
    if (!entry) {
      // no more files
      client.print("<br/>no more files");
      //entry.close();
      dir.rewindDirectory();
      break;
    }
    client.print("<br/>is dir/");
    client.print(entry.name());
    if (!entry.isDirectory()) {
      // files have sizes, directories do not
      client.print("\t\t");
      client.println(entry.size(), DEC);

    }

    entry.close();
  }
  client.println("<p>" + fakename + "</p></body></html>");
  dir.close();
  delay(10);
}

void http_header(String statuscode, String filetype) {
  client.println("HTTP/1.1 " + statuscode);
  client.println("Content-Type: " + filetype);
  client.println("Connection: close");
  client.println();
}
