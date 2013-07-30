#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS) 
#else

#ifndef _INCLUDED_HYDLA_VCS_REDUCE_LINK_IPC_H_
#define _INCLUDED_HYDLA_VCS_REDUCE_LINK_IPC_H_

#include "../../parser/SExpParser.h"
#include "REDUCELink.h"
#include <stdexcept>
#include <string>

namespace hydla {
namespace vcs {
namespace reduce {

/**
 * REDUCEサーバとの接続クライアント、サーバ接続とstringの送受信を行う
 * IPC経由による実装
 */
class REDUCELinkIpc : public REDUCELink {
public:
  enum{ MAXLINE = 512 };

  /**
   * reduceプロセスの生成
   */
  REDUCELinkIpc();
  /**
   * reduceプロセスに終了命令を出す
   */
  ~REDUCELinkIpc();

  /**
   * end_of_redeval_行まで文字列をgetlineする
   * skip_until_redevalを推奨
   * \return 0
   */
  int read_until_redeval();

  /**
   * end_of_redeval_行まで文字列をgetlineする
   * \return 0
   */
  int skip_until_redeval();

  /**
   * 受信した複数行のstringを結合して破損のないLisp式を戻す
   * \return REDUCEから受け取るS式
   */
  std::string get_s_expr();

  /**
   * 受信した複数行のstringを結合してSExpParserを戻す
   * \return REDUCEから受け取るS式をパースしたもの
   */
  const hydla::parser::SExpParser get_as_s_exp_parser();

  /**
   * stringの送信
   * \param cmd REDUCEへ送信する文字列
   * \return 0
   */
  int send_string(const std::string cmd);
private:

  /**
   * getlineを行い、異常を見つけた場合throwする
   * \param cmd 呼び出し元の関数名, デバッグ用
   * \return getlineの戻り値
   */
  std::string getline_with_throw(const std::string& cmd);

  /**
   * bufferから擬似的にgetlineを作成
   * \return getlineの戻り値
   */
  std::string getline();
 
  /**
   * reduceプロセスを生成し、送信用と受信用のパイプのディスクリプタを設定する
   */
  void initProcess();

  /** reduceプロセスへの受信用ディスクリプタ */
  int readfd_;

  /** reduceプロセスへの送信用ディスクリプタ */
  int writefd_;
  
  /** fork()の戻り値 */
  pid_t pid_;

  /** getline用 */
  std::string prev_line_;

  /** デリミタ */
  static const std::string end_of_redeval_;

};

} // namespace reduce
} // namespace vcs
} // namespace hydla

#endif // _INCLUDED_HYDLA_VCS_REDUCE_LINK_IPC_H_

#endif // defined(_MSC_VER) || defined(_MSC_EXTENSIONS) 

