#pragma once
#include <memory>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <array>
#include <queue>
#include <stack>
#include <numeric>
#include <functional>
#include <cstdint>

#include <boost/optional.hpp>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/sum.hpp>
#include <boost/accumulators/statistics/sum_kahan.hpp>

using namespace boost::accumulators;

#include <boost/multiprecision/cpp_dec_float.hpp>

namespace mp = boost::multiprecision;

#include <kv/affine.hpp>
#include <kv/ode-maffine.hpp>
#include <kv/psa.hpp>
#include <kv/dd.hpp>

enum Pos { BottomLeft, BottomRight, TopLeft, TopRight };
inline std::string ToS(Pos p)
{
    switch(p)
    {
        case BottomLeft:return "_";
        case BottomRight:return "-";
        case TopLeft:return "=";
        default:return "^";
    }
}

/*
//very slow
using Accessor = std::deque<Pos>;

namespace std
{
  template <> struct hash<Accessor>
  {
    size_t operator()(const Accessor& accessor)const
    {
      if(accessor.empty())
      {
        return 0;
      }
      const std::hash<unsigned> obj;
      size_t h = obj(static_cast<unsigned>(accessor[0]));
      for(size_t i = 1; i < accessor.size(); ++i)
      {
        const size_t h1 = obj(static_cast<unsigned>(accessor[i]));
        h = h ^ (h1 << 1);
      }
      return h;
    }
  };
}
//*/

//*
//optimized
class Accessor;
namespace std
{
  template <> struct hash<Accessor>;
}

class Accessor
{
public:
  Accessor() = default;

  void push_front(Pos pos)
  {
    const std::uint64_t p = static_cast<unsigned char>(pos);
    const std::uint64_t topElem = 0xC000000000000000ULL;

    const size_t shift = (size_ % 32);
    if(packed.empty())
    {
      packed.push_back(p);
    }
    else
    {
      if(shift == 0)
      {
        packed.push_back(0);
      }

      for(int i = packed.size() - 1; 1 <= i; --i)
      {
        packed[i] = (packed[i] << 2) + ((packed[i - 1] & topElem) >> 62);
      }
      packed[0] = (packed[0] << 2) + p;
    }
    ++size_;
  }

  void pop_front()
  {
    for(size_t i = 0; i + 1 < packed.size(); ++i)
    {
      packed[i] = (packed[i] >> 2) + (packed[i + 1] & 3ULL);
    }
    packed[packed.size() - 1] >>= 2;
    if(size_ % 32 == 0)
    {
        packed.pop_back();
    }
    --size_;
  }

  void push_back(Pos pos)
  {
    const std::uint64_t p = static_cast<unsigned char>(pos);
    
    const size_t shift = (size_ % 32);
    if(shift == 0)
    {
      packed.push_back(p);
    }
    else
    {
      packed.back() = packed.back() + (p << (shift*2));
    }
    ++size_;
  }

  size_t size()const {return size_;}
  bool empty()const {return size_ == 0;}

  Pos operator[](size_t index)const
  {
    const size_t localIndex = index % 32;
    return static_cast<Pos>((packed[index / 32] & (3ULL << (localIndex*2))) >> (localIndex*2));
  }
  Pos front()const{return operator[](0);}
  Pos back()const{return operator[](size_ - 1);}

  Accessor take(size_t takeNum)const
  {
    if(takeNum == 0)return {};

    Accessor clone;
    const size_t backIndex = (takeNum - 1) / 32;
    clone.packed = std::vector<std::uint64_t>(packed.begin(), packed.begin() + backIndex + 1);
    const int shiftSize = 32 - (takeNum % 32);
    clone.packed.back() = ((clone.packed.back() << (shiftSize*2)) >> (shiftSize*2));
    clone.size_ = takeNum;

    return clone;
  }

  bool operator==(const Accessor& other)const
  {
    return packed == other.packed;
  }
    
  void print()const
  {
    std::cout << "packedsize: " << packed.size() << ", realsize: " << size_<< std::endl;
    for(int i = 0; i < size_; ++i)
    {
      std::cout << ToS(operator[](i));
    }
  }

private:
  friend std::hash<Accessor>;
  size_t size_ = 0;
  std::vector<std::uint64_t> packed;
};

