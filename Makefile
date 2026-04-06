CXX = g++
CXXFLAGS = -std=c++11 -Wall -g

# 執行檔名稱
TARGET = Lab1

# 自動抓取資料夾下所有的 .cpp 檔案（例如 main.cpp, fm.cpp, cell.cpp ...）
SOURCES = $(wildcard *.cpp)

# 將 .cpp 檔名替換成 .o 檔名（物件檔）
OBJECTS = $(SOURCES:.cpp=.o)

# 預設動作：編譯出執行檔
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

# 編譯規則：如何將 .cpp 編譯成 .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 測試功能
test: $(TARGET)
	@echo "Running test with test.txt..."
	./$(TARGET) appendix/case1.in output.txt
	@echo "Test finished. Results saved in output.txt"
	@echo "--- Output Content ---"
	@cat output.txt

# 清除編譯產出的檔案
clean:
	rm -f $(TARGET) $(OBJECTS)