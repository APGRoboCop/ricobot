##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Release_Win32
ProjectName            :=bot
ConfigurationName      :=Release_Win32
WorkspacePath          :=E:/Dropbox/src/ricobot
ProjectPath            :=E:/Dropbox/src/ricobot
IntermediateDirectory  :=./Release/
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Ryan
Date                   :=15/08/2016
CodeLitePath           :="C:/Program Files/CodeLite"
LinkerName             :=C:/MinGW/bin/g++.exe
SharedObjectLinkerName :=C:/MinGW/bin/g++.exe -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=./Release/ricobot_mm.dll
Preprocessors          :=$(PreprocessorSwitch)NDEBUG $(PreprocessorSwitch)_WIN32 $(PreprocessorSwitch)_WINDOWS 
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="bot.txt"
PCHCompileFlags        :=
MakeDirCommand         :=makedir
RcCmpOptions           := 
RcCompilerName         :=C:/MinGW/bin/windres.exe
LinkOptions            :=  -O0
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch)..\hlsdk-2.3-p4\multiplayer\common $(IncludeSwitch)..\hlsdk-2.3-p4\multiplayer\ricochet\dlls $(IncludeSwitch)..\hlsdk-2.3-p4\multiplayer\ricochet\pm_shared $(IncludeSwitch)..\hlsdk-2.3-p4\multiplayer\engine $(IncludeSwitch)..\metamod-p-37\metamod 
IncludePCH             := 
RcIncludePath          := 
Libs                   := $(LibrarySwitch)kernel32 
ArLibs                 :=  "kernel32.a" 
LibPath                := $(LibraryPathSwitch). $(LibraryPathSwitch). $(LibraryPathSwitch)Debug 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := C:/MinGW/bin/ar.exe rcu
CXX      := C:/MinGW/bin/g++.exe
CC       := C:/MinGW/bin/gcc.exe
CXXFLAGS :=  -g -Wall -O2 $(Preprocessors)
CFLAGS   :=   $(Preprocessors)
ASFLAGS  := 
AS       := C:/MinGW/bin/as.exe


##
## User defined environment variables
##
CodeLiteDir:=C:\Program Files\CodeLite
VS_Configuration:=Release
VS_IntDir:=./Release/
VS_OutDir:=./Release/
VS_Platform:=Win32
VS_ProjectDir:=/root/Dropbox/src/ricobot/
VS_ProjectName:=bot
VS_SolutionDir:=/root/Dropbox/src/ricobot/
Objects0=$(IntermediateDirectory)/bot.cpp$(ObjectSuffix) $(IntermediateDirectory)/bot_client.cpp$(ObjectSuffix) $(IntermediateDirectory)/dll.cpp$(ObjectSuffix) $(IntermediateDirectory)/engine.cpp$(ObjectSuffix) $(IntermediateDirectory)/h_export.cpp$(ObjectSuffix) $(IntermediateDirectory)/util.cpp$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(SharedObjectLinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)
	@$(MakeDirCommand) "E:\Dropbox\src\ricobot/.build-release"
	@echo rebuilt > "E:\Dropbox\src\ricobot/.build-release/bot"

MakeIntermediateDirs:
	@$(MakeDirCommand) "./Release/"


$(IntermediateDirectory)/.d:
	@$(MakeDirCommand) "./Release/"

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/bot.cpp$(ObjectSuffix): bot.cpp $(IntermediateDirectory)/bot.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/Dropbox/src/ricobot/bot.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/bot.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/bot.cpp$(DependSuffix): bot.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/bot.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/bot.cpp$(DependSuffix) -MM bot.cpp

$(IntermediateDirectory)/bot.cpp$(PreprocessSuffix): bot.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/bot.cpp$(PreprocessSuffix)bot.cpp

$(IntermediateDirectory)/bot_client.cpp$(ObjectSuffix): bot_client.cpp $(IntermediateDirectory)/bot_client.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/Dropbox/src/ricobot/bot_client.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/bot_client.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/bot_client.cpp$(DependSuffix): bot_client.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/bot_client.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/bot_client.cpp$(DependSuffix) -MM bot_client.cpp

$(IntermediateDirectory)/bot_client.cpp$(PreprocessSuffix): bot_client.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/bot_client.cpp$(PreprocessSuffix)bot_client.cpp

$(IntermediateDirectory)/dll.cpp$(ObjectSuffix): dll.cpp $(IntermediateDirectory)/dll.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/Dropbox/src/ricobot/dll.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/dll.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/dll.cpp$(DependSuffix): dll.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/dll.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/dll.cpp$(DependSuffix) -MM dll.cpp

$(IntermediateDirectory)/dll.cpp$(PreprocessSuffix): dll.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/dll.cpp$(PreprocessSuffix)dll.cpp

$(IntermediateDirectory)/engine.cpp$(ObjectSuffix): engine.cpp $(IntermediateDirectory)/engine.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/Dropbox/src/ricobot/engine.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/engine.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/engine.cpp$(DependSuffix): engine.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/engine.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/engine.cpp$(DependSuffix) -MM engine.cpp

$(IntermediateDirectory)/engine.cpp$(PreprocessSuffix): engine.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/engine.cpp$(PreprocessSuffix)engine.cpp

$(IntermediateDirectory)/h_export.cpp$(ObjectSuffix): h_export.cpp $(IntermediateDirectory)/h_export.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/Dropbox/src/ricobot/h_export.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/h_export.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/h_export.cpp$(DependSuffix): h_export.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/h_export.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/h_export.cpp$(DependSuffix) -MM h_export.cpp

$(IntermediateDirectory)/h_export.cpp$(PreprocessSuffix): h_export.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/h_export.cpp$(PreprocessSuffix)h_export.cpp

$(IntermediateDirectory)/util.cpp$(ObjectSuffix): util.cpp $(IntermediateDirectory)/util.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "E:/Dropbox/src/ricobot/util.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/util.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/util.cpp$(DependSuffix): util.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/util.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/util.cpp$(DependSuffix) -MM util.cpp

$(IntermediateDirectory)/util.cpp$(PreprocessSuffix): util.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/util.cpp$(PreprocessSuffix)util.cpp


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Release/