namespace std
{
  template <> struct hash<Accessor>
  {
    size_t operator()(const Accessor& a)const
    {
      if(a.packed.empty())
      {
        return 0;
      }
      const std::hash<uint64_t> obj{};
      size_t h = obj(a.packed[0]);
      for(size_t i = 1; i < a.packed.size(); ++i)
      {
        const size_t h1 = obj(a.packed[i]);
        h = h ^ (h1 << 1);
      }
      return h;
    }
  };
}
//*/

namespace hydla
{
namespace interval
{
using afd = kv::affine<double>;
using itv = kv::interval<double>;

template<typename T>
T Lerp(const T& a, const T& b, double t)
{
  return a + (b - a)*t;
}

double ddd = 0.;

using itvd = itv;
using itv_stack_t = std::stack<itvd>;

struct Parallelogram
{
  Parallelogram() = default;

  Parallelogram(double x0, double x1, double a, const itvd& b) :
    m_x0(x0), m_x1(x1), m_a(a), m_b(b)
  {}

  double w()const
  {
    return width(m_b);
  }

  double x0()const
  {
    return m_x0;
  }

  double x1()const
  {
    return m_x1;
  }

  double a()const
  {
    return m_a;
  }

  itvd b()const
  {
    return m_b;
  }

  double m_x0;
  double m_x1;
  double m_a;
  itvd m_b;
};


struct QuadTree
{
  QuadTree() = default;
  /*QuadTree(const itvd& theta1, const itvd&theta2, const std::function<afd(const afd&, const afd&)>& func) : variable1(theta1), variable2(theta2)
  {
    make(func);
  }*/
  QuadTree(double& v1_l, double& v1_u, double& v2_l, double& v2_u, const std::function<afd(const afd&, const afd&)>& func) : 
    v1Lower(v1_l),
    v1Upper(v1_u),
    v2Lower(v2_l),
    v2Upper(v2_u)
  {
    make(func);
  }

  bool isLeaf()const {
    return m_childs.empty();
  }

  void make(const std::function<afd(const afd&, const afd&)>& func)const
  {
    //std::cout << "Make QuadTree begin" << std::endl;
    afd::maxnum() = 0;

    //q' = p
    //p' = K (1 - q^2) p - q
    const itvd variable1(v1Lower, v1Upper);
    const itvd variable2(v2Lower, v2Upper);
    afd p(variable1);
    afd q(variable2);
    afd output = func(p, q);
    /*
    std::cout << "--------------------" << std::endl;
    std::cout << "p.size: " << p.a.size() << ", q.size: " << q.a.size() << ", output.size: " << output.a.size() << std::endl;
    std::cout << output << std::endl;
    */

    const int inputIndex = p.a.size() - 1;
    const int errorIndex = q.a.size();

    //kv::dd xa(-1.25);
    
    //std::cout << "testabs: " << abs(xa) <<", " << abs(-0.25) << ", " << abs(1.25)   << std::endl;

    double er = 0;
    for (int j = errorIndex; j < output.a.size(); ++j)
    {
      //std::cout<<"er"<<j<<": " << abs(output.a[j]) << std::endl;
      er += std::abs(output.a[j]);
    }

    const double constant = output.a[0];
    const double w1 = output.a[inputIndex];
    const double w2 = output.a[inputIndex + 1];
    const double w3 = er;
    //std::cout << "w1: " << w1 << ", w2: " << w2 << ", w3: " << w3 << std::endl;

    const double halfWidthX = width(variable1)*0.5;
    const double aX = w1 / halfWidthX;

    const double halfWidthY = width(variable2)*0.5;
    const double aY = w2 / halfWidthY;

    const double midX = mid(variable1);
    const double midY = mid(variable2);
    //std::cout << "midX: " << midX << ", midY: " << midY << std::endl;

    afd::maxnum() = 0;
    const afd midTheta1(itv(midX, midX));
    const afd midTheta2(itv(midY, midY));
    const afd yCenter = func(midTheta1, midTheta2);
    const itv b = to_interval(yCenter) - itv(aX, aX)*midX - itv(aY, aY)*midY;
    //std::cout << "yCenter: " << yCenter << std::endl;
    //std::cout << "b: " << b << std::endl;

    m_a1 = aX;
    m_a2 = aY;
    m_b = itvd(b.lower() - er, b.upper() + er);
    //std::cout << m_b << std::endl;
    m_initialized = true;

    //std::cout << "Make QuadTree end" << std::endl;
    maxWidthCache = width(m_b);
  }

