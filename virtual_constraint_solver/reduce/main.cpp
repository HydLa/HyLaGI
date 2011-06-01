//コンパイル: g++ -o main main.cpp -lboost_system-gcc43-mt-1_38

#include <iostream>
#include <fstream>

#include <string>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>

#define END_TST "endtst___"
#define END_TST_OFFNAT "endtst___$"

using namespace std;
using namespace boost::asio;
using namespace boost::posix_time;

class asio_cl {
  //TCP/IPのREDUCEとのインタフェース
  //TODO: REDUCEが起動してないなどで接続できない場合の分岐

  ip::tcp::iostream s;
  boost::asio::io_service io_service;


public:
  asio_cl(){

/*
  system関数でreduceを起動するとreduceが終了するまで処理が進まない!
  system("cd /home/yysaki/HydLa/branches/HydLa_v0.5.x/reduce/");
  system("reduce -F-");
*/
  boost::system::error_code error = boost::asio::error::host_not_found;
    s.connect("localhost", "1206");
  }

  int send_to_reduce(string cmd){
    cout << "---- send phase ----" << endl;
    cout << "cmd: " << cmd << endl;
    s << cmd;
    s << "\r\n";

    return 0;
  }

  //TODO: recvが取得出来ない場合のタイムアウト処理
  //TODO: 終了条件の取得
  int recv_from_reduce(){
    int i = 0;
    string line;

    cout << "---- recv phase ----" << endl;
    while(getline(s, line)){
  //    cout << i++ << "番目, length: " << line.length() << endl;
      cout << line << endl;
      if(line.length()==0) break;
    }

    return 0;
  }

  int count_char(std::string str, char query){
    int count = 0, i = 0;
    while(true){
      i = str.find(query, i);
      if(i==std::string::npos) break;
//    std::cout << "i[" << count << "]: " << i << std::endl;

      count++; i++;
    }

    return count;
  }

  /*
  * 端末上で"solve({x+y=2,3x+2y=5},{x,y});" などと入力
  */
  void scanf_test(){
    cout << "---- scanf phase ----" << endl;
    string scaned;
    while(true){
      cout << "cmd: ";
      cin >> scaned;
      if(scaned=="exit") break;
      send_to_reduce(scaned);
      recv_from_reduce();
    }
  }

/*
Goal 以下の3命令を送り計算を成功させ答えをrecv
     recvのwhileの抜ける条件を、line[0]=='('とする
     '('の数と')'の数を数え、それぞれが一致するテキストを返り値とする


1: vars_:={y,yy,yyy,z,zz};
2: expr_:={zz - yy = 0, yyy + 10 = 0, yy = 0, True, z - y = 0, y - 10 = 0};
3: symbolic reval '(isconsistent vars_ expr_);
*/
  std::string isConsistent_test(){
    cout << "---- isConsistent_test() phase ----" << endl;
    std::string cmd, line;
    cmd = "vars_:={y,yy,yyy,z,zz};";
    send_to_reduce(cmd);
    if(getline(s, line)) std::cout << line;

    cmd = "expr_:={zz - yy = 0, yyy + 10 = 0, yy = 0, True, z - y = 0, y - 10 = 0};";
    send_to_reduce(cmd);
    if(getline(s, line)) std::cout << line;

    cmd = "symbolic reval '(isconsistent vars_ expr_);";
    send_to_reduce(cmd);
    if(getline(s, line)) std::cout << line;

    // "(.*"のものをとりあえず終了条件
    while(line[0]!='(' && line[line.length()-1]!=')'){
      getline(s, line);
      std::cout << line << std::endl;
//      std::cout << line[line.length()-1] << std::endl;
    }

//  string中の '('を数える関数
//  int i = count_char(line, '(');





    while(count_char(line, '(')!=count_char(line, ')')){
      std::string tmp;
      getline(s, tmp);
      line = line + tmp;
    }


    cout << "---- exit isConsistent_test() phase ----" << endl;
    return line;
  }


/*
  {";", "", "end;"} の順に文字列を取得したことを終了判定として、終了判定までgetline(s, line)をする
  TODO: write "endoffile___"; をファイル側に仕込み終了判定とする
*/
  void file_input_test(){
    std::cout << "<main> file_input_test: " << "lib.red" << std::endl;

    std::string line[3];
    std::string cmd = "in \"lib.red\";";
    send_to_reduce(cmd);

    while(true){
      getline(s, line[0]);
      std::cout << line[0] << std::endl;
//      cout << "line[0]" << line[0] << "         line[1] " << line[1] << "         line[2] " << line[2] << endl;
      if(line[0]=="end;" && line[1]=="" && line[2]==";") break;
      line[2] = line[1];
      line[1] = line[0];
    }
  }


  void func_test(const char* filename){
    std::cout << "<main> func_test: " << filename << std::endl;

    std::ifstream ifs(filename);
    std::string cmd;
//送信部
    while(getline(ifs, cmd)){
      s << cmd;
    }

	std::string line[3];
//受信部
    while(true){
      getline(s, line[0]);
      cout << line[0] << endl;
      if(line[0]==END_TST) break;
      if(line[0]==END_TST_OFFNAT) break;
      if(line[0]=="end;" && line[1]=="" && line[2]==";") break;
      line[2] = line[1];
      line[1] = line[0];
    }
  }
};



int main(int argc, char** argv){
  asio_cl cl;

  cl.recv_from_reduce();

//  "in \"lib.red\";"をsendする
  cl.file_input_test();

//  func_test呼出分岐
  if(argc==2){
    cl.func_test(argv[1]);
    return 0;
  }else{
    std::cout << "argc!=2" << std::endl;
  }


  std::string ans;
//  "isConsistent(expr, vars)"をsendする
//  ans = cl.isConsistent_test();
//  std::cout << "ans= " << ans << std::endl;
//  std::cout << "ans has " << cl.count_char(ans, '(') << "'('s, and "
//          << cl.count_char(ans, ')') << "')'s" << std::endl;

// scanf -> send -> recv -> scanfの繰り返し
  cl.scanf_test();

  cl.send_to_reduce("bye;");
  cl.recv_from_reduce();
}

