##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=Sockets
ConfigurationName      :=Debug
WorkspacePath          := "/home/mrssheep/Documents/Studium/3-Semester/Rechnernetze&VerteilteSysteme/Hausaufgaben/00/Sockets"
ProjectPath            := "/home/mrssheep/Documents/Studium/3-Semester/Rechnernetze&VerteilteSysteme/Hausaufgaben/00/Sockets"
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=Clemens M. Schöll
Date                   :=11/03/14
CodeLitePath           :="/home/mrssheep/.codelite"
LinkerName             :=/usr/bin/g++ 
SharedObjectLinkerName :=/usr/bin/g++ -shared -fPIC
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
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="Sockets.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := 
ArLibs                 :=  
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/g++ 
CC       := /usr/bin/gcc 
CXXFLAGS :=  -g -O0 -Wall $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as 


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/server.c$(ObjectSuffix) $(IntermediateDirectory)/tcpClient.c$(ObjectSuffix) $(IntermediateDirectory)/udpClient.c$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

$(IntermediateDirectory)/.d:
	@test -d ./Debug || $(MakeDirCommand) ./Debug

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/server.c$(ObjectSuffix): server.c $(IntermediateDirectory)/server.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/mrssheep/Documents/Studium/3-Semester/Rechnernetze&VerteilteSysteme/Hausaufgaben/00/Sockets/server.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/server.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/server.c$(DependSuffix): server.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/server.c$(ObjectSuffix) -MF$(IntermediateDirectory)/server.c$(DependSuffix) -MM "server.c"

$(IntermediateDirectory)/server.c$(PreprocessSuffix): server.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/server.c$(PreprocessSuffix) "server.c"

$(IntermediateDirectory)/tcpClient.c$(ObjectSuffix): tcpClient.c $(IntermediateDirectory)/tcpClient.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/mrssheep/Documents/Studium/3-Semester/Rechnernetze&VerteilteSysteme/Hausaufgaben/00/Sockets/tcpClient.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/tcpClient.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/tcpClient.c$(DependSuffix): tcpClient.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/tcpClient.c$(ObjectSuffix) -MF$(IntermediateDirectory)/tcpClient.c$(DependSuffix) -MM "tcpClient.c"

$(IntermediateDirectory)/tcpClient.c$(PreprocessSuffix): tcpClient.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/tcpClient.c$(PreprocessSuffix) "tcpClient.c"

$(IntermediateDirectory)/udpClient.c$(ObjectSuffix): udpClient.c $(IntermediateDirectory)/udpClient.c$(DependSuffix)
	$(CC) $(SourceSwitch) "/home/mrssheep/Documents/Studium/3-Semester/Rechnernetze&VerteilteSysteme/Hausaufgaben/00/Sockets/udpClient.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/udpClient.c$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/udpClient.c$(DependSuffix): udpClient.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/udpClient.c$(ObjectSuffix) -MF$(IntermediateDirectory)/udpClient.c$(DependSuffix) -MM "udpClient.c"

$(IntermediateDirectory)/udpClient.c$(PreprocessSuffix): udpClient.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/udpClient.c$(PreprocessSuffix) "udpClient.c"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) ./Debug/*$(ObjectSuffix)
	$(RM) ./Debug/*$(DependSuffix)
	$(RM) $(OutputFile)
	$(RM) ".build-debug/Sockets"


