#ifndef _INCLUDED_HYDLA_VCS_REDUCE_LINK_SOCKET_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_LINK_SOCKET_H_

#include <iostream>
#include <iomanip>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/system/error_code.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_io.hpp>

#include <sys/time.h>
#include <vector>

namespace hydla {
namespace vcs {
namespace reduce {
class REDUCELinkSocket {
public:
  REDUCELinkSocket();
  ~REDUCELinkSocket();
//  REDUCELinkSocket(const REDUCELinkSocket& old_cl);

  void connect();
  /*
   * stringÇÃëóêM
   */
  int send_string(std::string cmd);

  int read_until_redeval();
  int skip_until_redeval();

  std::string get_s_expr();
  int count_char(std::string str, char query);

private:
  class TimeLogger{
  public:
    typedef enum {begin, end, begin_read_some, end_read_some, begin_if, end_if} name_t;

    TimeLogger() : count(0) {
      names[0] = "begin";
      names[1] = "end";
      names[2] = "begin_read_some";
      names[3] = "end_read_some";
      names[4] = "begin_if";
      names[5] = "end_if";
    }
    ~TimeLogger(){
      output();
    }

    // ë™íËóp
    void get_dtime(name_t name){
      struct timeval tv;
      gettimeofday(&tv, NULL);
      timelog.push_back(timestamp_t(count, name, ((double)(tv.tv_sec) + (double)(tv.tv_usec) * 0.001 * 0.001)));
      count++;
    }
    // èoóÕ
    void output(){
      std::cout << "===TimeLogger::output()===" << std::endl;
      std::cout << "total time: " << std::fixed << std::setw(20) << ( (timelog.end()-1)->get<2>() - timelog.begin()->get<2>() ) << std::endl;
      timelog_t::iterator it = timelog.begin();
      int i=0;
      while(it!=timelog.end()){
	std::cout << "---read " << i << " ---" << std::endl;
      std::cout << std::setw(10) << "count id" << std::setw(20) << "name" << std::setw(20) << "time(diff)" << std::endl;
      if(it==timelog.begin()){
        std::cout << std::setw(10) << it->get<0>()
             << std::setw(20) << names[it->get<1>()]
	 	  << std::fixed << std::setw(20) << it->get<2>()<< std::endl;
      }else{
        std::cout << std::setw(10) << it->get<0>()
             << std::setw(20) << names[it->get<1>()]
	 	  << std::fixed << std::setw(20) << it->get<2>() - (it-1)->get<2>() << std::endl;
      }
	++it;
	for(;it->get<1>()!=begin && it!=timelog.end();it++){
        std::cout << std::setw(10) << it->get<0>()
             << std::setw(20) << names[it->get<1>()]
		  << std::fixed << std::setw(20) << it->get<2>() - (it-1)->get<2>() << std::endl;
	}
	++i;
      }
    }
  private:
    int count;
    std::string names[6];
    typedef boost::tuple<int, name_t, double> timestamp_t;
    typedef std::vector< timestamp_t > timelog_t;
    timelog_t timelog;
  };
  boost::asio::io_service io_service_;
  boost::asio::ip::tcp::socket socket_;

  int count_;
  TimeLogger tl_;
  std::string last_line_;

};

} // namespace reduce
} // namespace vcs
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_REDUCE_LINK_SOCKET_H_
