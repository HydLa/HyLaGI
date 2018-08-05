#include "Logger.h"

namespace hydla {
namespace logger {

Logger::Logger() :
  log_level_(Warn)
{
  debug_.push(std::cerr);
  warn_.push(std::cerr);
  error_.push(std::cerr);
  fatal_.push(std::cerr);
  standard_.push(std::cout);
}

Logger::~Logger()
{
  hydla::logger::Logger& i = hydla::logger::Logger::instance();
  if (!i.is_html_mode())
  {
    return;
  }

  std::cerr << std::endl;
  std::cout << std::endl;

  std::string inlineScript(R"*(
var container = document.querySelector('.article');
var markerHeight = '1%';
var customStyle = document.createElement('style');
container.appendChild(customStyle);
timers = document.getElementsByClassName('timer');
function renderScrollMarker(posArr, colArr)
{
  var _posArr = [];
  for(var i = 0; i < posArr.length; i++)
  {
    _posArr.push('transparent ' + posArr[i] + ', ' + colArr[i] + ' ' + posArr[i] + ', ' + colArr[i] + ' calc(' + posArr[i] + ' + ' + markerHeight + '), transparent calc(' + posArr[i] + ' + ' + markerHeight + ')');
  }
  customStyle.innerHTML = '::-webkit-scrollbar-track {\n        background-image: linear-gradient(' + _posArr.join() + ');\n      }';
}
var innerAutoProcess = false;
function reloadMakers() {
  if(innerAutoProcess)
  {
    return;
  }
  containerRect = container.getBoundingClientRect();
  threshold = 10000.0;
  var markerDictionary = {}
  function getElementPos(element)
  {
    var posProgress = (element.getBoundingClientRect().top - containerRect.top) / container.scrollHeight;
    return (posProgress * 100).toFixed(2) + '%';
  }
  for(var i = 0; i < timers.length; i++)
  {
    currentTime = Number(timers[i].innerText);
    var timerPosElem = $(timers[i]);
    var currentDetails = $(timers[i]).closest('details');
    var varOutermostClosedDetails = null;
    while(true)
    {
      if(!currentDetails.attr('open'))
      {
        varOutermostClosedDetails = currentDetails;
      }
      parent = currentDetails.parent().closest('details');
      if(parent.length == 0)
      {
        break;
      }
      currentDetails = parent;
    }
    var posElement = varOutermostClosedDetails ? varOutermostClosedDetails.get(0) : timerPosElem.get(0);
    key = getElementPos(posElement);
    if(key in markerDictionary)
    {
      markerDictionary[key] += currentTime;
    }
    else
    {
      markerDictionary[key] = currentTime;
    }
  }
  ys = [];
  cs = [];
  for (var key in markerDictionary)
  {
    colorRate = Math.min(markerDictionary[key], threshold) / threshold;
    c = 255 - 255*colorRate;
    colorString = "rgba(255," + Math.round(c)+","+Math.round(c) + ",255)";
    ys.push(key);
    cs.push(colorString);
  }
  renderScrollMarker(ys, cs);
}
function openAll() {
  var x = document.getElementsByTagName("details");
  for (var i = 0; i < x.length; i++) {
    x[i].setAttribute("open", "true");
  }
  innerAutoProcess = false;
  reloadMakers();
  innerAutoProcess = true;
  setTimeout(function(){ innerAutoProcess = false; }, 1000);
}
function closeAll() {
  var x = document.getElementsByTagName("details");
  for (var i = 0; i < x.length; i++) {
    x[i].removeAttribute("open");
  }
  innerAutoProcess = false;
  reloadMakers();
  innerAutoProcess = true;
  setTimeout(function(){ innerAutoProcess = false; }, 1000);
}
)*");

  i.debug_ << "</div>" << std::endl;
  i.debug_ << R"(<script type="text/javascript" src="https://ajax.googleapis.com/ajax/libs/jquery/1.9.1/jquery.min.js"></script>)" << std::endl;
  i.debug_ << R"(<script type="text/javascript">)" << std::endl;
  i.debug_ << inlineScript << std::endl;
  i.debug_ << "</script>" << std::endl;
  
  i.debug_ << "</body>\n</html>" << std::endl;
}

Logger& Logger::instance() {
  static Logger inst;
  return inst;
}

} // namespace logger
} // namespace hydla
