srcpp = $(wildcard ./*.cpp)

obj = $(patsubst ./%.cpp, ./obj/%.o, $(srcpp))

inc_path = ./

myArgs = -Wall -pthread

ALL:server

# 预处理-编译-汇编-目标文件
# 尽管这里指定了头文件路径，但如果源文件目录下有头文件的话，会优先使用源文件目录下的头文件
$(obj):./obj/%.o:./%.cpp
	g++ -c $< -o $@ $(myArgs) -I $(inc_path)  

# 目标文件连接-可执行文件
server: $(obj)
	g++ $^ -o $@ $(myArgs)

clean:
	-rm -rf $(obj) server

.PHONY:clean ALL