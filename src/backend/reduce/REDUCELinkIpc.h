#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS) 
#else

#pragma once

#include "REDUCELink.h"


namespace hydla {
namespace backend {
namespace reduce {

/**
 * REDUCEサーバとの接続クライアント、サーバ接続とstringの送受信を行う
 * IPC経由による実装
 */
class REDUCELinkIpc : public REDUCELink {
public:

  static const int MAXLINE = 512;

  void send_string_to_reduce(const char* str, int len);
  void send_string_to_reduce(const char* str);

  /**
   * REDUCEプロセスの生成
   */
  REDUCELinkIpc(const simulator::Opts &opts);
  /**
   * reduceプロセスに終了命令を出す
   */
  ~REDUCELinkIpc();

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
   * 受信した複数行のstringを結合してSExpParseTreeを戻す
   * \return REDUCEから受け取るS式をパースしたもの
   */
  const hydla::parser::SExpAST get_as_s_exp_parse_tree();


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
} // namespace backend
} // namespace hydla

#endif // defined(_MSC_VER) || defined(_MSC_EXTENSIONS) 
