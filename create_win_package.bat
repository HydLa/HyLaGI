devenv HydLa.sln /Build Release

for /F "tokens=1-3 delims=/ " %%a in ('date /t') do SET DT=HydLa_%%a%%b%%c
md %DT%

copy Release\HydLa_parser.exe %DT%\hydla.exe

md %DT%\examples
copy examples %DT%\examples