cd "%~dp0"
echo "STARTING UDP SERVER"
start TheComputerMazeUDPServer.exe
echo "DONE.."
echo "STARTING VISUALIZER"
cd "%~dp0"
runvisualizer.bat
pause