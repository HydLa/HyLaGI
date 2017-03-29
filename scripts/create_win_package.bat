devenv Hyrose.sln /Build Release

for /F "tokens=1-3 delims=/ " %%a in ('date /t') do SET DT=Hyrose_%%a%%b%%c
md %DT%

copy Hyrose.exe %DT%\hyrose.exe

md %DT%\examples
copy examples %DT%\examples
