/// MapleとMathematicaで数式をやり取りするC++プログラムのサンプル
#include "backend_sample.h"
#include "maplec.h"
#include "wstp.h"
#include <bits/stdc++.h>
using namespace std;

void init_and_openlink(void);
void initialize_maple(void);
void error(void);
void read_and_print_expression(void);
void skip_pkt_until(void);
void initialize_for_hylagi(void);
void wolfram_to_maple(void);
void maple_to_wolfram(void);

WSENV ep = (WSENV)0;
WSLINK lp = (WSLINK)0;

string buf;
char err[2048]; /* command input and error string buffers */

void M_DECL textCallBack(void *data, int tag, char *output) { buf = output; }

MKernelVector kv; /* Maple kernel handle */
MCallBackVectorDesc cb = {
    textCallBack,
    0, /* errorCallBack not used */
    0, /* statusCallBack not used */
    0, /* readLineCallBack not used */
    0, /* redirectCallBack not used */
    0, /* streamCallBack not used */
    0, /* queryInterrupt not used */
    0  /* callBackCallBack not used */
};

int main(void) {
  init_and_openlink();
  initialize_for_hylagi();
  initialize_maple();
  buf = "{uv1[t] + uv4[t] == 1, uv1[t] == pl*Derivative[1][ui2][t], ui1[t] == "
        "ui2[t] + ui3[t], pr2*ui1[t] == uv4[t], pr1*ui3[t] == uv2[t], ui3[t] "
        "== pc*Derivative[1][uv3][t], uv1[t] == uv2[t] + uv3[t]}";
  wolfram_to_maple();
  cout << buf << endl;
  buf = "dsolve(" + buf + ");";
  cout << buf << endl;
  EvalMapleStatement(kv, (char *)buf.c_str());
  cout << buf << endl;
  maple_to_wolfram();
  cout << buf << endl;
  WSPutFunction(lp, "ToExpression", 1);
  // WSPutFunction(lp, "HoldForm", 1);

  WSPutString(lp, (char *)buf.c_str());
  WSEndPacket(lp);

  skip_pkt_until();

  read_and_print_expression();
  cout << endl;
  WSPutFunction(lp, "Exit", 0);

  return 0;
}

void read_and_print_expression(void) {
  const char *s;
  int n;
  int i, len;
  double r;

  switch (WSGetNext(lp)) {
  case WSTKSYM:
    WSGetSymbol(lp, &s);
    cout << s;
    WSReleaseSymbol(lp, s);
    break;
  case WSTKSTR:
    WSGetString(lp, &s);
    cout << s;
    WSReleaseString(lp, s);
    break;
  case WSTKINT:
    WSGetInteger(lp, &n);
    cout << n;
    break;
  case WSTKREAL:
    WSGetReal(lp, &r);
    cout << r;
    break;
  case WSTKFUNC:
    if (WSGetArgCount(lp, &len) == 0) {
      error();
    } else {
      read_and_print_expression();
      cout << "[";
      for (i = 1; i <= len; ++i) {
        read_and_print_expression();
        if (i != len)
          cout << ", ";
      }
      cout << "]";
    }
    break;
  case WSTKERROR:
  default:
    error();
  }
}

void error(void) {
  if (WSError(lp)) {
    cerr << "Error detected by WSTP: " << WSErrorMessage(lp) << "." << endl;
  } else {
    cerr << "Error detected by this program." << endl;
  }
  exit(1);
}

void deinit(void) {
  if (ep)
    WSDeinitialize(ep);
}

void closelink(void) {
  if (lp)
    WSClose(lp);
}

void init_and_openlink(void) {
  int err;

  ep = WSInitialize((WSParametersPointer)0);
  if (ep == (WSENV)0)
    exit(1);
  atexit(deinit);

  lp = WSOpenString(ep, "-linkname 'math -wstp'", &err);
  if (lp == (WSLINK)0)
    exit(2);
  atexit(closelink);
}

void closemaple(void) {
  if (kv)
    StopMaple(kv);
}

void initialize_maple(void) {
  /* initialize Maple */
  if ((kv = StartMaple(0, NULL, &cb, NULL, NULL, err)) == NULL) {
    cout << "Fatal error, " << err << endl;
    exit(3);
  }
  atexit(closemaple);
}

void skip_pkt_until(void) {
  /* skip any packets before the first ReturnPacket */
  int pkt;
  while ((pkt = WSNextPacket(lp), pkt) && pkt != RETURNPKT) {
    WSNewPacket(lp);
    if (WSError(lp))
      error();
  }
}

void wolfram_to_maple(void) {
  string s = buf;

  s = regex_replace(s, regex("Derivative\\[1\\]\\[([a-zA-Z0-9]+)\\]\\[t\\]"),
                    "diff($1(t),t)");
  s = regex_replace(s, regex("Derivative\\[2\\]\\[([a-zA-Z0-9]+)\\]\\[t\\]"),
                    "diff($1(t),t,t)");
  s = regex_replace(s, regex("prev\\[([a-zA-Z0-9]+),\\s([0-9]+)\\]"), "$1_$2");
  s = regex_replace(s, regex("\\["), "(");
  s = regex_replace(s, regex("\\]"), ")");
  s = regex_replace(s, regex("=="), "=");
  s = regex_replace(s, regex("E\\^\\("), "exp(");
  s = regex_replace(s, regex("E\\^([a-zA-Z0-9]+)"), "exp($1)");
  s = regex_replace(s, regex("Sin\\("), "sin(");
  s = regex_replace(s, regex("Cos\\("), "cos(");
  s = regex_replace(s, regex("Tan\\("), "tan(");
  buf = s;
}