  static bool IsBottom(Pos pos) { return pos == BottomLeft || pos == BottomRight; }
  static bool IsTop(Pos pos) { return pos == TopLeft || pos == TopRight; }
  static bool IsLeft(Pos pos) { return pos == BottomLeft || pos == TopLeft; }
  static bool IsRight(Pos pos) { return pos == BottomRight || pos == TopRight; }

  static Pos Bottom(Pos pos) { return IsLeft(pos) ? BottomLeft : BottomRight; }
  static Pos Top(Pos pos) { return IsLeft(pos) ? TopLeft : TopRight; }
  static Pos Left(Pos pos) { return IsTop(pos) ? TopLeft : BottomLeft; }
  static Pos Right(Pos pos) { return IsTop(pos) ? TopRight : BottomRight; }

  double stepDivision(const std::function<afd(const afd&, const afd&)>& func)
  {
    //std::cout << "StepDivision QuadTree begin" << std::endl;
    if (isLeaf())
    {
      //std::cout << "StepDivision A 1" << std::endl;
    
      /*const double theta1_mid = (v1Lower + v1Upper)*0.5;
      const double theta2_mid = (v2Lower + v2Upper)*0.5;*/
      v1Center = (v1Lower + v1Upper)*0.5;
      v2Center = (v2Lower + v2Upper)*0.5;

      //m_childs.push_back(QuadTree(itvd(variable1.lower(), theta1_mid), itvd(variable2.lower(), theta2_mid), func)); //BottomLeft
      //m_childs.push_back(QuadTree(itvd(theta1_mid, variable1.upper()), itvd(variable2.lower(), theta2_mid), func)); //BottomRight
      //m_childs.push_back(QuadTree(itvd(variable1.lower(), theta1_mid), itvd(theta2_mid, variable2.upper()), func)); //TopLeft
      //m_childs.push_back(QuadTree(itvd(theta1_mid, variable1.upper()), itvd(theta2_mid, variable2.upper()), func)); //TopRight
      m_childs.push_back(QuadTree(v1Lower, v1Center, v2Lower, v2Center, func)); //BottomLeft
      //std::cout << "StepDivision A 2" << std::endl;
      m_childs.push_back(QuadTree(v1Center, v1Upper, v2Lower, v2Center, func)); //BottomRight
      //std::cout << "StepDivision A 3" << std::endl;
      m_childs.push_back(QuadTree(v1Lower, v1Center, v2Center, v2Upper, func)); //TopLeft
      //std::cout << "StepDivision A 4" << std::endl;
      m_childs.push_back(QuadTree(v1Center, v1Upper, v2Center, v2Upper, func)); //TopRight
      //std::cout << "StepDivision A 5" << std::endl;

      const std::vector<double> widths({m_childs[0].getWidth(func), m_childs[1].getWidth(func), m_childs[2].getWidth(func), m_childs[3].getWidth(func)});
      auto newMaxWidthIt = std::max_element(widths.begin(), widths.end());
      maxWidthCache = *newMaxWidthIt;
      maxWidthIndexCache = std::distance(widths.begin(), newMaxWidthIt);
      return maxWidthCache;
    }
    else
    {
      //std::cout << "StepDivision B 1" << std::endl;
      //const auto maxIt = std::max_element(m_childs.begin(), m_childs.end(), [&](const QuadTree& a, const QuadTree& b) {return a.getWidth(func) < b.getWidth(func); });
      std::vector<double> widths({m_childs[0].getWidth(func), m_childs[1].getWidth(func), m_childs[2].getWidth(func), m_childs[3].getWidth(func)});

      widths[maxWidthIndexCache] = m_childs[maxWidthIndexCache].stepDivision(func);
      auto newMaxWidthIt = std::max_element(widths.begin(), widths.end());
      maxWidthCache = *newMaxWidthIt;
      maxWidthIndexCache = std::distance(widths.begin(), newMaxWidthIt);
      return maxWidthCache;

      //std::cout << "StepDivision B 2" << std::endl;
      //maxIt->stepDivision(func);
    }
    //std::cout << "StepDivision QuadTree end" << std::endl;
  }


  enum Side { BottomSide, TopSide, LeftSide, RightSide };

