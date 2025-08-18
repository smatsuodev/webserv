NAME := webserv
SRC_DIR := src
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
BUILD_DIR := dist
OBJS := $(addprefix $(BUILD_DIR)/,$(SRCS:.cpp=.o))
CXXFLAGS := -std=c++98 -I$(SRC_DIR)/lib -Wall -Wextra -Werror

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	-$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(NAME)

format:
	clang-format -i $$(git ls-files '*.cpp' '*.hpp')

.PHONY: all clean format