void maple_to_wolfram(void) {
  string s = buf;

  s = regex_replace(s, regex("\\\\"), ""); // \を除去
  s = regex_replace(s, regex("([a-zA-Z0-9]+)_([0-9]+)"), "prev\[$1,$2\]");
  s = regex_replace(s, regex("\\(t\\)"), "\[t\]");    // "(t)" -> "[t]"
  s = regex_replace(s, regex("_C(\\d+)"), "C\[$1\]"); // "_Cn" -> "C[n]"
  s = regex_replace(s, regex("="), "->");             // "=" -> "->"
  s = regex_replace(s, regex("exp\\("), "E^(");       // "exp" -> "E^"
  s = regex_replace(s, regex("Int"), "Integrate");    // "Int" -> "Integrate"

  /*
   * アルゴリズムについて
   *
   * 例えばs =
   * "((()()(())))"においてs[1]とs[6]の'('と対応する')'をそれぞれ'[',']'に変えたいとする．
   * ちなみに対応する括弧はそれぞれs[9], s[10]．
   * sでi文字目までにいくつの括弧が開いているかを考えると次のような数列aが得られる．
   * a = 123232343210
   * これを見るとa[1] = 2, a[6] = 3である．
   * それぞれについてそれ以降で初めて値がより小さくなる位置を探すと，それぞれa[10]
   * = 1, a[9] = 2であり閉じる位置が分かる．
   * またa[6]未満になる場所を探している間はa[1]未満になることはないので，走査は2回ではなく1回で済むことが分かる．
   *
   * 一般に'['に置き換えたい括弧が来たら以前の値はstackに積んで処理が終わったら取り出すようにすれば，
   * 置き換える括弧の数によらず1回の走査で置き換えができる．
   */

  stack<int> st;      // 後で探す値を積む
  int v = 0, now = 0; // v：探す値，now：括弧がいくつ開いているか
  for (int i = 0; i < s.size(); ++i) {
    string sbstr = s.substr(i, 3);
    if (sbstr == "sin" or sbstr == "cos" or sbstr == "tan" or sbstr == "exp" or
        sbstr == "ate") { // 三角関数かIntegrate
      if (v != 0)
        st.push(v);
      if (sbstr != "ate")
        s[i] = toupper(s[i]); // sin -> Sin, cos -> Cos, tan -> Tan
      i += 3;                 // '('の場所に飛ぶ
      s[i] = '[';
      now++;
      v = now;
      continue;
    }

    if (s[i] == '(')
      now++;
    else if (s[i] == ')') {
      now--;
      if (now == v - 1) { // sinなどの括弧が閉じたら
        s[i] = ']';
        if (st.empty()) {
          v = 0;
        } else {
          v = st.top();
          st.pop();
        }
      }
    }
  }
  buf = s;
}

void initialize_for_hylagi(void) {
  WSPutFunction(lp, "SetOptions", 2);
  WSPutSymbol(lp, "$Output");
  WSPutFunction(lp, "Rule", 2);
  WSPutSymbol(lp, "PageWidth");
  WSPutSymbol(lp, "Infinity");
  WSEndPacket(lp);
  skip_pkt_until();
  WSNewPacket(lp);

  // 警告無視
  WSPutFunction(lp, "Set", 2);
  WSPutSymbol(lp, "optIgnoreWarnings");
  WSPutSymbol(lp, "True");
  WSEndPacket(lp);
  skip_pkt_until();
  WSNewPacket(lp);

  WSPutFunction(lp, "Set", 2);
  WSPutSymbol(lp, "optTimeConstraint");
  WSPutFunction(lp, "ToExpression", 1);
  WSPutString(lp, "1");
  WSEndPacket(lp);
  skip_pkt_until();
  WSNewPacket(lp);

  WSPutFunction(lp, "Set", 2);
  WSPutSymbol(lp, "optSimplifyLevel");
  WSPutInteger(lp, 1);
  WSEndPacket(lp);
  skip_pkt_until();
  WSNewPacket(lp);

  WSPutFunction(lp, "Set", 2);
  WSPutSymbol(lp, "optDSolveMethod");
  WSPutInteger(lp, 1);
  WSEndPacket(lp);
  skip_pkt_until();
  WSNewPacket(lp);

  WSPutFunction(lp, "Set", 2);
  WSPutSymbol(lp, "optSolveOverReals");
  WSPutSymbol(lp, "True");
  WSEndPacket(lp);
  skip_pkt_until();
  WSNewPacket(lp);

  WSPutFunction(lp, "ToExpression", 1);
  WSPutString(lp, backend_sample());
  WSEndPacket(lp);
  skip_pkt_until();
  WSNewPacket(lp);
}