  //Accessorを順に見て実際に存在する個々の要素へのAccessorに分解する
  std::vector<Accessor> interpretAccessor(const Accessor& accessor, Side accessSide)const
  {
    const auto addHead = [](Pos head, const std::vector<Accessor>& tails)
    {
      std::vector<Accessor> result = tails;
      for (auto& elem : result)
      {
//std::cout << "pop_front" << std::endl;
        elem.push_front(head);
//std::cout << "ok" << std::endl;
      }
      return result;
    };

    if (accessor.size() == 0)
    {
      const auto concat = [](std::vector<Accessor>& a, const std::vector<Accessor>& b)
      {
        a.insert(a.end(), b.begin(), b.end());
      };

      const auto singleSideChilds = [&](Pos pos1, Pos pos2)
      {
        auto vs = addHead(pos1, m_childs[pos1].interpretAccessor({}, accessSide));
        concat(vs, addHead(pos2, m_childs[pos2].interpretAccessor({}, accessSide)));
        return vs;
      };

      /*{
      auto vs = addHead(Pos::BottomLeft, m_childs[Pos::BottomLeft].interpretAccessor({}, accessSide));
      concat(vs, addHead(Pos::BottomRight, m_childs[Pos::BottomRight].interpretAccessor({}, accessSide)));
      return vs;
      }*/

      if (isLeaf())
      {
        std::vector<Accessor> result;
        result.emplace_back();
        return result;
        //return {};
      }
      else
      {
        //accessorが実際よりも短い場合: 最後のアクセッサ以降はaccessSide側の要素のみを返す
        //        0       1       2       3       4
        // () -> (TL) -> (BR) -> (TL) BottomSide
        // () -> (TL) -> (BR) -> (TL) -> (BL) -> (BL) -> ...
        // () -> (TL) -> (BR) -> (TL) -> (BL) -> (BR) -> ...
        // () -> (TL) -> (BR) -> (TL) -> (BR) -> (BL)
        // () -> (TL) -> (BR) -> (TL) -> (BR) -> (BR) -> ...

        switch (accessSide)
        {
        case Side::BottomSide:
          return singleSideChilds(Pos::BottomLeft, Pos::BottomRight);
        case Side::TopSide:
          return singleSideChilds(Pos::TopLeft, Pos::TopRight);
        case Side::LeftSide:
          return singleSideChilds(Pos::BottomLeft, Pos::TopLeft);
        case Side::RightSide:
          return singleSideChilds(Pos::BottomRight, Pos::TopRight);
        default:
          return {};
        }
      }
    }
    else
    {
      if (isLeaf())
      {
        //accessorが実際よりも長い場合: 単に実在する最後の要素を返せばいい
        //        0       1       2       3       4
        // () -> (TL) -> (TR) -> (BL) -> (BR) -> (TL) -> ...
        // () -> (TL) -> (TR) -> (BL)

        std::vector<Accessor> result;
        result.emplace_back();
        return result;
        //return {};
      }
      else
      {
        const Pos headAccessor = accessor.front();
        //Accessor tailAccessor(accessor.begin() + 1, accessor.end());
        Accessor tailAccessor = accessor;
//std::cout << "pop_front: " << accessor.size() << std::endl;
        tailAccessor.pop_front();
//std::cout << "ok" << std::endl;
        return addHead(headAccessor, m_childs[headAccessor].interpretAccessor(tailAccessor, accessSide));
      }
    }
  }

  std::vector<Accessor> getAllElements()const
  {
    if (isLeaf())
    {
      std::vector<Accessor> result;
      result.emplace_back();
      return result;
    }

    const auto addHead = [](Pos head, const std::vector<Accessor>& tails)
    {
      std::vector<Accessor> result = tails;
      for (auto& elem : result)
      {
        elem.push_front(head);
      }
      return result;
    };

    const auto concat = [](std::vector<Accessor>& a, const std::vector<Accessor>& b)
    {
      a.insert(a.end(), b.begin(), b.end());
    };

    std::vector<Accessor> result;
    concat(result, addHead(Pos::BottomLeft, m_childs[Pos::BottomLeft].getAllElements()));
    concat(result, addHead(Pos::BottomRight, m_childs[Pos::BottomRight].getAllElements()));
    concat(result, addHead(Pos::TopLeft, m_childs[Pos::TopLeft].getAllElements()));
    concat(result, addHead(Pos::TopRight, m_childs[Pos::TopRight].getAllElements()));
    //Logger << U"result.size(): " << result.size();
    return result;
  }

  Accessor getRegion(double theta1, double theta2)const
  {
    if (isLeaf())
    {
      return{};
    }

    /*const double theta1_mid = mid(variable1);
    const double theta2_mid = mid(variable2);*/
    const double theta1_mid = v1Center;
    const double theta2_mid = v2Center;

    //Right
    if (theta1_mid < theta1)
    {
      //Top
      if (theta2_mid < theta2)
      {
        auto accessor = m_childs[TopRight].getRegion(theta1, theta2);
        accessor.push_front(TopRight);
        return accessor;
      }
      //Bottom
      else
      {
        auto accessor = m_childs[BottomRight].getRegion(theta1, theta2);
        accessor.push_front(BottomRight);
        return accessor;
      }
    }
    //Left
    else
    {
      //Top
      if (theta2_mid < theta2)
      {
        auto accessor = m_childs[TopLeft].getRegion(theta1, theta2);
        accessor.push_front(TopLeft);
        return accessor;
      }
      //Bottom
      else
      {
        auto accessor = m_childs[BottomLeft].getRegion(theta1, theta2);
        accessor.push_front(BottomLeft);
        return accessor;
      }
    }
  }

  static Accessor BottomAdjacent(const Accessor& accessor)
  {
    const auto Clone = [&](int i)
    {
      //return Accessor(accessor.begin(), accessor.begin() + i);
      return accessor.take(i);
    };

    for (int i = accessor.size() - 1; 0 <= i; --i)
    {
      //元の要素と隣接要素が同じ深さの場合

      //        0       1       2
      // () -> (TL) -> (TR) -> (BL)
      // () -> (TL) -> (BR) -> (TL)

      //        0       1       2
      // () -> (TL) -> (BR) -> (BL)
      // () -> (BL) -> (TR) -> (TL)

      bool switched = false;
      if (IsTop(accessor[i]))
      {
        auto clone = Clone(i);
        clone.push_back(Bottom(accessor[i]));
        for (int j = i + 1; j < accessor.size(); ++j)
        {
          clone.push_back(Top(accessor[j]));
        }
        return clone;
      }
    }

    return {};
  }
  static Accessor TopAdjacent(const Accessor& accessor)
  {
    const auto Clone = [&](int i)
    {
      //return Accessor(accessor.begin(), accessor.begin() + i);
      return accessor.take(i);
    };

    for (int i = accessor.size() - 1; 0 <= i; --i)
    {
      if (IsBottom(accessor[i]))
      {
        auto clone = Clone(i);
        clone.push_back(Top(accessor[i]));
        for (int j = i + 1; j < accessor.size(); ++j)
        {
          clone.push_back(Bottom(accessor[j]));
        }
        return clone;
      }
    }

    return {};
  }
  static Accessor LeftAdjacent(const Accessor& accessor)
  {
    const auto Clone = [&](int i)
    {
      //return Accessor(accessor.begin(), accessor.begin() + i);
      return accessor.take(i);
    };

    for (int i = accessor.size() - 1; 0 <= i; --i)
    {
      if (IsRight(accessor[i]))
      {
        auto clone = Clone(i);
        clone.push_back(Left(accessor[i]));
        for (int j = i + 1; j < accessor.size(); ++j)
        {
          clone.push_back(Right(accessor[j]));
        }
        return clone;
      }
    }

    return {};
  }
  static Accessor RightAdjacent(const Accessor& accessor)
  {
    const auto Clone = [&](int i)
    {
      //return Accessor(accessor.begin(), accessor.begin() + i);
      return accessor.take(i);
    };

    for (int i = accessor.size() - 1; 0 <= i; --i)
    {
      if (IsLeft(accessor[i]))
      {
        auto clone = Clone(i);
        clone.push_back(Right(accessor[i]));
        for (int j = i + 1; j < accessor.size(); ++j)
        {
          clone.push_back(Left(accessor[j]));
        }
        return clone;
      }
    }

    return {};
  }

  const QuadTree& access(const Accessor& accessor)const
  {
    const QuadTree* element = this;
    /*for (auto pos : accessor)
    {
      element = &element->m_childs[pos];
    }*/
    for(size_t i = 0; i < accessor.size(); ++i)
    {
      element = &element->m_childs[accessor[i]];
    }
    return *element;
  }

  double getWidth(const std::function<afd(const afd&, const afd&)>& func)const
  {
    if (!m_initialized)
    {
      make(func);
    }

    return maxWidthCache;

    /*
    if (isLeaf())
    {
      return width(m_b);
    }
    else
    {
      const auto maxIt = std::max_element(m_childs.begin(), m_childs.end(), [&](const QuadTree& a, const QuadTree& b) {return a.getWidth(func) < b.getWidth(func); });
      return maxIt->getWidth(func);
    }*/
  }

  void dump(std::ostream& os)const
  {
    os << "[\n";
    dumpImpl(os);
    os << "]\n";
  }


  //全てのノードが自分の包囲する区間を持つ
  //itvd variable1, variable2;
  std::reference_wrapper<double> v1Lower = ddd, v1Upper = ddd;
  std::reference_wrapper<double> v2Lower = ddd, v2Upper = ddd;
  double v1Center, v2Center;

  //childs.empty() の時はleafであり以下のデータを持つ
  mutable bool m_initialized = false;
  mutable double m_a1, m_a2;
  mutable itvd m_b;

  mutable double maxWidthCache;
  size_t maxWidthIndexCache = -1;

  //mutable std::stringstream m_dbgData;

  std::vector<QuadTree> m_childs;

private:
  void dumpImpl(std::ostream& os)const
  {
    if (!m_initialized)
    {
      return;
    }

    if (isLeaf())
    {
      os << "  [" << v1Lower << ", " << v1Upper << ", " << v2Lower << ", " << v1Upper << ", " << width(m_b) << "]\n";
    }
    else
    {
      for(auto& child : m_childs)
      {
        child.dumpImpl(os);
      }
    }
  }
};


inline bool IsSame(const Accessor& a, const Accessor& b)
{
  if (a.size() != b.size())
  {
    return false;
  }

  for (size_t i = 0; i < a.size(); ++i)
  {
    if (a[i] != b[i])
    {
      return false;
    }
  }

  return true;
}

struct V2
{
  double x, y;
  V2(double x, double y) :x(x), y(y) {}
};

struct V3
{
  double x, y, z;
  V3(double x, double y, double z) :x(x), y(y), z(z) {}
};

inline void getPointsImpl(std::vector<V3>& output, const QuadTree& reference, double v1Lower, double v1Upper, double v2Lower, double v2Upper)
{
  if (reference.v1Upper < v1Lower || reference.v2Upper < v2Lower || v1Upper < reference.v1Lower || v2Upper < reference.v2Lower)
  {
    return;
  }

  if (reference.isLeaf())
  {
    //std::cout << "m_b: " << reference.m_b;

    const auto isEnclosed = [&](double x, double y)
    {
      return v1Lower < x && x < v1Upper && v2Lower < y && y < v2Upper;
    };

    /*
    const auto evalLower = [&](double x, double y)
    {
      return reference.m_b.lower() + reference.m_a1*x + reference.m_a2*y;
    };

    const auto evalUpper = [&](double x, double y)
    {
      return reference.m_b.upper() + reference.m_a1*x + reference.m_a2*y;
    };

    output.emplace_back(reference.v1Lower, reference.v2Lower, evalLower(reference.v1Lower, reference.v2Lower));
    output.emplace_back(reference.v1Lower, reference.v2Lower, evalUpper(reference.v1Lower, reference.v2Lower));

    output.emplace_back(reference.v1Lower, reference.v2Upper, evalLower(reference.v1Lower, reference.v2Upper));
    output.emplace_back(reference.v1Lower, reference.v2Upper, evalUpper(reference.v1Lower, reference.v2Upper));

    output.emplace_back(reference.v1Upper, reference.v2Lower, evalLower(reference.v1Upper, reference.v2Lower));
    output.emplace_back(reference.v1Upper, reference.v2Lower, evalUpper(reference.v1Upper, reference.v2Lower));

    output.emplace_back(reference.v1Upper, reference.v2Upper, evalLower(reference.v1Upper, reference.v2Upper));
    output.emplace_back(reference.v1Upper, reference.v2Upper, evalUpper(reference.v1Upper, reference.v2Upper));
    */

    const auto evalLower = [](double x, double y, double a1, double a2, const itvd& b)
    {
      return b.lower() + a1*x + a2*y;
    };

    const auto evalUpper = [](double x, double y, double a1, double a2, const itvd& b)
    {
      return b.upper() + a1*x + a2*y;
    };

    if(v1Lower < reference.v1Lower && reference.v1Upper < v1Upper && v2Lower < reference.v2Lower && reference.v2Upper < v2Upper)
    {
    output.emplace_back(reference.v1Lower, reference.v2Lower, evalLower(reference.v1Lower, reference.v2Lower, reference.m_a1, reference.m_a2, reference.m_b));
    output.emplace_back(reference.v1Lower, reference.v2Lower, evalUpper(reference.v1Lower, reference.v2Lower, reference.m_a1, reference.m_a2, reference.m_b));

    output.emplace_back(reference.v1Lower, reference.v2Upper, evalLower(reference.v1Lower, reference.v2Upper, reference.m_a1, reference.m_a2, reference.m_b));
    output.emplace_back(reference.v1Lower, reference.v2Upper, evalUpper(reference.v1Lower, reference.v2Upper, reference.m_a1, reference.m_a2, reference.m_b));

    output.emplace_back(reference.v1Upper, reference.v2Lower, evalLower(reference.v1Upper, reference.v2Lower, reference.m_a1, reference.m_a2, reference.m_b));
    output.emplace_back(reference.v1Upper, reference.v2Lower, evalUpper(reference.v1Upper, reference.v2Lower, reference.m_a1, reference.m_a2, reference.m_b));

    output.emplace_back(reference.v1Upper, reference.v2Upper, evalLower(reference.v1Upper, reference.v2Upper, reference.m_a1, reference.m_a2, reference.m_b));
    output.emplace_back(reference.v1Upper, reference.v2Upper, evalUpper(reference.v1Upper, reference.v2Upper, reference.m_a1, reference.m_a2, reference.m_b));
    }
  }
  else
  {
    for (int i = 0; i < 4; ++i)
    {
      getPointsImpl(output, reference.m_childs[i], v1Lower, v1Upper, v2Lower, v2Upper);
    }
  }
}

inline std::vector<V3> getPoints(const QuadTree& reference, double xLower, double xUpper, double yLower, double yUpper)
{
  std::vector<V3> output;
  getPointsImpl(output, reference, xLower, xUpper, yLower, yUpper);
  return output;
}

inline double Accumulate(const std::vector<V3>& points, const std::function<double(const V3&)>& func)
{
/*
  //boost::accumulators::accumulator_set<double, boost::accumulators::stats<boost::accumulators::tag::sum_kahan>> acc;
  boost::accumulators::accumulator_set<float, boost::accumulators::stats<boost::accumulators::tag::sum_kahan>> acc;
  for(size_t i = 0; i < points.size(); ++i)
  {
    acc(func(points[i]));
  }
  return boost::accumulators::sum_kahan(acc);
*/

  accumulator_set<double, stats<tag::sum_kahan>> acc;
  for(size_t i = 0; i < points.size(); ++i)
  {
    acc(func(points[i]));
  }
  return sum_kahan(acc);
}


inline void OptimizeQuadTree(const QuadTree& reference, QuadTree& target)
{
  if(!target.isLeaf())
  {
    for(auto& child : target.m_childs)
    {
      OptimizeQuadTree(reference, child);
    }
    return;
  }

  const auto points = getPoints(reference, target.v1Lower, target.v1Upper, target.v2Lower, target.v2Upper);

  static bool isFirst = true;
  if(isFirst)
  {
    const auto evalLower = [&](double x, double y, double a1, double a2, const itvd& b)
    {
      return b.lower() + a1*x + a2*y;
    };
    const auto evalUpper = [&](double x, double y, double a1, double a2, const itvd& b)
    {
      return b.upper() + a1*x + a2*y;
    };
    
    std::cout << "target width: " << width(target.m_b) << std::endl;
    double maxWidth = 0.0;
    for(int i = 0; i + 1 < points.size(); i += 2)
    {
      const double w = points[i+1].z - points[i].z;
      if(maxWidth < w)
      {
       maxWidth = w;
      }
    }
    std::cout << "reference max width: " << maxWidth << std::endl;
    std::cout << "original a1: " << target.m_a1 << ", a2: " << target.m_a2 << ", bmid: " << mid(target.m_b) << ", bwidth: "<< width(target.m_b) << "\n";
/*
    std::cout << "----------\n";
    std::cout << "target: (" << target.v1Lower << ", " << target.v2Lower << ", " << evalLower(target.v1Lower, target.v2Lower, target.m_a1, target.m_a2, target.m_b) << "), (" << target.v1Upper << ", " << target.v2Upper << ", " << evalUpper(target.v1Upper, target.v2Upper, target.m_a1, target.m_a2, target.m_b) << ")\n\n";
    for(auto& p : points)
    {
      std::cout << "(" << p.x << ", " << p.y << ", " << p.z << ")\n";
    }
    std::cout << std::endl;
*/
  }
  //std::cout << "size:"<< points.size() << std::endl;

/*
  const double M11 = points.size();
  
  //const double M12 = std::accumulate(points.begin(), points.end(), 0.0, [](double init, const V3& v) {return init + v.x; });
  //const double M13 = std::accumulate(points.begin(), points.end(), 0.0, [](double init, const V3& v) {return init + v.y; });
  //const double M21 = M12;
  //const double M22 = std::accumulate(points.begin(), points.end(), 0.0, [](double init, const V3& v) {return init + v.x * v.x; });
  //const double M23 = std::accumulate(points.begin(), points.end(), 0.0, [](double init, const V3& v) {return init + v.x * v.y; });
  //const double M31 = M13;
  //const double M32 = M23;
  //const double M33 = std::accumulate(points.begin(), points.end(), 0.0, [](double init, const V3& v) {return init + v.y * v.y; });

  //const double u1 = std::accumulate(points.begin(), points.end(), 0.0, [](double init, const V3& v) {return init + v.z; });
  //const double u2 = std::accumulate(points.begin(), points.end(), 0.0, [](double init, const V3& v) {return init + v.x * v.z; });
  //const double u3 = std::accumulate(points.begin(), points.end(), 0.0, [](double init, const V3& v) {return init + v.y * v.z; });
  
  const double M12 = Accumulate(points, [](const V3& v) {return v.x; });
  const double M13 = Accumulate(points, [](const V3& v) {return v.y; });
  const double M21 = M12;
  const double M22 = Accumulate(points, [](const V3& v) {return v.x * v.x; });
  const double M23 = Accumulate(points, [](const V3& v) {return v.x * v.y; });
  const double M31 = M13;
  const double M32 = M23;
  const double M33 = Accumulate(points, [](const V3& v) {return v.y * v.y; });

  const double u1 = Accumulate(points, [](const V3& v) {return v.z; });
  const double u2 = Accumulate(points, [](const V3& v) {return v.x * v.z; });
  const double u3 = Accumulate(points, [](const V3& v) {return v.y * v.z; });

  //Mv = u を解いた v が解
  //z = v[0] + x * v[1] + y * v[2]
  const double coef = 1.0/(M11*M22*M33 + M12*M23*M31 + M13*M21*M32 - M13*M22*M31 - M12*M21*M33 - M11*M23*M32);
  const double inv_M11 = coef * (M22*M33 - M23*M32);
  const double inv_M12 = coef * -(M12*M33 - M13*M32);
  const double inv_M13 = coef * (M12*M23 - M13*M22);
  const double inv_M21 = coef * -(M21*M33 - M23*M31);
  const double inv_M22 = coef * (M11*M33 - M13*M31);
  const double inv_M23 = coef * -(M11*M23 - M13*M21);
  const double inv_M31 = coef * (M21*M32 - M22*M31);
  const double inv_M32 = coef * -(M11*M32 - M12*M31);
  const double inv_M33 = coef * (M11*M22 - M12*M21);

  const double v0 = inv_M11 * u1 + inv_M12 * u2 + inv_M13 * u3;
  const double v1 = inv_M21 * u1 + inv_M22 * u2 + inv_M23 * u3;
  const double v2 = inv_M31 * u1 + inv_M32 * u2 + inv_M33 * u3;

  target.m_a1 = v1;
  target.m_a2 = v2;
  target.m_b = v0;
*/
  double v1 = target.m_a1;
  double v2 = target.m_a2;
  target.m_b = mid(target.m_b);
  for (const auto& p : points)
  {
    const double center = p.x * v1 + p.y * v2;
    if (p.z < target.m_b.lower() + center)
    {
      target.m_b.lower() = p.z - center;
    }
    if (target.m_b.upper() + center < p.z)
    {
      target.m_b.upper() = p.z - center;
    }
  }

  if(isFirst)
  {
    //std::cout << "a1: " << v1 << ", a2: " << v2  << ", bmid: " << v0 << ", b: "<< target.m_b << "\n";
    std::cout << "optimized a1: " << target.m_a1 << ", a2: " << target.m_a2 << ", bmid: " << mid(target.m_b) << ", bwidth: "<< width(target.m_b) << "\n";
    isFirst = false;
  }
}

}
}
